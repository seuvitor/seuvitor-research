#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>

#include "coloring.h"
#include "dsatur.h"
#include "rlf.h"

int main() {
    
    std::vector<std::string> filePaths;
    filePaths.push_back("./instances/le450_5a.col");
    filePaths.push_back("./instances/le450_5b.col");
    filePaths.push_back("./instances/le450_5c.col");
    filePaths.push_back("./instances/le450_5d.col");
    filePaths.push_back("./instances/le450_15a.col");
    filePaths.push_back("./instances/le450_15b.col");
    filePaths.push_back("./instances/le450_15c.col");
    filePaths.push_back("./instances/le450_15d.col");
    filePaths.push_back("./instances/le450_25a.col");
    filePaths.push_back("./instances/le450_25b.col");
    filePaths.push_back("./instances/le450_25c.col");
    filePaths.push_back("./instances/le450_25d.col");
    filePaths.push_back("./instances/latin_square_10.col");
    filePaths.push_back("./instances/fpsol2.i.1.col");
    filePaths.push_back("./instances/fpsol2.i.2.col");
    filePaths.push_back("./instances/fpsol2.i.3.col");
    filePaths.push_back("./instances/inithx.i.1.col");
    filePaths.push_back("./instances/inithx.i.2.col");
    filePaths.push_back("./instances/inithx.i.3.col");
    filePaths.push_back("./instances/mulsol.i.1.col");
    filePaths.push_back("./instances/mulsol.i.2.col");
    filePaths.push_back("./instances/mulsol.i.3.col");
    filePaths.push_back("./instances/mulsol.i.4.col");
    filePaths.push_back("./instances/mulsol.i.5.col");
    filePaths.push_back("./instances/school1.col");
    filePaths.push_back("./instances/school1_nsh.col");
    filePaths.push_back("./instances/zeroin.i.1.col");
    filePaths.push_back("./instances/zeroin.i.2.col");
    filePaths.push_back("./instances/zeroin.i.3.col");
	
    time_t start;
    start = time(NULL);

    for (std::vector<std::string>::iterator it = filePaths.begin(); it != filePaths.end(); ++it)
    {
        std::string& filePath = *it;
        std::ifstream in;
        in.open(filePath.c_str(), std::ifstream::in);
        
        Instance* instance = new Instance(in);
        Solution* solution = new Solution(instance);
        
        rlf_constructSolution(instance, solution);

        if (solution->isFeasible())
        {
            std::cout << filePath << " K = " << solution->k() << std::endl;
        }
        else
        {
            std::cout << filePath << " Feasible solution not found" << std::endl;
        }
        
        in.close();
		
        delete solution;
        delete instance;
    }
    
    std::cout << "Total time: " << time(NULL) - start << "s" << std::endl;
    
	return 0;
}
