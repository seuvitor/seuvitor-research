#include <set>

#include "timetabling.h"

Instance::Instance(std::istream& in)
{
    // Read instance params
    in >> nevents;
    in >> nrooms;
    in >> nfeatures;
    in >> nstudents;
    
    // Read room sizes
    std::vector<int> roomSizes(nrooms);
    for (int i = 0; i < nrooms; ++i)
    {
        in >> roomSizes[i];
    }
    
    // Adjacency (conflict) list for each events
    gamma = std::vector<std::set<int> >(nevents, std::set<int>());
    
    // Number of students attending to each event
    eventAttendance = std::vector<int>(nevents, 0);
    
    studentEvents = std::vector<std::set<int> >(nstudents, std::set<int>());
    for (int s = 0; s < nstudents; ++s)
    {
        for (int e = 0; e < nevents; ++e)
        {
            int attends;
            in >> attends;
            if (attends == 1)
            {
                eventAttendance[e] += 1;
                studentEvents[s].insert(e);
            }
        }
        
        // For each pair of events attended by a student, add conflict
        for (std::set<int>::iterator it1 = studentEvents[s].begin(); it1 != studentEvents[s].end(); ++it1)
        {
            for (std::set<int>::iterator it2 = studentEvents[s].begin(); it2 != studentEvents[s].end(); ++it2)
            {
                int e1 = *it1;
                int e2 = *it2;
                if (e1 < e2)
                {
                    gamma[e1].insert(e2);
                    gamma[e2].insert(e1);
                }
            }
        }
    }
    
    // Read features available in rooms
    std::vector<std::set<int> > roomFeatures(nrooms, std::set<int>());
    for (int r = 0; r < nrooms; ++r)
    {
        for (int f = 0; f < nfeatures; ++f)
        {
            int available;
            in >> available;
            if (available == 1)
            {
                roomFeatures[r].insert(f);
            }
        }
    }
    
    // Initially set all timeslots and rooms as candidates for all events
    eventTimeslots = std::vector<std::set<int> >(nevents, std::set<int>());
    eventRooms = std::vector<std::set<int> >(nevents, std::set<int>());
    for (int e = 0; e < nevents; ++e)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            eventTimeslots[e].insert(t);
        }
        for (int r = 0; r < nrooms; ++r)
        {
            eventRooms[e].insert(r);
        }
    }
    
    // Remove from candidate rooms if a feature required by the event is not
    // available on the room
    for (int e = 0; e < nevents; ++e)
    {
        for (int f = 0; f < nfeatures; ++f)
        {
            int requires;
            in >> requires;
            if (requires == 1)
            {
                for (int r = 0; r < nrooms; ++r)
                {
                    // If not found
                    if (roomFeatures[r].find(f) == roomFeatures[r].end())
                    {
                        eventRooms[e].erase(r);
                    }
                }
            }
        }
    }
    
    // Remove from candidate rooms if room capacity is not enough for the event
    for (int e = 0; e < nevents; ++e)
    {
        for (int r = 0; r < nrooms; ++r)
        {
            if (roomSizes[r] < eventAttendance[e])
            {
                eventRooms[e].erase(r);
            }
        }
    }
    
    // Look for pairs of events which have a single candidateRoom and set those
    // as conflicting events
    for (int e1 = 0; e1 < nevents; ++e1)
    {
        for (int e2 = 0; e2 < nevents; ++e2)
        {
            if (e1 < e2 && eventRooms[e1].size() == 1
                    && eventRooms[e2].size() == 1)
            {
                int e1Room = *eventRooms[e1].begin();
                int e2Room = *eventRooms[e2].begin();
                if (e1Room == e2Room)
                {
                    gamma[e1].insert(e2);
                    gamma[e2].insert(e1);
                }
            }
        }
    }
    
    // Remove from candidate timeslots if a timeslot is not available
    for (int e = 0; e < nevents; ++e)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            int available;
            in >> available;
            if (available == 0)
            {
                eventTimeslots[e].erase(t);
            }
        }
    }
    
    timeslotEvents = std::vector<std::set<int> >(NUM_TIMESLOTS, std::set<int>());
    for (int u = 0; u < nevents; ++u)
    {
        for (std::set<int>::iterator itT = eventTimeslots[u].begin(); itT != eventTimeslots[u].end(); ++itT)
        {
            int t = *itT;
            timeslotEvents[t].insert(u);
        }
    }
    
    roomEvents = std::vector<std::set<int> >(nrooms, std::set<int>());
    for (int u = 0; u < nevents; ++u)
    {
        for (std::set<int>::iterator itR = eventRooms[u].begin(); itR != eventRooms[u].end(); ++itR)
        {
            int r = *itR;
            roomEvents[r].insert(u);
        }
    }
    
    eventsAfter = std::vector<std::set<int> >(nevents, std::set<int>());
    eventsBefore = std::vector<std::set<int> >(nevents, std::set<int>());
    
    // Set precedence relations
    for (int e1 = 0; e1 < nevents; ++e1)
    {
        for (int e2 = 0; e2 < nevents; ++e2)
        {
            int preceeds;
            in >> preceeds;
            if (preceeds == 1)
            {
                std::vector<int> stackAfterEvent1(1, e2);
                while(stackAfterEvent1.size() > 0)
                {
                    int e = stackAfterEvent1.back();
                    stackAfterEvent1.pop_back();
                    
                    eventsAfter[e1].insert(e);
                    eventsBefore[e].insert(e1);

                    // Also add to adjacency (conflict) list
                    gamma[e1].insert(e);
                    gamma[e].insert(e1);
                    
                    // Also look for transitive precedence relations
                    std::copy(eventsAfter[e].begin(), eventsAfter[e].end(),
                            std::back_inserter(stackAfterEvent1));
                }
            }
        }
    }
}

void Instance::print(std::ostream& out)
{
}

void intersection(std::set<int>& set1, std::set<int>& set2,
        std::set<int>& result)
{
    std::set_intersection(set1.begin(), set1.end(),
                          set2.begin(), set2.end(),
                          std::inserter(result, result.end()));
}

void intersection(std::set<int>& set1, std::set<int>& set2,
        std::set<int>& set3, std::set<int>& result)
{
    std::set<int> intermediate;
    std::set_intersection(set1.begin(), set1.end(),
                          set2.begin(), set2.end(),
                          std::inserter(intermediate, intermediate.end()));
    
    std::set_intersection(set3.begin(), set3.end(),
                          intermediate.begin(), intermediate.end(),
                          std::inserter(result, result.end()));
}

void Instance::eventsForStudentTimeslotRoom(int student, int timeslot, int room, std::set<int>& events)
{
    intersection(studentEvents[student],
                 timeslotEvents[timeslot],
                 roomEvents[room],
                 events);
}

void Instance::eventsForStudentTimeslot(int student, int timeslot, std::set<int>& events)
{
    intersection(studentEvents[student],
                 timeslotEvents[timeslot],
                 events);
}

void Instance::eventsForTimeslotRoom(int timeslot, int room, std::set<int>& events)
{
    intersection(timeslotEvents[timeslot],
                 roomEvents[room],
                 events);
}

void Instance::commonTimeslots(int event1, int event2, std::set<int>& timeslots)
{
    intersection(eventTimeslots[event1],
                 eventTimeslots[event2],
                 timeslots);
}

Solution::Solution(Instance* instance) :
	instance(instance)
{
    eventTimeslots = std::vector<int>(instance->nevents, -1);
    eventRooms = std::vector<int>(instance->nevents, -1);
}

Solution::Solution(const Solution& solution)
{
    (*this) = solution;
}

bool Solution::isValid()
{
    return false;
}

int Solution::distanceToFeasibility()
{
    return -1;
}

int Solution::softCost()
{
    return -1;
}

void Solution::print(std::ostream& out)
{
    out << "event:(timeslot, room) --> ";
    for (int e = 0; e < instance->nevents; ++e)
    {
    	out << e << ":(" << eventTimeslots[e] << ", " << eventRooms[e] << "), ";
    }
    out << std::endl;
}

Solution& Solution::operator=(const Solution& solution)
{
    instance = solution.instance;
    eventTimeslots = std::vector<int>(solution.eventTimeslots.begin(), solution.eventTimeslots.end());
    eventRooms = std::vector<int>(solution.eventRooms.begin(), solution.eventRooms.end());
    return (*this);
}
