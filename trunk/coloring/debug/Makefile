PROG = coloring
CC = g++
CFLAGS = -c -g
SRC = ../src
OBJS = main.o coloring.o dsatur.o
TIMESTAMP = `date +%Y-%m-%d_%Hh%M`

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG)

main.o: $(SRC)/main.cpp $(SRC)/coloring.h $(SRC)/dsatur.h
	$(CC) $(CFLAGS) $(SRC)/main.cpp

coloring.o: $(SRC)/coloring.cpp
	$(CC) $(CFLAGS) $(SRC)/coloring.cpp

dsatur.o: $(SRC)/dsatur.cpp $(SRC)/coloring.h
	$(CC) $(CFLAGS) $(SRC)/dsatur.cpp

clean:
	rm -f $(PROG) $(PROG).exe $(OBJS)

rebuild: clean all

dist: clean
	tar -c -z -f ./../dist/$(PROG)_$(TIMESTAMP).tar.gz -C ./../.. coloring/src coloring/debug coloring/instances
