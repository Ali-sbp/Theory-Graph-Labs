#pragma once

#include <vector>

struct MinCostFlowResult {
    int flowValue = 0;
    int totalCost = 0;
    std::vector<std::vector<int>> flowMatrix;
    int iterations = 0;
    bool feasible = false;
    std::vector<std::vector<int>> augmentingPaths;
    std::vector<int> pathFlows;
    std::vector<int> pathCosts;
};

class MinCostFlow {
public:
    // Successive shortest path algorithm.
    // Uses Bellman-Ford to find minimum cost augmenting paths in the residual graph.
    static MinCostFlowResult solve(
        const std::vector<std::vector<int>>& capacityMatrix,
        const std::vector<std::vector<int>>& costMatrix,
        int source, int sink, int desiredFlow);
};
