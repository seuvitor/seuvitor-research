#ifndef SOLVER_H_
#define SOLVER_H_
#include <ilcplex/cplex.h>

#define MAX_NUM_COLS 130000
#define MAX_NUM_ROWS 450000
#define MAX_NUM_NZ 7000000

class ModelData
{
public:
    int numCols;
    int numRows;
    int numNz;
    
    char** colNames;
    double* obj;
    double* lb;
    double* ub;
    char* xctype;
    double* x;
    
    char** rowNames;
    double* rhs;
    char* sense;
    int* rmatbeg;

    int* rmatind;
    double* rmatval;
    
    ModelData()
    {
    }
    
    ~ModelData()
    {
    }
    
    void initArrays()
    {
        colNames = new char*[1 + MAX_NUM_COLS];
        for (int i = 0; i < 1 + MAX_NUM_COLS; ++i) colNames[i] = new char[20];
        obj = new double[1 + MAX_NUM_COLS];
        lb = new double[1 + MAX_NUM_COLS];
        ub = new double[1 + MAX_NUM_COLS];
        xctype = new char[1 + MAX_NUM_COLS];
        x = new double[1 + MAX_NUM_COLS];
        
        rowNames = new char*[1 + MAX_NUM_ROWS];
        for (int i = 0; i < 1 + MAX_NUM_ROWS; ++i) rowNames[i] = new char[20];
        rhs = new double[1 + MAX_NUM_ROWS];
        sense = new char[1 + MAX_NUM_ROWS];
        rmatbeg = new int[1 + MAX_NUM_ROWS];
        
        rmatind = new int[1 + MAX_NUM_NZ];
        rmatval = new double[1 + MAX_NUM_NZ];
    }
    
    void freeArrays()
    {
        delete[] rmatval;
        delete[] rmatind;
        
        delete[] rmatbeg;
        delete[] sense;
        delete[] rhs;
        for (int i = 0; i < 1 + MAX_NUM_ROWS; ++i) delete[] rowNames[i];
        delete[] rowNames;
        
        delete[] x;
        delete[] xctype;
        delete[] ub;
        delete[] lb;
        delete[] obj;
        for (int i = 0; i < 1 + MAX_NUM_COLS; ++i) delete[] colNames[i];
        delete[] colNames;
    }
};

class Solver
{
public:
    int status;
    CPXENVptr env;
    CPXLPptr lp;
    ModelData& model;
    
    Solver(ModelData& model): status(0), env(NULL), lp(NULL), model(model){} 
    
    void init();
    void solve();
    void close();
};

#endif /*SOLVER_H_*/
