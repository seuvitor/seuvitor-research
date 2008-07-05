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
	~Solution();

    bool isFeasible();
    
    void print(std::ostream& out);
};

#endif /*COLORING_H_*/
