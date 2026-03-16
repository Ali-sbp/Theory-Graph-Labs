#pragma once

#include <vector>

class Shimbel {
public:
    // Compute the min-weight walk matrix for paths of exactly k steps.
    static std::vector<std::vector<int>> minPath(
        const std::vector<std::vector<int>>& weightMatrix, int k);

    // Compute the max-weight walk matrix for paths of exactly k steps.
    static std::vector<std::vector<int>> maxPath(
        const std::vector<std::vector<int>>& weightMatrix, int k);
};
