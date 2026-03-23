#include "experiments.h"
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        return run_command(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
