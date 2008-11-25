#include <iostream>
#include <fstream>
#include <stdexcept>
#include <ctime>

#include "timetabling.h"
#include "bb.h"
#include "rep.h"
#include "mono.h"
#include "colrep.h"
#include "colmono.h"

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

    std::clog << "instance = " << fileName.substr(0, fileName.size() - 4) << std::endl;

    try {
        if (algorithmName.compare("bb") == 0)
        {
    	    bb_constructSolution(&instance, &solution);
        }
        else if (algorithmName.compare("rep") == 0)
        {
            rep_constructSolution(&instance, &solution);
        }
        else if (algorithmName.compare("mono") == 0)
        {
            mono_constructSolution(&instance, &solution);
        }
        else if (algorithmName.compare("colrep") == 0)
        {
            colrep_constructSolution(&instance, &solution);
        }
        else if (algorithmName.compare("colmono") == 0)
        {
            colmono_constructSolution(&instance, &solution);
        }
    }
    catch (std::logic_error e)
    {
        std::clog << "Error: " << e.what() << std::endl;
    }
    
    for (int e = 0; e < instance.nevents; ++e)
	{
        std::cout << solution.eventTimeslots[e] << " "
                << solution.eventRooms[e] << std::endl;
    }
    
    time_t elapsedTime = time(NULL) - start;
    std::clog << "elapsedTime " << elapsedTime << std::endl;
    
    if (solution.isValid())
    {
        std::clog << fileName.substr(0, fileName.size() - 4) << "\t"
        		<< solution.distanceToFeasibility() << "\t"
        		<< solution.softCost() << "\t"
        		<< elapsedTime << std::endl;
    }
    else
    {
        std::clog << fileName.substr(0, fileName.size() - 4)
        		<< " Valid solution not found. " << std::endl;
    }
    
	return 0;
}
