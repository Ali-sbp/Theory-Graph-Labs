#include "graphgenerator.h"
#include "distribution.h"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>

GraphData GraphGenerator::generate(int n, double degreeP, bool directed,
                                   WeightType wType, double weightP)
{
    GraphData data;
    data.n = n;
    data.directed = directed;
    data.adjMatrix.assign(n, std::vector<int>(n, 0));
    data.weightMatrix.assign(n, std::vector<int>(n, 0));

    if (n <= 0) return data;
    if (n == 1) {
        data.adjMatrix.assign(1, std::vector<int>(1, 0));
        data.weightMatrix.assign(1, std::vector<int>(1, 0));
        return data;
    }

    std::mt19937 rng(static_cast<unsigned>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    FarryDistribution degDist(degreeP);

    // --- 1. Generate degree sequence for a tree (sum must be 2*(n-1)) ---
    const int targetSum = 2 * (n - 1);
    std::vector<int> degrees(n);

    for (int i = 0; i < n; ++i)
        degrees[i] = std::max(1, std::min(n - 1, degDist.generate())); //calling degDist for value
                            //clamping it to [1 - n-1]

    int sum = std::accumulate(degrees.begin(), degrees.end(), 0);

    const int maxIter = n * 100;  // safety bound to avoid infinite loop
    int iter = 0;
    while (sum != targetSum && iter < maxIter) {
        ++iter;
        if (sum > targetSum) {
            std::vector<int> cands;
            for (int i = 0; i < n; ++i)
                if (degrees[i] > 1) cands.push_back(i);
            if (cands.empty()) break;
            int idx = cands[rng() % cands.size()];
            degrees[idx]--;
            sum--;
        } else {
            std::vector<int> cands;
            for (int i = 0; i < n; ++i)
                if (degrees[i] < n - 1) cands.push_back(i);
            if (cands.empty()) break;
            int idx = cands[rng() % cands.size()];
            degrees[idx]++;
            sum++;
        }
    }

    // --- 2. Build Prufer sequence from the degree sequence ---
    //        vertex i appears (degrees[i] - 1) times => total length = n - 2
    std::vector<int> prufer;
    prufer.reserve(n - 2);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < degrees[i] - 1; ++j)
            prufer.push_back(i);

    std::shuffle(prufer.begin(), prufer.end(), rng);

    // --- 3. Decode Prufer sequence to build tree edges ---
    std::vector<int> deg(n, 1);
    for (int x : prufer) deg[x]++;

    for (int x : prufer) {
        for (int v = 0; v < n; ++v) {
            if (deg[v] == 1) {
                data.adjMatrix[v][x] = 1;
                data.adjMatrix[x][v] = 1;
                deg[v]--;
                deg[x]--;
                break;
            }
        }
    }

    // Connect the last two vertices with remaining degree 1
    int u = -1, v = -1;
    for (int i = 0; i < n; ++i) {
        if (deg[i] == 1) {
            if (u == -1) u = i;
            else { v = i; break; }
        }
    }
    if (u != -1 && v != -1) {
        data.adjMatrix[u][v] = 1;
        data.adjMatrix[v][u] = 1;
    }

    // --- 4. For directed graph: randomly orient each edge ---
    if (directed) {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (data.adjMatrix[i][j]) {
                    if (rng() % 2)
                        data.adjMatrix[i][j] = 0;   // keep j -> i
                    else
                        data.adjMatrix[j][i] = 0;   // keep i -> j
                }
            }
        }
    }

    // --- 5. Generate weights using the Farry distribution ---
    FarryDistribution weightDist(weightP);
    std::uniform_int_distribution<int> signDist(0, 1);

    for (int i = 0; i < n; ++i) {
        int jStart = directed ? 0 : i + 1;
        for (int j = jStart; j < n; ++j) {
            if (data.adjMatrix[i][j]) {
                int w = weightDist.generate();
                switch (wType) {
                case WeightType::Positive:
                    break;
                case WeightType::Negative:
                    w = -w;
                    break;
                case WeightType::Mixed:
                    if (signDist(rng)) w = -w;
                    break;
                }
                data.weightMatrix[i][j] = w;
                if (!directed)
                    data.weightMatrix[j][i] = w;
            }
        }
    }

    return data;
}
