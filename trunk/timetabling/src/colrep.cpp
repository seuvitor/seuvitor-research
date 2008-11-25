#include <vector>
#include <iterator>
#include <algorithm>
#include "solver.h"
#include "graph.h"
#include "colrep.h"

#define xCol(u, v) xCols[(u * MAX_NUM_EVENTS) + v]


void colrep_constructSolution(Instance* instance, Solution* solution)
{
    int* xCols = new int[MAX_NUM_EVENTS * MAX_NUM_EVENTS];
    
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
            if (u == v)
            {
                model.obj[model.numCols] = 1.0;
            }
            else
            {
                model.obj[model.numCols] = 0.0;
            }
            model.numCols += 1;
        }
    }
    
    // Add constraints of type a
    for (int v = 0; v < nevents; ++v)
    {
        sprintf(model.rowNames[model.numRows], "a_%d", v);
        model.rmatbeg[model.numRows] = model.numNz;
        model.sense[model.numRows] = 'E';
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
    
    Solver solver(model);
    solver.init();
    solver.solve();
    solver.close();
    
    model.freeArrays();
    
    delete[] xCols;
}
