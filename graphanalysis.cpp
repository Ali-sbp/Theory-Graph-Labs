#include "graphanalysis.h"

#include <algorithm>
#include <climits>
#include <queue>

AnalysisResult GraphAnalysis::analyze(const std::vector<std::vector<int>>& adj,
                                      bool /*directed*/)
{
    const int n = static_cast<int>(adj.size());
    const int INF = INT_MAX;

    AnalysisResult result;
    result.eccentricities.resize(n, 0);
    result.diameter = 0;

    for (int src = 0; src < n; ++src) {
        // BFS from src (follows adjMatrix edges — directed or not)
        std::vector<int> dist(n, INF);
        dist[src] = 0;
        std::queue<int> q;
        q.push(src);

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v = 0; v < n; ++v) {
                if (adj[u][v] && dist[v] == INF) {
                    dist[v] = dist[u] + 1;
                    q.push(v);
                }
            }
        }

        // Eccentricity = max distance to reachable vertices (hops, not weights)
        int ecc = 0;
        for (int v = 0; v < n; ++v) {
            if (v == src) continue;
            if (dist[v] != INF)
                ecc = std::max(ecc, dist[v]);
        }
        result.eccentricities[src] = ecc;
    }

    // Diameter = max eccentricity,  Center = min eccentricity
    int minEcc = INT_MAX;
    int maxEcc = 0;

    for (int i = 0; i < n; ++i) {
        minEcc = std::min(minEcc, result.eccentricities[i]);
        maxEcc = std::max(maxEcc, result.eccentricities[i]);
    }

    result.diameter = maxEcc;

    for (int i = 0; i < n; ++i) {
        if (result.eccentricities[i] == minEcc)
            result.center.push_back(i);
        if (result.eccentricities[i] == result.diameter)
            result.diametralVertices.push_back(i);
    }

    return result;
}
