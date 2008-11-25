#include "solver.h"
#include "colmono.h"

#define xCol(e, t) xCols[(e * NUM_TIMESLOTS) + t]
#define wCol(t) wCols[t]

void colmono_constructSolution(Instance* instance, Solution* solution)
{
    int* xCols = new int[MAX_NUM_EVENTS * NUM_TIMESLOTS];
    int* wCols = new int[NUM_TIMESLOTS];
    
    ModelData model;
    model.initArrays();
    model.numCols = 0;
    model.numRows = 0;
    model.numNz = 0;
    
    int nevents = instance->nevents;
    
    // Add x_{et} variables
    for (int e = 0; e < nevents; ++e)
    {
        for (std::set<int>::iterator itT = instance->eventTimeslots[e].begin(); itT != instance->eventTimeslots[e].end(); ++itT)
        {
            int t = *itT;
        
            xCol(e, t) = model.numCols;
            sprintf(model.colNames[model.numCols], "x_%d_%d", e, t);
            model.lb[model.numCols] = 0.0;
            model.ub[model.numCols] = 1.0;
            model.xctype[model.numCols] = 'B';
            model.numCols += 1;
        }
    }
    
    // Add w_{t} variables
    for (int t = 0; t < NUM_TIMESLOTS; ++t)
    {
        wCol(t) = model.numCols;
        sprintf(model.colNames[model.numCols], "w_%d", t);
        model.lb[model.numCols] = 0.0;
        model.ub[model.numCols] = 1.0;
        model.xctype[model.numCols] = 'B';
        model.obj[model.numCols] = 1.0;
        model.numCols += 1;
    }
    
    // Add constraints of type a
    for (int e = 0; e < nevents; ++e)
    {
        sprintf(model.rowNames[model.numRows], "a_%d", e);
        model.rmatbeg[model.numRows] = model.numNz;
        model.sense[model.numRows] = 'E';
        model.rhs[model.numRows] = 1.0;
        
        for (std::set<int>::iterator itT = instance->eventTimeslots[e].begin(); itT != instance->eventTimeslots[e].end(); ++itT)
        {
            int t = *itT;
            model.rmatind[model.numNz] = xCol(e, t);
            model.rmatval[model.numNz] = 1.0;
            model.numNz += 1;
        }
        model.numRows += 1;
    }
    
    // Add constraints of type b
    for (int t = 0; t < NUM_TIMESLOTS; ++t)
    {
        for (std::set<int>::iterator itU = instance->timeslotEvents[t].begin(); itU != instance->timeslotEvents[t].end(); ++itU)
        {
            int u = *itU;
            for (std::set<int>::iterator itV = instance->timeslotEvents[t].begin(); itV != instance->timeslotEvents[t].end(); ++itV)
            {
                int v = *itV;
                if (u < v) continue; // Avoid repeated constraints
                if (instance->gamma[u].find(v) == instance->gamma[u].end()) continue;
                
                // (u, v) is an edge on the conflict graph
                
                // x_{ut} + x_{vt} <= 1
                sprintf(model.rowNames[model.numRows], "b_%d_%d_%d", t, u, v);
                model.rmatbeg[model.numRows] = model.numNz;
                model.sense[model.numRows] = 'L';
                model.rhs[model.numRows] = 1.0;
                
                model.rmatind[model.numNz] = xCol(u, t);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
                
                model.rmatind[model.numNz] = xCol(v, t);
                model.rmatval[model.numNz] = 1.0;
                model.numNz += 1;
                
                model.numRows += 1;
            }
        }
    }
    
    // Add constraints of type c
    for (int t = 0; t < NUM_TIMESLOTS; ++t)
    {
        for (std::set<int>::iterator itE = instance->timeslotEvents[t].begin(); itE != instance->timeslotEvents[t].end(); ++itE)
        {
            int e = *itE;
            sprintf(model.rowNames[model.numRows], "c_%d_%d", t, e);
            model.rmatbeg[model.numRows] = model.numNz;
            model.sense[model.numRows] = 'L';
            model.rhs[model.numRows] = 0.0;
            
            model.rmatind[model.numNz] = xCol(e, t);
            model.rmatval[model.numNz] = 1.0;
            model.numNz += 1;
            
            model.rmatind[model.numNz] = wCol(t);
            model.rmatval[model.numNz] = -1.0;
            model.numNz += 1;
            
            model.numRows += 1;
        }
    }
    
    Solver solver(model);
    solver.init();
    solver.solve();
    solver.close();
    
    model.freeArrays();
    
    delete[] xCols;
}
