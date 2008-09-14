#include <iostream>
#include <fstream>
#include <ctime>

#include "timetabling.h"
#include "bb.h"

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
    Instance instance(in);
    in.close();
    
    Solution solution(&instance);

    time_t start;
    start = time(NULL);

    if (algorithmName.compare("bb") == 0)
    {
	    bb_constructSolution(&instance, &solution);
    }
    
    time_t elapsedTime = time(NULL) - start;
    
    if (solution.isValid())
    {
        std::cout << fileName.substr(0, fileName.size() - 4) << "\t"
        		<< solution.distanceToFeasibility() << "\t"
        		<< solution.softCost() << "\t"
        		<< elapsedTime << std::endl;
    }
    else
    {
        std::cout << fileName.substr(0, fileName.size() - 4)
        		<< " Valid solution not found. " << std::endl;
    }
    
	return 0;
}
