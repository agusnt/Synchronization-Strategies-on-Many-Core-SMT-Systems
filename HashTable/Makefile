CPP=g++
CPPFLAGS=-O2 -march=native -Wall -std=c++11 -g -faligned-new
.PHONY: ilock serie htm lockfree

default:
	@ echo "make ilock/lockfree/htm"

ilock: ilock/ht.cpp main.cpp
	$(CPP) $(CPPFLAGS) -fopenmp -I./ilock ilock/ht.cpp main.cpp -o ht.ilock


htm: htm/ht.cpp main.cpp
	$(CPP) $(CPPFLAGS) -mrtm -fopenmp -DHTM -I./htm htm/ht.cpp main.cpp -o ht.htm

lockfree: lockfree/ht.cpp main.cpp
	$(CPP) $(CPPFLAGS) -fopenmp -I./lockfree lockfree/ht.cpp main.cpp -o ht.lockfree

clean: 
	rm ht
