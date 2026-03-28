"use strict";

window.MaxFlow = {
    solve(capacityMatrix, source, sink) {
        const n = capacityMatrix.length;
        const flowMatrix = Array.from({ length: n }, () => new Array(n).fill(0));
        const residual = capacityMatrix.map(row => row.slice());
        const augmentingPaths = [];
        const pathFlows = [];
        let maxFlow = 0;
        let iterations = 0;

        while (true) {
            // BFS to find shortest augmenting path
            const parent = new Array(n).fill(-1);
            const visited = new Array(n).fill(false);
            const queue = [source];
            visited[source] = true;
            let found = false;

            while (queue.length > 0 && !found) {
                const u = queue.shift();
                for (let v = 0; v < n; v++) {
                    if (!visited[v] && residual[u][v] > 0) {
                        visited[v] = true;
                        parent[v] = u;
                        if (v === sink) {
                            found = true;
                            break;
                        }
                        queue.push(v);
                    }
                }
            }

            if (!found) break;

            // Find bottleneck
            let pathFlow = Infinity;
            for (let v = sink; v !== source; v = parent[v]) {
                const u = parent[v];
                pathFlow = Math.min(pathFlow, residual[u][v]);
            }

            // Record augmenting path
            const path = [];
            for (let v = sink; v !== -1; v = parent[v])
                path.push(v);
            path.reverse();
            augmentingPaths.push(path);
            pathFlows.push(pathFlow);

            // Update residual graph and flow
            for (let v = sink; v !== source; v = parent[v]) {
                const u = parent[v];
                residual[u][v] -= pathFlow;
                residual[v][u] += pathFlow;
                flowMatrix[u][v] += pathFlow;
                flowMatrix[v][u] -= pathFlow;
            }

            maxFlow += pathFlow;
            iterations++;
        }

        // Clean up negative flow values
        for (let i = 0; i < n; i++)
            for (let j = 0; j < n; j++)
                if (flowMatrix[i][j] < 0)
                    flowMatrix[i][j] = 0;

        return { maxFlow, flowMatrix, iterations, augmentingPaths, pathFlows };
    }
};
