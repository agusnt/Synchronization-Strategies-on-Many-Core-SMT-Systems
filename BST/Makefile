CPP=g++
CPPFLAGS=-O2 -march=native -Wall -std=c++11 -g -faligned-new
.PHONY: ilock serie htm lockfree ideal

default:
	@ echo "make serie/ilock/lockfree/htm/stm"

ilock: ilock/bst.cpp main.cpp
	$(CPP) $(CPPFLAGS) $(FOO) -fopenmp -I./ilock ilock/bst.cpp main.cpp -o bst.ilock

htm: htm/bst.cpp main.cpp
	$(CPP) $(CPPFLAGS) $(FOO) -mrtm -fopenmp -DHTM -I./htm htm/bst.cpp main.cpp -o bst.htm

lockfree: lockfree/bst.cpp main.cpp
	$(CPP) $(CPPFLAGS) $(FOO) -fopenmp -I./lockfree lockfree/bst.cpp main.cpp -o bst.lockfree

clean: 
	rm bst
