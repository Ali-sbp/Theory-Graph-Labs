window.RouteSearch = {
  findRoute(adjMatrix, from, to) {
    var n = adjMatrix.length;

    if (from < 0 || from >= n || to < 0 || to >= n) {
      return { exists: false, count: 0, path: [] };
    }

    if (from === to) {
      return { exists: true, count: 1, path: [from] };
    }

    // BFS for shortest path
    var parent = new Array(n).fill(-1);
    var visited = new Array(n).fill(false);
    var queue = [from];
    visited[from] = true;
    var found = false;
    var head = 0;

    while (head < queue.length) {
      var u = queue[head++];
      if (u === to) {
        found = true;
        break;
      }
      for (var v = 0; v < n; v++) {
        if (adjMatrix[u][v] !== 0 && !visited[v]) {
          visited[v] = true;
          parent[v] = u;
          queue.push(v);
        }
      }
    }

    // Reconstruct shortest path
    var path = [];
    if (found) {
      var cur = to;
      while (cur !== -1) {
        path.push(cur);
        cur = parent[cur];
      }
      path.reverse();
    }

    // DFS to count ALL simple paths (recursive backtracking)
    var count = 0;
    var inPath = new Array(n).fill(false);

    function dfs(u) {
      if (u === to) {
        count++;
        return;
      }
      for (var v = 0; v < n; v++) {
        if (adjMatrix[u][v] !== 0 && !inPath[v]) {
          inPath[v] = true;
          dfs(v);
          inPath[v] = false;
        }
      }
    }

    inPath[from] = true;
    dfs(from);

    return {
      exists: found,
      count: count,
      path: path
    };
  }
};
