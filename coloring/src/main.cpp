#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>

#include "coloring.h"
#include "dsatur.h"
#include "rlf.h"

int main(int argc, char** argv) {

	// Check if algorithm specification and input file were given
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " ALGORITHM_NAME INSTANCE_FILE" << std::endl;
		return 1;
	}
	std::string algorithmName(argv[1]);
	std::string fileName(argv[2]);

    std::ifstream in;
    in.open(fileName.c_str(), std::ifstream::in);
    
    Instance* instance = new Instance(in);
    Solution* solution = new Solution(instance);

    time_t start;
    start = time(NULL);

    if (algorithmName.compare("dsatur"))
    {
	    dsatur_constructSolution(instance, solution);
    }
    else if (algorithmName.compare("rlf"))
    {
	    rlf_constructSolution(instance, solution);
    }
    
    time_t elapsedTime = time(NULL) - start;

    if (solution->isFeasible())
    {
        std::cout << fileName.substr(0, fileName.size() - 4) << "\t"
        		<< instance->nvertices << "\t"
        		<< instance->nedges << "\t"
        		<< solution->k() << "\t"
        		<< elapsedTime << std::endl;
    }
    else
    {
        std::cout << fileName.substr(0, fileName.size() - 4)
        		<< " Feasible solution not found" << std::endl;
    }
    
    in.close();
	
    delete solution;
    delete instance;
    
	return 0;
}
