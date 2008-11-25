#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "bb.h"

int nevents;
int nrooms;

void TimeslotSet::discardTimeslot(int timeslot, int(*candidatesCount)[NUM_TIMESLOTS])
{
#ifdef USE_BITSET
    unsigned long long int bitstring = bits;
    bits = bitstring & (~(1ULL << timeslot));
    if (bits != bitstring)
    {
        size -= 1;
        (*candidatesCount)[timeslot] -= 1;
    }
#else
    if (available[timeslot])
    {
        available[timeslot] = false;
        size -= 1;
        (*candidatesCount)[timeslot] -= 1;
        if ((*candidatesCount)[timeslot] < 0)
        {
            throw std::domain_error ("TimeslotSet::discardTimeslot: candidatesCount[t] < 0.");
        }
    }
#endif
}

void TimeslotSet::toVector(std::vector<int>& vector)
{
#ifdef USE_BITSET
    for (int t = 0; t < NUM_TIMESLOTS; ++t)
    {
        if (((bits >> t) & 1) == 1)
        {
            vector.push_back(t);
        }
    }
#else
    for (int t = 0; t < NUM_TIMESLOTS; ++t)
    {
        if (available[t])
        {
            vector.push_back(t);
        }
    }
#endif
}

TimeslotSet& TimeslotSet::operator=(const TimeslotSet& timeslotSet)
{
    size = timeslotSet.size;
#ifdef USE_BITSET    
    bits = timeslotSet.bits;
#else
    std::memcpy(available, timeslotSet.available, sizeof(timeslotSet.available));
#endif
    return (*this);
}

State& State::operator=(const State& state)
{
    std::memcpy(coloring, state.coloring, sizeof(int) * nevents);
    std::memcpy(C, state.C, sizeof(TimeslotSet) * nevents);
    std::memcpy(candidatesCount, state.candidatesCount, sizeof(state.candidatesCount));
    std::memcpy(roomAllocation, state.roomAllocation, sizeof(EventRoomAllocation) * NUM_TIMESLOTS);
    visitable = state.visitable;
    return (*this);
}

int selectEvent(Instance* instance, State& state)
{
    int bestEvent = -1;
    int bestNumCandidates = -1;
    int bestDegree = -1;
    for (int e = 0; e < nevents; ++e)
    {
        if (state.coloring[e] != -1) continue;
        
        int numCandidates = state.C[e].size;
        int degree = instance->gamma[e].size();
        
        if (bestEvent == -1 || numCandidates < bestNumCandidates ||
                (numCandidates == bestNumCandidates && degree > bestDegree))
        {
            bestEvent = e;
            bestNumCandidates = numCandidates;
            bestDegree = degree;
        }
    }
    return bestEvent;
}

int selectTimeslot(Instance* instance, State& state, int event)
{
    std::vector<int> candidates;
    state.C[event].toVector(candidates);
    
    int bestTimeslot = -1;
    int bestCandidateCount = -1;
    int bestUsedRooms = -1;
    int countEqual = 0;
    for(unsigned int i = 0; i < candidates.size(); ++i)
    {
        int t = candidates[i];
        int candidateCount = state.candidatesCount[t];
        int usedRooms = state.roomAllocation[t].usedRooms;
        
        if (bestTimeslot == -1 || candidateCount < bestCandidateCount ||
                candidateCount == bestCandidateCount && usedRooms > bestUsedRooms)
        {
            bestTimeslot = t;
            bestCandidateCount = candidateCount;
            bestUsedRooms = usedRooms;
            countEqual = 1;
        }
        else if (candidateCount == bestCandidateCount && usedRooms == bestUsedRooms)
        {
            countEqual += 1;
            
            // With probability 1/countEqual, prefer this equivalent timeslot
            if (rand() % 1000 <= 1000 / countEqual)
            {
                bestTimeslot = t;
                bestCandidateCount = candidateCount;
                bestUsedRooms = usedRooms;
            }
        }
    }
    return bestTimeslot;
}

bool updateCandidateTimeslots(Instance* instance, State& state, int event, int timeslot)
{
    // If there are no remaining free rooms, discard timeslot from all events
    if (state.roomAllocation[timeslot].usedRooms >= nrooms)
    {
        for (int e = 0; e < nevents; ++e)
        {
            if (state.coloring[e] != -1)
            {
                continue;
            }
            state.C[e].discardTimeslot(timeslot, &state.candidatesCount);
            if (state.C[e].size == 0)
            {
                return true;
            }
        }
    }
    else
    {
        // Otherwise, discard timeslot for all events that conflict with event
        for (std::set<int>::iterator it = instance->gamma[event].begin(); it != instance->gamma[event].end(); ++it)
        {
            int e = *it;
            if (state.coloring[e] != -1)
            {
                continue;
            }
            
            state.C[e].discardTimeslot(timeslot, &state.candidatesCount);
            if (state.C[e].size == 0)
            {
                return true;
            }
        }
    }
    
    // For all events that must occur after event, remove candidate timeslots
	// that cannot be used after allocating event to timeslot
    for (std::set<int>::iterator it = instance->eventsAfter[event].begin(); it != instance->eventsAfter[event].end(); ++it)
    {
        int e = *it;
        if (state.coloring[e] != -1)
        {
            continue;
        }
        for (int t = 0; t <= timeslot; ++t)
        {
            state.C[e].discardTimeslot(t, &state.candidatesCount);
        }
        if (state.C[e].size == 0)
        {
            return true;
        }
    }
    
    // For all events that must occur before event, remove candidate timeslots
    // that cannot be used after allocating event to timeslot
    for (std::set<int>::iterator it = instance->eventsBefore[event].begin(); it != instance->eventsBefore[event].end(); ++it)
    {
        int e = *it;
        if (state.coloring[e] != -1)
        {
            continue;
        }
        for (int t = timeslot; t < NUM_TIMESLOTS; ++t)
        {
            state.C[e].discardTimeslot(t, &state.candidatesCount);
        }
        if (state.C[e].size == 0)
        {
            return true;
        }
    }
    
    return false;
}

bool augmenting(Instance* instance, EventRoomAllocation *t, int event)
{
    for (int room = 0; room < nrooms; room++)
    {
        bool candidateRoom = instance->eventRooms[event].find(room)
                != instance->eventRooms[event].end();
        
        if (candidateRoom && !t->visitedRoom[room])
        {
            t->visitedRoom[room] = true;
            if (t->matchingRoom[room] == -1 || augmenting(instance, t, t->matchingRoom[room]))
            {
                t->matchingEvent[event] = room;
                t->matchingRoom[room] = event;
                return true;
            }
        }
    }
    return false;
}

bool insertIfFeasible(Instance* instance, State& state, int event, int timeslot)
{
    // Calculate feasibility of insertion using maximum bipartite matching
    EventRoomAllocation *t = &(state.roomAllocation[timeslot]);
    for (int r = 0; r < nrooms; r++)
    {
        t->visitedRoom[r] = false;
    }
    
    bool foundAugmentingPath = augmenting(instance, t, event);
    if (foundAugmentingPath)
    {
        t->usedRooms += 1;
    }
    
    return foundAugmentingPath;
}

void bb_constructSolution(Instance* instance, Solution* solution)
{
    nevents = instance->nevents;
    nrooms = instance->nrooms;
    
    if (nevents > MAX_NUM_EVENTS)
    {
        throw std::invalid_argument("The number of events is larger than MAX_NUM_EVENTS.");
    }
    if (nrooms > MAX_NUM_ROOMS)
    {
        throw std::invalid_argument("The number of rooms is larger than MAX_NUM_ROOMS.");
    }
    
    State* stateStack = new State[MAX_NUM_EVENTS + 1];
    
    int level = 0;

    // Set initial coloring
    for (int e = 0; e < nevents; ++e)
    {
        stateStack[level].coloring[e] = -1;
    }
    
    // C is the set of candidate timeslots for each event
    for (int e = 0; e < nevents; ++e)
    {
        std::fill(stateStack[level].candidatesCount,
                stateStack[level].candidatesCount + NUM_TIMESLOTS, nevents);
    }
    
    for (int e = 0; e < nevents; ++e)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            // If timeslot is not a candidate for this event
            if (instance->eventTimeslots[e].find(t) == instance->eventTimeslots[e].end())
            {
                stateStack[level].C[e].discardTimeslot(t, &stateStack[level].candidatesCount);
            }
        }
    }
    
    std::clog << "> Starting branch-and-bound..." << std::endl;
    int maxLevel = 0;
    int it = 0;
    int maxIt = 100000000;
    
    while (level < nevents && it < maxIt)
    {
        it++;
        
        if (level > maxLevel)
        {
            maxLevel = level;
        }
        if (!(it % 10000)) std::clog << "\rcurrentLevel=" << level << ", maxLevel=" << maxLevel << "              ";
        
        int event = selectEvent(instance, stateStack[level]);
        int timeslot = selectTimeslot(instance, stateStack[level], event);
        
        stateStack[level].C[event].discardTimeslot(timeslot, &stateStack[level].candidatesCount);
        
        // Copy current state data to one stack level higher
        stateStack[level + 1] = stateStack[level];
        
        // Set current state as not visitable, if this event has no candidates
        if (stateStack[level].C[event].size == 0)
        {
            stateStack[level].visitable = false;
        }
        
        // Go to one level higher in the stack
        level += 1;
        
        // Try to insert event on timeslot, suceeds if there is any feasible
        // room allocation for the group of events on this timeslot.
        bool backtrack = !insertIfFeasible(instance, stateStack[level], event, timeslot);
        
        if (!backtrack)
        {
            // Remove all candidates
            for (int t = 0; t < NUM_TIMESLOTS; ++t)
            {
                stateStack[level].C[event].discardTimeslot(t,
                        &stateStack[level].candidatesCount);
            }
            stateStack[level].coloring[event] = timeslot;
            backtrack = updateCandidateTimeslots(instance, stateStack[level], event, timeslot);
        }
        
        if (backtrack)
        {
            level -= 1;
            bool foundVisitable = false;
            while (!foundVisitable && level >= 0)
            {
                if (stateStack[level].visitable)
                {
                    foundVisitable = true;
                }
                else
                {
                    level -= 1;
                }
            }
            
            if (!foundVisitable)
            {
                break;
            }
        }
        
        if (level == nevents)
        {
            for (int e = 0; e < nevents; ++e)
            {
                solution->eventTimeslots[e] = stateStack[level].coloring[e];
                for (int t = 0; t < NUM_TIMESLOTS; ++t)
                {
                    int r = stateStack[level].roomAllocation[t].matchingEvent[e];
                    if (r != -1)
                    {
                        solution->eventRooms[e] = r;
                        break;
                    }
                }
            }
            break;

            // Backtrack if possible
            level -= 1;
            bool foundVisitable = false;
            while (!foundVisitable && level >= 0)
            {
                if (stateStack[level].visitable)
                {
                    foundVisitable = true;
                }
                else
                {
                    level -= 1;
                }
            }
            
            if (!foundVisitable)
            {
                break;
            }
        }
    }
    std::clog << std::endl;

    
    delete[] stateStack;
}
