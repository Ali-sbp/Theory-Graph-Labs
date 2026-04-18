"use strict";

window.MSTPrim = {
    solve(adjUndirected, weightUndirected) {
        const n = adjUndirected.length;
        const PRIM_INF = Number.MAX_SAFE_INTEGER;
        const result = {
            edges: [], weights: [], totalWeight: 0, connected: true,
            pruferCode: [], pruferWeights: [],
            decodedEdges: [], decodedWeights: [], roundTripOk: false
        };
        if (n <= 0) return result;
        if (n === 1) return result;

        const dist   = new Array(n).fill(PRIM_INF);
        const parent = new Array(n).fill(-1);
        const inMST  = new Array(n).fill(false);
        dist[0] = 0;

        let reached = 0;
        for (let step = 0; step < n; step++) {
            let u = -1;
            for (let v = 0; v < n; v++)
                if (!inMST[v] && (u === -1 || dist[v] < dist[u]))
                    u = v;

            if (u === -1 || dist[u] === PRIM_INF) break;
            inMST[u] = true;
            reached++;

            if (parent[u] !== -1) {
                const p = parent[u];
                const lo = Math.min(p, u), hi = Math.max(p, u);
                result.edges.push([lo, hi]);
                result.weights.push(weightUndirected[p][u]);
                result.totalWeight += weightUndirected[p][u];
            }

            for (let v = 0; v < n; v++) {
                if (!inMST[v] && adjUndirected[u][v] && weightUndirected[u][v] < dist[v]) {
                    dist[v] = weightUndirected[u][v];
                    parent[v] = u;
                }
            }
        }

        result.connected = (reached === n);
        if (!result.connected) return result;

        this._pruferEncode(result, n);
        this._pruferDecode(result, n);
        return result;
    },

    _pruferEncode(res, n) {
        if (n <= 1) return;

        // Build MST adjacency + weight (n×n)
        const mstAdj = Array.from({ length: n }, () => new Array(n).fill(0));
        const mstW   = Array.from({ length: n }, () => new Array(n).fill(0));
        const deg    = new Array(n).fill(0);

        for (let k = 0; k < res.edges.length; k++) {
            const [u, v] = res.edges[k];
            const w = res.weights[k];
            mstAdj[u][v] = mstAdj[v][u] = 1;
            mstW[u][v]   = mstW[v][u]   = w;
            deg[u]++;
            deg[v]++;
        }

        if (n === 2) {
            if (res.weights.length > 0) res.pruferWeights.push(res.weights[0]);
            return;
        }

        for (let step = 0; step < n - 2; step++) {
            let leaf = -1;
            for (let v = 0; v < n; v++) { if (deg[v] === 1) { leaf = v; break; } }
            if (leaf === -1) break;

            let neighbor = -1;
            for (let v = 0; v < n; v++) { if (mstAdj[leaf][v]) { neighbor = v; break; } }
            if (neighbor === -1) break;

            res.pruferCode.push(neighbor);
            res.pruferWeights.push(mstW[leaf][neighbor]);

            mstAdj[leaf][neighbor] = mstAdj[neighbor][leaf] = 0;
            deg[leaf]--;
            deg[neighbor]--;
        }

        // Last two vertices with deg === 1
        let u = -1, v = -1;
        for (let i = 0; i < n; i++) {
            if (deg[i] === 1) { if (u === -1) u = i; else { v = i; break; } }
        }
        res.pruferWeights.push((u !== -1 && v !== -1) ? mstW[u][v] : 0);
    },

    _pruferDecode(res, n) {
        if (n <= 1) return;
        const A = res.pruferCode;
        const W = res.pruferWeights;

        if (n === 2) {
            res.decodedEdges.push([0, 1]);
            res.decodedWeights.push(W.length > 0 ? W[0] : 0);
        } else {
            const deg = new Array(n).fill(1);
            for (const x of A) deg[x]++;

            for (let i = 0; i < n - 2; i++) {
                let leaf = -1;
                for (let v = 0; v < n; v++) { if (deg[v] === 1) { leaf = v; break; } }
                if (leaf === -1) break;
                const neighbor = A[i];
                const w = i < W.length ? W[i] : 0;
                res.decodedEdges.push([Math.min(leaf, neighbor), Math.max(leaf, neighbor)]);
                res.decodedWeights.push(w);
                deg[leaf]--;
                deg[neighbor]--;
            }

            let u = -1, v = -1;
            for (let i = 0; i < n; i++) {
                if (deg[i] === 1) { if (u === -1) u = i; else { v = i; break; } }
            }
            if (u !== -1 && v !== -1) {
                const lastW = W.length >= n - 1 ? W[n - 2] : 0;
                res.decodedEdges.push([Math.min(u, v), Math.max(u, v)]);
                res.decodedWeights.push(lastW);
            }
        }

        // Round-trip check
        const toKey = (e, w) => e[0] + ',' + e[1] + ',' + w;
        const origSet  = new Set(res.edges.map((e, k) => toKey([Math.min(e[0],e[1]), Math.max(e[0],e[1])], res.weights[k])));
        const decoSet  = new Set(res.decodedEdges.map((e, k) => toKey(e, res.decodedWeights[k])));
        res.roundTripOk = (origSet.size === decoSet.size) &&
            [...origSet].every(k => decoSet.has(k));
    }
};
