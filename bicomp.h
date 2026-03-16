#pragma once

#include <vector>
#include <set>

struct BiCompResult {
    std::set<int> articulationPoints;
    std::vector<std::vector<std::pair<int,int>>> blocks; // each block = list of edges
    int iterations = 0;
};

class BiComp {
public:
    // Find articulation points and biconnected components using the
    // lecture algorithm (DFS with num/L arrays + edge stack).
    // Works on the UNDIRECTED adjacency matrix (ignores direction).
    static BiCompResult find(const std::vector<std::vector<int>>& adjMatrix);
};
