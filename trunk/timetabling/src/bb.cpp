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

void PlaceSet::fulfill()
{
    onesCount = NUM_TIMESLOTS * nrooms;
    
    for (int r = 0; r < nrooms; ++r)
    {
        bits[r] = ~(~(0ULL) << NUM_TIMESLOTS);
    }
}

void PlaceSet::clear()
{
    for (int r = 0; r < nrooms; ++r)
    {
        bits[r] = 0ULL;
    }
    onesCount = 0;
}

void PlaceSet::discardPlace(int place)
{
    int t = place / nrooms;
    int r = place % nrooms;
    unsigned long long int bitstring = bits[r];
    bits[r] = bitstring & (~(1ULL << t));
    if (bits[r] != bitstring)
    {
        onesCount -= 1;
    }
}

void PlaceSet::discardRoom(int room)
{
    bits[room] = 0ULL;
    onesCount = -1;
}

void PlaceSet::discardTimeslot(int timeslot)
{
    unsigned long long int timeslotMask = (~(1ULL << timeslot));
    for (int r = 0; r < nrooms; ++r)
    {
        unsigned long long int bitstring = bits[r];
        bits[r] = bitstring & timeslotMask;
        if (bits[r] != bitstring)
        {
            onesCount -= 1;
        }
    }
}

void PlaceSet::discardBeforeTimeslot(int timeslot)
{
    for (int r = 0; r < nrooms; ++r)
    {
        bits[r] &= (~(0ULL) << (timeslot + 1));
    }
    onesCount = -1;
}

void PlaceSet::discardAfterTimeslot(int timeslot)
{
    for (int r = 0; r < nrooms; ++r)
    {
        bits[r] &= ~(~(0ULL) << timeslot);
    }
    onesCount = -1;
}

int PlaceSet::size()
{
    if (onesCount != -1)
    {
        return onesCount;
    }
    
    int count = 0;
    for (int r = 0; r < nrooms; ++r)
    {
        unsigned long long int bitstring = bits[r];
        while (bitstring != 0ULL)
        {
            count += (bitstring & 1ULL);
            bitstring = (bitstring >> 1);
        }
    }
    
    onesCount = count;
    return count;
}

bool PlaceSet::empty()
{
    for (int r = 0; r < nrooms; ++r)
    {
        if (bits[r] != 0ULL)
        {
            return false;
        }
    }
    return true;
    
}

void PlaceSet::toVector(std::vector<int>& vector)
{
    for (int r = 0; r < nrooms; ++r)
    {
        if (bits[r] != 0ULL)
        {
            for (int t = 0; t < NUM_TIMESLOTS; ++t)
            {
                if (((bits[r] >> t) & 1) == 1)
                {
                    int place = (t * nrooms) + r;
                    vector.push_back(place);
                }
            }
        }
    }
}

void PlaceSet::print(std::ostream& out)
{
    out << "bits:" << std::endl;
    for (int r = 0; r < nrooms; ++r)
    {
        for (int t = (NUM_TIMESLOTS - 1); t >= 0; --t)
        {
            out << ((bits[r] >> t) & 1ULL);
        }
        out << std::endl;
    }
}

PlaceSet& PlaceSet::operator=(const PlaceSet& placeSet)
{
    for (int r = 0; r < nrooms; ++r)
    {
        bits[r] = placeSet.bits[r];
    }
    onesCount = placeSet.onesCount;
    
    return (*this);
}

State& State::operator=(const State& state)
{
    std::memcpy(coloring, state.coloring, sizeof(int) * nevents);
    P = state.P;
    std::memcpy(C, state.C, sizeof(PlaceSet) * nevents);
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

int selectPlace(Instance* instance, State& state, int event)
{
    std::vector<int> candidates;
    state.C[event].toVector(candidates);
    int idx = rand() % candidates.size();
    return candidates[idx];
}

bool updateCandidatePlaces(Instance* instance, State& state, int event, int place)
{
    // For all events that must occur after event, remove candidate places that
    // cannot be used after allocating event to place
    for (std::set<int>::iterator it = instance->eventsAfter[event].begin(); it != instance->eventsAfter[event].end(); ++it)
    {
        int e = *it;
        if (state.coloring[e] != -1)
        {
            continue;
        }
        int t = place / nrooms;
        state.C[e].discardBeforeTimeslot(t);
        if (state.C[e].empty())
        {
            return true;
        }
    }
    
    // For all events that must occur before event, remove candidate places that
    // cannot be used after allocating event to place
    for (std::set<int>::iterator it = instance->eventsBefore[event].begin(); it != instance->eventsBefore[event].end(); ++it)
    {
        int e = *it;
        if (state.coloring[e] != -1)
        {
            continue;
        }
        int t = place / nrooms;
        state.C[e].discardAfterTimeslot(t);
        if (state.C[e].empty())
        {
            return true;
        }
    }
    
    // For all events that conflict with event, remove candidate places that
    // are in the same timeslot of place
    for (std::set<int>::iterator it = instance->gamma[event].begin(); it != instance->gamma[event].end(); ++it)
    {
        int e = *it;
        if (state.coloring[e] != -1)
        {
            continue;
        }
        
        int t = place / nrooms;
        state.C[e].discardTimeslot(t);
        if (state.C[e].empty())
        {
            return true;
        }
    }
    
    for (int e = 0; e < nevents; ++e)
    {
        if (state.coloring[e] != -1)
        {
            continue;
        }
        state.C[e].discardPlace(place);
        if (state.C[e].empty())
        {
            return true;
        }
    }
    
    return false;
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
    
    // P is the set of unused places
    stateStack[level].P = PlaceSet();
    stateStack[level].P.fulfill();
    
    // C is the set of candidate places for each event
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
        for (int r = 0; r < nrooms; ++r)
        {
            // If room is not a candidate for this event
            if (instance->candidateRooms[e].find(r) == instance->candidateRooms[e].end())
            {
                stateStack[level].C[e].discardRoom(r);
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
        int place = selectPlace(instance, stateStack[level], event);
        
        stateStack[level].P.discardPlace(place);
        stateStack[level].C[event].discardPlace(place);
        
        // Copy current state data to one stack level higher
        stateStack[level + 1] = stateStack[level];
        
        // Set current state as not visitable, if this event has no candidates
        if (stateStack[level].C[event].empty())
        {
            stateStack[level].visitable = false;
        }
        
        // Go to one level higher in the stack
        level += 1;
        
        stateStack[level].C[event].clear();
        
        stateStack[level].coloring[event] = place;
        
        bool backtrack = updateCandidatePlaces(instance, stateStack[level], event, place);
        
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
            std::cout << "coloring found (it=" << it << "):";
            for (int e = 0; e < nevents; ++e)
            {
                std::cout << stateStack[level].coloring[e] << ", ";
            }
            std::cout << std::endl;

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
    
    std::cout << "state.P" << std::endl;
    stateStack[level].P.print(std::cout);
    
    std::cout << "numBacktracks: ";
    for (std::vector<int>::iterator it = numBacktracks.begin(); it != numBacktracks.end(); ++it)
    {
        std::cout << *it << ", ";
    }
    std::cout << std::endl;
}
