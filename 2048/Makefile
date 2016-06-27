CXX      = g++
CXXFLAGS = -Wall -Wshadow -O2 -Im -std=c++11

all: kevinptt learn gen

gen: genattr.cpp
	$(CXX) $(CXXFLAGS) -DKEVINPTT $< -o $@.out

learn: learn.cpp
	$(CXX) $(CXXFLAGS) -DKEVINPTT $< -o $@.out

kevinptt: main.cpp board.hpp board-kevinptt.hpp
	$(CXX) $(CXXFLAGS) -DKEVINPTT $< -o $@.out

bsnipers: main.cpp board.hpp board-bsnipers.hpp
	$(CXX) $(CXXFLAGS) -DBSNIPERS $< -o $@.out

clean:
	$(RM) -f *.out

