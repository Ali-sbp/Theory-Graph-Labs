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

    // Generate a connected DAG with richer connectivity than a tree.
    // Edges are added with cycle checking (BFS reachability) to ensure acyclicity.
    // Produces more paths between vertices — better suited for flow algorithms.
    static GraphData generateDAG(int n, double degreeP, bool directed,
                                 WeightType wType, double weightP);

    // Regenerate only the weights for an existing graph (keeps adjMatrix intact).
    static void regenerateWeights(GraphData& data, WeightType wType, double weightP);

    // BFS reachability treating all edges as undirected.
    // Returns true if target is reachable from source via adj[u][v] || adj[v][u].
    static bool canReachUndirected(const std::vector<std::vector<int>>& adj,
                                   int source, int target);
};
