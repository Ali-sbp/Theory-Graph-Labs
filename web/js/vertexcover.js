"use strict";

window.VertexCover = {
    solve(adjIn) {
        const n = adjIn.length;
        const result = { cover: [], pickedEdges: [], removedEdges: [] };
        if (n <= 0) return result;

        // Working copy: symmetrize
        const work = Array.from({ length: n }, (_, i) =>
            Array.from({ length: n }, (__, j) =>
                (i !== j && (adjIn[i][j] || adjIn[j][i])) ? 1 : 0
            )
        );
        // Ensure only upper-triangle drives iteration (already symmetric)
        for (let i = 0; i < n; i++) work[i][i] = 0;

        const inCover = new Array(n).fill(false);

        while (true) {
            let u = -1, v = -1;
            outer: for (let i = 0; i < n; i++) {
                for (let j = i + 1; j < n; j++) {
                    if (work[i][j]) { u = i; v = j; break outer; }
                }
            }
            if (u === -1) break;

            inCover[u] = inCover[v] = true;
            result.pickedEdges.push([u, v]);

            const removedSet = new Set();
            for (let k = 0; k < n; k++) {
                if (k === u || k === v) continue;
                if (work[u][k]) removedSet.add(Math.min(u, k) + ',' + Math.max(u, k));
                if (work[v][k]) removedSet.add(Math.min(v, k) + ',' + Math.max(v, k));
            }
            removedSet.forEach(s => result.removedEdges.push(s.split(',').map(Number)));

            for (let k = 0; k < n; k++) {
                work[u][k] = work[k][u] = 0;
                work[v][k] = work[k][v] = 0;
            }
        }

        for (let i = 0; i < n; i++)
            if (inCover[i]) result.cover.push(i);

        return result;
    }
};
