PROG = timetabling
CC = g++
CFLAGS = -O3 -c -Wall
SRC = ../src
OBJS = main.o timetabling.o bb.o
TIMESTAMP = `date +%Y-%m-%d_%Hh%M`

all: $(PROG) checkcinsln

checkcinsln: $(SRC)/checkcinsln.cpp
	$(CC) -O3 -o checkcinsln $(SRC)/checkcinsln.cpp

$(PROG): $(OBJS)
	$(CC) -O3 -o $(PROG) $(OBJS)

main.o: $(SRC)/main.cpp $(SRC)/timetabling.h $(SRC)/bb.h
	$(CC) $(CFLAGS) $(SRC)/main.cpp

timetabling.o: $(SRC)/timetabling.cpp $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/timetabling.cpp

bb.o: $(SRC)/bb.cpp $(SRC)/bb.h $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/bb.cpp

clean:
	rm -f $(PROG) *.exe $(OBJS)

rebuild: clean all

dist: clean
	tar -c -z --exclude=.svn -f ./../dist/$(PROG)_$(TIMESTAMP).tar.gz -C ./../.. timetabling/src timetabling/debug timetabling/instances timetabling/reports
