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
    std::vector<int> eventAttendance(nevents, 0);
    
    std::set<int> studentEvents;
    for (int s = 0; s < nstudents; ++s)
    {
        for (int e = 0; e < nevents; ++e)
        {
            int attends;
            in >> attends;
            if (attends == 1)
            {
                eventAttendance[e] += 1;
                studentEvents.insert(e);
            }
        }
        
        // For each pair of events attended by a student, add conflict
        for (std::set<int>::iterator it1 = studentEvents.begin(); it1 != studentEvents.end(); ++it1)
        {
            for (std::set<int>::iterator it2 = studentEvents.begin(); it2 != studentEvents.end(); ++it2)
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
        
        studentEvents.clear();
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
    candidateTimeslots = std::vector<std::set<int> >(nevents, std::set<int>());
    candidateRooms = std::vector<std::set<int> >(nevents, std::set<int>());
    for (int e = 0; e < nevents; ++e)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            candidateTimeslots[e].insert(t);
        }
        for (int r = 0; r < nrooms; ++r)
        {
            candidateRooms[e].insert(r);
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
                        candidateRooms[e].erase(r);
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
                candidateRooms[e].erase(r);
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
                candidateTimeslots[e].erase(t);
            }            
        }
    }
    
    eventsAfter = std::vector<std::set<int> >(nevents, std::set<int>());
    eventsBefore = std::vector<std::set<int> >(nevents, std::set<int>());
    
    for (int e1 = 0; e1 < nevents; ++e1)
    {
        for (int e2 = 0; e2 < nevents; ++e2)
        {
            int preceeds;
            in >> preceeds;
            if (preceeds == 1)
            {
                eventsAfter[e1].insert(e2);
                eventsBefore[e2].insert(e1);
            }
        }
    }
}

void Instance::print(std::ostream& out)
{
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
