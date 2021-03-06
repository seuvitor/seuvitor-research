#include <vector>
#include <set>
#include <string>
#include <sstream>

#include "coloring.h"

Instance::Instance(std::istream& in)
{
    // Skip header
    std::string tmp("");
    while (tmp[0] != 'p') std::getline(in, tmp);
  
    // Read instance params
    std::stringstream ss(tmp);
    ss >> tmp;
    ss >> tmp;
    ss >> nvertices;
    ss >> nedges;
    ss.clear();
    
    std::vector<std::vector<int> > adjacency = std::vector<std::vector<int> >(nvertices, std::vector<int>());
    
    int e1, e2;
    
    // Read lines until the end of file
    while (!in.eof())
    {
    	std::getline(in, tmp);
    	if (tmp[0] == 'c') continue; // Skip comment lines
    	
    	// Parse edge info
        ss.str(tmp);
        ss >> tmp;
        ss >> e1;
        ss >> e2;
        e1 -= 1;
        e2 -= 1;
        adjacency[e1].push_back(e2);
        adjacency[e2].push_back(e1);
        ss.clear();
    }

    // Convert vector of int vectors to array of int arrays
    gamma = new int*[nvertices];
    for (int i = 0; i < nvertices; ++i)
    {
        int numNeighbors = adjacency[i].size();
        gamma[i] = new int[numNeighbors + 1];
        gamma[i][0] = numNeighbors;
        std::copy(adjacency[i].begin(), adjacency[i].end(), gamma[i] + 1);
    }
}

Instance::~Instance()
{
    for (int i = 0; i < nvertices; ++i)
    {
        if (gamma[i])
        {
            delete[] gamma[i];
        }
    }
    if (gamma)
    {
        delete[] gamma;
    }
}

void Instance::print(std::ostream& out)
{
    out << "Num vertices: " << nvertices << std::endl;
    out << "Num edges: " << nedges << std::endl;
    
    out << "Adjacency lists:" << std::endl;
    for (int i = 0; i < nvertices; ++i)
    {
    	out << i << ": ";
        
        // Gets adjacency of vertex vertexId
        int* adj = gamma[i];
        
        // Iteration starts on index 1 and ends adj[0] indices after the start
        for (int *it = (adj + 1), *end = (it + adj[0]); it != end; ++it)
    	{
    		out << *it << ", ";
    	}
    	out << std::endl;
    }
}

Solution::Solution(Instance* instance) :
	instance(instance)
{
	maxColor = 0;
    coloring = new int[instance->nvertices];
    for (int i = 0; i < instance->nvertices; ++i) coloring[i] = -1;
}

Solution::Solution(const Solution& solution)
{
    (*this) = solution;
}

Solution::~Solution()
{
    if (coloring)
    {
        delete[] coloring;
    }
}

int Solution::k()
{
	std::set<int> uniqueColors(coloring, (coloring + instance->nvertices));
	return uniqueColors.size();
}

int Solution::numViolations()
{
    int violationCount = 0;
	
	for (int u = 0; u < instance->nvertices; ++u)
	{
		// Check whether all vertices are colored
		int uColor = coloring[u];
		if (uColor == -1)
		{
			violationCount++;
		}
		
        // Gets adjacency of vertex u
        int* adj = instance->gamma[u];
        
        // Iteration starts on index 1 and ends adj[0] indices after the start
        for (int *it = (adj + 1), *end = (it + adj[0]); it != end; ++it)
		{
            // Check whether there are conflicts between adjacent vertices
			int v = *it;
            if (u < v)
            {
                int vColor = coloring[v];
                if (uColor == vColor)
                {
                    violationCount++;
                }
            }
		}
	}

	return violationCount;
}

void Solution::print(std::ostream& out)
{
    out << "coloring: ";
    for (int i = 0; i < instance->nvertices; ++i)
    {
    	out << coloring[i] << ", ";
    }
    out << std::endl;
}

Solution& Solution::operator=(const Solution& solution)
{
    instance = solution.instance;
    maxColor = solution.maxColor;
    coloring = new int[instance->nvertices];
    for (int i = 0; i < instance->nvertices; ++i)
    {
        coloring[i] = solution.coloring[i];
    }
    return (*this);
}
