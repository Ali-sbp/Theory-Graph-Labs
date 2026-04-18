#pragma once

#include <vector>
#include <utility>

struct VertexCoverResult {
    std::vector<int>               cover;       // chosen vertex set S (sorted)
    std::vector<std::pair<int,int>> pickedEdges; // edges selected by the algorithm
};

class VertexCover {
public:
    // 2-approximation for minimum vertex cover (lab4.pdf algorithm).
    // Repeatedly picks the lexicographically-smallest uncovered edge (u,v),
    // adds both u and v to the cover, and removes all edges incident to u or v.
    // Guarantees |S| <= 2 * |optimal|.
    // adjUndirected must be a symmetric 0/1 matrix.
    static VertexCoverResult solve(const std::vector<std::vector<int>>& adjUndirected);
};
