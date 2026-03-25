CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -pedantic -Iinclude

SRC = src/main.cpp src/util.cpp src/dataset.cpp src/ga.cpp src/experiments.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = ga_rules

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
