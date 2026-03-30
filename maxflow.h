#pragma once

#include <vector>

struct MaxFlowResult {
    int maxFlow = 0;
    std::vector<std::vector<int>> flowMatrix;
    int iterations = 0;
    std::vector<std::vector<int>> augmentingPaths;
    std::vector<int> pathFlows;
    std::vector<std::vector<bool>> pathEdgeTypes; // true = backward edge
};

class MaxFlow {
public:
    // Edmonds-Karp (BFS-based Ford-Fulkerson) algorithm.
    // capacityMatrix[i][j] = capacity of edge (i,j), 0 if no edge.
    static MaxFlowResult solve(const std::vector<std::vector<int>>& capacityMatrix,
                               int source, int sink);
};
