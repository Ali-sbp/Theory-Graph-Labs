#include "cyclespace.h"
#include "mstprim.h"

#include <algorithm>
#include <map>
#include <queue>
#include <set>

CycleSpaceResult CycleSpace::solve(const std::vector<std::vector<int>>& adj,
                                    const std::vector<std::vector<int>>& weight)
{
    CycleSpaceResult res;
    const int n = static_cast<int>(adj.size());
    if (n <= 0) return res;

    // Build MST using Prim (input already undirected — caller's responsibility)
    MSTResult mst = MSTPrim::solve(adj, weight);
    if (!mst.connected) return res;

    // Build MST edge set for fast lookup
    std::set<std::pair<int,int>> mstEdgeSet;
    for (auto& e : mst.edges)
        mstEdgeSet.insert({std::min(e.first, e.second), std::max(e.first, e.second)});

    // Build MST adjacency lists for BFS path finding
    std::vector<std::vector<int>> mstAdj(n);
    for (auto& e : mst.edges) {
        mstAdj[e.first].push_back(e.second);
        mstAdj[e.second].push_back(e.first);
    }

    // Find non-tree edges
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (!adj[i][j]) continue;
            auto key = std::make_pair(i, j);
            if (mstEdgeSet.count(key)) continue;  // tree edge

            // BFS in MST from i to j to find the tree path
            std::vector<int> parent(n, -1);
            std::vector<bool> visited(n, false);
            std::queue<int> q;
            q.push(i);
            visited[i] = true;
            while (!q.empty() && !visited[j]) {
                int v = q.front(); q.pop();
                for (int u : mstAdj[v]) {
                    if (!visited[u]) {
                        visited[u] = true;
                        parent[u] = v;
                        q.push(u);
                    }
                }
            }

            // Reconstruct path from i to j
            std::vector<int> path;
            for (int v = j; v != -1; v = parent[v])
                path.push_back(v);
            std::reverse(path.begin(), path.end());

            // Build cycle: close the loop
            FundamentalCycle cycle;
            cycle.vertices = path;
            cycle.vertices.push_back(path[0]);
            cycle.cotreeEdge = {i, j};

            // Collect edges: tree path edges + the non-tree closing edge
            for (int k = 0; k + 1 < static_cast<int>(path.size()); ++k) {
                int a = path[k], b = path[k + 1];
                cycle.edges.push_back({std::min(a, b), std::max(a, b)});
            }
            cycle.edges.push_back({std::min(i, j), std::max(i, j)});

            res.cycles.push_back(cycle);
        }
    }

    return res;
}

std::vector<std::pair<int,int>>
CycleSpace::symmetricDifference(const CycleSpaceResult& result,
                                 const std::vector<int>& selectedIndices)
{
    std::map<std::pair<int,int>, int> freq;
    for (int idx : selectedIndices) {
        if (idx < 0 || idx >= static_cast<int>(result.cycles.size())) continue;
        for (auto& e : result.cycles[idx].edges)
            freq[e]++;
    }

    std::vector<std::pair<int,int>> symDiff;
    for (auto& [e, cnt] : freq)
        if (cnt % 2 == 1) symDiff.push_back(e);

    std::sort(symDiff.begin(), symDiff.end());
    return symDiff;
}
