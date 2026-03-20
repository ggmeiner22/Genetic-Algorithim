#ifndef DATASET_H
#define DATASET_H

#include <string>
#include <vector>

struct Attribute {
    std::string name;
    bool continuous = false;
    std::vector<std::string> values;
    std::vector<double> cut_points;
};

struct Example {
    std::vector<int> values;
    int label = -1;
    std::vector<double> raw_values;
};

struct DatasetInfo {
    std::vector<Attribute> attributes;
    std::vector<std::string> class_names;
    int majority_class = 0;
};

struct DatasetSplit {
    DatasetInfo info;
    std::vector<Example> train;
    std::vector<Example> test;
};

DatasetSplit load_dataset(const std::string& attr_path,
                          const std::string& train_path,
                          const std::string& test_path,
                          int continuous_bins = 3);

std::string value_name(const DatasetInfo& info, int attr_idx, int value_idx);

#endif
