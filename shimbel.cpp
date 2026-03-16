#include "shimbel.h"

#include <algorithm>
#include <climits>

// Sentinels chosen to avoid overflow on addition
static const int POS_INF = INT_MAX / 2;
static const int NEG_INF = INT_MIN / 2;

// Min-plus matrix multiplication:  C[i][j] = min_m ( A[i][m] + B[m][j] )
static std::vector<std::vector<int>> minPlusMul(
    const std::vector<std::vector<int>>& A,
    const std::vector<std::vector<int>>& B, int n)
{
    std::vector<std::vector<int>> C(n, std::vector<int>(n, POS_INF));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int m = 0; m < n; ++m)
                if (A[i][m] < POS_INF && B[m][j] < POS_INF)
                    C[i][j] = std::min(C[i][j], A[i][m] + B[m][j]);
    return C;
}

// Max-plus matrix multiplication:  C[i][j] = max_m ( A[i][m] + B[m][j] )
static std::vector<std::vector<int>> maxPlusMul(
    const std::vector<std::vector<int>>& A,
    const std::vector<std::vector<int>>& B, int n)
{
    std::vector<std::vector<int>> C(n, std::vector<int>(n, NEG_INF));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int m = 0; m < n; ++m)
                if (A[i][m] > NEG_INF && B[m][j] > NEG_INF)
                    C[i][j] = std::max(C[i][j], A[i][m] + B[m][j]);
    return C;
}

std::vector<std::vector<int>> Shimbel::minPath(
    const std::vector<std::vector<int>>& weightMatrix, int k)
{
    const int n = static_cast<int>(weightMatrix.size());
    if (n == 0 || k <= 0)
        return std::vector<std::vector<int>>(n, std::vector<int>(n, POS_INF));

    // Build initial weight matrix W (POS_INF = no edge)
    std::vector<std::vector<int>> W(n, std::vector<int>(n, POS_INF));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (weightMatrix[i][j] != 0)
                W[i][j] = weightMatrix[i][j];

    auto result = W;  // S(1) = W
    for (int step = 2; step <= k; ++step)
        result = minPlusMul(result, W, n);

    return result;
}

std::vector<std::vector<int>> Shimbel::maxPath(
    const std::vector<std::vector<int>>& weightMatrix, int k)
{
    const int n = static_cast<int>(weightMatrix.size());
    if (n == 0 || k <= 0)
        return std::vector<std::vector<int>>(n, std::vector<int>(n, NEG_INF));

    // Build initial weight matrix W (NEG_INF = no edge)
    std::vector<std::vector<int>> W(n, std::vector<int>(n, NEG_INF));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (weightMatrix[i][j] != 0)
                W[i][j] = weightMatrix[i][j];

    auto result = W;  // S(1) = W
    for (int step = 2; step <= k; ++step)
        result = maxPlusMul(result, W, n);

    return result;
}
