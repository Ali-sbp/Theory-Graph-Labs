"use strict";

// Eulerian cycle algorithm (Hierholzer / stack variant, labelled "Флери" in lectures).
// solve() expects a symmetric 0/1 adjacency matrix (caller symmetrizes directed graphs).

window.Eulerian = {
    solve(adjMatrix) {
        const n = adjMatrix.length;
        const result = {
            isEulerian: false,
            modifications: [],
            addedEdges: [],
            removedEdges: [],
            modifiedAdj: null,
            cycle: [],
            found: false,
            failureReason: ''
        };
        if (n <= 0) return result;

        // Working copy (mutable)
        const work = adjMatrix.map(row => row.slice());

        // Degree array
        const deg = new Array(n).fill(0);
        for (let i = 0; i < n; i++)
            for (let j = 0; j < n; j++)
                if (work[i][j]) deg[i]++;

        // BFS to find connected components of non-isolated vertices
        const buildComponents = () => {
            const visited = new Array(n).fill(false);
            const comps = [];
            for (let s = 0; s < n; s++) {
                if (deg[s] === 0 || visited[s]) continue;
                const comp = [];
                const queue = [s];
                visited[s] = true;
                while (queue.length > 0) {
                    const v = queue.shift();
                    comp.push(v);
                    for (let u = 0; u < n; u++) {
                        if (!visited[u] && work[v][u]) {
                            visited[u] = true;
                            queue.push(u);
                        }
                    }
                }
                comps.push(comp);
            }
            return comps;
        };

        // Check original Eulerian property
        let components = buildComponents();
        let allEven = true;
        for (let v = 0; v < n; v++)
            if (deg[v] % 2 !== 0) { allEven = false; break; }
        result.isEulerian = (components.length <= 1 && allEven);

        // Fix connectivity
        while (components.length > 1) {
            const rep0 = components[0][0];
            const rep1 = components[1][0];
            work[rep0][rep1] = work[rep1][rep0] = 1;
            deg[rep0]++;
            deg[rep1]++;
            result.addedEdges.push([rep0, rep1]);
            result.modifications.push(
                'Добавлено ребро (' + rep0 + ', ' + rep1 + ') для связности компонент'
            );
            // Merge component 1 into component 0
            for (const v of components[1]) components[0].push(v);
            components.splice(1, 1);
        }

        const isConnectedAllVertices = () => {
            if (n <= 1) return true;
            const visited = new Array(n).fill(false);
            const queue = [0];
            visited[0] = true;
            while (queue.length > 0) {
                const v = queue.shift();
                for (let u = 0; u < n; u++) {
                    if (work[v][u] && !visited[u]) {
                        visited[u] = true;
                        queue.push(u);
                    }
                }
            }
            return visited.every(Boolean);
        };

        const canRemoveWithoutDisconnecting = (u, v) => {
            if (!work[u][v]) return false;
            work[u][v] = work[v][u] = 0;
            const ok = isConnectedAllVertices();
            work[u][v] = work[v][u] = 1;
            return ok;
        };

        const applyToggle = (u, v, add) => {
            work[u][v] = work[v][u] = add ? 1 : 0;
            deg[u] += add ? 1 : -1;
            deg[v] += add ? 1 : -1;
        };

        const commitToggle = (op, suffix) => {
            if (op.added) {
                result.addedEdges.push([op.u, op.v]);
                result.modifications.push(
                    'Добавлено ребро (' + op.u + ', ' + op.v + ') ' + suffix
                );
            } else {
                result.removedEdges.push([op.u, op.v]);
                result.modifications.push(
                    'Удалено ребро (' + op.u + ', ' + op.v + ') ' + suffix
                );
            }
        };

        const tryFixOddVerticesSafely = () => {
            const odds = [];
            for (let v = 0; v < n; v++)
                if (deg[v] % 2 !== 0) odds.push(v);
            if (odds.length === 0) return true;

            const used = new Array(odds.length).fill(false);
            const ops = [];

            const dfs = () => {
                let first = -1;
                for (let i = 0; i < odds.length; i++) {
                    if (!used[i]) { first = i; break; }
                }
                if (first === -1) return true;

                const u = odds[first];
                used[first] = true;

                const candidates = [];
                for (let i = 0; i < odds.length; i++)
                    if (!used[i]) candidates.push(i);

                // Prefer adding absent edges. Additions cannot disconnect the graph,
                // and they avoid consuming removable cycle edges too early.
                candidates.sort((a, b) => {
                    const addA = !work[u][odds[a]];
                    const addB = !work[u][odds[b]];
                    if (addA !== addB) return addA ? -1 : 1;
                    return odds[a] - odds[b];
                });

                for (const idx of candidates) {
                    const v = odds[idx];
                    const add = !work[u][v];
                    if (!add && !canRemoveWithoutDisconnecting(u, v)) continue;

                    used[idx] = true;
                    applyToggle(u, v, add);
                    ops.push({ u, v, added: add });

                    if (dfs()) return true;

                    ops.pop();
                    applyToggle(u, v, !add);
                    used[idx] = false;
                }

                used[first] = false;
                return false;
            };

            if (!dfs()) return false;

            for (const op of ops)
                commitToggle(op, 'для чётности степени');
            return true;
        };

        const findBridges = () => {
            const tin = new Array(n).fill(-1);
            const low = new Array(n).fill(-1);
            const parent = new Array(n).fill(-1);
            const bridges = new Set();
            let timer = 0;

            const dfs = (v) => {
                tin[v] = low[v] = timer++;
                for (let u = 0; u < n; u++) {
                    if (!work[v][u]) continue;
                    if (u === parent[v]) continue;
                    if (tin[u] !== -1) {
                        low[v] = Math.min(low[v], tin[u]);
                    } else {
                        parent[u] = v;
                        dfs(u);
                        low[v] = Math.min(low[v], low[u]);
                        if (low[u] > tin[v])
                            bridges.add(Math.min(u, v) + ',' + Math.max(u, v));
                    }
                }
            };

            for (let v = 0; v < n; v++)
                if (tin[v] === -1) dfs(v);
            return bridges;
        };

        const addBridgeBypassEdges = () => {
            const bridges = findBridges();
            if (bridges.size === 0) return false;

            const compId = new Array(n).fill(-1);
            const compVertices = [];
            for (let s = 0; s < n; s++) {
                if (compId[s] !== -1) continue;
                const id = compVertices.length;
                compVertices.push([]);
                const queue = [s];
                compId[s] = id;
                while (queue.length > 0) {
                    const v = queue.shift();
                    compVertices[id].push(v);
                    for (let u = 0; u < n; u++) {
                        if (!work[v][u] || compId[u] !== -1) continue;
                        const key = Math.min(u, v) + ',' + Math.max(u, v);
                        if (bridges.has(key)) continue;
                        compId[u] = id;
                        queue.push(u);
                    }
                }
            }

            const treeDegree = new Array(compVertices.length).fill(0);
            for (const key of bridges) {
                const [u, v] = key.split(',').map(Number);
                const cu = compId[u], cv = compId[v];
                if (cu === cv) continue;
                treeDegree[cu]++;
                treeDegree[cv]++;
            }

            const leaves = [];
            for (let c = 0; c < treeDegree.length; c++)
                if (treeDegree[c] === 1) leaves.push(c);
            if (leaves.length < 2) return false;

            let changed = false;
            const half = Math.ceil(leaves.length / 2);
            for (let i = 0; i < half; i++) {
                const ca = leaves[i];
                const cb = leaves[(i + half) % leaves.length];
                if (ca === cb) continue;

                let a = -1, b = -1;
                for (const x of compVertices[ca]) {
                    for (const y of compVertices[cb]) {
                        if (x !== y && !work[x][y]) {
                            a = x;
                            b = y;
                            break;
                        }
                    }
                    if (a !== -1) break;
                }
                if (a === -1) continue;

                applyToggle(a, b, true);
                commitToggle(
                    { u: a, v: b, added: true },
                    'для сохранения связности перед исправлением чётности'
                );
                changed = true;
            }
            return changed;
        };

        let parityFixed = false;
        for (let attempt = 0; attempt <= n + 1; attempt++) {
            if (tryFixOddVerticesSafely()) {
                parityFixed = true;
                break;
            }
            if (!addBridgeBypassEdges()) break;
        }

        if (!parityFixed) {
            result.modifiedAdj = work.map(row => row.slice());
            result.failureReason =
                'Не удалось исправить чётность степеней без нарушения связности в простом графе.';
            return result;
        }

        // Check if there are any edges
        let totalDeg = 0;
        for (let v = 0; v < n; v++) totalDeg += deg[v];

        // Snapshot the modified adjacency matrix before building lists
        result.modifiedAdj = work.map(row => row.slice());

        if (totalDeg === 0) return result;  // found stays false

        // Build adjacency lists from the final working matrix
        const adjList = Array.from({ length: n }, () => []);
        for (let i = 0; i < n; i++)
            for (let j = 0; j < n; j++)
                if (work[i][j]) adjList[i].push(j);

        // Find start vertex (first with non-empty adjacency list)
        let start = 0;
        while (start < n && adjList[start].length === 0) start++;
        if (start === n) return result;

        // Hierholzer stack algorithm (mirrors the given pseudocode exactly)
        const stack = [start];
        const emitted = [];
        while (stack.length > 0) {
            const v = stack[stack.length - 1];  // top
            if (adjList[v].length === 0) {
                stack.pop();
                emitted.push(v);
            } else {
                const u = adjList[v][0];
                stack.push(u);
                // Remove first occurrence of u from adjList[v]
                adjList[v].splice(0, 1);
                // Remove first occurrence of v from adjList[u]
                const idx = adjList[u].indexOf(v);
                if (idx !== -1) adjList[u].splice(idx, 1);
            }
        }

        emitted.reverse();
        result.cycle = emitted;
        result.found = true;
        return result;
    }
};
