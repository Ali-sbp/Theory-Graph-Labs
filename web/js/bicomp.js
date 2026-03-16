window.BiComp = {
  find(adjMatrix) {
    var n = adjMatrix.length;
    var articulationPoints = new Set();
    var blocks = [];
    var iterations = 0;

    if (n === 0) {
      return { articulationPoints: articulationPoints, blocks: blocks, iterations: 0 };
    }

    // Build adjacency list from matrix
    var adj = [];
    for (var i = 0; i < n; i++) {
      adj.push([]);
      for (var j = 0; j < n; j++) {
        if (adjMatrix[i][j] !== 0) {
          adj[i].push(j);
        }
      }
    }

    var num = new Array(n).fill(-1);
    var low = new Array(n).fill(-1);
    var father = new Array(n).fill(-1);
    var timer = 0;

    // Edge stack: each entry is [u, v] with u < v to avoid duplicates
    var edgeStack = [];

    function dfs(v, parent) {
      num[v] = timer;
      low[v] = timer;
      timer++;
      iterations++;

      var childCount = 0;

      for (var idx = 0; idx < adj[v].length; idx++) {
        var u = adj[v][idx];
        iterations++;

        if (num[u] === -1) {
          // Tree edge
          childCount++;
          father[u] = v;

          // Push edge to stack
          var edgeA = Math.min(v, u);
          var edgeB = Math.max(v, u);
          edgeStack.push([edgeA, edgeB]);

          dfs(u, v);

          // Update low
          if (low[u] < low[v]) {
            low[v] = low[u];
          }

          // Check articulation point condition
          if ((parent === -1 && childCount >= 2) ||
              (parent !== -1 && low[u] >= num[v])) {
            articulationPoints.add(v);
          }

          // Extract biconnected component if low[u] >= num[v]
          if (low[u] >= num[v]) {
            var block = [];
            while (edgeStack.length > 0) {
              var top = edgeStack[edgeStack.length - 1];
              edgeStack.pop();
              block.push(top);
              if (top[0] === edgeA && top[1] === edgeB) {
                break;
              }
            }
            blocks.push(block);
          }
        } else if (u !== parent && num[u] < num[v]) {
          // Back edge (only process once: when num[u] < num[v])
          var edgeA = Math.min(v, u);
          var edgeB = Math.max(v, u);
          edgeStack.push([edgeA, edgeB]);

          // Update low
          if (num[u] < low[v]) {
            low[v] = num[u];
          }
        }
      }
    }

    // Handle disconnected components
    for (var i = 0; i < n; i++) {
      if (num[i] === -1) {
        dfs(i, -1);
      }
    }

    return {
      articulationPoints: articulationPoints,
      blocks: blocks,
      iterations: iterations
    };
  }
};
