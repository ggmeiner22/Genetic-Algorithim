#include "ga.h"
#include "util.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <stdexcept>

// Constructor
// Stores dataset metadata, configuration, and initializes
// the random number generator with the provided seed.
GeneticRuleLearner::GeneticRuleLearner(const DatasetInfo& info, const GAConfig& config)
    : info_(info), config_(config), rng_(config.seed) {}

// Create one random rule.
// Each attribute gets a bitmask saying which values are allowed.
// Example: for {Sunny, Overcast, Rain}, mask 101 means
// {Sunny, Rain} are allowed.
// A rule also predicts one class label.
Rule GeneticRuleLearner::random_rule() {
    Rule r;

    // One mask per attribute
    r.masks.resize(info_.attributes.size(), 0U);

    // For each attricbute
    for (size_t i = 0; i < info_.attributes.size(); i++) {

        // Get the number of possible values this attribute has
        int value_count = static_cast<int>(info_.attributes[i].values.size());
        unsigned int mask = 0U; // empty mask

        // For each value
        for (int v = 0; v < value_count; ++v) {
            //  decide (50% chance) whether to include this value in the rule
            if (rand01(rng_) < 0.5) {
                mask |= (1U << v);  // turn on bit v and add it to the mask
            }
        }
        if (mask == 0U) {
            // if no values were selected, force one random value to be included
            mask = (1U << randint(rng_, 0, value_count - 1));
        }
        r.masks[i] = mask;  // Store the mask for this attribute in the rule
    }
    // Pick a random class label for the rule conclusion
    r.label = randint(rng_, 0, static_cast<int>(info_.class_names.size()) - 1);
    return r;
}

// Create one random individual (candidate solution).
// An individual is a list of rules.
// The number of rules is chosen randomly from 1 ... max_rules.
Individual GeneticRuleLearner::random_individual() {
    Individual ind;

    // Pick a random number of rules between 1 and max_rules
    int count = randint(rng_, 1, config_.max_rules);
    ind.rules.reserve(count); // Reserve space for count rules

    // Create each rule -> Generate a random rule and add it to the individual
    for (int i = 0; i < count; i++) {
        ind.rules.push_back(random_rule());
    }
    return ind;
}

// Check whether a single rule matches a given example.
// A rule matches only if every attribute value in the example
// is allowed by that rule's bitmask.
bool GeneticRuleLearner::matches(const Rule& rule, const Example& ex) const {
    // For each value of the specified example
    for (size_t i = 0; i < ex.values.size(); i++) {

        // If the bit for this example's value is not set in the rule mask,
        // then the rule does not match.
        if ((rule.masks[i] & (1U << ex.values[i])) == 0U) {
            return false;
        }
    }
    return true;
}

// Predict the class for one example using an individual's rule list.
// Rules are checked in order.
// First matching rule wins.
// If no rule matches, fall back to the majority/default class.
int GeneticRuleLearner::predict(const Individual& ind, const Example& ex) const {
    // For each rule in the individual
    for (const auto& rule : ind.rules) {

        // If the rule matches the example:
        // immediately return that rule’s class label
        if (matches(rule, ex)) {
            return rule.label;
        }
    }
    // return the default class (most common class in training data)
    return info_.majority_class;
}

// Compute accuracy of an individual on a dataset.
// Accuracy = correct predictions / total examples.
double GeneticRuleLearner::accuracy(const Individual& ind, const std::vector<Example>& data) const {
    if (data.empty()) {
        return 0.0;
    }
    int correct = 0;

    // For each example in the dataset
    for (const auto& ex : data) {
        if (predict(ind, ex) == ex.label) {
            correct++;
        }
    }
    return static_cast<double>(correct) / data.size();
}

// Evaluate an individual on train and test sets.
// - train_accuracy is used for fitness
// - test_accuracy is stored for reporting
// Fitness is squared training accuracy to put extra pressure on
// higher-accuracy solutions.
void GeneticRuleLearner::evaluate(Individual& ind,
                                  const std::vector<Example>& train,
                                  const std::vector<Example>& test) const {
    
    ind.train_accuracy = accuracy(ind, train);
    ind.test_accuracy = accuracy(ind, test);

    // Squared accuracy emphasizes better individuals more strongly
    ind.fitness = ind.train_accuracy * ind.train_accuracy;
}

// One-point crossover at the rule-list level.
// Take prefix of parent a + suffix of parent b.
// This recombines whole rules rather than mixing rule internals.
Individual GeneticRuleLearner::crossover(const Individual& a, const Individual& b) {
    // If one parent has no rules, return the other
    if (a.rules.empty()) {
        return b;
    }
    if (b.rules.empty()) {
        return a;
    }

    // Choose cut positions
    int cut_a = randint(rng_, 0, static_cast<int>(a.rules.size()));
    int cut_b = randint(rng_, 0, static_cast<int>(b.rules.size()));

    Individual child;

    // Copy rules before cut from parent a
    for (int i = 0; i < cut_a; i++) {
        child.rules.push_back(a.rules[i]);
    }

    // Copy rules from cut to end from parent b
    for (size_t i = cut_b; i < b.rules.size(); i++) {
        child.rules.push_back(b.rules[i]);
    }

    // Ensure child has at least one rule
    if (child.rules.empty()) {
        child.rules.push_back(random_rule());
    }
    
    // Enforce maximum number of rules
    if (static_cast<int>(child.rules.size()) > config_.max_rules) {
        child.rules.resize(config_.max_rules);
    }
    return child;
}

// Mutate one rule.
// Mutation flips one random bit in one random attribute mask.
// This changes which values that rule accepts.
// There is also a 25% chance to mutate the rule's class label.
void GeneticRuleLearner::mutate_rule(Rule& rule) {
    // Pick a random attribute to modify
    int attr = randint(rng_, 0, static_cast<int>(rule.masks.size()) - 1);

    // Get how many possible values this attribute has
    int value_count = static_cast<int>(info_.attributes[attr].values.size());

    // Pick one value (bit) to change
    int bit = randint(rng_, 0, value_count - 1);

    // Pick one value (bit) to change
    rule.masks[attr] ^= (1U << bit);

    // Prevent empty mask. The rule must allow at least one value
    if (rule.masks[attr] == 0U) {
        rule.masks[attr] = (1U << bit);
    }

    // Occasionally mutate predicted class label too
    if (rand01(rng_) < 0.25) {
        rule.label = randint(rng_, 0, static_cast<int>(info_.class_names.size()) - 1);
    }
}

// Mutate an entire individual.
void GeneticRuleLearner::mutate(Individual& ind) {
    // 1. Mutate existing rules
    // For each rule in the individual
    for (auto& rule : ind.rules) {
        // With some probability (mutation rate), change the rule
        if (rand01(rng_) < config_.mutation_rate) {
            mutate_rule(rule);
        }
    }

    // 2. Add a rule
    // With some probability AND if we are under the max number of rules, add a new random rul
    if (rand01(rng_) < config_.mutation_rate && static_cast<int>(ind.rules.size()) < config_.max_rules) {
        ind.rules.push_back(random_rule());
    }

    // 3. Remove a rule
    // With some probability AND if we have more than 1 rule, pick a random rule index
    if (rand01(rng_) < config_.mutation_rate && ind.rules.size() > 1) {
        int idx = randint(rng_, 0, static_cast<int>(ind.rules.size()) - 1);
        ind.rules.erase(ind.rules.begin() + idx);  // Remove that rule
    }
}

// Fitness-proportional (roulette wheel) selection.
// Probability of selection is proportional to fitness.
// Small epsilon avoids zero-probability individuals.
int GeneticRuleLearner::roulette_index(const std::vector<Individual>& pop) {
    double total = 0.0;
    // Add up all fitness values
    for (const auto& ind : pop) {
        total += std::max(ind.fitness, 1e-12);
    }

    // Pick a random number between:
    // 0 and total fitness
    double pick = rand01(rng_) * total;
    double running = 0.0;

    // For each individual in the population
    for (size_t i = 0; i < pop.size(); i++) {

        // Add this individual’s fitness to the running total
        running += std::max(pop[i].fitness, 1e-12);

        // When running total passes the random pick:
        // select this individual
        if (running >= pick) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(pop.size()) - 1;  // Return last individual
}

// Tournament selection.
// Randomly sample tournament_size individuals and return the best.
// This increases selection pressure while keeping diversity.
int GeneticRuleLearner::tournament_index(const std::vector<Individual>& pop) {
    // Pick a random individual to start as the “best”.
    int best = randint(rng_, 0, static_cast<int>(pop.size()) - 1);

    // For each pick
    for (int i = 1; i < config_.tournament_size; ++i) {

        // Pick another random individual
        int cand = randint(rng_, 0, static_cast<int>(pop.size()) - 1);

        // If this candidate is better, update best
        if (pop[cand].fitness > pop[best].fitness) {
            best = cand;
        }
    }
    return best;  // Return the best individual found
}

// Rank-based selection.
// Individuals are sorted by fitness, then selected by rank rather
// than raw fitness. This reduces domination by a few very fit
// individuals and can help preserve diversity.
int GeneticRuleLearner::rank_index(const std::vector<Individual>& pop) {
    std::vector<int> order(pop.size());  // list to store indices of individuals

    // [0, 1, 2, ..., n-1]
    std::iota(order.begin(), order.end(), 0);

    // Sort indices by population fitness (lowest to highest)
    std::sort(order.begin(), order.end(), [&](int x, int y) {
        return pop[x].fitness < pop[y].fitness;
    });

    int n = static_cast<int>(pop.size());  // population size
    int total_rank = n * (n + 1) / 2;  // Total sum of ranks: 1 + 2 + ... + n

    // Pick a rank position
    int pick = randint(rng_, 1, total_rank);
    int running = 0;

    // Loop through sorted individuals
    for (int i = 0; i < n; i++) {
        
        // Add rank weight:
        // worst gets 1
        // best gets n
        running += (i + 1);

        // When running sum passes the random pick:
        // select this individual
        if (running >= pick) {
            return order[i];
        }
    }
    return order.back();
}

// Go to the correct parent-selection strategy based on
// current configuration.
int GeneticRuleLearner::select_index(const std::vector<Individual>& pop) {
    switch (config_.selection) {
        case SelectionStrategy::FITNESS_PROPORTIONAL: 
            return roulette_index(pop);
        case SelectionStrategy::TOURNAMENT: 
            return tournament_index(pop);
        case SelectionStrategy::RANK: 
            return rank_index(pop);
    }
    return 0;
}

// 1) initialize random population
// 2) evaluate population
// 3) repeat for generations:
//    - sort by fitness
//    - keep best survivors (elitist replacement)
//    - create children using selection + crossover + mutation
//    - evaluate new children
// 4) return best solution found
GARunResult GeneticRuleLearner::run(const std::vector<Example>& train,
                                    const std::vector<Example>& test) {
    // Create the starting population
    std::vector<Individual> pop(config_.population_size);

    // Fill the population with random individuals and evaluate each one
    for (auto& ind : pop) {
        ind = random_individual();
        evaluate(ind, train, test);
    }

    // Find the best individual in the starting population
    auto best_it = std::max_element(pop.begin(), pop.end(),
        [](const Individual& x, const Individual& y) { 
            return x.fitness < y.fitness; 
        });

    // Store the best individual found so far
    Individual global_best = *best_it;

    GARunResult result;

    // Run the GA for the requested number of generations
    for (int gen = 0; gen < config_.generations; gen++) {

        // Sort population from best fitness to worst fitness
        std::sort(pop.begin(), pop.end(), [](const Individual& x, const Individual& y) {
            return x.fitness > y.fitness;
        });

        // Save the best train/test accuracy for this generation
        result.best_train_by_generation.push_back(pop.front().train_accuracy);
        result.best_test_by_generation.push_back(pop.front().test_accuracy);

        // Update the best overall individual if current best is better
        if (pop.front().fitness > global_best.fitness) {
            global_best = pop.front();
        }

        // Stop early if training accuracy reached the target
        if (pop.front().train_accuracy >= config_.fitness_threshold) {
            break;
        }

        // Decide how many individuals to replace this generation
        int replace_count = static_cast<int>(std::round(
            config_.replacement_rate * config_.population_size));

        // Keep replacement count in a safe range
        replace_count = std::max(1, std::min(
            replace_count, config_.population_size - 1));

        // Number of top individuals that survive unchanged
        int survivors = config_.population_size - replace_count;

        std::vector<Individual> next;
        next.reserve(config_.population_size);

        // Copy the best survivors into the next population
        for (int i = 0; i < survivors; i++) {
            next.push_back(pop[i]);
        }

        // Fill the rest of the next population with new children
        while (static_cast<int>(next.size()) < config_.population_size) {
            int ia = select_index(pop);  // Pick first parent
            int ib = select_index(pop);  // Pick second parent
            Individual child = crossover(pop[ia], pop[ib]);  // Make child
            mutate(child);  // Mutate child
            evaluate(child, train, test);  // Score child
            next.push_back(child);  // Add child to next generation
        }
        pop.swap(next);  // Replace old population with the new one
    }

    // Sort one last time so the best current individual is at the front
    std::sort(pop.begin(), pop.end(), [](const Individual& x, const Individual& y) {
        return x.fitness > y.fitness;
    });

    // Update global best if final population has a better one
    if (pop.front().fitness > global_best.fitness) {
        global_best = pop.front();
    }

    // Save final best individual in the result
    result.best = global_best;
    return result;
}

// Convert one individual into readable text for printing
std::string GeneticRuleLearner::individual_to_string(const Individual& ind) const {
    std::ostringstream out;

    // Print the default class used when no rule matches
    out << "Default class: " << info_.class_names[info_.majority_class] << "\n";

    // Print each rule
    for (size_t r = 0; r < ind.rules.size(); r++) {
        out << "Rule " << (r + 1) << ": IF ";

        // Print each attribute condition in the rule
        for (size_t a = 0; a < info_.attributes.size(); a++) {
            out << info_.attributes[a].name << " IN {";
            bool first = true;

            // Print all allowed values for this attribute
            for (size_t v = 0; v < info_.attributes[a].values.size(); v++) {
                if (ind.rules[r].masks[a] & (1U << v)) {
                    if (!first) {
                        out << ", ";
                    }
                    out << info_.attributes[a].values[v];
                    first = false;
                }
            }
            out << "}";

            // Add AND between attributes
            if (a + 1 != info_.attributes.size()) {
                out << " AND ";
            }
        }
        // Print the class predicted by this rule
        out << " THEN class = " << info_.class_names[ind.rules[r].label] << "\n";
    }
    return out.str();
}

// Convert a string into a selection strategy enum
SelectionStrategy parse_selection(const std::string& s) {
    if (s == "fitness" || s == "fitness-proportional") {
        return SelectionStrategy::FITNESS_PROPORTIONAL;
    }
    if (s == "tournament") {
        return SelectionStrategy::TOURNAMENT;
    }
    if (s == "rank") {
        return SelectionStrategy::RANK;
    }
    throw std::runtime_error("Unknown selection strategy: " + s);
}

// Convert a selection strategy enum into a printable string
std::string selection_to_string(SelectionStrategy s) {
    switch (s) {
        case SelectionStrategy::FITNESS_PROPORTIONAL: 
            return "fitness-proportional";
        case SelectionStrategy::TOURNAMENT: 
            return "tournament";
        case SelectionStrategy::RANK: 
            return "rank";
    }
    return "unknown";
}
