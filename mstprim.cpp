#include "mstprim.h"
#include <climits>
#include <algorithm>

static constexpr int PRIM_INF = INT_MAX / 2;

// ---------------------------------------------------------------------------
//  Prüfer encode
//  Input: MST edges (stored in res.edges / res.weights), vertex count n.
//  Output: fills res.pruferCode (size n-2) and res.pruferWeights (size n-1).
// ---------------------------------------------------------------------------
static void pruferEncode(MSTResult& res, int n)
{
    if (n <= 1) return;

    // Build MST adjacency (n×n) and weights
    std::vector<std::vector<int>> mstAdj(n, std::vector<int>(n, 0));
    std::vector<std::vector<int>> mstW  (n, std::vector<int>(n, 0));
    std::vector<int> deg(n, 0);

    for (int k = 0; k < (int)res.edges.size(); ++k) {
        auto [u, v] = res.edges[k];
        int w = res.weights[k];
        mstAdj[u][v] = mstAdj[v][u] = 1;
        mstW  [u][v] = mstW  [v][u] = w;
        deg[u]++;
        deg[v]++;
    }

    if (n == 2) {
        // Prüfer code is empty; one weight for the single edge
        if (!res.weights.empty())
            res.pruferWeights.push_back(res.weights[0]);
        return;
    }

    // n-2 leaf-removal steps
    for (int step = 0; step < n - 2; ++step) {
        int leaf = -1;
        for (int v = 0; v < n; ++v) {
            if (deg[v] == 1) { leaf = v; break; }
        }
        if (leaf == -1) break;

        int neighbor = -1;
        for (int v = 0; v < n; ++v) {
            if (mstAdj[leaf][v]) { neighbor = v; break; }
        }
        if (neighbor == -1) break;

        res.pruferCode.push_back(neighbor);
        res.pruferWeights.push_back(mstW[leaf][neighbor]);

        mstAdj[leaf][neighbor] = mstAdj[neighbor][leaf] = 0;
        deg[leaf]--;
        deg[neighbor]--;
    }

    // Last two vertices with deg == 1
    int u = -1, v = -1;
    for (int i = 0; i < n; ++i) {
        if (deg[i] == 1) {
            if (u == -1) u = i;
            else { v = i; break; }
        }
    }
    int lastW = (u != -1 && v != -1) ? mstW[u][v] : 0;
    res.pruferWeights.push_back(lastW);
}

// ---------------------------------------------------------------------------
//  Prüfer decode
//  Input: res.pruferCode, res.pruferWeights, vertex count n.
//  Output: fills res.decodedEdges, res.decodedWeights, res.roundTripOk.
// ---------------------------------------------------------------------------
static void pruferDecode(MSTResult& res, int n)
{
    if (n <= 1) return;

    const auto& A = res.pruferCode;
    const auto& W = res.pruferWeights;

    if (n == 2) {
        res.decodedEdges.push_back({0, 1});
        res.decodedWeights.push_back(W.empty() ? 0 : W[0]);
    } else {
        // Compute degree: deg[v] = 1 + frequency of v in A
        std::vector<int> deg(n, 1);
        for (int x : A) deg[x]++;

        for (int i = 0; i < n - 2; ++i) {
            int leaf = -1;
            for (int v = 0; v < n; ++v) {
                if (deg[v] == 1) { leaf = v; break; }
            }
            if (leaf == -1) break;

            int neighbor = A[i];
            int w = (i < (int)W.size()) ? W[i] : 0;
            int lo = std::min(leaf, neighbor), hi = std::max(leaf, neighbor);
            res.decodedEdges.push_back({lo, hi});
            res.decodedWeights.push_back(w);
            deg[leaf]--;
            deg[neighbor]--;
        }

        // Last edge
        int u = -1, v = -1;
        for (int i = 0; i < n; ++i) {
            if (deg[i] == 1) {
                if (u == -1) u = i;
                else { v = i; break; }
            }
        }
        if (u != -1 && v != -1) {
            int lastW = ((int)W.size() >= n - 1) ? W[n - 2] : 0;
            res.decodedEdges.push_back({std::min(u,v), std::max(u,v)});
            res.decodedWeights.push_back(lastW);
        }
    }

    // Round-trip check: sort both edge lists and compare
    using EW = std::pair<std::pair<int,int>, int>;
    std::vector<EW> orig, decoded;
    for (int k = 0; k < (int)res.edges.size(); ++k) {
        auto [a, b] = res.edges[k];
        orig.push_back({{std::min(a,b), std::max(a,b)}, res.weights[k]});
    }
    for (int k = 0; k < (int)res.decodedEdges.size(); ++k) {
        auto [a, b] = res.decodedEdges[k];
        decoded.push_back({{std::min(a,b), std::max(a,b)}, res.decodedWeights[k]});
    }
    std::sort(orig.begin(), orig.end());
    std::sort(decoded.begin(), decoded.end());
    res.roundTripOk = (orig == decoded);
}

// ---------------------------------------------------------------------------
//  MSTPrim::solve
// ---------------------------------------------------------------------------
MSTResult MSTPrim::solve(const std::vector<std::vector<int>>& adj,
                          const std::vector<std::vector<int>>& weight)
{
    const int n = static_cast<int>(adj.size());
    MSTResult res;
    if (n <= 0) return res;
    if (n == 1) { pruferEncode(res, 1); pruferDecode(res, 1); return res; }

    std::vector<int>  dist  (n, PRIM_INF);
    std::vector<int>  parent(n, -1);
    std::vector<bool> inMST (n, false);
    dist[0] = 0;

    int reached = 0;
    for (int step = 0; step < n; ++step) {
        // Pick min-dist vertex not in MST
        int u = -1;
        for (int v = 0; v < n; ++v)
            if (!inMST[v] && (u == -1 || dist[v] < dist[u]))
                u = v;

        if (u == -1 || dist[u] == PRIM_INF) break;

        inMST[u] = true;
        ++reached;

        if (parent[u] != -1) {
            int p = parent[u];
            int lo = std::min(p, u), hi = std::max(p, u);
            res.edges.push_back({lo, hi});
            res.weights.push_back(weight[p][u]);
            res.totalWeight += weight[p][u];
        }

        // Relax neighbors
        for (int v = 0; v < n; ++v) {
            if (!inMST[v] && adj[u][v] && weight[u][v] < dist[v]) {
                dist[v] = weight[u][v];
                parent[v] = u;
            }
        }
    }

    res.connected = (reached == n);

    pruferEncode(res, n);
    pruferDecode(res, n);

    return res;
}
