PROG = timetabling
CC = g++
CFLAGS = -O3 -c -Wall
SRC = ../src
OBJS = main.o timetabling.o bb.o
TIMESTAMP = `date +%Y-%m-%d_%Hh%M`

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -O3 $(OBJS) -o $(PROG)

main.o: $(SRC)/main.cpp $(SRC)/timetabling.h $(SRC)/bb.h
	$(CC) $(CFLAGS) $(SRC)/main.cpp

timetabling.o: $(SRC)/timetabling.cpp
	$(CC) $(CFLAGS) $(SRC)/timetabling.cpp

bb.o: $(SRC)/bb.cpp $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/bb.cpp

clean:
	rm -f $(PROG) $(PROG).exe $(OBJS)

rebuild: clean all

dist: clean
	tar -c -z --exclude=.svn -f ./../dist/$(PROG)_$(TIMESTAMP).tar.gz -C ./../.. timetabling/src timetabling/debug timetabling/instances timetabling/reports
