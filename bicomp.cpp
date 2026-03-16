#include "bicomp.h"

#include <algorithm>
#include <stack>

struct BiCompState {
    int n;
    int counter = 1;
    int iterations = 0;
    std::vector<int> num;
    std::vector<int> L;
    std::vector<int> father;
    std::vector<std::vector<int>> adj;          // adjacency list
    std::stack<std::pair<int,int>> SE;           // edge stack
    std::set<int> artPoints;
    std::vector<std::vector<std::pair<int,int>>> blocks;

    void bicomp(int v, bool isRoot)
    {
        num[v] = counter;
        L[v]   = counter;
        ++counter;

        int childCount = 0;

        for (int u : adj[v]) {
            ++iterations;

            if (num[u] == 0) {
                // tree edge
                ++childCount;
                SE.push({v, u});
                father[u] = v;

                bicomp(u, false);

                L[v] = std::min(L[v], L[u]);

                if (L[u] >= num[v]) {
                    // v is articulation point (or root with ≥2 children)
                    if (!isRoot || childCount >= 2)
                        artPoints.insert(v);

                    // Extract block from edge stack
                    std::vector<std::pair<int,int>> block;
                    while (true) {
                        auto edge = SE.top();
                        SE.pop();
                        block.push_back(edge);
                        if (edge.first == v && edge.second == u)
                            break;
                    }
                    blocks.push_back(std::move(block));
                }
            } else if (num[u] < num[v] && u != father[v]) {
                // back edge
                SE.push({v, u});
                L[v] = std::min(L[v], num[u]);
            }
        }
    }
};

BiCompResult BiComp::find(const std::vector<std::vector<int>>& adjMatrix)
{
    const int n = static_cast<int>(adjMatrix.size());
    BiCompResult result;

    if (n <= 1) return result;

    BiCompState state;
    state.n = n;
    state.num.assign(n, 0);
    state.L.assign(n, 0);
    state.father.assign(n, -1);
    state.adj.resize(n);

    // Build undirected adjacency list from the matrix
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (adjMatrix[i][j])
                state.adj[i].push_back(j);

    // Run for all components (in case of directed graph treated as undirected)
    for (int v0 = 0; v0 < n; ++v0) {
        if (state.num[v0] == 0) {
            state.father[v0] = -1;
            state.bicomp(v0, true);
        }
    }

    result.articulationPoints = std::move(state.artPoints);
    result.blocks = std::move(state.blocks);
    result.iterations = state.iterations;
    return result;
}
