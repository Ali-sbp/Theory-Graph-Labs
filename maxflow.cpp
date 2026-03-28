#include "maxflow.h"
#include <climits>
#include <queue>
#include <algorithm>

static constexpr int INF = INT_MAX / 2;

MaxFlowResult MaxFlow::solve(const std::vector<std::vector<int>>& capacityMatrix,
                             int source, int sink)
{
    const int n = static_cast<int>(capacityMatrix.size());
    MaxFlowResult res;
    res.flowMatrix.assign(n, std::vector<int>(n, 0));

    // Residual graph
    auto residual = capacityMatrix;

    while (true) {
        // BFS to find shortest augmenting path
        std::vector<int> parent(n, -1);
        std::vector<bool> visited(n, false);
        std::queue<int> q;
        q.push(source);
        visited[source] = true;

        bool found = false;
        while (!q.empty() && !found) {
            int u = q.front();
            q.pop();
            for (int v = 0; v < n; ++v) {
                if (!visited[v] && residual[u][v] > 0) {
                    visited[v] = true;
                    parent[v] = u;
                    if (v == sink) {
                        found = true;
                        break;
                    }
                    q.push(v);
                }
            }
        }

        if (!found) break;

        // Find bottleneck
        int pathFlow = INF;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            pathFlow = std::min(pathFlow, residual[u][v]);
        }

        // Record augmenting path
        std::vector<int> path;
        for (int v = sink; v != -1; v = parent[v])
            path.push_back(v);
        std::reverse(path.begin(), path.end());
        res.augmentingPaths.push_back(path);
        res.pathFlows.push_back(pathFlow);

        // Update residual graph and flow
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            residual[u][v] -= pathFlow;
            residual[v][u] += pathFlow;
            res.flowMatrix[u][v] += pathFlow;
            res.flowMatrix[v][u] -= pathFlow;
        }

        res.maxFlow += pathFlow;
        ++res.iterations;
    }

    // Clean up: remove negative flow values
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (res.flowMatrix[i][j] < 0)
                res.flowMatrix[i][j] = 0;

    return res;
}
