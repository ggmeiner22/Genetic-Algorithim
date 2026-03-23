#include "dataset.h"
#include "util.h"
#include <algorithm>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {

std::vector<std::string> read_nonempty_lines(const std::string& path) {
    std::ifstream in(path.c_str());
    if (!in) throw std::runtime_error("Could not open file: " + path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

DatasetInfo parse_attr_file(const std::string& path) {
    auto lines = read_nonempty_lines(path);
    if (lines.size() < 2) throw std::runtime_error("Invalid attr file: " + path);

    DatasetInfo info;
    for (size_t i = 0; i + 1 < lines.size(); ++i) {
        auto toks = split_ws(lines[i]);
        if (toks.size() < 2) throw std::runtime_error("Bad attribute line: " + lines[i]);
        Attribute a;
        a.name = toks[0];
        if (toks[1] == "continuous") {
            a.continuous = true;
        } else {
            a.continuous = false;
            for (size_t j = 1; j < toks.size(); ++j) a.values.push_back(toks[j]);
        }
        info.attributes.push_back(a);
    }

    auto class_toks = split_ws(lines.back());
    if (class_toks.size() < 2) throw std::runtime_error("Bad class line: " + lines.back());
    for (size_t i = 1; i < class_toks.size(); ++i) info.class_names.push_back(class_toks[i]);
    return info;
}

std::vector<Example> parse_examples_raw(const std::string& path, const DatasetInfo& info) {
    auto lines = read_nonempty_lines(path);
    std::vector<Example> data;
    for (const auto& line : lines) {
        auto toks = split_ws(line);
        if (toks.size() != info.attributes.size() + 1) {
            throw std::runtime_error("Bad data line: " + line);
        }
        Example ex;
        ex.values.resize(info.attributes.size(), -1);
        ex.raw_values.resize(info.attributes.size(), 0.0);

        for (size_t i = 0; i < info.attributes.size(); ++i) {
            if (info.attributes[i].continuous) {
                ex.raw_values[i] = std::atof(toks[i].c_str());
            } else {
                auto it = std::find(info.attributes[i].values.begin(),
                                    info.attributes[i].values.end(), toks[i]);
                if (it == info.attributes[i].values.end()) {
                    throw std::runtime_error("Unknown symbolic value: " + toks[i]);
                }
                ex.values[i] = static_cast<int>(it - info.attributes[i].values.begin());
            }
        }
        auto cit = std::find(info.class_names.begin(), info.class_names.end(), toks.back());
        if (cit == info.class_names.end()) {
            throw std::runtime_error("Unknown class label: " + toks.back());
        }
        ex.label = static_cast<int>(cit - info.class_names.begin());
        data.push_back(ex);
    }
    return data;
}

void fit_discretization(DatasetInfo& info, const std::vector<Example>& train, int bins) {
    for (size_t a = 0; a < info.attributes.size(); ++a) {
        if (!info.attributes[a].continuous) continue;
        double mn = std::numeric_limits<double>::infinity();
        double mx = -std::numeric_limits<double>::infinity();
        for (const auto& ex : train) {
            mn = std::min(mn, ex.raw_values[a]);
            mx = std::max(mx, ex.raw_values[a]);
        }
        info.attributes[a].values.clear();
        info.attributes[a].cut_points.clear();
        if (bins < 2 || mx <= mn) {
            info.attributes[a].values.push_back("bin0");
            continue;
        }
        for (int b = 1; b < bins; ++b) {
            double cut = mn + (mx - mn) * (static_cast<double>(b) / bins);
            info.attributes[a].cut_points.push_back(cut);
        }
        static const char* names[] = {"low", "medium", "high", "very_high", "ultra"};
        for (int b = 0; b < bins; ++b) {
            if (b < 5) info.attributes[a].values.push_back(names[b]);
            else info.attributes[a].values.push_back("bin" + std::to_string(b));
        }
    }
}

void apply_discretization(std::vector<Example>& data, const DatasetInfo& info) {
    for (auto& ex : data) {
        for (size_t a = 0; a < info.attributes.size(); ++a) {
            if (!info.attributes[a].continuous) continue;
            double x = ex.raw_values[a];
            int bin = 0;
            while (bin < static_cast<int>(info.attributes[a].cut_points.size()) &&
                   x > info.attributes[a].cut_points[bin]) {
                ++bin;
            }
            ex.values[a] = bin;
        }
    }
}

int majority_label(const std::vector<Example>& data, int class_count) {
    std::vector<int> counts(class_count, 0);
    for (const auto& ex : data) counts[ex.label]++;
    int best = 0;
    for (int i = 1; i < class_count; ++i) {
        if (counts[i] > counts[best]) best = i;
    }
    return best;
}

} // namespace

DatasetSplit load_dataset(const std::string& attr_path,
                          const std::string& train_path,
                          const std::string& test_path,
                          int continuous_bins) {
    DatasetSplit ds;
    ds.info = parse_attr_file(attr_path);
    ds.train = parse_examples_raw(train_path, ds.info);
    ds.test = parse_examples_raw(test_path, ds.info);

    fit_discretization(ds.info, ds.train, continuous_bins);
    apply_discretization(ds.train, ds.info);
    apply_discretization(ds.test, ds.info);
    ds.info.majority_class = majority_label(ds.train, static_cast<int>(ds.info.class_names.size()));
    return ds;
}

std::string value_name(const DatasetInfo& info, int attr_idx, int value_idx) {
    return info.attributes[attr_idx].values[value_idx];
}
