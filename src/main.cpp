#include "experiments.h"
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        // Try to run the program based on command-line input
        return run_command(argc, argv);
    } catch (const std::exception& ex) {
        // If something goes wrong, print the error message
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
