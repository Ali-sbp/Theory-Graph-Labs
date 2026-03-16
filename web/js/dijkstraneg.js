window.DijkstraNeg = (function () {
  // Binary min-heap storing [distance, vertex] pairs
  function MinHeap() {
    this.data = [];
  }

  MinHeap.prototype.push = function (item) {
    this.data.push(item);
    this._bubbleUp(this.data.length - 1);
  };

  MinHeap.prototype.pop = function () {
    var top = this.data[0];
    var last = this.data.pop();
    if (this.data.length > 0) {
      this.data[0] = last;
      this._sinkDown(0);
    }
    return top;
  };

  MinHeap.prototype.size = function () {
    return this.data.length;
  };

  MinHeap.prototype._bubbleUp = function (i) {
    while (i > 0) {
      var parentIdx = (i - 1) >> 1;
      if (this.data[i][0] < this.data[parentIdx][0]) {
        var tmp = this.data[i];
        this.data[i] = this.data[parentIdx];
        this.data[parentIdx] = tmp;
        i = parentIdx;
      } else {
        break;
      }
    }
  };

  MinHeap.prototype._sinkDown = function (i) {
    var length = this.data.length;
    while (true) {
      var left = 2 * i + 1;
      var right = 2 * i + 2;
      var smallest = i;

      if (left < length && this.data[left][0] < this.data[smallest][0]) {
        smallest = left;
      }
      if (right < length && this.data[right][0] < this.data[smallest][0]) {
        smallest = right;
      }
      if (smallest !== i) {
        var tmp = this.data[i];
        this.data[i] = this.data[smallest];
        this.data[smallest] = tmp;
        i = smallest;
      } else {
        break;
      }
    }
  };

  var INF = 1e9;

  function solve(weightMatrix, source, directed) {
    var n = weightMatrix.length;
    var dist = new Array(n).fill(INF);
    var parent = new Array(n).fill(-1);
    var relaxCount = new Array(n).fill(0);
    var iterations = 0;
    var hasNegativeCycle = false;

    dist[source] = 0;

    var pq = new MinHeap();
    pq.push([0, source]);

    while (pq.size() > 0) {
      var top = pq.pop();
      var d = top[0];
      var u = top[1];
      iterations++;

      // Skip stale entries
      if (d > dist[u]) {
        continue;
      }

      relaxCount[u]++;
      if (relaxCount[u] > n) {
        hasNegativeCycle = true;
        break;
      }

      for (var v = 0; v < n; v++) {
        if (weightMatrix[u][v] === 0) continue;

        // Anti-bounce for undirected graphs: skip going back to parent
        if (!directed && v === parent[u]) continue;

        var newDist = dist[u] + weightMatrix[u][v];
        if (newDist < dist[v]) {
          dist[v] = newDist;
          parent[v] = u;
          pq.push([newDist, v]);
        }
      }
    }

    return {
      dist: dist,
      parent: parent,
      hasNegativeCycle: hasNegativeCycle,
      iterations: iterations
    };
  }

  function reconstructPath(parent, source, target) {
    var n = parent.length;
    if (parent[target] === -1 && target !== source) {
      return [];
    }

    var path = [];
    var cur = target;
    var steps = 0;

    while (cur !== -1 && steps <= n) {
      path.push(cur);
      if (cur === source) break;
      cur = parent[cur];
      steps++;
    }

    // Guard against cycles in parent chain
    if (steps > n) {
      return [];
    }

    if (path[path.length - 1] !== source) {
      return [];
    }

    path.reverse();
    return path;
  }

  return {
    MinHeap: MinHeap,
    solve: solve,
    reconstructPath: reconstructPath
  };
})();
