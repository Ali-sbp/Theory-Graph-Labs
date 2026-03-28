#include "mincostflow.h"
#include "bellmanford.h"
#include <climits>
#include <algorithm>

static constexpr int INF = INT_MAX / 2;

MinCostFlowResult MinCostFlow::solve(
    const std::vector<std::vector<int>>& capacityMatrix,
    const std::vector<std::vector<int>>& costMatrix,
    int source, int sink, int desiredFlow)
{
    const int n = static_cast<int>(capacityMatrix.size());
    MinCostFlowResult res;
    res.flowMatrix.assign(n, std::vector<int>(n, 0));

    if (desiredFlow <= 0) {
        res.feasible = true;
        return res;
    }

    // Build residual capacity and cost graphs
    std::vector<std::vector<int>> resCap(n, std::vector<int>(n, 0));
    std::vector<std::vector<int>> resCost(n, std::vector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (capacityMatrix[i][j] > 0) {
                resCap[i][j] = capacityMatrix[i][j];
                resCost[i][j] = costMatrix[i][j];
                // Reverse edge cost (will become active when flow is sent)
                resCost[j][i] = -costMatrix[i][j];
            }
        }
    }

    int totalFlow = 0;
    int totalCost = 0;

    while (totalFlow < desiredFlow) {
        // Find shortest cost path using Bellman-Ford
        auto bf = BellmanFord::solve(resCost, resCap, source);

        if (bf.dist[sink] >= INF)
            break;  // no more augmenting paths

        // Reconstruct path
        auto path = BellmanFord::reconstructPath(bf.parent, source, sink);
        if (path.empty()) break;

        // Find bottleneck, capped at remaining needed flow
        int pathFlow = desiredFlow - totalFlow;
        for (size_t k = 0; k + 1 < path.size(); ++k) {
            int u = path[k], v = path[k + 1];
            pathFlow = std::min(pathFlow, resCap[u][v]);
        }

        if (pathFlow <= 0) break;

        // Update residual graph and flow matrix
        for (size_t k = 0; k + 1 < path.size(); ++k) {
            int u = path[k], v = path[k + 1];
            resCap[u][v] -= pathFlow;
            resCap[v][u] += pathFlow;
            res.flowMatrix[u][v] += pathFlow;
            res.flowMatrix[v][u] -= pathFlow;
        }

        totalFlow += pathFlow;
        totalCost += pathFlow * bf.dist[sink];

        res.augmentingPaths.push_back(path);
        res.pathFlows.push_back(pathFlow);
        res.pathCosts.push_back(bf.dist[sink]);
        ++res.iterations;
    }

    res.flowValue = totalFlow;
    res.totalCost = totalCost;
    res.feasible = (totalFlow >= desiredFlow);

    // Clean up negative flow values
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (res.flowMatrix[i][j] < 0)
                res.flowMatrix[i][j] = 0;

    return res;
}
