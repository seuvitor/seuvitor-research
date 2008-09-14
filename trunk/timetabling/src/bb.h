#ifndef BB_H_
#define BB_H_

#include "timetabling.h"

#define MAX_NUM_EVENTS 200
#define MAX_NUM_ROOMS 10

struct PlaceSet
{
    int onesCount;
    unsigned long long int bits[MAX_NUM_ROOMS];
    
    void fulfill();
    void clear();
    void discardPlace(int place);
    void discardRoom(int room);
    void discardTimeslot(int timeslot);
    void discardBeforeTimeslot(int timeslot);
    void discardAfterTimeslot(int timeslot);
    
    int size();
    bool empty();
    void toVector(std::vector<int>& vector);
    void print(std::ostream& out);
    
    PlaceSet()
    {
    }
    
    ~PlaceSet()
    {
    }
    
    PlaceSet(const PlaceSet& placeSet)
    {
        (*this) = placeSet;
    }
    
    PlaceSet& operator=(const PlaceSet& placeSet);
};

struct State
{
    int coloring[MAX_NUM_EVENTS];
    PlaceSet P;
    PlaceSet C[MAX_NUM_EVENTS];
    bool visitable;
    
    State() : visitable(true)
    {
    }
    
    State(const State& state)
    {
        (*this) = state;
    }
    
    State& operator=(const State& state);
};

void bb_constructSolution(Instance* instance, Solution* solution);

#endif /*BB_H_*/
