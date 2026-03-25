#ifndef DATASET_H
#define DATASET_H

#include <string>
#include <vector>

// One attribute (feature) in the dataset
struct Attribute {
    std::string name;  // One attribute (feature) in the dataset
    bool continuous = false;  // True if numeric, false if symbolic
    std::vector<std::string> values;  // Possible values (or bin names)
    std::vector<double> cut_points;  // Cut points for binning continuous values
};

// One data example (row)
struct Example {
    std::vector<int> values;  // Discrete values (after encoding/binning)
    int label = -1;  // Class label (as index)
    std::vector<double> raw_values;  // Original numeric values (before binning)
};

// Metadata about the dataset
struct DatasetInfo {
    std::vector<Attribute> attributes;  // List of all attributes
    std::vector<std::string> class_names;  // List of class labels
    int majority_class = 0;  // Most common class (default prediction)
};

// Holds the full dataset split
struct DatasetSplit {
    DatasetInfo info;  // Dataset metadata
    std::vector<Example> train;  // Training data
    std::vector<Example> test;  // Test data
};

// Load dataset from files and apply discretization
DatasetSplit load_dataset(const std::string& attr_path,
                          const std::string& train_path,
                          const std::string& test_path,
                          int continuous_bins = 3);

// Load dataset from files and apply discretization
std::string value_name(const DatasetInfo& info, int attr_idx, int value_idx);

#endif
