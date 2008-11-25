#ifndef GRAPH_H_
#define GRAPH_H_

#include <vector>
#include <set>

void buildRepresentativesSets(std::vector<std::set<int> >& adjList,
        std::vector<std::set<int> >& eventsRepresentedBy,
        std::vector<std::set<int> >& eventsThatRepresent);

#endif /*GRAPH_H_*/
