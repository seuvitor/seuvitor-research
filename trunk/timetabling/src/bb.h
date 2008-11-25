#ifndef BB_H_
#define BB_H_

#include "timetabling.h"

struct TimeslotSet
{
    int size;
#ifdef USE_BITSET    
    unsigned long long int bits;
#else
    bool available[NUM_TIMESLOTS];
#endif
    
    void discardTimeslot(int timeslot, int (*candidatesCount)[NUM_TIMESLOTS]);
    void toVector(std::vector<int>& vector);
    
    TimeslotSet()
    {
#ifdef USE_BITSET
        bits = ~(~(0ULL) << NUM_TIMESLOTS);
#else
        std::fill(available, available + NUM_TIMESLOTS, true);
#endif
        size = NUM_TIMESLOTS;
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
        std::fill(matchingEvent, matchingEvent + MAX_NUM_EVENTS, -1);
        std::fill(matchingRoom, matchingRoom + MAX_NUM_ROOMS, -1);
        std::fill(visitedRoom, visitedRoom + MAX_NUM_ROOMS, false);
        usedRooms = 0;
    }
};

struct State
{
    int coloring[MAX_NUM_EVENTS];
    TimeslotSet C[MAX_NUM_EVENTS];
    int candidatesCount[NUM_TIMESLOTS];
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
