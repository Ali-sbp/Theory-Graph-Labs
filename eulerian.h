#pragma once

#include <vector>
#include <string>
#include <utility>

struct EulerianResult {
    bool isEulerian;                            // was graph already Eulerian (no changes needed)?
    std::vector<std::string> modifications;     // log of edge additions/removals
    std::vector<std::pair<int,int>> addedEdges;
    std::vector<std::pair<int,int>> removedEdges;
    std::vector<std::vector<int>> modifiedAdj;  // adjacency matrix after all modifications
    std::vector<int> cycle;                     // vertex sequence; cycle[0] == cycle.back()
    bool found;                                 // false if no cycle can be produced
    std::string failureReason;                  // set if no safe simple-graph correction was found
};

class Eulerian {
public:
    // Input: symmetric 0/1 adjacency matrix (caller symmetrizes directed graphs before calling).
    // Checks Eulerian property, modifies graph minimally if not Eulerian, then builds cycle.
    static EulerianResult solve(const std::vector<std::vector<int>>& adj);
};
