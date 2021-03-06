PROG = timetabling
CC = g++
CFLAGS = -O3 -c -Wall -IC:/ILOG/CPLEX110/include
SRC = ../src
OBJS = main.o timetabling.o bb.o solver.o colrep.o rep.o colmono.o mono.o graph.o
TIMESTAMP = `date +%Y-%m-%d_%Hh%M`

all: $(PROG) checkcinsln

checkcinsln: $(SRC)/checkcinsln.cpp
	$(CC) -O3 -o checkcinsln $(SRC)/checkcinsln.cpp

$(PROG): $(OBJS)
	$(CC) -O3 -o $(PROG) $(OBJS) -LC:/ILOG/CPLEX110/lib/x86_.net2005_8.0/stat_mda -lcplex110

main.o: $(SRC)/main.cpp $(SRC)/timetabling.h $(SRC)/bb.h $(SRC)/solver.h $(SRC)/colrep.h $(SRC)/rep.h $(SRC)/colmono.h $(SRC)/mono.h
	$(CC) $(CFLAGS) $(SRC)/main.cpp

timetabling.o: $(SRC)/timetabling.cpp $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/timetabling.cpp

bb.o: $(SRC)/bb.cpp $(SRC)/bb.h $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/bb.cpp

solver.o: $(SRC)/solver.cpp $(SRC)/solver.h
	$(CC) $(CFLAGS) $(SRC)/solver.cpp
	
graph.o: $(SRC)/graph.cpp $(SRC)/graph.h
	$(CC) $(CFLAGS) $(SRC)/graph.cpp
	
colrep.o: $(SRC)/colrep.cpp $(SRC)/colrep.h ${SRC}/solver.h ${SRC}/graph.h $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/colrep.cpp
	
rep.o: $(SRC)/rep.cpp $(SRC)/rep.h ${SRC}/solver.h ${SRC}/graph.h $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/rep.cpp

mono.o: $(SRC)/mono.cpp $(SRC)/mono.h ${SRC}/solver.h $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/mono.cpp

colmono.o: $(SRC)/colmono.cpp $(SRC)/colmono.h ${SRC}/solver.h $(SRC)/timetabling.h
	$(CC) $(CFLAGS) $(SRC)/colmono.cpp

clean:
	rm -f $(PROG) *.exe $(OBJS)

rebuild: clean all

dist: clean
	tar -c -z --exclude=.svn -f ./../dist/$(PROG)_$(TIMESTAMP).tar.gz -C ./../.. timetabling/src timetabling/debug timetabling/instances timetabling/reports
