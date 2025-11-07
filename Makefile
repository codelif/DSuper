CXX := g++
CXXFLAGS := -std=gnu++11 -O2 -Wall -I./src
SRCDIR = src

SRC := $(shell find $(SRCDIR) -name '*.cpp')
OBJ := $(SRC:.cpp=.o)

dsuper: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) dsuper
