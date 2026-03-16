window.Shimbel = (function () {
  var POS_INF = 1e9;
  var NEG_INF = -1e9;

  function minPlusMul(A, B, n) {
    var C = [];
    for (var i = 0; i < n; i++) {
      C.push(new Array(n).fill(POS_INF));
    }
    for (var i = 0; i < n; i++) {
      for (var j = 0; j < n; j++) {
        for (var m = 0; m < n; m++) {
          if (A[i][m] !== POS_INF && B[m][j] !== POS_INF) {
            var s = A[i][m] + B[m][j];
            if (s < C[i][j]) {
              C[i][j] = s;
            }
          }
        }
      }
    }
    return C;
  }

  function maxPlusMul(A, B, n) {
    var C = [];
    for (var i = 0; i < n; i++) {
      C.push(new Array(n).fill(NEG_INF));
    }
    for (var i = 0; i < n; i++) {
      for (var j = 0; j < n; j++) {
        for (var m = 0; m < n; m++) {
          if (A[i][m] !== NEG_INF && B[m][j] !== NEG_INF) {
            var s = A[i][m] + B[m][j];
            if (s > C[i][j]) {
              C[i][j] = s;
            }
          }
        }
      }
    }
    return C;
  }

  function minPath(weightMatrix, k) {
    var n = weightMatrix.length;
    // Build initial matrix W: 0 entries become POS_INF, others kept
    var W = [];
    for (var i = 0; i < n; i++) {
      W.push(new Array(n));
      for (var j = 0; j < n; j++) {
        W[i][j] = (weightMatrix[i][j] === 0) ? POS_INF : weightMatrix[i][j];
      }
    }

    // Multiply k-1 times: result = W^k in min-plus semiring
    var result = W;
    for (var step = 1; step < k; step++) {
      result = minPlusMul(result, W, n);
    }
    return result;
  }

  function maxPath(weightMatrix, k) {
    var n = weightMatrix.length;
    // Build initial matrix W: 0 entries become NEG_INF, others kept
    var W = [];
    for (var i = 0; i < n; i++) {
      W.push(new Array(n));
      for (var j = 0; j < n; j++) {
        W[i][j] = (weightMatrix[i][j] === 0) ? NEG_INF : weightMatrix[i][j];
      }
    }

    // Multiply k-1 times: result = W^k in max-plus semiring
    var result = W;
    for (var step = 1; step < k; step++) {
      result = maxPlusMul(result, W, n);
    }
    return result;
  }

  return {
    POS_INF: POS_INF,
    NEG_INF: NEG_INF,
    minPlusMul: minPlusMul,
    maxPlusMul: maxPlusMul,
    minPath: minPath,
    maxPath: maxPath
  };
})();
