"use strict";

window.SpanningTreeCount = {
    solve(adjUndirected) {
        const n = adjUndirected.length;
        const result = { count: 0, laplacian: [], cofactor: [] };
        if (n <= 0) return result;

        // Build combinatorial Laplacian L = D - A
        result.laplacian = Array.from({ length: n }, () => new Array(n).fill(0));
        for (let i = 0; i < n; i++) {
            for (let j = 0; j < n; j++) {
                if (i !== j && adjUndirected[i][j]) {
                    result.laplacian[i][j] = -1;
                    result.laplacian[i][i] += 1;
                }
            }
        }

        if (n === 1) { result.count = 1; return result; }

        // Cofactor: delete last row + last column
        const m = n - 1;
        result.cofactor = Array.from({ length: m }, (_, i) =>
            Array.from({ length: m }, (__, j) => result.laplacian[i][j])
        );

        if (n === 2) {
            result.count = result.cofactor[0][0];
            return result;
        }

        // Gaussian elimination on floating-point copy
        const mat = result.cofactor.map(row => row.map(v => v));
        let det = 1;
        for (let col = 0; col < m; col++) {
            // Partial pivot
            let pivotRow = -1, best = 0;
            for (let row = col; row < m; row++) {
                if (Math.abs(mat[row][col]) > best) {
                    best = Math.abs(mat[row][col]);
                    pivotRow = row;
                }
            }
            if (pivotRow < 0 || best < 1e-9) { result.count = 0; return result; }
            if (pivotRow !== col) {
                [mat[pivotRow], mat[col]] = [mat[col], mat[pivotRow]];
                det = -det;
            }
            det *= mat[col][col];
            for (let row = col + 1; row < m; row++) {
                const factor = mat[row][col] / mat[col][col];
                for (let k = col; k < m; k++)
                    mat[row][k] -= factor * mat[col][k];
            }
        }

        const rounded = Math.round(det);
        result.count = rounded < 0 ? 0 : rounded;
        return result;
    }
};
