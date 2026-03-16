#pragma once

#include <vector>

struct AnalysisResult {
    std::vector<int> eccentricities;   // eccentricity per vertex (INT_MAX = ∞)
    std::vector<int> center;           // vertices with minimum eccentricity
    int diameter;                      // maximum eccentricity
    std::vector<int> diametralVertices;// vertices with eccentricity == diameter
};

class GraphAnalysis {
public:
    static AnalysisResult analyze(const std::vector<std::vector<int>>& adjMatrix,
                                  bool directed);
};
