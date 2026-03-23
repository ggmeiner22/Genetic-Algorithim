#include "experiments.h"
#include "dataset.h"
#include "ga.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace {

std::string get_arg(int argc, char** argv, const std::string& key, const std::string& def = "") {
    for (int i = 0; i + 1 < argc; ++i) {
        if (argv[i] == key) return argv[i + 1];
    }
    return def;
}

bool has_flag(int argc, char** argv, const std::string& key) {
    for (int i = 0; i < argc; ++i) if (argv[i] == key) return true;
    return false;
}

GAConfig build_config(int argc, char** argv) {
    GAConfig cfg;
    if (!get_arg(argc, argv, "--p").empty()) cfg.population_size = std::atoi(get_arg(argc, argv, "--p").c_str());
    if (!get_arg(argc, argv, "--r").empty()) cfg.replacement_rate = std::atof(get_arg(argc, argv, "--r").c_str());
    if (!get_arg(argc, argv, "--m").empty()) cfg.mutation_rate = std::atof(get_arg(argc, argv, "--m").c_str());
    if (!get_arg(argc, argv, "--gens").empty()) cfg.generations = std::atoi(get_arg(argc, argv, "--gens").c_str());
    if (!get_arg(argc, argv, "--threshold").empty()) cfg.fitness_threshold = std::atof(get_arg(argc, argv, "--threshold").c_str());
    if (!get_arg(argc, argv, "--max-rules").empty()) cfg.max_rules = std::atoi(get_arg(argc, argv, "--max-rules").c_str());
    if (!get_arg(argc, argv, "--selection").empty()) cfg.selection = parse_selection(get_arg(argc, argv, "--selection"));
    if (!get_arg(argc, argv, "--seed").empty()) cfg.seed = static_cast<unsigned int>(std::strtoul(get_arg(argc, argv, "--seed").c_str(), NULL, 10));
    cfg.verbose = has_flag(argc, argv, "--verbose");
    return cfg;
}

void print_result(const std::string& title, const GARunResult& res, GeneticRuleLearner& ga) {
    std::cout << "===== " << title << " =====\n";
    std::cout << ga.individual_to_string(res.best);
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Training accuracy: " << res.best.train_accuracy << "\n";
    std::cout << "Test accuracy:     " << res.best.test_accuracy << "\n";
}

DatasetSplit load_named_dataset(const std::string& name, int bins) {
    if (name == "tennis") {
        return load_dataset("data/tennis-attr.txt", "data/tennis-train.txt", "data/tennis-test.txt", bins);
    }
    if (name == "iris") {
        return load_dataset("data/iris-attr.txt", "data/iris-train.txt", "data/iris-test.txt", bins);
    }
    throw std::runtime_error("Unknown dataset: " + name);
}

int command_single_run(const std::string& dataset_name, int argc, char** argv) {
    int bins = 3;
    if (!get_arg(argc, argv, "--bins").empty()) bins = std::atoi(get_arg(argc, argv, "--bins").c_str());
    DatasetSplit ds = load_named_dataset(dataset_name, bins);
    GAConfig cfg = build_config(argc, argv);
    GeneticRuleLearner ga(ds.info, cfg);
    GARunResult res = ga.run(ds.train, ds.test);
    print_result(dataset_name, res, ga);
    return 0;
}

int command_selection_study(int argc, char** argv) {
    DatasetSplit ds = load_named_dataset("iris", 3);
    GAConfig cfg = build_config(argc, argv);
    std::vector<SelectionStrategy> methods = {
        SelectionStrategy::FITNESS_PROPORTIONAL,
        SelectionStrategy::TOURNAMENT,
        SelectionStrategy::RANK
    };
    std::vector<int> gens = {25, 50, 75, 100, 150, 200, 250, 300};

    std::cout << std::fixed << std::setprecision(4);
    for (auto sel : methods) {
        std::cout << "=== Selection: " << selection_to_string(sel) << " ===\n";
        for (int g : gens) {
            cfg.selection = sel;
            cfg.generations = g;
            GeneticRuleLearner ga(ds.info, cfg);
            auto res = ga.run(ds.train, ds.test);
            std::cout << "generation=" << std::setw(3) << g
                      << " train=" << res.best.train_accuracy
                      << " test=" << res.best.test_accuracy << "\n";
        }
        std::cout << "\n";
    }
    return 0;
}

int command_replacement_study(int argc, char** argv) {
    DatasetSplit ds = load_named_dataset("iris", 3);
    GAConfig base = build_config(argc, argv);
    std::vector<SelectionStrategy> methods = {
        SelectionStrategy::FITNESS_PROPORTIONAL,
        SelectionStrategy::TOURNAMENT,
        SelectionStrategy::RANK
    };

    std::cout << std::fixed << std::setprecision(4);
    for (auto sel : methods) {
        std::cout << "=== Selection: " << selection_to_string(sel) << " ===\n";
        for (int step = 1; step <= 9; ++step) {
            GAConfig cfg = base;
            cfg.selection = sel;
            cfg.replacement_rate = step / 10.0;
            GeneticRuleLearner ga(ds.info, cfg);
            auto res = ga.run(ds.train, ds.test);
            std::cout << "replacement_rate=" << std::setw(3) << cfg.replacement_rate
                      << " test=" << res.best.test_accuracy << "\n";
        }
        std::cout << "\n";
    }
    return 0;
}

void usage() {
    std::cout << "Usage:\n"
              << "  ./ga_rules testTennis [options]\n"
              << "  ./ga_rules testIris [options]\n"
              << "  ./ga_rules testIrisSelection [options]\n"
              << "  ./ga_rules testIrisReplacement [options]\n\n"
              << "Options:\n"
              << "  --p <int>           population size\n"
              << "  --r <double>        replacement rate\n"
              << "  --m <double>        mutation rate\n"
              << "  --gens <int>        number of generations\n"
              << "  --threshold <dbl>   training accuracy stopping threshold\n"
              << "  --selection <name>  fitness | tournament | rank\n"
              << "  --max-rules <int>   maximum rules per individual\n"
              << "  --seed <int>        random seed\n"
              << "  --bins <int>        bins for continuous attributes (iris)\n";
}

} // namespace

int run_command(int argc, char** argv) {
    if (argc < 2) {
        usage();
        return 1;
    }
    std::string cmd = argv[1];
    if (cmd == "testTennis") return command_single_run("tennis", argc, argv);
    if (cmd == "testIris") return command_single_run("iris", argc, argv);
    if (cmd == "testIrisSelection") return command_selection_study(argc, argv);
    if (cmd == "testIrisReplacement") return command_replacement_study(argc, argv);
    usage();
    return 1;
}
