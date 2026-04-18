#include "vertexcover.h"
#include <algorithm>

VertexCoverResult VertexCover::solve(const std::vector<std::vector<int>>& adjIn)
{
    const int n = static_cast<int>(adjIn.size());
    VertexCoverResult res;
    if (n <= 0) return res;

    // Working copy: symmetrize just in case a directed graph slipped through
    std::vector<std::vector<int>> work(n, std::vector<int>(n, 0));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (adjIn[i][j] || adjIn[j][i])
                work[i][j] = work[j][i] = 1;

    std::vector<bool> inCover(n, false);

    while (true) {
        // Find lexicographically-smallest uncovered edge (i < j)
        int u = -1, v = -1;
        for (int i = 0; i < n && u == -1; ++i)
            for (int j = i + 1; j < n; ++j)
                if (work[i][j]) { u = i; v = j; break; }

        if (u == -1) break;

        inCover[u] = inCover[v] = true;
        res.pickedEdges.push_back({u, v});

        // Remove all edges incident to u or v
        for (int k = 0; k < n; ++k) {
            work[u][k] = work[k][u] = 0;
            work[v][k] = work[k][v] = 0;
        }
    }

    for (int i = 0; i < n; ++i)
        if (inCover[i]) res.cover.push_back(i);

    return res;
}
