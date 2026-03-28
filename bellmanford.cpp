#include "bellmanford.h"
#include <climits>
#include <algorithm>

static constexpr int INF = INT_MAX / 2;

BellmanFordResult BellmanFord::solve(
    const std::vector<std::vector<int>>& costMatrix,
    const std::vector<std::vector<int>>& capacityMatrix,
    int source)
{
    const int n = static_cast<int>(costMatrix.size());
    BellmanFordResult res;
    res.dist.assign(n, INF);
    res.parent.assign(n, -1);
    res.dist[source] = 0;

    // Relax edges n-1 times
    for (int iter = 0; iter < n - 1; ++iter) {
        bool updated = false;
        for (int u = 0; u < n; ++u) {
            if (res.dist[u] >= INF) continue;
            for (int v = 0; v < n; ++v) {
                if (capacityMatrix[u][v] > 0) {
                    int newDist = res.dist[u] + costMatrix[u][v];
                    if (newDist < res.dist[v]) {
                        res.dist[v] = newDist;
                        res.parent[v] = u;
                        updated = true;
                    }
                }
            }
        }
        ++res.iterations;
        if (!updated) break;
    }

    // Check for negative cycles
    for (int u = 0; u < n && !res.hasNegativeCycle; ++u) {
        if (res.dist[u] >= INF) continue;
        for (int v = 0; v < n; ++v) {
            if (capacityMatrix[u][v] > 0 &&
                res.dist[u] + costMatrix[u][v] < res.dist[v]) {
                res.hasNegativeCycle = true;
                break;
            }
        }
    }

    return res;
}

std::vector<int> BellmanFord::reconstructPath(const std::vector<int>& parent,
                                              int source, int target)
{
    std::vector<int> path;
    if (parent[target] == -1 && target != source)
        return path;

    for (int v = target; v != -1; v = parent[v])
        path.push_back(v);

    std::reverse(path.begin(), path.end());
    return path;
}
