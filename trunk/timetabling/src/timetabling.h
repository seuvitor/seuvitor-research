#ifndef TIMETABLING_H_
#define TIMETABLING_H_

#include <iostream>
#include <vector>
#include <set>

#define NUM_TIMESLOTS 45

struct Instance
{
    int nevents;
    int nrooms;
    int nfeatures;
    int nstudents;
    std::vector<std::set<int> > candidateTimeslots;
    std::vector<std::set<int> > candidateRooms;
    std::vector<std::set<int> > gamma;
    std::vector<std::set<int> > eventsBefore;
    std::vector<std::set<int> > eventsAfter;
    
    Instance(std::istream& in);
    
    void print(std::ostream& out);
};

struct Solution
{
    Instance* instance;
    std::vector<int> eventTimeslots;
    std::vector<int> eventRooms;
    
    Solution(Instance* instance);
    Solution(const Solution& solution);
    
    Solution& operator=(const Solution& solution);
    void print(std::ostream& out);
    
    bool isValid();
    int distanceToFeasibility();
    int softCost();
};

#endif /*TIMETABLING_H_*/
