#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>

#include "coloring.h"
#include "dsatur.h"
#include "rlf.h"
#include "ts.h"

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

    if (algorithmName.compare("dsatur") == 0)
    {
	    dsatur_constructSolution(instance, solution);
    }
    else if (algorithmName.compare("rlf") == 0)
    {
	    rlf_constructSolution(instance, solution);
    }
    else if (algorithmName.compare("ts") == 0)
    {
	    ts_constructSolution(instance, solution);
    }
    
    time_t elapsedTime = time(NULL) - start;
    
    int numViolations = solution->numViolations();
    if (numViolations == 0)
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
        		<< " Feasible solution not found. "
                << "(" << numViolations << " violations)" << std::endl;
    }
    
    in.close();
	
    delete solution;
    delete instance;
    
	return 0;
}
