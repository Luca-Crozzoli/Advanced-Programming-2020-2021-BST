CXX = c++
CXXFLAGS = -Wall -Wextra -g -std=c++17

SRC = bst.cpp

EXE = $(SRC:.cpp=.x)

.SUFFIXES:
SUFFIXES =

.SUFFIXES: .cpp .x

.PHONY: all
all: $(EXE)

%.x:%.cpp
	$(CXX) $< -o $@ $(CXXFLAGS)

.PHONY: clean
clean:
	rm -f $(EXE) *~ 

.PHONY: valgrind
valgrind:
	valgrind ./$(EXE)
.PHONY: run
run:
	./$(EXE)
