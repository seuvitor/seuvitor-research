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

void TimeslotSet::fulfill()
{
    onesCount = NUM_TIMESLOTS;
    bits = ~(~(0ULL) << NUM_TIMESLOTS);
}

void TimeslotSet::clear()
{
    bits = 0ULL;
    onesCount = 0;
}

void TimeslotSet::discardTimeslot(int timeslot)
{
    unsigned long long int bitstring = bits;
    bits = bitstring & (~(1ULL << timeslot));
    if (bits != bitstring)
    {
        onesCount -= 1;
    }
}

void TimeslotSet::discardBeforeTimeslot(int timeslot)
{
    bits &= (~(0ULL) << (timeslot + 1));
    onesCount = -1;
}

void TimeslotSet::discardAfterTimeslot(int timeslot)
{
    bits &= ~(~(0ULL) << timeslot);
    onesCount = -1;
}

int TimeslotSet::size()
{
    if (onesCount != -1)
    {
        return onesCount;
    }
    
    onesCount = 0;
    unsigned long long int bitstring = bits;
    while (bitstring != 0ULL)
    {
        onesCount += (bitstring & 1ULL);
        bitstring = (bitstring >> 1);
    }
    
    return onesCount;
}

bool TimeslotSet::empty()
{
    return (bits == 0ULL);
}

void TimeslotSet::toVector(std::vector<int>& vector)
{
    for (int t = 0; t < NUM_TIMESLOTS; ++t)
    {
        if (((bits >> t) & 1) == 1)
        {
            vector.push_back(t);
        }
    }
}

void TimeslotSet::print(std::ostream& out)
{
    out << "bits:" << std::endl;
    for (int t = (NUM_TIMESLOTS - 1); t >= 0; --t)
    {
        out << ((bits >> t) & 1ULL);
    }
    out << std::endl;
}

TimeslotSet& TimeslotSet::operator=(const TimeslotSet& timeslotSet)
{
    bits = timeslotSet.bits;
    onesCount = timeslotSet.onesCount;
    
    return (*this);
}

State& State::operator=(const State& state)
{
    std::memcpy(coloring, state.coloring, sizeof(int) * nevents);
    std::memcpy(C, state.C, sizeof(TimeslotSet) * nevents);
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
        
        int numCandidates = state.C[e].size();
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
    int idx = rand() % candidates.size();
    return candidates[idx];
}

bool updateCandidateTimeslots(Instance* instance, State& state, int event, int timeslot)
{
    // For all events that must occur after event, remove candidate timeslots
	// that cannot be used after allocating event to timeslot
    for (std::set<int>::iterator it = instance->eventsAfter[event].begin(); it != instance->eventsAfter[event].end(); ++it)
    {
        int e = *it;
        if (state.coloring[e] != -1)
        {
            continue;
        }
        state.C[e].discardBeforeTimeslot(timeslot);
        if (state.C[e].empty())
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
        state.C[e].discardAfterTimeslot(timeslot);
        if (state.C[e].empty())
        {
            return true;
        }
    }
    
    // For all events that conflict with event, remove candidate timeslots that
    // equal to timeslot
    for (std::set<int>::iterator it = instance->gamma[event].begin(); it != instance->gamma[event].end(); ++it)
    {
        int e = *it;
        if (state.coloring[e] != -1)
        {
            continue;
        }
        
        state.C[e].discardTimeslot(timeslot);
        if (state.C[e].empty())
        {
            return true;
        }
    }
    
    // TODO: Check need for this code
    /*
    for (int e = 0; e < nevents; ++e)
    {
        if (state.coloring[e] != -1)
        {
            continue;
        }
        state.C[e].discardTimeslot(timeslot);
        if (state.C[e].empty())
        {
            return true;
        }
    }
    */
    
    return false;
}

bool augmenting(Instance* instance, EventRoomAllocation *t, int event)
{
    for (int room = 0; room < MAX_NUM_ROOMS; room++)
    {
        bool candidateRoom = instance->candidateRooms[event].find(room)
                != instance->candidateRooms[event].end();
        
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
    // TODO: Calculate feasibility of insertion using maximum bipartite matching
    EventRoomAllocation *t = &(state.roomAllocation[timeslot]);
    for (int r = 0; r < MAX_NUM_ROOMS; r++)
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
    std::cout << "bb_constructSolution" << std::endl;
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
    
    solution->print(std::cout);
    
    State stateStack[MAX_NUM_EVENTS + 1];
    
    int level = 0;

    // Set initial coloring
    for (int e = 0; e < nevents; ++e)
    {
        stateStack[level].coloring[e] = -1;
    }
    
    // C is the set of candidate timeslots for each event
    for (int e = 0; e < nevents; ++e)
    {
        stateStack[level].C[e].fulfill();
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            // If timeslot is not a candidate for this event
            if (instance->candidateTimeslots[e].find(t) == instance->candidateTimeslots[e].end())
            {
                stateStack[level].C[e].discardTimeslot(t);
            }
        }
    }
    
    std::cout << "> Starting branch-and-bound..." << std::endl;
    int maxLevel = 0;
    int it = 0;
    int maxIt = 1000000;
    
    std::vector<int> numBacktracks(nevents, 0);
    
    while (level < nevents && it < maxIt)
    {
        it++;
        
        if (level > maxLevel)
        {
            maxLevel = level;
            std::cout << "maxLevel " << maxLevel << std::endl;
        }
        
        int event = selectEvent(instance, stateStack[level]);
        int timeslot = selectTimeslot(instance, stateStack[level], event);
        
        stateStack[level].C[event].discardTimeslot(timeslot);
        
        // Copy current state data to one stack level higher
        stateStack[level + 1] = stateStack[level];
        
        // Set current state as not visitable, if this event has no candidates
        if (stateStack[level].C[event].empty())
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
            stateStack[level].C[event].clear();
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
            numBacktracks[level] += 1;
        }
        
        if (level == nevents)
        {
            std::cout << "coloring found (it=" << it << "):" << std::endl;
            for (int e = 0; e < nevents; ++e)
            {
                std::cout << stateStack[level].coloring[e] << " ";
                for (int t = 0; t < NUM_TIMESLOTS; ++t)
                {
                    int r = stateStack[level].roomAllocation[t].matchingEvent[e];
                    if (r != -1)
                    {
                        std::cout << r;
                        break;
                    }
                }
                std::cout << std::endl;
            }
            return;

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
            numBacktracks[level] += 1;
        }
    }
    
    std::cout << "numBacktracks: ";
    for (std::vector<int>::iterator it = numBacktracks.begin(); it != numBacktracks.end(); ++it)
    {
        std::cout << *it << ", ";
    }
    std::cout << std::endl;
}
