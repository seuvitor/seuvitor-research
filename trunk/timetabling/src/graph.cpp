#include <algorithm>
#include "graph.h"

void ignoreVertex(std::vector<std::set<int> >& adjList, int vertex, std::vector<int>& degree, std::set<int>& candidates)
{
    candidates.erase(vertex);
    degree.at(vertex) = 0;
    
    for (std::set<int>::iterator itV = adjList[vertex].begin(); itV != adjList[vertex].end(); ++itV)
    {
        int v = *itV;
        degree.at(v) -= 1;
    }
}

void ignoreAntiAdjacency(std::vector<std::set<int> >& adjList, int vertex, std::vector<int>& degree, std::set<int>& candidates)
{
    candidates.erase(vertex);
    degree.at(vertex) = 0;
    
    std::set<int> candidatesCopy(candidates);
    for (std::set<int>::iterator itV = candidatesCopy.begin(); itV != candidatesCopy.end(); ++itV)
    {
        int v = *itV;
        if (v != vertex && adjList[vertex].find(v) == adjList[vertex].end())
        {
            ignoreVertex(adjList, v, degree, candidates);
        }
    }
}

void findClique(std::vector<std::set<int> >& adjList, std::vector<int> degree, std::set<int> candidates, std::set<int>& clique)
{
    while (candidates.size() > 0)
    {
        std::vector<int>::iterator maxIt = std::max_element(degree.begin(), degree.end());
        unsigned int max = maxIt - degree.begin();
        
        if (*maxIt == 0) break;
        
        clique.insert(max);
        ignoreAntiAdjacency(adjList, max, degree, candidates);
    }
}

void findNonOverlappingCliques(std::vector<std::set<int> >& adjList, std::vector<int>& vertexOrder)
{
    int nvertices = adjList.size();
    
    std::vector<std::set<int> > allCliques;

    std::set<int> candidates;
    for (int v = 0; v < nvertices; ++v)
    {
        candidates.insert(v);
    }
    
    std::vector<int> degree(nvertices);
    for (std::set<int>::iterator itV = candidates.begin(); itV != candidates.end(); ++itV)
    {
        int v = *itV;
        degree.at(v) = adjList.at(v).size();
    }
    
    while (candidates.size() > 0)
    {
        std::set<int> clique;
        findClique(adjList, degree, candidates, clique);
        
        if (clique.size() > 0)
        {
            // Insert on clique set, keeping it sorted by clique size
            std::vector<std::set<int> >::iterator it;
            for (it = allCliques.begin(); it != allCliques.end(); ++it)
            {
                if (it->size() < clique.size()) break;
            }
            allCliques.insert(it, clique);
            
            // Ignore all vertices from this clique on further clique searches
            for (std::set<int>::iterator itV = clique.begin(); itV != clique.end(); ++itV)
            {
                int v = *itV;
                ignoreVertex(adjList, v, degree, candidates);
            }
        }
        else
        {
            break;
        }
    }
    
    std::vector<int> sortedVertices;
    for (std::vector<std::set<int> >::iterator itC = allCliques.begin(); itC != allCliques.end(); ++itC)
    {
        std::copy(itC->begin(), itC->end(), std::back_inserter(sortedVertices));
    }
    std::copy(candidates.begin(), candidates.end(), std::back_inserter(sortedVertices));
    
    for (int v = 0; v < nvertices; ++v)
    {
        vertexOrder[sortedVertices[v]] = v;
    }
}

void buildRepresentativesSets(std::vector<std::set<int> >& adjList,
        std::vector<std::set<int> >& verticesRepresentedBy,
        std::vector<std::set<int> >& verticesThatRepresent)
{
    int nvertices = adjList.size();
    
    std::vector<int> vertexOrder(nvertices);
    findNonOverlappingCliques(adjList, vertexOrder);
    
    std::set<int> allVertices;
    for (int v = 0; v < nvertices; ++v)
    {
        allVertices.insert(v);
    }
    for (int u = 0; u < nvertices; ++u)
    {
        std::set_difference(allVertices.begin(), allVertices.end(),
                            adjList[u].begin(),
                            adjList[u].end(),
                            std::inserter(verticesRepresentedBy[u],
                                          verticesRepresentedBy[u].end()));
        std::set_difference(allVertices.begin(), allVertices.end(),
                            adjList[u].begin(),
                            adjList[u].end(),
                            std::inserter(verticesThatRepresent[u],
                                          verticesThatRepresent[u].end()));
        
        for (int v = 0; v < nvertices; ++v)
        {
            if (vertexOrder[u] < vertexOrder[v])
            {
                verticesThatRepresent[u].erase(v);
            }
            else if (vertexOrder[u] > vertexOrder[v])
            {
                verticesRepresentedBy[u].erase(v);
            }
        }
    }
}
