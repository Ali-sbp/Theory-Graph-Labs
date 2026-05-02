#include "eulerian.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <set>
#include <stack>
#include <string>

EulerianResult Eulerian::solve(const std::vector<std::vector<int>>& adj)
{
    const int n = static_cast<int>(adj.size());
    EulerianResult res;
    res.isEulerian = false;
    res.found = false;

    if (n <= 0) return res;

    // Working copy (mutable)
    std::vector<std::vector<int>> work(adj);

    // Degree array
    std::vector<int> deg(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (work[i][j]) deg[i]++;

    // BFS to find connected components of non-isolated vertices
    auto buildComponents = [&]() -> std::vector<std::vector<int>> {
        std::vector<bool> visited(n, false);
        std::vector<std::vector<int>> comps;
        for (int s = 0; s < n; ++s) {
            if (deg[s] == 0 || visited[s]) continue;
            std::vector<int> comp;
            std::queue<int> q;
            q.push(s);
            visited[s] = true;
            while (!q.empty()) {
                int v = q.front(); q.pop();
                comp.push_back(v);
                for (int u = 0; u < n; ++u) {
                    if (!visited[u] && work[v][u]) {
                        visited[u] = true;
                        q.push(u);
                    }
                }
            }
            comps.push_back(comp);
        }
        return comps;
    };

    // Check original Eulerian property
    auto components = buildComponents();
    bool allEven = true;
    for (int v = 0; v < n; ++v)
        if (deg[v] % 2 != 0) { allEven = false; break; }
    res.isEulerian = (components.size() <= 1 && allEven);

    // Fix connectivity: add one edge between representative vertices of different components
    while (components.size() > 1) {
        int rep0 = components[0][0];
        int rep1 = components[1][0];
        work[rep0][rep1] = work[rep1][rep0] = 1;
        deg[rep0]++;
        deg[rep1]++;
        res.addedEdges.push_back({rep0, rep1});
        res.modifications.push_back(
            "Добавлено ребро (" + std::to_string(rep0) + ", " + std::to_string(rep1) +
            ") для связности компонент");
        // Merge component 1 into component 0
        for (int v : components[1]) components[0].push_back(v);
        components.erase(components.begin() + 1);
    }

    auto isConnectedAllVertices = [&]() -> bool {
        if (n <= 1) return true;
        std::vector<bool> visited(n, false);
        std::queue<int> q;
        q.push(0);
        visited[0] = true;
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int u = 0; u < n; ++u) {
                if (work[v][u] && !visited[u]) {
                    visited[u] = true;
                    q.push(u);
                }
            }
        }
        return std::all_of(visited.begin(), visited.end(), [](bool x) { return x; });
    };

    auto canRemoveWithoutDisconnecting = [&](int u, int v) -> bool {
        if (!work[u][v]) return false;
        work[u][v] = work[v][u] = 0;
        bool ok = isConnectedAllVertices();
        work[u][v] = work[v][u] = 1;
        return ok;
    };

    struct ToggleOp {
        int u;
        int v;
        bool added;
    };

    auto applyToggle = [&](int u, int v, bool add) {
        work[u][v] = work[v][u] = add ? 1 : 0;
        deg[u] += add ? 1 : -1;
        deg[v] += add ? 1 : -1;
    };

    auto commitToggle = [&](const ToggleOp& op, const std::string& suffix) {
        if (op.added) {
            res.addedEdges.push_back({op.u, op.v});
            res.modifications.push_back(
                "Добавлено ребро (" + std::to_string(op.u) + ", " + std::to_string(op.v) +
                ") " + suffix);
        } else {
            res.removedEdges.push_back({op.u, op.v});
            res.modifications.push_back(
                "Удалено ребро (" + std::to_string(op.u) + ", " + std::to_string(op.v) +
                ") " + suffix);
        }
    };

    auto tryFixOddVerticesSafely = [&]() -> bool {
        std::vector<int> odds;
        for (int v = 0; v < n; ++v)
            if (deg[v] % 2 != 0) odds.push_back(v);
        if (odds.empty()) return true;

        std::vector<bool> used(odds.size(), false);
        std::vector<ToggleOp> ops;

        std::function<bool()> dfs = [&]() -> bool {
            int first = -1;
            for (int i = 0; i < static_cast<int>(odds.size()); ++i) {
                if (!used[i]) { first = i; break; }
            }
            if (first == -1) return true;

            const int u = odds[first];
            used[first] = true;

            std::vector<int> candidates;
            for (int i = 0; i < static_cast<int>(odds.size()); ++i)
                if (!used[i]) candidates.push_back(i);

            // Prefer adding absent edges. Additions cannot disconnect the graph,
            // and they avoid consuming removable cycle edges too early.
            std::sort(candidates.begin(), candidates.end(), [&](int a, int b) {
                const bool addA = !work[u][odds[a]];
                const bool addB = !work[u][odds[b]];
                if (addA != addB) return addA > addB;
                return odds[a] < odds[b];
            });

            for (int idx : candidates) {
                const int v = odds[idx];
                const bool add = !work[u][v];
                if (!add && !canRemoveWithoutDisconnecting(u, v)) continue;

                used[idx] = true;
                applyToggle(u, v, add);
                ops.push_back({u, v, add});

                if (dfs()) return true;

                ops.pop_back();
                applyToggle(u, v, !add);
                used[idx] = false;
            }

            used[first] = false;
            return false;
        };

        if (!dfs()) return false;

        for (const auto& op : ops)
            commitToggle(op, "для чётности степени");
        return true;
    };

    auto findBridges = [&]() -> std::set<std::pair<int,int>> {
        std::vector<int> tin(n, -1), low(n, -1), parent(n, -1);
        std::set<std::pair<int,int>> bridges;
        int timer = 0;

        std::function<void(int)> dfs = [&](int v) {
            tin[v] = low[v] = timer++;
            for (int u = 0; u < n; ++u) {
                if (!work[v][u]) continue;
                if (u == parent[v]) continue;
                if (tin[u] != -1) {
                    low[v] = std::min(low[v], tin[u]);
                } else {
                    parent[u] = v;
                    dfs(u);
                    low[v] = std::min(low[v], low[u]);
                    if (low[u] > tin[v])
                        bridges.insert({std::min(u, v), std::max(u, v)});
                }
            }
        };

        for (int v = 0; v < n; ++v)
            if (tin[v] == -1) dfs(v);
        return bridges;
    };

    auto addBridgeBypassEdges = [&]() -> bool {
        const auto bridges = findBridges();
        if (bridges.empty()) return false;

        std::vector<int> compId(n, -1);
        std::vector<std::vector<int>> compVertices;
        for (int s = 0; s < n; ++s) {
            if (compId[s] != -1) continue;
            const int id = static_cast<int>(compVertices.size());
            compVertices.push_back({});
            std::queue<int> q;
            q.push(s);
            compId[s] = id;
            while (!q.empty()) {
                int v = q.front(); q.pop();
                compVertices[id].push_back(v);
                for (int u = 0; u < n; ++u) {
                    if (!work[v][u] || compId[u] != -1) continue;
                    auto key = std::make_pair(std::min(u, v), std::max(u, v));
                    if (bridges.count(key)) continue;
                    compId[u] = id;
                    q.push(u);
                }
            }
        }

        std::vector<int> treeDegree(compVertices.size(), 0);
        for (auto [u, v] : bridges) {
            int cu = compId[u], cv = compId[v];
            if (cu == cv) continue;
            treeDegree[cu]++;
            treeDegree[cv]++;
        }

        std::vector<int> leaves;
        for (int c = 0; c < static_cast<int>(treeDegree.size()); ++c)
            if (treeDegree[c] == 1) leaves.push_back(c);
        if (leaves.size() < 2) return false;

        bool changed = false;
        const int half = static_cast<int>((leaves.size() + 1) / 2);
        for (int i = 0; i < half; ++i) {
            int ca = leaves[i];
            int cb = leaves[(i + half) % leaves.size()];
            if (ca == cb) continue;

            int a = -1, b = -1;
            for (int x : compVertices[ca]) {
                for (int y : compVertices[cb]) {
                    if (x != y && !work[x][y]) {
                        a = x;
                        b = y;
                        break;
                    }
                }
                if (a != -1) break;
            }
            if (a == -1) continue;

            applyToggle(a, b, true);
            commitToggle({a, b, true}, "для сохранения связности перед исправлением чётности");
            changed = true;
        }
        return changed;
    };

    bool parityFixed = false;
    for (int attempt = 0; attempt <= n + 1; ++attempt) {
        if (tryFixOddVerticesSafely()) {
            parityFixed = true;
            break;
        }
        if (!addBridgeBypassEdges()) break;
    }

    if (!parityFixed) {
        res.modifiedAdj = work;
        res.found = false;
        res.failureReason =
            "Не удалось исправить чётность степеней без нарушения связности в простом графе.";
        return res;
    }

    // Check if there are any edges at all
    int totalDeg = 0;
    for (int v = 0; v < n; ++v) totalDeg += deg[v];

    // Snapshot the modified adjacency matrix before building lists
    res.modifiedAdj = work;

    if (totalDeg == 0) {
        res.found = false;
        return res;
    }

    // Build adjacency lists from the final working matrix
    std::vector<std::vector<int>> adjList(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (work[i][j]) adjList[i].push_back(j);

    // Find start vertex (first with non-empty adjacency list)
    int start = 0;
    while (start < n && adjList[start].empty()) ++start;
    if (start == n) { res.found = false; return res; }

    // Hierholzer stack algorithm (as given in spec)
    std::stack<int> S;
    std::vector<int> emitted;
    S.push(start);
    while (!S.empty()) {
        int v = S.top();
        if (adjList[v].empty()) {
            S.pop();
            emitted.push_back(v);
        } else {
            int u = adjList[v][0];
            S.push(u);
            // Remove one occurrence of u from adjList[v]
            adjList[v].erase(adjList[v].begin());
            // Remove one occurrence of v from adjList[u]
            auto it = std::find(adjList[u].begin(), adjList[u].end(), v);
            if (it != adjList[u].end()) adjList[u].erase(it);
        }
    }

    std::reverse(emitted.begin(), emitted.end());
    res.cycle = emitted;
    res.found = true;
    return res;
}
