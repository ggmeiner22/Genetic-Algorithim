#ifndef UTIL_H
#define UTIL_H

#include <random>
#include <string>
#include <vector>

std::vector<std::string> split_ws(const std::string& s);
std::string trim(const std::string& s);

double rand01(std::mt19937& rng);
int randint(std::mt19937& rng, int lo, int hi);

#endif
