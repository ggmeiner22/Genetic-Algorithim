#ifndef GA_H
#define GA_H

#include "dataset.h"
#include <random>
#include <string>
#include <vector>

// Different ways to select individuals in the GA
enum class SelectionStrategy {
    FITNESS_PROPORTIONAL,  // Roulette wheel selection
    TOURNAMENT,  // Roulette wheel selection
    RANK  // Select based on rank (sorted fitness)
};

// A single rule
struct Rule {
    std::vector<unsigned int> masks;  // Bitmasks for allowed values per attribute
    int label = 0;  // Class label this rule predicts
};

// One individual (solution) is a set of rules
struct Individual {
    std::vector<Rule> rules;  // List of rules
    double fitness = 0.0;  // Fitness score
    double train_accuracy = 0.0;  // Accuracy on training data
    double test_accuracy = 0.0;  // Accuracy on test data
};

// Configuration settings
struct GAConfig {
    int population_size = 120;  // Number of individuals
    double replacement_rate = 0.6;  // Fraction replaced each generation
    double mutation_rate = 0.02;  // Chance of mutation
    int generations = 250;  // Max number of generations
    double fitness_threshold = 1.0;  // Stop early if reached
    int max_rules = 8;  // Max rules per individual
    SelectionStrategy selection = SelectionStrategy::TOURNAMENT;  // Selection type
    int tournament_size = 3;  // Number of candidates in tournament
    unsigned int seed = 42;  // Random seed
};

// Result from running the GA
struct GARunResult {
    Individual best;  // Best individual found
    std::vector<double> best_train_by_generation;  // Train accuracy over time
    std::vector<double> best_test_by_generation;   // Test accuracy over time
};

// Main class for the genetic algorithm rule learner
class GeneticRuleLearner {
public:
    // Constructor: takes dataset info and configuration
    GeneticRuleLearner(const DatasetInfo& info, const GAConfig& config);

    // Run the GA and return results
    GARunResult run(const std::vector<Example>& train,
                    const std::vector<Example>& test);

    // Convert an individual into readable text (rules)
    std::string individual_to_string(const Individual& ind) const;

    // Compute accuracy on a dataset
    double accuracy(const Individual& ind, const std::vector<Example>& data) const;

private:
    DatasetInfo info_;  // Dataset info (attributes, labels)
    GAConfig config_;  // GA settings
    std::mt19937 rng_;  // Random number generator

    // Create random individual (set of rules)
    Individual random_individual();

    // Create random rule
    Rule random_rule();

    // Evaluate fitness and accuracy
    void evaluate(Individual& ind, const std::vector<Example>& train,
                  const std::vector<Example>& test) const;

    // Evaluate fitness and accuracy
    int predict(const Individual& ind, const Example& ex) const;

    // Check if a rule matches an example
    bool matches(const Rule& rule, const Example& ex) const;

    // Combine two individuals (crossover)
    Individual crossover(const Individual& a, const Individual& b);

    // Mutate an individual
    void mutate(Individual& ind);

    // Mutate a single rule
    void mutate_rule(Rule& rule);

    // Choose an individual index (based on selection strategy)
    int select_index(const std::vector<Individual>& pop);

    // Different selection methods
    int roulette_index(const std::vector<Individual>& pop);  // Fitness-based
    int tournament_index(const std::vector<Individual>& pop);  // Best of random group
    int rank_index(const std::vector<Individual>& pop);  // Based on rank
};

// Convert string to selection strategy
SelectionStrategy parse_selection(const std::string& s);

// Convert selection strategy to string
std::string selection_to_string(SelectionStrategy s);

#endif
