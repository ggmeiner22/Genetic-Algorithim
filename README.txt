Genetic Algorithm Rule Learner (C++ / Modularized)
==================================================

Files
-----
- Makefile
- include/*.h
- src/*.cpp
- data/*.txt

Overview
--------
This project implements a modular genetic algorithm for learning variable-length
rule sets. Each individual is a disjunctive list of rules:

  IF attribute constraints match THEN predict class

The system supports:
- variable-length individuals (rule sets)
- population size p
- replacement rate r
- mutation rate m
- stopping by generation limit and/or training accuracy threshold
- selection strategies: fitness-proportional, tournament, rank
- both datasets required by the assignment: Tennis and Iris

Important note about Iris
-------------------------
The textbook GABIL example is propositional. The Iris attributes are continuous,
so this implementation discretizes each continuous attribute into bins using the
training data range. By default, 3 bins are used:
- low
- medium
- high

This keeps the internal rule representation uniform across Tennis and Iris.

Compilation on code01.fit.edu
-----------------------------
This project uses only standard C++17 and the GNU g++ toolchain.
No non-standard libraries are required.

Compile with:

  make

This produces the executable:

  ./ga_rules

Clean with:

  make clean

How to run the required tests
-----------------------------
1. Tennis learned rules + training/test accuracy

   ./ga_rules testTennis --p 80 --r 0.6 --m 0.05 --gens 200 --selection tournament --max-rules 6 --seed 42

2. Iris learned rules + training/test accuracy

   ./ga_rules testIris --p 150 --r 0.6 --m 0.03 --gens 300 --selection tournament --max-rules 8 --seed 42 --bins 3

3. Iris selection strategy experiment
   Varies the generation number and prints generation number, training accuracy,
   and test accuracy for the three selection strategies.

   ./ga_rules testIrisSelection --p 150 --r 0.6 --m 0.03 --max-rules 8 --seed 42

4. Iris replacement-rate experiment
   Varies replacement rate from 0.1 to 0.9 and prints replacement rate and
   test-set accuracy for the three selection strategies.

   ./ga_rules testIrisReplacement --p 150 --m 0.03 --gens 300 --max-rules 8 --seed 42

Command-line options
--------------------
--p <int>           population size
--r <double>        replacement rate
--m <double>        mutation rate
--gens <int>        number of generations
--threshold <dbl>   stop early if training accuracy >= threshold
--selection <name>  fitness | tournament | rank
--max-rules <int>   max rules in an individual
--seed <int>        RNG seed
--bins <int>        number of bins for continuous attrs (Iris)

Representation details
----------------------
- A rule stores, for each attribute, a bitmask of allowed values.
- A rule matches an example if each attribute value is allowed by the
  corresponding bitmask.
- An individual is an ordered list of rules.
- Prediction uses the first matching rule.
- If no rule matches, the default prediction is the majority training class.

Fitness
-------
Fitness is:

  fitness = (training_accuracy)^2

which follows the spirit of the textbook example.

Genetic operators
-----------------
- Selection: fitness-proportional, tournament, rank
- Crossover: variable-length one-point crossover at rule boundaries
- Mutation:
  - flip an allowed value in a rule constraint
  - optionally change a rule class
  - optionally add a rule
  - optionally delete a rule

Suggested parameters
--------------------
These worked reasonably well in local testing:
- Tennis: p=80, r=0.6, m=0.05, generations=200, tournament selection
- Iris:   p=150, r=0.6, m=0.03, generations=300, tournament selection, 3 bins

Notes
-----
Because this is a stochastic algorithm, results can change with the seed.
To make runs reproducible, pass --seed <value>.
