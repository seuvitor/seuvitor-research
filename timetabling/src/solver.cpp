#include <iostream>
#include <stdexcept>
#include <windows.h>
#include "solver.h"

void throwError(const char* msg, int status)
{
    char msgBuffer[1024];
    sprintf(msgBuffer, "%s (status %d)", msg, status);
    throw std::logic_error(msgBuffer);
}

static int CPXPUBLIC
logCallback (CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle)
{
    int status = 0;
    
    int        hasIncumbent = 0;
    int        nodeCount;
    int        nodesLeft;
    int        iterations;
    double     objval;
    double     bound;
    
    status = CPXgetcallbackinfo (env, cbdata, wherefrom,
                                 CPX_CALLBACK_INFO_NODE_COUNT, &nodeCount);
    status = CPXgetcallbackinfo (env, cbdata, wherefrom,
                                 CPX_CALLBACK_INFO_NODES_LEFT, &nodesLeft);
    status = CPXgetcallbackinfo (env, cbdata, wherefrom,
                                 CPX_CALLBACK_INFO_MIP_ITERATIONS, &iterations);
    status = CPXgetcallbackinfo (env, cbdata, wherefrom,
                                 CPX_CALLBACK_INFO_MIP_FEAS, &hasIncumbent);
    status = CPXgetcallbackinfo (env, cbdata, wherefrom,
                                 CPX_CALLBACK_INFO_BEST_REMAINING, &bound);
    
    unsigned long long startTime = *(unsigned long long*)cbhandle;
    unsigned long long elapsedTime = GetTickCount() - startTime;
    
    std::clog << "iterations = " << iterations << ", time ms = " << elapsedTime
            << ", bound = " << bound;
    if (hasIncumbent)
    {
        status = CPXgetcallbackinfo (env, cbdata, wherefrom,
                                     CPX_CALLBACK_INFO_BEST_INTEGER, &objval);
        std::clog << ", incumbent = " << objval;
    }
    std::clog << std::endl;
    
//    if (elapsedTime > 7200000)
//    {
//        // End optimization.
//        status = 1;
//    }
    
    return (status);
}

void Solver::init()
{
    std::clog << "numCols = " << model.numCols << std::endl;
    std::clog << "numRows = " << model.numRows << std::endl;
    std::clog << "numNz = " << model.numNz << std::endl;
    
    env = CPXopenCPLEX(&status);
    
    if (env == NULL) throwError("Could not open CPLEX environment.", status);
    
    lp = CPXcreateprob(env, &status, "timetabling");
    
    if (lp == NULL) throwError("Failed to create LP.", status);
    
    CPXchgobjsen(env, lp, CPX_MIN);
    double* obj = model.obj;
    char* xctype = model.xctype;
    
#ifdef LINEAR_RELAXATION
    xctype = NULL;
#endif
#ifdef IGNORE_COSTS
    obj = NULL;
#endif

    status = CPXnewcols(env, lp, model.numCols,
                        obj, model.lb, model.ub, xctype, model.colNames);
    
    if (status) throwError("Failed to populate columns", status);
    
    status = CPXaddrows(env, lp, 0, model.numRows, model.numNz,
                        model.rhs, model.sense, model.rmatbeg,
                        model.rmatind, model.rmatval, NULL, model.rowNames);
    
    if (status) throwError("Failed to populate rows", status);
    
    status = CPXwriteprob (env, lp, "timetabling.lp", NULL);
    
    if (status) throwError("Failed to write LP file", status);
    
}

void Solver::solve()
{
    double objval;
    int lpstat;

    unsigned long long startTime = GetTickCount();
    
    status = CPXsetinfocallbackfunc(env, logCallback, &startTime);
    
    if (status) throwError("Failed to set callback function", status);
    
#ifdef LINEAR_RELAXATION
    status = CPXlpopt(env, lp);
#else
    status = CPXmipopt(env, lp);
#endif
    
    if (status) throwError("Failed to optimize LP", status);
    
    unsigned long long solverTime = GetTickCount() - startTime;
    std::clog << "solverTime = " << (solverTime / 1000.0) << std::endl;
    
    status = CPXsetlpcallbackfunc (env, NULL, NULL);
    
    if (status) throwError("Failed to turn off callback function", status);
    
    status = CPXsolution(env, lp, &lpstat, &objval, model.x, NULL, NULL, NULL);
    
    if (status) throwError("Failed to obtain solution", status);
    
    CPXgetobjval(env, lp, &objval);
    std::clog << "objval = " << objval << std::endl;
    
    lpstat = CPXgetstat(env, lp);
    
#ifndef LINEAR_RELAXATION
    if (lpstat != CPXMIP_OPTIMAL)
    {
        double bound;
        status = CPXgetbestobjval(env, lp, &bound);
        if (status) throwError("Failed to obtain bound", status);
        
        std::clog << "bound = " << bound << "\t";
    }
#endif
}

void Solver::close()
{
    if (lp != NULL)
    {
        status = CPXfreeprob(env, &lp);
        if (status) throwError("CPXfreeprob failed", status);
    }
    if (env != NULL)
    {
        status = CPXcloseCPLEX(&env);
        if (status) throwError("Could not close CPLEX environment", status);
    }
}
