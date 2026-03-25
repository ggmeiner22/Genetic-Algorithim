# Genetic-Algorithim

This project implements a Genetic Algorithm (GA) for learning rule-based classifiers from data. 
Each solution (individual) is a set of IF-THEN rules that classify examples. The algorithm evolves 
these rule sets over multiple generations using selection, crossover, and mutation.

The system supports both:
- **Discrete datasets** (e.g., Tennis)
- **Continuous datasets** (e.g., Iris, using discretization into bins)

The goal is to learn rules that maximize classification accuracy on training data while generalizing well to test data.

Key features:
- Multiple selection strategies (fitness-proportional, tournament, rank)
- Structural mutation (rules can be added or removed)
- Discretization of continuous attributes into bins
- Experiment scripts for evaluating performance across parameters

## Compalation and Execution
```
chmod -R u+w .
chmod +x *.sh
chmod +x scripts/*.sh
chmod +x src/*.cpp
```
### Run All
```
./run_all.sh
```
### Run Individually
#### Build
```
make
```
#### Tennis
```
./scripts/testTennis.sh
```
#### Iris
```
./scripts/testIris.sh
```
#### Iris Replacement
```
./scripts/testIrisReplacement.sh
```
#### Iris Selection
```
./scripts/testIrisSelection.sh
```
#### Clean object files
```
make clean
```

## File Structure
```
.
├── data
│   ├── iris-attr.txt
│   ├── iris-test.txt
│   ├── iris-train.txt
│   ├── tennis-attr.txt
│   ├── tennis-test.txt
│   └── tennis-train.txt
├── include
│   ├── dataset.h
│   ├── experiments.h
│   ├── ga.h
│   └── util.h
├── scripts
│   ├── gridSearch.sh
│   ├── testIris.sh
│   ├── testIrisReplacement.sh
│   ├── testIrisSelection.sh
│   └── testTennis.sh
├── src
│   ├── dataset.cpp
│   ├── experiments.cpp
│   ├── ga.cpp
│   ├── main.cpp
│   └── util.cpp
│
├── Makefile
└── run_all.sh
```

## File Overview

### dataset.h / dataset.cpp
Handles all dataset loading and preprocessing.
- Parses attribute files and data files
- Converts symbolic values into indices
- Discretizes continuous values into bins (low, medium, high, etc.)
- Splits data into training and test sets
- Computes the majority class for default predictions

---

### experiments.h / experiments.cpp
Controls experiment execution based on command-line input.
- Runs different experiment modes (Tennis, Iris, selection tests, replacement tests)
- Configures GA parameters
- Prints results and tracks performance across runs

---

### ga.h / ga.cpp
Core implementation of the Genetic Algorithm.
- Defines rules, individuals, and GA configuration
- Implements:
  - Selection (roulette, tournament, rank)
  - Crossover (combine individuals)
  - Mutation (modify rules and structure)
- Evaluates individuals using training and test accuracy
- Runs evolution across generations to find the best rule set

---

### util.h / util.cpp
Utility/helper functions used across the project.
- String processing (split, trim)
- Random number generation (uniform float and integer)
> These functions support parsing and stochastic behavior in the GA

---

### main.cpp
Entry point of the program.
- Calls `run_command()` to execute experiments
- Handles errors safely using try-catch
> Keeps the program simple and delegates logic to experiments.cpp

### data/
Contains datasets used for experiments.
- **tennis**-* – discrete attribute classification dataset
- **iris**-* – continuous attribute classification dataset
> Each dataset includes an attribute file describing the schema and corresponding training/testing files.

### scripts/
Automation scripts for running individual experiments, hyperparameter searches, and plotting runs.
- `run_identity.sh`
- `run_tennis.sh`
- `run_iris.sh`
- `run_irisNoisy.sh`
- `grid_search.sh`
- `make_plot.sh`

### Makefile
Defines compilation rules for building the project.
- Uses C++11 standard
- Compiles source files into object files
- Links the final executable
> Ensures consistent builds across machines

### run_all.sh
- Builds the project
- Runs all experiment modes
> Provides a reproducible workflow for testing the Genetic Algorithim on all datasets and experiments.