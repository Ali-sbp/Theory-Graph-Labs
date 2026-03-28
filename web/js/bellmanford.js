"use strict";

window.BellmanFord = {
    INF: Math.floor(Number.MAX_SAFE_INTEGER / 2),

    solve(costMatrix, capacityMatrix, source) {
        const n = costMatrix.length;
        const dist = new Array(n).fill(this.INF);
        const parent = new Array(n).fill(-1);
        dist[source] = 0;
        let iterations = 0;
        let hasNegativeCycle = false;

        // Relax edges n-1 times
        for (let iter = 0; iter < n - 1; iter++) {
            let updated = false;
            for (let u = 0; u < n; u++) {
                if (dist[u] >= this.INF) continue;
                for (let v = 0; v < n; v++) {
                    if (capacityMatrix[u][v] > 0) {
                        const newDist = dist[u] + costMatrix[u][v];
                        if (newDist < dist[v]) {
                            dist[v] = newDist;
                            parent[v] = u;
                            updated = true;
                        }
                    }
                }
            }
            iterations++;
            if (!updated) break;
        }

        // Check for negative cycles
        for (let u = 0; u < n && !hasNegativeCycle; u++) {
            if (dist[u] >= this.INF) continue;
            for (let v = 0; v < n; v++) {
                if (capacityMatrix[u][v] > 0 &&
                    dist[u] + costMatrix[u][v] < dist[v]) {
                    hasNegativeCycle = true;
                    break;
                }
            }
        }

        return { dist, parent, hasNegativeCycle, iterations };
    },

    reconstructPath(parent, source, target) {
        if (parent[target] === -1 && target !== source) return [];
        const path = [];
        for (let v = target; v !== -1; v = parent[v])
            path.push(v);
        path.reverse();
        return path;
    }
};
