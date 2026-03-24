#include "dataset.h"
#include "util.h"
#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

// Read a file and return only non-empty trimmed lines
std::vector<std::string> read_nonempty_lines(const std::string& path) {
    std::ifstream in(path.c_str());  // Opens the file at path to read
    if (!in) { // File failed to open
        throw std::runtime_error("Could not open file: " + path);
    }
    std::vector<std::string> lines;  // Stores all valid lines
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);  // Remove whitespace
        if (!line.empty()) {  
            lines.push_back(line);  // Keep non-empty
        }
    }
    return lines;
}

// Takes attrubute file and turns it into a DatasetInfo object
DatasetInfo parse_attr_file(const std::string& path) {
    auto lines = read_nonempty_lines(path);
    if (lines.size() < 2) {
        throw std::runtime_error("Invalid attr file: " + path);
    }

    DatasetInfo info;

    // All lines except the last describe attributes
    for (size_t i = 0; i + 1 < lines.size(); i++) {
        auto toks = split_ws(lines[i]);  // Split line into whitespace-separated tokens
        if (toks.size() < 2) {
            throw std::runtime_error("Bad attribute line: " + lines[i]);
        }
        Attribute a;
        a.name = toks[0];  // First token is attribute name

        // If attribute is continuous
        if (toks[1] == "continuous") {
            a.continuous = true;
        } else {  // Otherwise this is a symbolic attribute with listed values
            a.continuous = false;
            for (size_t j = 1; j < toks.size(); j++) {
                a.values.push_back(toks[j]);
            }
        }
        info.attributes.push_back(a);
    }

    // Last line contains the class labels
    auto class_toks = split_ws(lines.back());
    if (class_toks.size() < 2) {
        throw std::runtime_error("Bad class line: " + lines.back());
    }

    // Skip the first token and store class names
    for (size_t i = 1; i < class_toks.size(); i++) {
        info.class_names.push_back(class_toks[i]);
    }
    return info;
}

// Makes a train/test data file into a vector of Example objects
std::vector<Example> parse_examples_raw(const std::string& path, const DatasetInfo& info) {
    auto lines = read_nonempty_lines(path);
    std::vector<Example> data;
    for (const auto& line : lines) {
        auto toks = split_ws(line);

        // Each line must have one token per attribute plus one class label
        if (toks.size() != info.attributes.size() + 1) {
            throw std::runtime_error("Bad data line: " + line);
        }
        Example ex;
        ex.values.resize(info.attributes.size(), -1);  // Encoded symbolic/discretized values
        ex.raw_values.resize(info.attributes.size(), 0.0);  // Encoded symbolic/discretized values

        // Parse each attribute value
        for (size_t i = 0; i < info.attributes.size(); i++) {
            // Store raw numeric value for continuous attribute
            if (info.attributes[i].continuous) {
                ex.raw_values[i] = std::atof(toks[i].c_str());
            } else {
                // For symbolic attributes, map token to its index in the allowed value list
                auto it = std::find(info.attributes[i].values.begin(),
                                    info.attributes[i].values.end(),
                                    toks[i]);
                
                // Not found
                if (it == info.attributes[i].values.end()) {
                    throw std::runtime_error("Unknown symbolic value: " + toks[i]);
                }

                // Converts the symbolic value into its integer index
                ex.values[i] = static_cast<int>(it - info.attributes[i].values.begin());
            }
        }

        // Looks up the class label, which is the last token in the line
        auto cit = std::find(info.class_names.begin(), info.class_names.end(), toks.back());

        // Not found
        if (cit == info.class_names.end()) {
            throw std::runtime_error("Unknown class label: " + toks.back());
        }
        // Stores the class label as an integer index
        ex.label = static_cast<int>(cit - info.class_names.begin());
        data.push_back(ex);  // Adds parsed example to the dataset
    }
    return data;  // returns dataset
}

// Learn value ranges (bins) from the training data only
void fit_discretization(DatasetInfo& info, const std::vector<Example>& train, int bins) {
    // For each attribute
    for (size_t a = 0; a < info.attributes.size(); a++) {
        if (!info.attributes[a].continuous) {  // skip categorical or symbolic attributes
            continue;
        }

        // Find min and max over the training set for this attribute
        double mn = std::numeric_limits<double>::infinity();
        double mx = -std::numeric_limits<double>::infinity();
        for (const auto& ex : train) {
            mn = std::min(mn, ex.raw_values[a]);
            mx = std::max(mx, ex.raw_values[a]);
        }

        // clears previous data
        info.attributes[a].values.clear();
        info.attributes[a].cut_points.clear();

        // If binning is invalid or all values are the same, create one fallback bin
        if (bins < 2 || mx <= mn) {
            info.attributes[a].values.push_back("bin0");  
            continue;
        }

        // Create equal-width cut points between min and max
        for (int b = 1; b < bins; b++) {
            double cut = mn + (mx - mn) * (static_cast<double>(b) / bins);
            info.attributes[a].cut_points.push_back(cut);
        }

        // Give each bin a readable name
        static const char* names[] = {"low", "medium", "high", "very_high", "ultra"};
        for (int b = 0; b < bins; b++) {
            if (b < 5) {
                info.attributes[a].values.push_back(names[b]);
            } else {
                // For bins beyond 5, use generic names like bin5, bin6, etc...
                info.attributes[a].values.push_back("bin" + std::to_string(b));
            }
        }
    }
}

// Convert training data numbers into value groups (discretization)
void apply_discretization(std::vector<Example>& data, const DatasetInfo& info) {
    // For each example
    for (auto& ex : data) {
        for (size_t a = 0; a < info.attributes.size(); a++) {
            if (!info.attributes[a].continuous) {  // skip categorical or symbolic attributes
                continue;
            }
            double x = ex.raw_values[a];  // raw continuous value for this attribute.
            int bin = 0;  // assuming the value belongs in the first bin

            // Move to the next bin while the value is greater than the current cut point
            while (bin < static_cast<int>(info.attributes[a].cut_points.size()) &&
                   x > info.attributes[a].cut_points[bin]) {
                bin++;
            }
            ex.values[a] = bin;  // Stores the final discrete bin index into values
        }
    }
}

// Computes the most frequent class label in a dataset
int majority_label(const std::vector<Example>& data, int class_count) {
    std::vector<int> counts(class_count, 0);

    // Counts how many times each class appears
    for (const auto& ex : data) {
        counts[ex.label]++;
    }
    int best = 0;

    // Checks all remaining classes
    for (int i = 1; i < class_count; i++) {
        // If class i has more examples than the current best, update best
        if (counts[i] > counts[best]) {
            best = i;
        }
    }
    return best;
}

// Load attributes, training data, test data, fit discretization, and compute majority class
DatasetSplit load_dataset(const std::string& attr_path,
                          const std::string& train_path,
                          const std::string& test_path,
                          int continuous_bins) {
    DatasetSplit ds;
    ds.info = parse_attr_file(attr_path);  // Read dataset metadata
    ds.train = parse_examples_raw(train_path, ds.info);  // Load raw training examples
    ds.test = parse_examples_raw(test_path, ds.info);  // Load raw test examples

    fit_discretization(ds.info, ds.train, continuous_bins);  // Learn value ranges (bins) from the training data only
    apply_discretization(ds.train, ds.info);
    apply_discretization(ds.test, ds.info);
    ds.info.majority_class = majority_label(ds.train, static_cast<int>(ds.info.class_names.size()));
    return ds;
}

// Convert an encoded attribute value index back to its string name
std::string value_name(const DatasetInfo& info, int attr_idx, int value_idx) {
    return info.attributes[attr_idx].values[value_idx];
}
