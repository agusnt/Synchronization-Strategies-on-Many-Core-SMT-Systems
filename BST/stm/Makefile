ROOT = ../..

include $(ROOT)/Makefile.common

BINS = main

.PHONY:	all clean

all:	$(BINS)

%.o:	%.cpp
		$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -std=c++11 -fopenmp -DSTM -c -o $@ $<

$(BINS):	%:	%.o $(TMLIB)
		g++ bst.cpp -std=c++11 -fopenmp -DSTM -o $@ $< $(LDFLAGS)

clean:
		rm -f $(BINS) *.o
