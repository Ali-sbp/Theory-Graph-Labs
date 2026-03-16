#pragma once

#include <vector>

struct DijkstraNegResult {
    std::vector<int> dist;          // distance vector from source
    std::vector<int> parent;        // predecessor array for path reconstruction
    bool hasNegativeCycle = false;
    int iterations = 0;
};

class DijkstraNeg {
public:
    // Modified Dijkstra for graphs that may contain negative edge weights.
    // Uses re-insertion into the priority queue when a shorter path is found
    // (Bellman-Ford-style relaxation within Dijkstra's framework).
    // If there is a negative cycle reachable from source, sets the flag.
    static DijkstraNegResult solve(const std::vector<std::vector<int>>& weightMatrix,
                                   int source, bool directed);

    // Reconstruct path from source to target using parent array.
    static std::vector<int> reconstructPath(const std::vector<int>& parent,
                                            int source, int target);
};
