#include "experiments.h"
#include "dataset.h"
#include "ga.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>

// Parse command-line argument value for a given key
// Example: --p 100 -> get_arg(..., "--p") returns "100"
std::string get_arg(int argc, char** argv, const std::string& key, const std::string& def = "") {
    for (int i = 0; i + 1 < argc; i++) {
        if (argv[i] == key) {
            return argv[i + 1];  // Return value after key
        }
    }
    return def;  // Return default if not found
}

// Build Config from command-line arguments
// This converts CLI inputs into structured GA parameters
// Each parameter is optional, it overrides defaults only if provided
GAConfig build_config(int argc, char** argv) {
    GAConfig cfg;
    if (!get_arg(argc, argv, "--p").empty()) {
        cfg.population_size = std::atoi(get_arg(argc, argv, "--p").c_str());
    }
    if (!get_arg(argc, argv, "--r").empty()) {
        cfg.replacement_rate = std::atof(get_arg(argc, argv, "--r").c_str());
    }
    if (!get_arg(argc, argv, "--m").empty()) {
        cfg.mutation_rate = std::atof(get_arg(argc, argv, "--m").c_str());
    }
    if (!get_arg(argc, argv, "--gens").empty()) {
        cfg.generations = std::atoi(get_arg(argc, argv, "--gens").c_str());
    }
    if (!get_arg(argc, argv, "--threshold").empty()) {
        cfg.fitness_threshold = std::atof(get_arg(argc, argv, "--threshold").c_str());
    }
    if (!get_arg(argc, argv, "--max-rules").empty()) {
        cfg.max_rules = std::atoi(get_arg(argc, argv, "--max-rules").c_str());
    }
    if (!get_arg(argc, argv, "--selection").empty()) {
        cfg.selection = parse_selection(get_arg(argc, argv, "--selection"));
    }
    if (!get_arg(argc, argv, "--seed").empty()) {
        cfg.seed = static_cast<unsigned int>(std::strtoul(get_arg(argc, argv, "--seed").c_str(), NULL, 10));
    }
    return cfg;
}

// Print results of a GA run
// Includes:
// - learned rules (via GA)
// - training accuracy
// - test accuracy
void print_result(const std::string& title, const GARunResult& res, GeneticRuleLearner& ga) {
    std::cout << "===== " << title << " =====\n";

    // Convert best individual (rule set) into human-readable string
    std::cout << ga.individual_to_string(res.best);
    std::cout << std::fixed << std::setprecision(4); // Format floating point output to 4 decimal places
    std::cout << "Training accuracy: " << res.best.train_accuracy << "\n";
    std::cout << "Test accuracy:     " << res.best.test_accuracy << "\n";
}

// Loads dataset by name
DatasetSplit load_named_dataset(const std::string& name, int bins) {
    if (name == "tennis") {
        return load_dataset("data/tennis-attr.txt",
            "data/tennis-train.txt",
            "data/tennis-test.txt",
            bins
        );
    }
    if (name == "iris") {
        return load_dataset("data/iris-attr.txt",
            "data/iris-train.txt",
            "data/iris-test.txt",
            bins
        );
    }  // Fail fast if dataset name is invalid
    throw std::runtime_error("Unknown dataset: " + name);
}

// Run a single experiment (used for testTennis/testIris)
int test_single_run(const std::string& dataset_name, int argc, char** argv) {
    int bins = 3;  // Default discretization bins for continuous features

    // Allow override via CLI
    if (!get_arg(argc, argv, "--bins").empty()) {
        bins = std::atoi(get_arg(argc, argv, "--bins").c_str());
    }

    // Load dataset
    DatasetSplit ds = load_named_dataset(dataset_name, bins);

    // Build GA configuration
    GAConfig cfg = build_config(argc, argv);

    // Initialize GA with dataset metadata
    GeneticRuleLearner ga(ds.info, cfg);

    // Run GA on train/test data
    GARunResult res = ga.run(ds.train, ds.test);

    // Print results
    print_result(dataset_name, res, ga);
    return 0;
}

// Look at effect of selection strategy + generations
// Used for testIrisSelection
int test_selection(int argc, char** argv) {
    DatasetSplit ds = load_named_dataset("iris", 3);
    GAConfig cfg = build_config(argc, argv);

    // Different selection strategies to compare
    std::vector<SelectionStrategy> methods = {
        SelectionStrategy::FITNESS_PROPORTIONAL,
        SelectionStrategy::TOURNAMENT,
        SelectionStrategy::RANK
    };

    // Generation values to test
    std::vector<int> gens = {25, 50, 75, 100, 150, 200, 250, 300};

    std::cout << std::fixed << std::setprecision(4);

    // Loop through each selection strategy
    for (auto sel : methods) {
        std::cout << "=== Selection: " << selection_to_string(sel) << " ===\n";

        // Test different generation counts
        for (int g : gens) {
            cfg.selection = sel;  // Set which selection method to use
            cfg.generations = g;  // Set the number of generations

            // Create a new GA model using the dataset info and current settings
            GeneticRuleLearner ga(ds.info, cfg);
            auto res = ga.run(ds.train, ds.test);  // Run the GA

            // Print results
            std::cout << "generation=" << std::setw(3) << g
                      << " train=" << res.best.train_accuracy
                      << " test=" << res.best.test_accuracy << "\n";
        }
        std::cout << "\n";
    }
    return 0;
}

// Look at effect of replacement rate (r)
// Used for testIrisReplacement
int test_replacement(int argc, char** argv) {
    DatasetSplit ds = load_named_dataset("iris", 3);
    GAConfig base = build_config(argc, argv);

    // Different selection strategies to compare
    std::vector<SelectionStrategy> methods = {
        SelectionStrategy::FITNESS_PROPORTIONAL,
        SelectionStrategy::TOURNAMENT,
        SelectionStrategy::RANK
    };

    std::cout << std::fixed << std::setprecision(4);

    // Loop over selection strategies
    for (auto sel : methods) {
        std::cout << "=== Selection: " << selection_to_string(sel) << " ===\n";

        // Test replacement rates from 0.1 to 0.9
        for (int step = 1; step <= 9; ++step) {
            GAConfig cfg = base;  // Copy base config
            cfg.selection = sel;  // Set which selection method to use
            cfg.replacement_rate = step / 10.0;  // Convert from int to double

            // Create a new GA model using the dataset info and current settings
            GeneticRuleLearner ga(ds.info, cfg);
            auto res = ga.run(ds.train, ds.test);  // Run the GA

            // Print results
            std::cout << "replacement_rate=" << std::setw(3) << cfg.replacement_rate
                      << " test=" << res.best.test_accuracy << "\n";
        }
        std::cout << "\n";
    }
    return 0;
}

// Print usage/help message
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

// Runs the tests
int run_command(int argc, char** argv) {
    // Too many args
    if (argc < 2) {
        usage();
        return 1;
    }

    std::string cmd = argv[1];
    if (cmd == "testTennis") {
        return test_single_run("tennis", argc, argv);
    }
    if (cmd == "testIris") {
        return test_single_run("iris", argc, argv);
    }
    if (cmd == "testIrisSelection") {
        return test_selection(argc, argv);
    }
    if (cmd == "testIrisReplacement") {
        return test_replacement(argc, argv);
    }

    // If incorrect usage of flags
    usage();
    return 1;
}
