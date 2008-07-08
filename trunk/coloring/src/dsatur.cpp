#include "dsatur.h"

/**
 * Returns the maximum saturated vertex. Saturation is given by the number of
 * different colors in the adjacency of a vertex. In case of ties, choose a
 * vertex maximally saturated with more uncolored neighbours.
 * As a side effect, swaps the positions of the returned vertex and the last
 * vertex in the uncolored list.
 */
int maximumSaturatedVertex(int* uncolored, int uncoloredSize,
		int* numAdjColors, int* numAdjUncolored)
{
    int sat, maxSat, maxVertex, maxVertexIdx, maxSatNumColoredNeighbors;
    
    maxVertexIdx = 0;
    maxVertex = uncolored[0];
    maxSat = numAdjColors[maxVertex];
    for (int i = 1; i < uncoloredSize; ++i)
    {
        sat = numAdjColors[uncolored[i]];
        if (sat > maxSat)
        {
            maxSat = sat;
            maxVertexIdx = i;
            maxVertex = uncolored[maxVertexIdx];
        }
        else if (sat == maxSat)
        {
        	if (numAdjUncolored[uncolored[i]] > numAdjUncolored[maxVertex])
        	{
	            maxSat = sat;
	            maxVertexIdx = i;
	            maxVertex = uncolored[maxVertexIdx];
        	}
        }
    }
    uncolored[maxVertexIdx] = uncolored[uncoloredSize - 1];
    
    return maxVertex;
}

/**
 * Finds the minimum color for coloring vertex without creating any conflicts.
 */
int minFeasibleColor(int v, char** adjColors)
{
	char* colors = adjColors[v];
	int minFeasibleColor = -1;
	bool foundMinFeasibleColor = false;
	for (int color = 0; !foundMinFeasibleColor; ++color)
	{
		if (colors[color] == 0)
		{
			minFeasibleColor = color;
			foundMinFeasibleColor = true;
		}
	}
	return minFeasibleColor;
}

/**
 * Update adjacencies of vertex after setting its color.
 */
void updateAdjacencies(Instance* instance, Solution* solution, int vertexId,
		int color, char** adjColors, int* numAdjColors, int* numAdjUncolored)
{
    // Gets adjacency of vertex vertexId
    int* adj = instance->gamma[vertexId];
    
    // Iteration starts on index 1 and ends adj[0] indices after the start
	for (int *it = (adj + 1), *end = (it + adj[0]); it != end; ++it)
	{
		int adjVertexId = *it;
		
		// Only update if the adjacent vertex is uncolored
		if (solution->coloring[adjVertexId] == -1)
		{
			char* colors = adjColors[adjVertexId];
			if (colors[color] == 0)
			{
				colors[color] = 1;
				++numAdjColors[adjVertexId];
			}
			--numAdjUncolored[adjVertexId];
		}
	}
}

/**
 * Builds a solution using the DSATUR constructive heuristic.
 */
void dsatur_constructSolution(Instance* instance, Solution* solution)
{
	int numVertices = instance->nvertices;

	// Initialize data structures

    // Array with uncolored vertices (initially, all vertices are uncolored)
    int* uncolored = new int[numVertices];
    for (int i = 0; i < numVertices; ++i) uncolored[i] = i;

    // The v-th position of numAdjUncolored contains the number of uncolored
    // vertices in the adjacency of v
    int* numAdjUncolored = new int[numVertices];
    for (int i = 0; i < numVertices; ++i)
    {
        // Index zero of gamma[i] contains the its size
        numAdjUncolored[i] = instance->gamma[i][0];
    }
    
    // The v-th position of numAdjUncolored contains the number of different
    // colors in the adjacency of v
    int* numAdjColors = new int[numVertices];
    for (int i = 0; i < numVertices; ++i) numAdjColors[i] = 0;
    
	// For each vertex v, adjColors contains a 0-1 vector where the content of
	// index c indicates if color c is used in the adjacency of v
    char** adjColors = new char*[numVertices];
    for (int i = 0; i < numVertices; ++i)
    {
    	adjColors[i] = new char[numVertices];
    	for (int j = 0; j < numVertices; ++j)
    	{
    		adjColors[i][j] = 0;
    	}
    }
	
	// Color one vertex at a time, until all vertices are colored
    for (int uncoloredSize = numVertices; uncoloredSize >= 0; --uncoloredSize)
	{
    	int maxVertex = maximumSaturatedVertex(uncolored, uncoloredSize,
                numAdjColors, numAdjUncolored);
		int color = minFeasibleColor(maxVertex, adjColors);
		
		solution->coloring[maxVertex] = color;
		updateAdjacencies(instance, solution, maxVertex, color, adjColors,
                numAdjColors, numAdjUncolored);
	}

    // Clean data structures
    if (uncolored) delete[] uncolored;
    for (int i = 0; i < numVertices; ++i)
    {
    	if (adjColors[i]) delete[] adjColors[i];
    }
    if (adjColors) delete[] adjColors;
    if (numAdjColors) delete[] numAdjColors;
    if (numAdjUncolored) delete[] numAdjUncolored;
}
