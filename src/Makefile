CXX := g++
CXXFLAGS := -std=gnu++11 -O2 -Wall -I.
SRCDIR = .

SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(SRC:.cpp=.o)

dsuper: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) dsuper
