#ifndef GA_H
#define GA_H

#include "dataset.h"
#include <random>
#include <string>
#include <vector>

enum class SelectionStrategy {
    FITNESS_PROPORTIONAL,
    TOURNAMENT,
    RANK
};

struct Rule {
    std::vector<unsigned int> masks;
    int label = 0;
};

struct Individual {
    std::vector<Rule> rules;
    double fitness = 0.0;
    double train_accuracy = 0.0;
    double test_accuracy = 0.0;
};

struct GAConfig {
    int population_size = 120;
    double replacement_rate = 0.6;
    double mutation_rate = 0.02;
    int generations = 250;
    double fitness_threshold = 1.0;
    int max_rules = 8;
    SelectionStrategy selection = SelectionStrategy::TOURNAMENT;
    int tournament_size = 3;
    unsigned int seed = 42;
    bool verbose = false;
};

struct GARunResult {
    Individual best;
    std::vector<double> best_train_by_generation;
    std::vector<double> best_test_by_generation;
};

class GeneticRuleLearner {
public:
    GeneticRuleLearner(const DatasetInfo& info, const GAConfig& config);

    GARunResult run(const std::vector<Example>& train,
                    const std::vector<Example>& test);

    std::string individual_to_string(const Individual& ind) const;
    double accuracy(const Individual& ind, const std::vector<Example>& data) const;

private:
    DatasetInfo info_;
    GAConfig config_;
    std::mt19937 rng_;

    Individual random_individual();
    Rule random_rule();
    void evaluate(Individual& ind, const std::vector<Example>& train,
                  const std::vector<Example>& test) const;
    int predict(const Individual& ind, const Example& ex) const;
    bool matches(const Rule& rule, const Example& ex) const;

    Individual crossover(const Individual& a, const Individual& b);
    void mutate(Individual& ind);
    void mutate_rule(Rule& rule);

    int select_index(const std::vector<Individual>& pop);
    int roulette_index(const std::vector<Individual>& pop);
    int tournament_index(const std::vector<Individual>& pop);
    int rank_index(const std::vector<Individual>& pop);
};

SelectionStrategy parse_selection(const std::string& s);
std::string selection_to_string(SelectionStrategy s);

#endif
