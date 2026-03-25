#ifndef UTIL_H
#define UTIL_H

#include <random>
#include <string>
#include <vector>

// Split a string into words based on spaces
std::vector<std::string> split_ws(const std::string& s);

// Remove spaces from the start and end of a string
std::string trim(const std::string& s);

// Generate a random number between 0.0 and 1.0
double rand01(std::mt19937& rng);

// Generate a random integer between lo and hi (inclusive)
int randint(std::mt19937& rng, int lo, int hi);

#endif
