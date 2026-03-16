#include "routesearch.h"

#include <algorithm>
#include <queue>

// DFS that counts all simple paths from cur to target
static int countSimplePaths(const std::vector<std::vector<int>>& adj, int n,
                            int cur, int target, std::vector<bool>& visited)
{
    if (cur == target) return 1;
    int total = 0;
    visited[cur] = true;
    for (int v = 0; v < n; ++v) {
        if (adj[cur][v] && !visited[v])
            total += countSimplePaths(adj, n, v, target, visited);
    }
    visited[cur] = false;
    return total;
}

RouteResult RouteSearch::findRoute(const std::vector<std::vector<int>>& adj,
                                   int from, int to)
{
    RouteResult result;
    const int n = static_cast<int>(adj.size());

    if (from < 0 || from >= n || to < 0 || to >= n)
        return result;

    if (from == to) {
        result.exists = true;
        result.count = 1;
        result.path = {from};
        return result;
    }

    // BFS to find shortest path
    std::vector<int> parent(n, -1);
    std::vector<bool> visited(n, false);
    std::queue<int> q;
    q.push(from);
    visited[from] = true;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v = 0; v < n; ++v) {
            if (adj[u][v] && !visited[v]) {
                visited[v] = true;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    if (!visited[to])
        return result;   // no route

    result.exists = true;

    // Reconstruct one (shortest) path
    for (int cur = to; cur != -1; cur = parent[cur])
        result.path.push_back(cur);
    std::reverse(result.path.begin(), result.path.end());

    // Count all simple paths via DFS
    std::vector<bool> vis(n, false);
    result.count = countSimplePaths(adj, n, from, to, vis);

    return result;
}
