CXX      = g++
CXXFLAGS = -Wall -Wshadow -O2 -Im -std=c++11
RM       = rm -f
ID       = 1

all: kevinptt learn gen

gen: genattr.cpp
	$(CXX) $(CXXFLAGS) -DKEVINPTT $< -o $@.out

learn: learn.cpp
	$(CXX) $(CXXFLAGS) -DKEVINPTT $< -o $@.out

kevinptt: main.cpp board.hpp board-kevinptt.hpp
	$(CXX) $(CXXFLAGS) -DKEVINPTT $< -o $@.out

bsnipers: main.cpp board.hpp board-bsnipers.hpp
	$(CXX) $(CXXFLAGS) -DBSNIPERS $< -o $@.out

learnall:
	./learn.out zero$(ID).dat LR 10000 0.04 &
	./learn.out zero$(ID).dat LR 10000 0.01 &
	./learn.out zero$(ID).dat LR 10000 0.005 &

clean:
	$(RM) *.out

cleanLR:
	$(RM) LR*.dat

