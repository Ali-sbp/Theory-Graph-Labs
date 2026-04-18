#pragma once

#include <vector>

struct SpanningTreeCountResult {
    long long count = 0;
    std::vector<std::vector<int>> laplacian;  // n×n Laplacian matrix
    std::vector<std::vector<int>> cofactor;   // (n-1)×(n-1) cofactor (last row/col deleted)
};

class SpanningTreeCount {
public:
    // Kirchhoff's matrix-tree theorem.
    // adjUndirected must be a symmetric 0/1 matrix (no self-loops).
    // Returns the number of spanning trees as the determinant of the (n-1)×(n-1) cofactor.
    static SpanningTreeCountResult solve(const std::vector<std::vector<int>>& adjUndirected);
};
