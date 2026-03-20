#include "ga.h"
#include "util.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <stdexcept>

GeneticRuleLearner::GeneticRuleLearner(const DatasetInfo& info, const GAConfig& config)
    : info_(info), config_(config), rng_(config.seed) {}

Rule GeneticRuleLearner::random_rule() {
    Rule r;
    r.masks.resize(info_.attributes.size(), 0U);
    for (size_t i = 0; i < info_.attributes.size(); ++i) {
        int value_count = static_cast<int>(info_.attributes[i].values.size());
        unsigned int mask = 0U;
        for (int v = 0; v < value_count; ++v) {
            if (rand01(rng_) < 0.5) mask |= (1U << v);
        }
        if (mask == 0U) mask = (1U << randint(rng_, 0, value_count - 1));
        r.masks[i] = mask;
    }
    r.label = randint(rng_, 0, static_cast<int>(info_.class_names.size()) - 1);
    return r;
}

Individual GeneticRuleLearner::random_individual() {
    Individual ind;
    int count = randint(rng_, 1, config_.max_rules);
    ind.rules.reserve(count);
    for (int i = 0; i < count; ++i) ind.rules.push_back(random_rule());
    return ind;
}

bool GeneticRuleLearner::matches(const Rule& rule, const Example& ex) const {
    for (size_t i = 0; i < ex.values.size(); ++i) {
        if ((rule.masks[i] & (1U << ex.values[i])) == 0U) return false;
    }
    return true;
}

int GeneticRuleLearner::predict(const Individual& ind, const Example& ex) const {
    for (const auto& rule : ind.rules) {
        if (matches(rule, ex)) return rule.label;
    }
    return info_.majority_class;
}

double GeneticRuleLearner::accuracy(const Individual& ind, const std::vector<Example>& data) const {
    if (data.empty()) return 0.0;
    int correct = 0;
    for (const auto& ex : data) {
        if (predict(ind, ex) == ex.label) ++correct;
    }
    return static_cast<double>(correct) / data.size();
}

void GeneticRuleLearner::evaluate(Individual& ind,
                                  const std::vector<Example>& train,
                                  const std::vector<Example>& test) const {
    ind.train_accuracy = accuracy(ind, train);
    ind.test_accuracy = accuracy(ind, test);
    ind.fitness = ind.train_accuracy * ind.train_accuracy;
}

Individual GeneticRuleLearner::crossover(const Individual& a, const Individual& b) {
    if (a.rules.empty()) return b;
    if (b.rules.empty()) return a;

    int cut_a = randint(rng_, 0, static_cast<int>(a.rules.size()));
    int cut_b = randint(rng_, 0, static_cast<int>(b.rules.size()));

    Individual child;
    for (int i = 0; i < cut_a; ++i) child.rules.push_back(a.rules[i]);
    for (size_t i = cut_b; i < b.rules.size(); ++i) child.rules.push_back(b.rules[i]);

    if (child.rules.empty()) child.rules.push_back(random_rule());
    if (static_cast<int>(child.rules.size()) > config_.max_rules) {
        child.rules.resize(config_.max_rules);
    }
    return child;
}

void GeneticRuleLearner::mutate_rule(Rule& rule) {
    int attr = randint(rng_, 0, static_cast<int>(rule.masks.size()) - 1);
    int value_count = static_cast<int>(info_.attributes[attr].values.size());
    int bit = randint(rng_, 0, value_count - 1);
    rule.masks[attr] ^= (1U << bit);
    if (rule.masks[attr] == 0U) rule.masks[attr] = (1U << bit);

    if (rand01(rng_) < 0.25) {
        rule.label = randint(rng_, 0, static_cast<int>(info_.class_names.size()) - 1);
    }
}

void GeneticRuleLearner::mutate(Individual& ind) {
    for (auto& rule : ind.rules) {
        if (rand01(rng_) < config_.mutation_rate) mutate_rule(rule);
    }

    if (rand01(rng_) < config_.mutation_rate && static_cast<int>(ind.rules.size()) < config_.max_rules) {
        ind.rules.push_back(random_rule());
    }
    if (rand01(rng_) < config_.mutation_rate && ind.rules.size() > 1) {
        int idx = randint(rng_, 0, static_cast<int>(ind.rules.size()) - 1);
        ind.rules.erase(ind.rules.begin() + idx);
    }
}

int GeneticRuleLearner::roulette_index(const std::vector<Individual>& pop) {
    double total = 0.0;
    for (const auto& ind : pop) total += std::max(ind.fitness, 1e-12);
    double pick = rand01(rng_) * total;
    double running = 0.0;
    for (size_t i = 0; i < pop.size(); ++i) {
        running += std::max(pop[i].fitness, 1e-12);
        if (running >= pick) return static_cast<int>(i);
    }
    return static_cast<int>(pop.size()) - 1;
}

int GeneticRuleLearner::tournament_index(const std::vector<Individual>& pop) {
    int best = randint(rng_, 0, static_cast<int>(pop.size()) - 1);
    for (int i = 1; i < config_.tournament_size; ++i) {
        int cand = randint(rng_, 0, static_cast<int>(pop.size()) - 1);
        if (pop[cand].fitness > pop[best].fitness) best = cand;
    }
    return best;
}

int GeneticRuleLearner::rank_index(const std::vector<Individual>& pop) {
    std::vector<int> order(pop.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int x, int y) {
        return pop[x].fitness < pop[y].fitness;
    });
    int n = static_cast<int>(pop.size());
    int total_rank = n * (n + 1) / 2;
    int pick = randint(rng_, 1, total_rank);
    int running = 0;
    for (int i = 0; i < n; ++i) {
        running += (i + 1);
        if (running >= pick) return order[i];
    }
    return order.back();
}

int GeneticRuleLearner::select_index(const std::vector<Individual>& pop) {
    switch (config_.selection) {
        case SelectionStrategy::FITNESS_PROPORTIONAL: return roulette_index(pop);
        case SelectionStrategy::TOURNAMENT: return tournament_index(pop);
        case SelectionStrategy::RANK: return rank_index(pop);
    }
    return 0;
}

GARunResult GeneticRuleLearner::run(const std::vector<Example>& train,
                                    const std::vector<Example>& test) {
    std::vector<Individual> pop(config_.population_size);
    for (auto& ind : pop) {
        ind = random_individual();
        evaluate(ind, train, test);
    }

    auto best_it = std::max_element(pop.begin(), pop.end(),
        [](const Individual& x, const Individual& y) { return x.fitness < y.fitness; });
    Individual global_best = *best_it;

    GARunResult result;

    for (int gen = 0; gen < config_.generations; ++gen) {
        std::sort(pop.begin(), pop.end(), [](const Individual& x, const Individual& y) {
            return x.fitness > y.fitness;
        });

        result.best_train_by_generation.push_back(pop.front().train_accuracy);
        result.best_test_by_generation.push_back(pop.front().test_accuracy);
        if (pop.front().fitness > global_best.fitness) global_best = pop.front();
        if (pop.front().train_accuracy >= config_.fitness_threshold) break;

        int replace_count = static_cast<int>(std::round(config_.replacement_rate * config_.population_size));
        replace_count = std::max(1, std::min(replace_count, config_.population_size - 1));
        int survivors = config_.population_size - replace_count;

        std::vector<Individual> next;
        next.reserve(config_.population_size);
        for (int i = 0; i < survivors; ++i) next.push_back(pop[i]);

        while (static_cast<int>(next.size()) < config_.population_size) {
            int ia = select_index(pop);
            int ib = select_index(pop);
            Individual child = crossover(pop[ia], pop[ib]);
            mutate(child);
            evaluate(child, train, test);
            next.push_back(child);
        }
        pop.swap(next);
    }

    std::sort(pop.begin(), pop.end(), [](const Individual& x, const Individual& y) {
        return x.fitness > y.fitness;
    });
    if (pop.front().fitness > global_best.fitness) global_best = pop.front();
    result.best = global_best;
    return result;
}

std::string GeneticRuleLearner::individual_to_string(const Individual& ind) const {
    std::ostringstream out;
    out << "Default class: " << info_.class_names[info_.majority_class] << "\n";
    for (size_t r = 0; r < ind.rules.size(); ++r) {
        out << "Rule " << (r + 1) << ": IF ";
        for (size_t a = 0; a < info_.attributes.size(); ++a) {
            out << info_.attributes[a].name << " IN {";
            bool first = true;
            for (size_t v = 0; v < info_.attributes[a].values.size(); ++v) {
                if (ind.rules[r].masks[a] & (1U << v)) {
                    if (!first) out << ", ";
                    out << info_.attributes[a].values[v];
                    first = false;
                }
            }
            out << "}";
            if (a + 1 != info_.attributes.size()) out << " AND ";
        }
        out << " THEN class = " << info_.class_names[ind.rules[r].label] << "\n";
    }
    return out.str();
}

SelectionStrategy parse_selection(const std::string& s) {
    if (s == "fitness" || s == "fitness-proportional") return SelectionStrategy::FITNESS_PROPORTIONAL;
    if (s == "tournament") return SelectionStrategy::TOURNAMENT;
    if (s == "rank") return SelectionStrategy::RANK;
    throw std::runtime_error("Unknown selection strategy: " + s);
}

std::string selection_to_string(SelectionStrategy s) {
    switch (s) {
        case SelectionStrategy::FITNESS_PROPORTIONAL: return "fitness-proportional";
        case SelectionStrategy::TOURNAMENT: return "tournament";
        case SelectionStrategy::RANK: return "rank";
    }
    return "unknown";
}
