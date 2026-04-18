#pragma once

#include <vector>
#include <utility>

struct MSTResult {
    std::vector<std::pair<int,int>> edges;    // MST edges, each stored as (min,max)
    std::vector<int>                weights;  // weights[i] = weight of edges[i]
    long long totalWeight = 0;
    bool connected = true;

    // Prüfer representation
    // pruferCode[0..n-3]: standard Prüfer sequence (empty if n <= 2)
    // pruferWeights[0..n-2]: pruferWeights[i] = weight of i-th leaf-removal edge;
    //                         pruferWeights[n-2] = weight of the final remaining edge
    std::vector<int> pruferCode;
    std::vector<int> pruferWeights;

    // Round-trip decoded result
    std::vector<std::pair<int,int>> decodedEdges;
    std::vector<int>                decodedWeights;
    bool roundTripOk = false;
};

class MSTPrim {
public:
    // Prim's O(n²) MST on a connected undirected weighted graph.
    // adjUndirected[i][j] = 1 if edge exists (determines connectivity).
    // weightUndirected[i][j] = edge weight (used for MST selection).
    static MSTResult solve(const std::vector<std::vector<int>>& adjUndirected,
                           const std::vector<std::vector<int>>& weightUndirected);
};
