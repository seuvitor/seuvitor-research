#include <stdexcept>
#include <vector>
#include <iterator>
#include <algorithm>
#include "solver.h"
#include "graph.h"
#include "rep.h"

#define xCol(u, v) xCols[(u * MAX_NUM_EVENTS) + v]

#define yCol(u, v, r) yCols[(u * MAX_NUM_EVENTS * MAX_NUM_ROOMS) + (MAX_NUM_ROOMS * v) + r]
#define pCol(u, t) pCols[(u * NUM_TIMESLOTS) + t]
#define qCol(v, t) qCols[(v * NUM_TIMESLOTS) + t]

#ifdef COMPLEX_PENALTIES
#define oCol(s, t) oCols[(s * NUM_TIMESLOTS) + t]
#define wCol(s, t) wCols[(s * NUM_TIMESLOTS) + t]
#define alphaCol(s, t) alphaCols[(s * NUM_TIMESLOTS) + t]
#define betaCol(s, t) betaCols[(s * NUM_TIMESLOTS) + t]
#endif /* COMPLEX_PENALTIES */

void rep_constructSolution(Instance* instance, Solution* solution)
{
    int* xCols = new int[MAX_NUM_EVENTS * MAX_NUM_EVENTS];
    
    int* yCols = new int[MAX_NUM_EVENTS * MAX_NUM_EVENTS * MAX_NUM_ROOMS];
    int* pCols = new int[MAX_NUM_EVENTS * NUM_TIMESLOTS];
    int* qCols = new int[MAX_NUM_EVENTS * NUM_TIMESLOTS];
    
#ifdef COMPLEX_PENALTIES
    int* oCols = new int[MAX_NUM_STUDENTS * NUM_TIMESLOTS];
    int* wCols = new int[MAX_NUM_STUDENTS * NUM_TIMESLOTS];
    int* alphaCols = new int[MAX_NUM_STUDENTS * NUM_TIMESLOTS];
    int* betaCols = new int[MAX_NUM_STUDENTS * NUM_TIMESLOTS];
#endif /* COMPLEX_PENALTIES */
    
    ModelData model;
    model.initArrays();
    model.numCols = 0;
    model.numRows = 0;
    model.numNz = 0;
    
    int nevents = instance->nevents;
    
    // Construct anti-adjacency sets for all events
    std::vector<std::set<int> > eventsRepresentedBy(instance->nevents, std::set<int>());
    std::vector<std::set<int> > eventsThatRepresent(instance->nevents, std::set<int>());
    buildRepresentativesSets(instance->gamma, eventsRepresentedBy, eventsThatRepresent);
    
    // Add x_{uv} variables
    for (int u = 0; u < nevents; ++u)
    {
        for (std::set<int>::iterator itV = eventsRepresentedBy[u].begin(); itV != eventsRepresentedBy[u].end(); ++itV)
        {
            int v = *itV;
            
            xCol(u, v) = model.numCols;
            sprintf(model.colNames[model.numCols], "x_%d_%d", u, v);
            model.lb[model.numCols] = 0.0;
            model.ub[model.numCols] = 1.0;
            model.xctype[model.numCols] = 'B';
            model.obj[model.numCols] = 0.0;
            model.numCols += 1;
        }
    }
    
    // Add y_{uvr} variables
    for (int u = 0; u < nevents; ++u)
    {
        for (std::set<int>::iterator itV = eventsRepresentedBy[u].begin(); itV != eventsRepresentedBy[u].end(); ++itV)
        {
            int v = *itV;
            for (std::set<int>::iterator itR =
                    instance->eventRooms[v].begin(); itR
                    != instance->eventRooms[v].end(); ++itR) {
                int r = *itR;
                
                yCol(u, v, r) = model.numCols;
                sprintf(model.colNames[model.numCols], "y_%d_%d_%d", u, v, r);
                model.lb[model.numCols] = 0.0;
                model.ub[model.numCols] = 1.0;
                model.xctype[model.numCols] = 'B';
                model.obj[model.numCols] = 0.0;
                
                model.numCols += 1;
            }
        }
    }
    
    // Add p_{ut} variables
    for (int u = 0; u < nevents; ++u)
    {
        for (std::set<int>::iterator itT = instance->eventTimeslots[u].begin(); itT != instance->eventTimeslots[u].end(); ++itT)
        {
            int t = *itT;
            
            pCol(u, t) = model.numCols;
            sprintf(model.colNames[model.numCols], "p_%d_%d", u, t);
            model.lb[model.numCols] = 0.0;
            model.ub[model.numCols] = 1.0;
            model.xctype[model.numCols] = 'B';
            model.obj[model.numCols] = 0.0;
            
            model.numCols += 1;
        }
    }
    
    // Add q_{vt} variables
    for (int v = 0; v < nevents; ++v)
    {
        for (std::set<int>::iterator itT = instance->eventTimeslots[v].begin(); itT != instance->eventTimeslots[v].end(); ++itT)
        {
            int t = *itT;
            
            qCol(v, t) = model.numCols;
            sprintf(model.colNames[model.numCols], "q_%d_%d", v, t);
            model.lb[model.numCols] = 0.0;
            model.ub[model.numCols] = 1.0;
            model.xctype[model.numCols] = 'B';
            model.obj[model.numCols] = 0.0;
            
            model.numCols += 1;
        }
    }
    
    // Set costs of q_{vt} variables
    for (int t = (TIMESLOTS_PER_DAY - 1); t < NUM_TIMESLOTS; t += TIMESLOTS_PER_DAY)
    {
        for (std::set<int>::iterator itV = instance->timeslotEvents[t].begin(); itV != instance->timeslotEvents[t].end(); ++itV)
        {
            int v = *itV;
            model.obj[qCol(v, t)] = instance->eventAttendance[v];
        }
    }
    
#ifdef COMPLEX_PENALTIES
    // Add o_{st} variables
    for (int s = 0; s < instance->nstudents; ++s)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            oCol(s, t) = model.numCols;
            sprintf(model.colNames[model.numCols], "o_%d_%d", s, t);
            model.lb[model.numCols] = 0.0;
            model.ub[model.numCols] = 1.0;
            model.xctype[model.numCols] = 'B';
            model.obj[model.numCols] = 1.0;
            model.numCols += 1;
        }
    }
    
    // Add w_{st} variables
    for (int s = 0; s < instance->nstudents; ++s)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            wCol(s, t) = model.numCols;
            sprintf(model.colNames[model.numCols], "w_%d_%d", s, t);
            model.lb[model.numCols] = 0.0;
            model.ub[model.numCols] = 1.0;
            model.xctype[model.numCols] = 'B';
            model.obj[model.numCols] = 1.0;
            model.numCols += 1;
        }
    }
    
    // Add alpha_{st} variables
    for (int s = 0; s < instance->nstudents; ++s)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            alphaCol(s, t) = model.numCols;
            sprintf(model.colNames[model.numCols], "alpha_%d_%d", s, t);
            model.lb[model.numCols] = 0.0;
            model.ub[model.numCols] = 2.0;
            model.xctype[model.numCols] = 'C';
            model.obj[model.numCols] = 0.0;
            model.numCols += 1;
        }
    }
    
    // Add beta_{st} variables
    for (int s = 0; s < instance->nstudents; ++s)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            betaCol(s, t) = model.numCols;
            sprintf(model.colNames[model.numCols], "beta_%d_%d", s, t);
            model.lb[model.numCols] = -8.0;
            model.ub[model.numCols] = 0.0;
            model.xctype[model.numCols] = 'C';
            model.obj[model.numCols] = 0.0;
            model.numCols += 1;
        }
    }
#endif /* COMPLEX_PENALTIES */
    
    // Add constraints of type a
    for (int v = 0; v < nevents; ++v)
    {
        sprintf(model.rowNames[model.numRows], "a_%d", v);
        model.rmatbeg[model.numRows] = model.numNz;
        model.sense[model.numRows] = 'G';
        model.rhs[model.numRows] = 1.0;
        
        for (std::set<int>::iterator itU = eventsThatRepresent[v].begin(); itU != eventsThatRepresent[v].end(); ++itU)
        {
            int u = *itU;
            model.rmatind[model.numNz] = xCol(u, v);
            model.rmatval[model.numNz] = 1.0;
            model.numNz += 1;
        }
        model.numRows += 1;
    }
    
    // Add constraints of type b
    for (int u = 0; u < nevents; ++u)
    {
        // Iterate over all pairs (v, w) on the anti-adjacency of u
        for (std::set<int>::iterator itV = eventsRepresentedBy[u].begin(); itV != eventsRepresentedBy[u].end(); ++itV)
        {
            int v = *itV;
            for (std::set<int>::iterator itW = eventsRepresentedBy[u].begin(); itW != eventsRepresentedBy[u].end(); ++itW)
            {
                int w = *itW;
                if (w >= v) break; // Avoid double checking pairs
                
                if (instance->gamma[v].find(w) != instance->gamma[v].end())
                {
                    // (v, w) is an edge on the anti-adjacency of u
                    sprintf(model.rowNames[model.numRows], "b_%d_%d_%d", u, v, w);
                    model.rmatbeg[model.numRows] = model.numNz;
                    model.sense[model.numRows] = 'L';
                    model.rhs[model.numRows] = 0.0;
                    
                    model.rmatind[model.numNz] = xCol(u, v);
                    model.rmatval[model.numNz] = 1.0;
                    model.numNz += 1;
                    
                    model.rmatind[model.numNz] = xCol(u, w);
                    model.rmatval[model.numNz] = 1.0;
                    model.numNz += 1;
                    
                    model.rmatind[model.numNz] = xCol(u, u);
                    model.rmatval[model.numNz] = -1.0;
                    model.numNz += 1;
                    
                    model.numRows += 1;
                }                
            }
        }
    }
    
    // Add constraints of type c
    for (int u = 0; u < nevents; ++u)
    {
        for (std::set<int>::iterator itV = eventsRepresentedBy[u].begin(); itV != eventsRepresentedBy[u].end(); ++itV)
        {
            int v = *itV;
            sprintf(model.rowNames[model.numRows], "c_%d_%d", u, v);
            model.rmatbeg[model.numRows] = model.numNz;
            model.sense[model.numRows] = 'E';
            model.rhs[model.numRows] = 0.0;
            
            model.rmatind[model.numNz] = xCol(u, v);
            model.rmatval[model.numNz] = -1.0;
            model.numNz += 1;
            
            for (std::set<int>::iterator itR = instance->eventRooms[v].begin(); itR != instance->eventRooms[v].end(); ++itR)
            {
                int r = *itR;
                model.rmatind[model.numNz] = yCol(u, v, r);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
            }
            model.numRows += 1;
        }
    }
    
    // Add constraints of type d
    for (int u = 0; u < nevents; ++u)
    {
        for (int r = 0; r < instance->nrooms; ++r)
        {
            sprintf(model.rowNames[model.numRows], "d_%d_%d", u, r);
            model.rmatbeg[model.numRows] = model.numNz;
            model.sense[model.numRows] = 'L';
            model.rhs[model.numRows] = 1.0;
            
            for (std::set<int>::iterator itV = instance->roomEvents[r].begin(); itV != instance->roomEvents[r].end(); ++itV)
            {
                int v = *itV;
                if (eventsRepresentedBy[u].find(v) != eventsRepresentedBy[u].end())
                {
                    // v is on the anti-adjacency of u
                    model.rmatind[model.numNz] = yCol(u, v, r);
                    model.rmatval[model.numNz] = 1.0;
                    model.numNz += 1;
                }
            }
            model.numRows += 1;
        }
    }
    
    // Add constraints of type e
    for (int u = 0; u < instance->nevents; ++u)
    {
        sprintf(model.rowNames[model.numRows], "e_%d", u);
        model.rmatbeg[model.numRows] = model.numNz;
        model.sense[model.numRows] = 'E';
        model.rhs[model.numRows] = 0.0;
        
        model.rmatind[model.numNz] = xCol(u, u);
        model.rmatval[model.numNz] = -1.0;
        model.numNz += 1;
        
        for (std::set<int>::iterator itT = instance->eventTimeslots[u].begin(); itT != instance->eventTimeslots[u].end(); ++itT)
        {
            int t = *itT;
            model.rmatind[model.numNz] = pCol(u, t);
            model.rmatval[model.numNz] = 1.0;
            model.numNz += 1;
        }
        model.numRows += 1;
    }
    
    // Add constraints of type f
    for (int t = 0; t < NUM_TIMESLOTS; ++t)
    {
        sprintf(model.rowNames[model.numRows], "f_%d", t);
        model.rmatbeg[model.numRows] = model.numNz;
        model.sense[model.numRows] = 'L';
        model.rhs[model.numRows] = 1.0;
        
        for (std::set<int>::iterator itU = instance->timeslotEvents[t].begin(); itU != instance->timeslotEvents[t].end(); ++itU)
        {
            int u = *itU;
            model.rmatind[model.numNz] = pCol(u, t);
            model.rmatval[model.numNz] = 1.0;
            model.numNz += 1;
        }
        model.numRows += 1;
    }
    
    // Add constraints of type g
    for (int v = 0; v < nevents; ++v)
    {
        for (std::set<int>::iterator itU = eventsThatRepresent[v].begin(); itU != eventsThatRepresent[v].end(); ++itU)
        {
            int u = *itU;
            for (std::set<int>::iterator itT = instance->eventTimeslots[u].begin(); itT != instance->eventTimeslots[u].end(); ++itT)
            {
                int t = *itT;
                sprintf(model.rowNames[model.numRows], "g_%d_%d_%d", u, v, t);
                model.rmatbeg[model.numRows] = model.numNz;
                model.sense[model.numRows] = 'L';
                model.rhs[model.numRows] = 1.0;
                
                model.rmatind[model.numNz] = xCol(u, v);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
                
                model.rmatind[model.numNz] = pCol(u, t);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
                
                if (instance->eventTimeslots[v].find(t) != instance->eventTimeslots[v].end())
                {
                    model.rmatind[model.numNz] = qCol(v, t);
                    model.rmatval[model.numNz] = -1.0;
                    model.numNz += 1;
                }
                
                model.numRows += 1;
            }
        }
    }
    
    // Add constraints of type h
    for (int v = 0; v < nevents; ++v)
    {
        sprintf(model.rowNames[model.numRows], "h_%d", v);
        model.rmatbeg[model.numRows] = model.numNz;
        model.sense[model.numRows] = 'E';
        model.rhs[model.numRows] = 1.0;
        
        for (std::set<int>::iterator itT = instance->eventTimeslots[v].begin(); itT != instance->eventTimeslots[v].end(); ++itT)
        {
            int t = *itT;
            model.rmatind[model.numNz] = qCol(v, t);
            model.rmatval[model.numNz] = 1.0;
            model.numNz += 1;
        }
        model.numRows += 1;
    }
    
    // Add constraints of type i
    for (int v = 0; v < nevents; ++v)
    {
        for (std::set<int>::iterator itW = instance->eventsAfter[v].begin(); itW != instance->eventsAfter[v].end(); ++itW)
        {
            int w = *itW;
            
            for (std::set<int>::iterator itT = instance->eventTimeslots[v].begin(); itT != instance->eventTimeslots[v].end(); ++itT)
            {
                int t = *itT;
                sprintf(model.rowNames[model.numRows], "i_%d_%d_%d", v, w, t);
                model.rmatbeg[model.numRows] = model.numNz;
                model.sense[model.numRows] = 'L';
                model.rhs[model.numRows] = 1.0;
                
                model.rmatind[model.numNz] = qCol(v, t);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
                
                for (std::set<int>::iterator itK = instance->eventTimeslots[w].begin(); itK != instance->eventTimeslots[w].end(); ++itK)
                {
                    int k = *itK;
                    if (k > t) continue;
                    
                    model.rmatind[model.numNz] = qCol(w, k);
                    model.rmatval[model.numNz] = 1.0;
                    model.numNz += 1;
                }
                
                model.numRows += 1;
            }
        }
    }
    
#ifdef COMPLEX_PENALTIES
    // Add j constraints
    for (int s = 0; s < instance->nstudents; ++s)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            // Skip timeslots at the end of the day
            if ((t % TIMESLOTS_PER_DAY) > (TIMESLOTS_PER_DAY - 3)) continue;
            
            sprintf(model.rowNames[model.numRows], "j_%d_%d", s, t);
            model.rmatbeg[model.numRows] = model.numNz;
            model.sense[model.numRows] = 'E';
            model.rhs[model.numRows] = 0.0;
            
            model.rmatind[model.numNz] = oCol(s, t);
            model.rmatval[model.numNz] = -1.0;
            model.numNz += 1;
            
            model.rmatind[model.numNz] = alphaCol(s, t);
            model.rmatval[model.numNz] = -1.0;
            model.numNz += 1;
            
            // for k in gamma(t)
            for (int k = t; k <= (t + 2); ++k)
            {
                std::set<int> events;
                instance->eventsForStudentTimeslot(s, k, events);
                for (std::set<int>::iterator itV = events.begin(); itV != events.end(); ++itV)
                {
                    int v = *itV;
                    model.rmatind[model.numNz] = qCol(v, k);
                    model.rmatval[model.numNz] = 1.0;
                    model.numNz += 1;
                }
            }
            model.numRows += 1;
        }
    }
    
    // Add k constraints
    for (int s = 0; s < instance->nstudents; ++s)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            sprintf(model.rowNames[model.numRows], "k_%d_%d", s, t);
            model.rmatbeg[model.numRows] = model.numNz;
            model.sense[model.numRows] = 'E';
            model.rhs[model.numRows] = 0.0;
            
            model.rmatind[model.numNz] = wCol(s, t);
            model.rmatval[model.numNz] = -1.0;
            model.numNz += 1;
            
            model.rmatind[model.numNz] = betaCol(s, t);
            model.rmatval[model.numNz] = -1.0;
            model.numNz += 1;

            std::set<int> eventsT;
            instance->eventsForStudentTimeslot(s, t, eventsT);
            for (std::set<int>::iterator itV = eventsT.begin(); itV != eventsT.end(); ++itV)
            {
                int v = *itV;
                model.rmatind[model.numNz] = qCol(v, t);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
            }
            
            // Calculate first timeslot in the same day of t
            int firstOfDay = t - (t % TIMESLOTS_PER_DAY);
            
            // for k in delta(t)
            for (int k = firstOfDay; k < (firstOfDay + TIMESLOTS_PER_DAY); ++k)
            {
                // This loop deals only with timeslots other than t
                if (k == t) continue;
                
                std::set<int> eventsK;
                instance->eventsForStudentTimeslot(s, k, eventsK);
                for (std::set<int>::iterator itV = eventsK.begin(); itV != eventsK.end(); ++itV)
                {
                    int v = *itV;
                    model.rmatind[model.numNz] = qCol(v, k);
                    model.rmatval[model.numNz] = -1.0;
                    model.numNz += 1;
                }
            }
            model.numRows += 1;
        }
    }
#endif /* COMPLEX_PENALTIES */
    
    Solver solver(model);
    solver.init();
    solver.solve();
    
    // Map variables to solution
    for (int v = 0; v < instance->nevents; ++v)
    {
        // Find allocated timeslot
        for (std::set<int>::iterator itT = instance->eventTimeslots[v].begin(); itT != instance->eventTimeslots[v].end(); ++itT)
        {
            int t = *itT;
            if (model.x[qCol(v, t)] == 1.0)
            {
                if (solution->eventTimeslots[v] != -1) 
                {
                    std::cerr << "Warning: event " << v << " allocated to more than one timeslot." << std::endl;
                }
                solution->eventTimeslots[v] = t;
            }
        }
        
        // Find allocated rooms
        for (std::set<int>::iterator itU = eventsThatRepresent[v].begin(); itU != eventsThatRepresent[v].end(); ++itU)
        {
            int u = *itU;
            for (std::set<int>::iterator itR =
                    instance->eventRooms[v].begin(); itR
                    != instance->eventRooms[v].end(); ++itR) {
                int r = *itR;
                
                if (model.x[yCol(u, v, r)] == 1.0)
                {
                    if (solution->eventRooms[v] != -1)
                    {
                        std::cerr << "Warning: Event " << v << " allocated to more than one room." << std::endl;
                    }
                    solution->eventRooms[v] = r;
                }
            }    
        }
    }
    std::clog << "timeslots = (";
    std::copy(solution->eventTimeslots.begin(), solution->eventTimeslots.end(), std::ostream_iterator<int>(std::clog, ", "));
    std::clog << ")" << std::endl;
    
    std::clog << "rooms = (";
    std::copy(solution->eventRooms.begin(), solution->eventRooms.end(), std::ostream_iterator<int>(std::clog, ", "));
    std::clog << ")" << std::endl;
    
    solver.close();
    
    model.freeArrays();
    
#ifdef COMPLEX_PENALTIES
    delete[] betaCols;
    delete[] alphaCols;
    delete[] wCols;
    delete[] oCols;
#endif /* COMPLEX_PENALTIES */
    
    delete[] qCols;
    delete[] pCols;
    delete[] yCols;
    
    delete[] xCols;
}
