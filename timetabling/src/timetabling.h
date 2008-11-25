#ifndef TIMETABLING_H_
#define TIMETABLING_H_

#include <iostream>
#include <vector>
#include <set>

#define NUM_TIMESLOTS 45
#define TIMESLOTS_PER_DAY 9
#define MAX_NUM_EVENTS 600
#define MAX_NUM_ROOMS 20
#define MAX_NUM_STUDENTS 1000

struct Instance
{
    int nevents;
    int nrooms;
    int nfeatures;
    int nstudents;
    std::vector<std::set<int> > studentEvents;
    std::vector<std::set<int> > timeslotEvents;
    std::vector<std::set<int> > eventTimeslots;
    std::vector<std::set<int> > roomEvents;
    std::vector<std::set<int> > eventRooms;
    std::vector<std::set<int> > gamma;
    std::vector<std::set<int> > eventsBefore;
    std::vector<std::set<int> > eventsAfter;
    std::vector<int> eventAttendance;
    
    void eventsForStudentTimeslotRoom(int student, int timeslot, int room, std::set<int>& events);
    void eventsForStudentTimeslot(int student, int timeslot, std::set<int>& events);
    void eventsForTimeslotRoom(int timeslot, int room, std::set<int>& events);
    void commonTimeslots(int event1, int event2, std::set<int>& timeslots);
    
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
