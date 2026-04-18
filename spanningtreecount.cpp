#include "spanningtreecount.h"
#include <cmath>
#include <algorithm>

SpanningTreeCountResult SpanningTreeCount::solve(const std::vector<std::vector<int>>& adj)
{
    const int n = static_cast<int>(adj.size());
    SpanningTreeCountResult res;
    if (n <= 0) return res;

    // Build combinatorial Laplacian L = D - A
    res.laplacian.assign(n, std::vector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && adj[i][j]) {
                res.laplacian[i][j] = -1;
                res.laplacian[i][i] += 1;
            }
        }
    }

    if (n == 1) { res.count = 1; return res; }

    // Build cofactor: delete last row + last column
    const int m = n - 1;
    res.cofactor.assign(m, std::vector<int>(m, 0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            res.cofactor[i][j] = res.laplacian[i][j];

    if (n == 2) {
        res.count = res.cofactor[0][0];
        return res;
    }

    // Gaussian elimination on floating-point copy to compute det(cofactor)
    std::vector<std::vector<double>> mat(m, std::vector<double>(m));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            mat[i][j] = static_cast<double>(res.cofactor[i][j]);

    double det = 1.0;
    for (int col = 0; col < m; ++col) {
        // Partial pivot
        int pivot = -1;
        double best = 0.0;
        for (int row = col; row < m; ++row) {
            if (std::abs(mat[row][col]) > best) {
                best = std::abs(mat[row][col]);
                pivot = row;
            }
        }
        if (pivot < 0 || best < 1e-9) { res.count = 0; return res; }
        if (pivot != col) {
            std::swap(mat[pivot], mat[col]);
            det = -det;
        }
        det *= mat[col][col];
        for (int row = col + 1; row < m; ++row) {
            double factor = mat[row][col] / mat[col][col];
            for (int k = col; k < m; ++k)
                mat[row][k] -= factor * mat[col][k];
        }
    }

    long long rounded = static_cast<long long>(std::round(det));
    res.count = (rounded < 0) ? 0 : rounded;
    return res;
}
