#include "dijkstraneg.h"

#include <algorithm>
#include <climits>
#include <queue>

static const int INF = INT_MAX / 2;

DijkstraNegResult DijkstraNeg::solve(
    const std::vector<std::vector<int>>& weightMatrix, int source, bool directed)
{
    const int n = static_cast<int>(weightMatrix.size());
    DijkstraNegResult result;
    result.dist.assign(n, INF);
    result.parent.assign(n, -1);

    if (source < 0 || source >= n)
        return result;

    result.dist[source] = 0;

    // Modified Dijkstra for negative weights:
    // Track how many times each vertex is relaxed. If > n, negative cycle.
    std::vector<int> relaxCount(n, 0);

    // Priority queue: (distance, vertex)
    using PII = std::pair<int, int>;
    std::priority_queue<PII, std::vector<PII>, std::greater<PII>> pq;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        ++result.iterations;

        // Skip stale entries
        if (d > result.dist[u])
            continue;

        ++relaxCount[u];
        if (relaxCount[u] > n) {
            result.hasNegativeCycle = true;
            break;
        }

        for (int v = 0; v < n; ++v) {
            if (weightMatrix[u][v] == 0)
                continue;   // no edge

            // For undirected graphs: don't traverse back to the parent vertex.
            // This prevents artificial 2-vertex negative cycles (u→v→u→v...)
            // on undirected edges with negative weights.
            if (!directed && v == result.parent[u])
                continue;

            int newDist = result.dist[u] + weightMatrix[u][v];
            if (newDist < result.dist[v]) {
                result.dist[v] = newDist;
                result.parent[v] = u;
                pq.push({newDist, v});
            }
        }
    }

    return result;
}

std::vector<int> DijkstraNeg::reconstructPath(const std::vector<int>& parent,
                                              int source, int target)
{
    std::vector<int> path;
    if (parent.empty()) return path;

    const int n = static_cast<int>(parent.size());
    if (source < 0 || source >= n || target < 0 || target >= n)
        return path;

    // Check if target is reachable
    if (source != target && parent[target] == -1)
        return path;

    int steps = 0;
    for (int cur = target; cur != -1; cur = parent[cur]) {
        path.push_back(cur);
        if (++steps > n) {
            // Cycle in parent chain (caused by negative cycle) — no valid path
            path.clear();
            return path;
        }
    }

    std::reverse(path.begin(), path.end());
    return path;
}
