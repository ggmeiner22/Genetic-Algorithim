#include "util.h"
#include <algorithm>
#include <cctype>
#include <sstream>

// Split a string by whitespace into words
std::vector<std::string> split_ws(const std::string& s) {
    std::istringstream iss(s);  // Treat string like a stream
    std::vector<std::string> out;
    std::string tok;

    // Read each word separated by spaces
    while (iss >> tok) {
        out.push_back(tok);
    }
    return out;
}

// Remove whitespace from the start and end of a string
std::string trim(const std::string& s) {
    size_t i = 0;

    // Move forward until first non-space character
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
        i++;
    }
    size_t j = s.size();

    // Move backward until last non-space character
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) {
        j--;
    }

    // Return substring without leading or trailing spaces
    return s.substr(i, j - i);
}

// Generate a random double between 0.0 and 1.0
double rand01(std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

// Generate a random integer between lo and hi (inclusive)
int randint(std::mt19937& rng, int lo, int hi) {
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(rng);
}
