#include "rlf.h"
#include <set>

/**
 * Returns the vertex with most links to the frontier, i.e., to vertices that
 * are uncolored, but are linked to a colored vertex.
 */
int mostLinkedToFrontier(std::set<int>& uncoloredUnlinked, int* numLinksToFrontier)
{
    int vertex, links, maxLinks, maxVertex;
    
    maxVertex = -1;
    maxLinks = -1;
    for (std::set<int>::iterator it = uncoloredUnlinked.begin(); it != uncoloredUnlinked.end(); ++it)
    {
    	vertex = *it;
    	links = numLinksToFrontier[vertex];
    	if (links > maxLinks)
    	{
            maxLinks = links;
            maxVertex = vertex;
    	}
    }
    
    return maxVertex;
}

/**
 * Move vertex to the frontier and update its adjacencies accordingly.
 */
void moveVertexToFrontier(Instance* instance, int vertexId, std::set<int>& uncoloredUnlinked, int* numLinksToFrontier)
{
	// Remove from unlinked, since all vertices on the frontier are supposed
	// to be linked to a colored vertex
	uncoloredUnlinked.erase(vertexId);
	
    // Gets adjacency of vertex vertexId
    int* adj = instance->gamma[vertexId];
    int sizeAdj = adj[0]; // Index zero contains size
    
    // Iteration starts on index 1 and ends sizeAdj indices after the start
	for (int *it = (adj + 1), *end = (it + sizeAdj); it != end; ++it)
	{
		int adjVertexId = *it;
		++numLinksToFrontier[adjVertexId];
	}
}

/**
 * Update adjacencies of a vertex after setting its colors.
 */
void updateAdjacencies(Instance* instance, Solution* solution, int vertexId, std::set<int>& uncolored, std::set<int>& uncoloredUnlinked, int* numLinksToFrontier)
{
	uncolored.erase(vertexId);
	uncoloredUnlinked.erase(vertexId);
	
    // Gets adjacency of vertex vertexId
    int* adj = instance->gamma[vertexId];
    int sizeAdj = adj[0]; // Index zero contains size
    
    // Iteration starts on index 1 and ends sizeAdj indices after the start
	for (int *it = (adj + 1), *end = (it + sizeAdj); it != end; ++it)
	{
		int adjVertexId = *it;
		
		// Only update if the adjacent vertex is uncolored
		if (solution->coloring[adjVertexId] != -1) continue;

		moveVertexToFrontier(instance, adjVertexId, uncoloredUnlinked, numLinksToFrontier);
	}
}

void rlf_constructSolution(Instance* instance, Solution* solution)
{
	int numVertices = instance->nvertices;

	// Initialize data structures

    // Set with uncolored vertices (initially, all vertices are uncolored)
    std::set<int> uncolored;
    for (int i = 0; i < numVertices; ++i) uncolored.insert(i);

    // Set with uncolored vertices that are unlinked, i.e. not neighbors of a
    // colored vertex (initially, all vertices are uncolored and unlinked)
    std::set<int> uncoloredUnlinked;

    // For all uncolored unlinked vertices, we keep the number of links that
    // it has to the frontier, i.e., to vertices that are uncolored, but
    // linked to a colored vertex
    int* numLinksToFrontier = new int[numVertices];
    
    int currentColor = 0;
    while (uncolored.size() > 0)
	{
		uncoloredUnlinked.clear();
		for (std::set<int>::iterator it = uncolored.begin(); it != uncolored.end(); ++it)
		{
			int v = *it;
			uncoloredUnlinked.insert(v);
			numLinksToFrontier[v] = 0;
		}
		
		while (uncoloredUnlinked.size() > 0)
		{
			int vertexId = mostLinkedToFrontier(uncoloredUnlinked, numLinksToFrontier);
			
			solution->coloring[vertexId] = currentColor;
			updateAdjacencies(instance, solution, vertexId, uncolored, uncoloredUnlinked, numLinksToFrontier);
		}
		
		// When all uncolored vertices are linked to a vertex in the current
		// color class, a new color is needed in order to avoid conflicts.
		++currentColor;
	}
    
    if (numLinksToFrontier) delete[] numLinksToFrontier;
}
