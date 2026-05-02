#pragma once

#include <set>
#include <vector>
#include <utility>

struct FundamentalCycle {
    std::vector<int>               vertices;  // ordered cycle: [u, ..., v, u] (closed)
    std::vector<std::pair<int,int>> edges;    // each stored as {min(a,b), max(a,b)}
    std::pair<int,int>              cotreeEdge; // non-tree edge that closes the cycle
};

struct CycleSpaceResult {
    std::vector<FundamentalCycle> cycles;  // one per non-tree edge; empty if disconnected or tree
};

class CycleSpace {
public:
    // Input: symmetric 0/1 adj + weight matrix (caller symmetrizes directed graphs before calling).
    // Builds MST via Prim, identifies non-tree edges, constructs one fundamental cycle per edge.
    static CycleSpaceResult solve(const std::vector<std::vector<int>>& adj,
                                  const std::vector<std::vector<int>>& weight);

    // Returns the symmetric difference (XOR on edge sets) of the selected fundamental cycles.
    // selectedIndices: 0-based indices into result.cycles.
    static std::vector<std::pair<int,int>>
    symmetricDifference(const CycleSpaceResult& result,
                        const std::vector<int>& selectedIndices);
};
