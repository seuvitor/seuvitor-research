#include "ts.h"
#include <cstdlib>
#include <utility>
#include <list>
#include <set>
#include <algorithm>

#define K 16

void generateRandomSolution(Solution& solution, char** countAdjColors, std::set<int>& conflictingVertices)
{
    for (int u = 0; u < solution.instance->nvertices; ++u)
    {
        int color = rand() % K;
        solution.coloring[u] = color;

        // Gets adjacency of vertex u
        int* adj = solution.instance->gamma[u];
        
        // Iteration starts on index 1 and ends adj[0] indices after the start
        for (int *it = (adj + 1), *end = (it + adj[0]); it != end; ++it)
        {
            int v = *it;
            countAdjColors[v][color] += 1;
        }
    }
    
    for (int u = 0; u < solution.instance->nvertices; ++u)
    {
        if (countAdjColors[u][solution.coloring[u]] != 0)
        {
            conflictingVertices.insert(u);
        }
    }
}

int calculateValue(const Solution& solution)
{
    int value = 0;
    for (int u = 0; u < solution.instance->nvertices; ++u)
    {
        // Gets adjacency of vertex u
        int* adj = solution.instance->gamma[u];
        
        // Iteration starts on index 1 and ends adj[0] indices after the start
        for (int *it = (adj + 1), *end = (it + adj[0]); it != end; ++it)
        {
            int v = *it;
            if (u < v)
            {
                if (solution.coloring[u] == solution.coloring[v])
                    value += 1;
            }            
        }
    }
    return value;
}

void chooseBestMove(const Solution& solution, char** countAdjColors,
        const std::set<int>& conflictingVertices, const int diffToBestValue,
        const std::list<std::pair<int, int> >& tabuList,
        std::pair<int, int>& bestMove, int& bestMoveDelta)
{
    std::pair<int, int> move;
    for (std::set<int>::iterator vertexIt = conflictingVertices.begin(); vertexIt != conflictingVertices.end(); ++vertexIt)
    {
        int u = *vertexIt;
        int currentColor = solution.coloring[u];
        int currentConflicts = countAdjColors[u][currentColor];
        for (int newColor = 0; newColor < K; ++newColor)
        {
            if (newColor == currentColor) continue;
            move = std::make_pair(u, newColor);
            int newConflicts = countAdjColors[u][newColor];
            int delta = newConflicts - currentConflicts;
            
            // If the neighbour solution is better than the current, move to it
            if (delta < 0)
            {
                // Ignore if this move is tabu and the neighbor solution
                // doesn't represent a global improvement
                bool isTabu = (std::find(tabuList.begin(), tabuList.end(), move) != tabuList.end());
                if (isTabu && (diffToBestValue + delta >= 0))
                    continue;
                
                bestMove = move;
                bestMoveDelta = delta;
                break;
            }                
            // Otherwise, keep looking for the best neighbour
            else if (bestMove.first == -1 || delta < bestMoveDelta)
            {
                // Ignore if this move is tabu
                bool isTabu = (std::find(tabuList.begin(), tabuList.end(), move) != tabuList.end());
                if (isTabu) continue;
                
                bestMove = move;
                bestMoveDelta = delta;
            }
        }
    }

}

void applyMove(Solution& solution, char** countAdjColors,
        std::set<int>& conflictingVertices, const std::pair<int, int>& move)
{
    int u = move.first;
    int newColor = move.second;
    int oldColor = solution.coloring[u];
    
    // Change color of vertex
    solution.coloring[u] = newColor;
    
    // Update count of adjacent colors
    
    // Gets adjacency of vertex u
    int* adj = solution.instance->gamma[u];
    
    // Iteration starts on index 1 and ends adj[0] indices after the start
    for (int *it = (adj + 1), *end = (it + adj[0]); it != end; ++it)
    {
        int v = *it;
        countAdjColors[v][oldColor] -= 1;
        countAdjColors[v][newColor] += 1;
        
        // If this move removes the last conflict of vertex v
        if (solution.coloring[v] == oldColor
                && countAdjColors[v][oldColor] == 0)
        {
            conflictingVertices.erase(v);
        }
        // If this move creates the first conflict for vertex v
        else if (solution.coloring[v] == newColor
                && countAdjColors[v][newColor] == 1)
        {
            conflictingVertices.insert(v);
        }
    }
    
    // If the new color causes no conflict for vertex u
    if (countAdjColors[u][newColor] == 0)
    {
        conflictingVertices.erase(u);
    }
}

void tabuSearch(Solution& bestSolution, char** countAdjColors,
        std::set<int>& conflictingVertices)
{
    int bestValue = calculateValue(bestSolution);

    Solution currentSolution = bestSolution;
    int currentValue = bestValue;
    int diffToBestValue = 0;
    
    int maxIt = currentSolution.instance->nvertices * 5000;
    int it = 0;
    int lastImprovementIt = 0;
    
    std::list<std::pair<int, int> > tabuList;
    
    while (it < maxIt || it - lastImprovementIt < maxIt / 10)
    {
        std::pair<int, int> bestMove = std::make_pair(-1, -1);
        int bestMoveDelta = -1;
        chooseBestMove(currentSolution, countAdjColors, conflictingVertices,
                diffToBestValue, tabuList, bestMove, bestMoveDelta);
        
        // Append to tabu list and move, if any non-tabu movement was available
        if (bestMove.first != -1)
        {
            tabuList.push_back(bestMove);
            applyMove(currentSolution, countAdjColors, conflictingVertices, bestMove);
            currentValue += bestMoveDelta;
        }
        
        // Remove least recent tabu
        unsigned int tabuTenure = (conflictingVertices.size() * K) / 15;
        int tabuExcess = tabuList.size() > tabuTenure;
        if (tabuExcess > 0)
        {
            std::list<std::pair<int, int> >::iterator it1, it2;
            it1 = it2 = tabuList.begin();
            std::advance(it2, tabuExcess);
            tabuList.erase(it1, it2);
        }
        
        // Update best solution found until now, if needed
        if (currentValue < bestValue)
        {
            lastImprovementIt = it;
            bestSolution = currentSolution;
            bestValue = currentValue;
            diffToBestValue = 0;
        }
        else
        {
            diffToBestValue = currentValue - bestValue;
        }
        
        // Increment iteration
        it += 1;
    }
}

void ts_constructSolution(Instance* instance, Solution* solution)
{
    // Setup data structures
    std::set<int> conflictingVertices;
    char** countAdjColors = new char*[instance->nvertices];
    for (int i = 0; i < instance->nvertices; ++i)
    {
        countAdjColors[i] = new char[K];
        std::fill(countAdjColors[i], (countAdjColors[i] + K), 0);
    }
    
    // Create a randomized initial solution
    Solution tabuSolution(instance);
    generateRandomSolution(tabuSolution, countAdjColors, conflictingVertices);

    // Perform tabu search to improve the current solution
    tabuSearch(tabuSolution, countAdjColors, conflictingVertices);
        
    for (int u = 0; u < instance->nvertices; ++u)
    {
        solution->coloring[u] = tabuSolution.coloring[u];
    }
}
