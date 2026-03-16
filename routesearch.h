#pragma once

#include <vector>

struct RouteResult {
    bool exists = false;
    int count = 0;                  // number of simple paths
    std::vector<int> path;          // one shortest path (if exists)
};

class RouteSearch {
public:
    static RouteResult findRoute(const std::vector<std::vector<int>>& adjMatrix,
                                 int from, int to);
};
