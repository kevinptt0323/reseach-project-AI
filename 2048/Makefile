CXX      = g++
CXXFLAGS = -Wall -Wshadow -O2 -Im -std=c++11

all: kevinptt bsnipers

kevinptt: main.cpp board.hpp board-kevinptt.hpp
	$(CXX) $(CXXFLAGS) -DKEVINPTT $< -o $@

bsnipers: main.cpp board.hpp board-bsnipers.hpp
	$(CXX) $(CXXFLAGS) -DBSNIPERS $< -o $@

clean:
	$(RM) -f kevinptt bsnipers

