"use strict";

window.MinCostFlow = {
    solve(capacityMatrix, costMatrix, source, sink, desiredFlow) {
        const n = capacityMatrix.length;
        const flowMatrix = Array.from({ length: n }, () => new Array(n).fill(0));
        const augmentingPaths = [];
        const pathFlows = [];
        const pathCosts = [];
        let iterations = 0;

        if (desiredFlow <= 0) {
            return {
                flowValue: 0, totalCost: 0, flowMatrix,
                iterations: 0, feasible: true,
                augmentingPaths, pathFlows, pathCosts
            };
        }

        // Build residual capacity and cost graphs
        const resCap = Array.from({ length: n }, () => new Array(n).fill(0));
        const resCost = Array.from({ length: n }, () => new Array(n).fill(0));

        for (let i = 0; i < n; i++) {
            for (let j = 0; j < n; j++) {
                if (capacityMatrix[i][j] > 0) {
                    resCap[i][j] = capacityMatrix[i][j];
                    resCost[i][j] = costMatrix[i][j];
                    resCost[j][i] = -costMatrix[i][j];
                }
            }
        }

        let totalFlow = 0;
        let totalCost = 0;

        while (totalFlow < desiredFlow) {
            // Find shortest cost path using Bellman-Ford
            const bf = BellmanFord.solve(resCost, resCap, source);

            if (bf.dist[sink] >= BellmanFord.INF) break;

            const path = BellmanFord.reconstructPath(bf.parent, source, sink);
            if (path.length === 0) break;

            // Find bottleneck, capped at remaining needed flow
            let pathFlow = desiredFlow - totalFlow;
            for (let k = 0; k + 1 < path.length; k++) {
                const u = path[k], v = path[k + 1];
                pathFlow = Math.min(pathFlow, resCap[u][v]);
            }

            if (pathFlow <= 0) break;

            // Update residual graph and flow matrix
            for (let k = 0; k + 1 < path.length; k++) {
                const u = path[k], v = path[k + 1];
                resCap[u][v] -= pathFlow;
                resCap[v][u] += pathFlow;
                flowMatrix[u][v] += pathFlow;
                flowMatrix[v][u] -= pathFlow;
            }

            totalFlow += pathFlow;
            totalCost += pathFlow * bf.dist[sink];

            augmentingPaths.push(path);
            pathFlows.push(pathFlow);
            pathCosts.push(bf.dist[sink]);
            iterations++;
        }

        // Clean up negative flow values
        for (let i = 0; i < n; i++)
            for (let j = 0; j < n; j++)
                if (flowMatrix[i][j] < 0)
                    flowMatrix[i][j] = 0;

        return {
            flowValue: totalFlow,
            totalCost,
            flowMatrix,
            iterations,
            feasible: totalFlow >= desiredFlow,
            augmentingPaths,
            pathFlows,
            pathCosts
        };
    }
};
