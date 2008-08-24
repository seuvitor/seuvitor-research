#ifndef COLORING_H_
#define COLORING_H_

#include <iostream>

struct Instance
{
    int nvertices;
    int nedges;
    int** gamma;
    
    Instance(std::istream& in);
    ~Instance();
    
    void print(std::ostream& out);
};

struct Solution
{
    Instance* instance;
    int maxColor;
    int* coloring;
    
	Solution(Instance* instance);
	Solution(const Solution& solution);
	~Solution();

    int numViolations();
    int k();
    
    void print(std::ostream& out);
    
    Solution& operator=(const Solution& solution);
};

#endif /*COLORING_H_*/
