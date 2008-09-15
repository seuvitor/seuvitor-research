#ifndef BB_H_
#define BB_H_

#include "timetabling.h"

#define MAX_NUM_EVENTS 200
#define MAX_NUM_ROOMS 10

struct TimeslotSet
{
    int onesCount;
    unsigned long long int bits;
    
    void fulfill();
    void clear();
    void discardTimeslot(int timeslot);
    void discardBeforeTimeslot(int timeslot);
    void discardAfterTimeslot(int timeslot);
    
    int size();
    bool empty();
    void toVector(std::vector<int>& vector);
    void print(std::ostream& out);
    
    TimeslotSet()
    {
    }
    
    ~TimeslotSet()
    {
    }
    
    TimeslotSet(const TimeslotSet& timeslotSet)
    {
        (*this) = timeslotSet;
    }
    
    TimeslotSet& operator=(const TimeslotSet& timeslotSet);
};

struct EventRoomAllocation
{
    char matchingEvent[MAX_NUM_EVENTS];
    int matchingRoom[MAX_NUM_ROOMS];
    bool visitedRoom[MAX_NUM_ROOMS];
    int usedRooms;
    
    EventRoomAllocation()
    {
        memset(matchingEvent, -1, sizeof(matchingEvent));
        memset(matchingRoom, -1, sizeof(matchingRoom));
        usedRooms = 0;
    }
};

struct State
{
    int coloring[MAX_NUM_EVENTS];
    TimeslotSet C[MAX_NUM_EVENTS];
    EventRoomAllocation roomAllocation[NUM_TIMESLOTS];
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
