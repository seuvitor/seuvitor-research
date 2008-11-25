#include "solver.h"
#include "mono.h"

#define xCol(e, t, r) xCols[(e * NUM_TIMESLOTS * MAX_NUM_ROOMS) + (MAX_NUM_ROOMS * t) + r]

#ifdef COMPLEX_PENALTIES
#define oCol(s, t) oCols[(s * NUM_TIMESLOTS) + t]
#define wCol(s, t) wCols[(s * NUM_TIMESLOTS) + t]
#define alphaCol(s, t) alphaCols[(s * NUM_TIMESLOTS) + t]
#define betaCol(s, t) betaCols[(s * NUM_TIMESLOTS) + t]
#endif /* COMPLEX_PENALTIES */

void mono_constructSolution(Instance* instance, Solution* solution)
{
    int* xCols = new int[MAX_NUM_EVENTS * NUM_TIMESLOTS * MAX_NUM_ROOMS];
    
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
    
    // Add x_{etr} variables
    for (int e = 0; e < nevents; ++e)
    {
        for (std::set<int>::iterator itT = instance->eventTimeslots[e].begin(); itT != instance->eventTimeslots[e].end(); ++itT)
        {
            for (std::set<int>::iterator itR = instance->eventRooms[e].begin(); itR != instance->eventRooms[e].end(); ++itR)
            {
                int t = *itT;
                int r = *itR;
            
                xCol(e, t, r) = model.numCols;
                sprintf(model.colNames[model.numCols], "x_%d_%d_%d", e, t, r);
                model.lb[model.numCols] = 0.0;
                model.ub[model.numCols] = 1.0;
                model.obj[model.numCols] = 0.0; // Costs will be set later
                model.numCols += 1;
            }
        }
    }
    
    // Set costs of x_{etr} variables
    for (int t = (TIMESLOTS_PER_DAY - 1); t < NUM_TIMESLOTS; t += TIMESLOTS_PER_DAY)
    {
        for (std::set<int>::iterator itV = instance->timeslotEvents[t].begin(); itV != instance->timeslotEvents[t].end(); ++itV)
        {
            int v = *itV;
            for (std::set<int>::iterator itR = instance->eventRooms[v].begin(); itR != instance->eventRooms[v].end(); ++itR)
            {
                int r = *itR;
                model.obj[xCol(v, t, r)] = instance->eventAttendance[v];
            }
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
    for (int e = 0; e < nevents; ++e)
    {
        sprintf(model.rowNames[model.numRows], "a_%d", e);
        model.rmatbeg[model.numRows] = model.numNz;
        model.sense[model.numRows] = 'G';
        model.rhs[model.numRows] = 1.0;

        for (std::set<int>::iterator itT = instance->eventTimeslots[e].begin(); itT != instance->eventTimeslots[e].end(); ++itT)
        {
            int t = *itT;
            for (std::set<int>::iterator itR = instance->eventRooms[e].begin(); itR != instance->eventRooms[e].end(); ++itR)
            {
                int r = *itR;
                
                model.rmatind[model.numNz] = xCol(e, t, r);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
            }
        }
        model.numRows += 1;
    }
    
    // Add constraints of type b
    for (int s = 0; s < instance->nstudents; ++s)
    {
        for (int t = 0; t < NUM_TIMESLOTS; ++t)
        {
            sprintf(model.rowNames[model.numRows], "b_%d_%d", s, t);
            model.rmatbeg[model.numRows] = model.numNz;
            model.sense[model.numRows] = 'L';
            model.rhs[model.numRows] = 1.0;
            
            for (int r = 0; r < instance->nrooms; ++r)
            {
                std::set<int> events;
                instance->eventsForStudentTimeslotRoom(s, t, r, events);
                for (std::set<int>::iterator itE = events.begin(); itE != events.end(); ++itE)
                {
                    int e = *itE;
                    model.rmatind[model.numNz] = xCol(e, t, r);
                    model.rmatval[model.numNz] = 1.0;
                    model.numNz += 1;
                }
            }
            model.numRows += 1;
        }
    }
    
    // Add constraints of type c
    for (int t = 0; t < NUM_TIMESLOTS; ++t)
    {
        for (int r = 0; r < instance->nrooms; ++r)
        {
            sprintf(model.rowNames[model.numRows], "c_%d_%d", t, r);
            model.rmatbeg[model.numRows] = model.numNz;
            model.sense[model.numRows] = 'L';
            model.rhs[model.numRows] = 1.0;
            
            std::set<int> events;
            instance->eventsForTimeslotRoom(t, r, events);
            for (std::set<int>::iterator itE = events.begin(); itE != events.end(); ++itE)
            {
                int e = *itE;
                model.rmatind[model.numNz] = xCol(e, t, r);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
            }
            model.numRows += 1;
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
                    for (std::set<int>::iterator itR = instance->eventRooms[v].begin(); itR != instance->eventRooms[v].end(); ++itR)
                    {
                        int r = *itR;
                        model.rmatind[model.numNz] = xCol(v, k, r);
                        model.rmatval[model.numNz] = 1.0;
                        model.numNz += 1;
                    }
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
                for (std::set<int>::iterator itR = instance->eventRooms[v].begin(); itR != instance->eventRooms[v].end(); ++itR)
                {
                    int r = *itR;
                    model.rmatind[model.numNz] = xCol(v, t, r);
                    model.rmatval[model.numNz] = 1.0;
                    model.numNz += 1;
                }
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
                    for (std::set<int>::iterator itR = instance->eventRooms[v].begin(); itR != instance->eventRooms[v].end(); ++itR)
                    {
                        int r = *itR;
                        model.rmatind[model.numNz] = xCol(v, k, r);
                        model.rmatval[model.numNz] = -1.0;
                        model.numNz += 1;
                    }
                }
            }
            model.numRows += 1;
        }
    }
#endif /* COMPLEX_PENALTIES */
    
    Solver solver(model);
    solver.init();
    solver.solve();
    solver.close();
    
    model.freeArrays();
    
#ifdef COMPLEX_PENALTIES
    delete[] betaCols;
    delete[] alphaCols;
    delete[] wCols;
    delete[] oCols;
#endif /* COMPLEX_PENALTIES */
    
    delete[] xCols;
}
