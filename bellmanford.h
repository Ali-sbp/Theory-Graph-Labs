#pragma once

#include <vector>

struct BellmanFordResult {
    std::vector<int> dist;
    std::vector<int> parent;
    bool hasNegativeCycle = false;
    int iterations = 0;
};

class BellmanFord {
public:
    // Standard Bellman-Ford on a residual-style graph.
    // Edge existence is determined by capacityMatrix[u][v] > 0,
    // so zero-cost edges are valid (unlike DijkstraNeg which skips weight==0).
    static BellmanFordResult solve(
        const std::vector<std::vector<int>>& costMatrix,
        const std::vector<std::vector<int>>& capacityMatrix,
        int source);

    static std::vector<int> reconstructPath(const std::vector<int>& parent,
                                            int source, int target);
};
