# Genetic-Algorithim

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


### experiments.h / experiments.cpp


### ga.h / ga.cpp


### util.h / util.cpp


### main.cpp


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