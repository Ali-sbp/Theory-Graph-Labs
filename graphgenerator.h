#pragma once

#include <vector>
#include <utility>

enum class WeightType { Positive, Negative, Mixed };

struct GraphData {
    int n = 0;
    bool directed = false;
    std::vector<std::vector<int>> adjMatrix;      // 0/1 unweighted
    std::vector<std::vector<int>> weightMatrix;    // weighted (0 = no edge)
};

class GraphGenerator {
public:
    // Generate a connected acyclic graph (tree) with n vertices.
    // Degree distribution and weights follow the Farry (geometric) distribution.
    static GraphData generate(int n, double degreeP, bool directed,
                              WeightType wType, double weightP);
};
