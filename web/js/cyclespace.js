"use strict";

// Fundamental cycle system based on MST (Prim's algorithm from Lab 4).
// Both methods expect already-symmetrized (undirected) input — caller's responsibility.

window.CycleSpace = {

    solve(adjMatrix, weightMatrix) {
        const n = adjMatrix.length;
        const result = { cycles: [] };
        if (n <= 0) return result;

        // Build MST via Prim (reuse Lab 4 module)
        const mst = MSTPrim.solve(adjMatrix, weightMatrix);
        if (!mst.connected) return result;

        // MST edge set for fast lookup: keys are "min,max"
        const mstEdgeSet = new Set();
        for (const [u, v] of mst.edges)
            mstEdgeSet.add(Math.min(u, v) + ',' + Math.max(u, v));

        // MST adjacency lists for BFS
        const mstAdj = Array.from({ length: n }, () => []);
        for (const [u, v] of mst.edges) {
            mstAdj[u].push(v);
            mstAdj[v].push(u);
        }

        // Find non-tree edges and build one fundamental cycle per edge
        for (let i = 0; i < n; i++) {
            for (let j = i + 1; j < n; j++) {
                if (!adjMatrix[i][j]) continue;
                if (mstEdgeSet.has(i + ',' + j)) continue;  // tree edge

                // BFS in MST from i to j
                const parent = new Array(n).fill(-1);
                const visited = new Array(n).fill(false);
                const queue = [i];
                visited[i] = true;
                while (queue.length > 0 && !visited[j]) {
                    const v = queue.shift();
                    for (const u of mstAdj[v]) {
                        if (!visited[u]) {
                            visited[u] = true;
                            parent[u] = v;
                            queue.push(u);
                        }
                    }
                }

                // Reconstruct path i → j
                const path = [];
                for (let v = j; v !== -1; v = parent[v]) path.push(v);
                path.reverse();

                // Build cycle: close the loop
                const vertices = [...path, path[0]];

                // Collect edges: tree path edges + non-tree closing edge
                const edges = [];
                for (let k = 0; k + 1 < path.length; k++) {
                    const a = path[k], b = path[k + 1];
                    edges.push([Math.min(a, b), Math.max(a, b)]);
                }
                edges.push([Math.min(i, j), Math.max(i, j)]);

                result.cycles.push({ vertices, edges, cotreeEdge: [i, j] });
            }
        }

        return result;
    },

    symmetricDifference(result, selectedIndices) {
        // Count how many selected cycles contain each edge
        const freq = new Map();
        for (const idx of selectedIndices) {
            if (idx < 0 || idx >= result.cycles.length) continue;
            for (const [u, v] of result.cycles[idx].edges) {
                const key = u + ',' + v;
                freq.set(key, (freq.get(key) || 0) + 1);
            }
        }

        // Keep edges with odd frequency
        const symDiff = [];
        for (const [key, cnt] of freq) {
            if (cnt % 2 === 1) {
                const parts = key.split(',');
                symDiff.push([parseInt(parts[0], 10), parseInt(parts[1], 10)]);
            }
        }

        symDiff.sort((a, b) => a[0] !== b[0] ? a[0] - b[0] : a[1] - b[1]);
        return symDiff;
    }
};
