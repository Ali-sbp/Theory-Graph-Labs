window.GraphGenerator = {
  generate(n, degreeP, directed, weightType, weightP) {
    if (n < 2) {
      return {
        n: n,
        directed: directed,
        adjMatrix: n === 1 ? [[0]] : [],
        weightMatrix: n === 1 ? [[0]] : []
      };
    }

    const degreeDist = new window.FarryDistribution(degreeP);

    // Generate degree sequence, clamp to [1, n-1]
    const degrees = new Array(n);
    for (let i = 0; i < n; i++) {
      let d = degreeDist.generate();
      degrees[i] = Math.max(1, Math.min(n - 1, d));
    }

    // Adjust sum to exactly 2*(n-1) for a tree
    const targetSum = 2 * (n - 1);
    let sum = 0;
    for (let i = 0; i < n; i++) sum += degrees[i];

    while (sum > targetSum) {
      const idx = Math.floor(Math.random() * n);
      if (degrees[idx] > 1) {
        degrees[idx]--;
        sum--;
      }
    }
    while (sum < targetSum) {
      const idx = Math.floor(Math.random() * n);
      if (degrees[idx] < n - 1) {
        degrees[idx]++;
        sum++;
      }
    }

    // Build Prufer sequence: vertex i appears (degrees[i] - 1) times
    const prufer = [];
    for (let i = 0; i < n; i++) {
      for (let j = 0; j < degrees[i] - 1; j++) {
        prufer.push(i);
      }
    }

    // Prufer sequence length should be n-2
    // Adjust if needed
    while (prufer.length < n - 2) {
      const idx = Math.floor(Math.random() * n);
      prufer.push(idx);
      degrees[idx]++;
    }
    while (prufer.length > n - 2) {
      const removeIdx = Math.floor(Math.random() * prufer.length);
      degrees[prufer[removeIdx]]--;
      prufer.splice(removeIdx, 1);
    }

    // Fisher-Yates shuffle
    for (let i = prufer.length - 1; i > 0; i--) {
      const j = Math.floor(Math.random() * (i + 1));
      const tmp = prufer[i];
      prufer[i] = prufer[j];
      prufer[j] = tmp;
    }

    // Decode Prufer sequence to tree edges
    const deg = new Array(n).fill(1);
    for (let i = 0; i < prufer.length; i++) {
      deg[prufer[i]]++;
    }

    const adjMatrix = [];
    for (let i = 0; i < n; i++) {
      adjMatrix.push(new Array(n).fill(0));
    }

    for (let i = 0; i < prufer.length; i++) {
      // Find smallest leaf (degree == 1)
      let leaf = -1;
      for (let j = 0; j < n; j++) {
        if (deg[j] === 1) {
          leaf = j;
          break;
        }
      }
      const node = prufer[i];
      adjMatrix[leaf][node] = 1;
      adjMatrix[node][leaf] = 1;
      deg[leaf]--;
      deg[node]--;
    }

    // Connect the last two remaining vertices with degree 1
    const remaining = [];
    for (let i = 0; i < n; i++) {
      if (deg[i] === 1) remaining.push(i);
    }
    if (remaining.length === 2) {
      adjMatrix[remaining[0]][remaining[1]] = 1;
      adjMatrix[remaining[1]][remaining[0]] = 1;
    }

    // For directed graphs: randomly orient each edge
    if (directed) {
      for (let i = 0; i < n; i++) {
        for (let j = i + 1; j < n; j++) {
          if (adjMatrix[i][j] === 1) {
            if (Math.random() < 0.5) {
              // Keep i -> j, remove j -> i
              adjMatrix[j][i] = 0;
            } else {
              // Keep j -> i, remove i -> j
              adjMatrix[i][j] = 0;
            }
          }
        }
      }
    }

    // Generate weight matrix
    const weightDist = new window.FarryDistribution(weightP);
    const weightMatrix = [];
    for (let i = 0; i < n; i++) {
      weightMatrix.push(new Array(n).fill(0));
    }

    for (let i = 0; i < n; i++) {
      for (let j = 0; j < n; j++) {
        if (adjMatrix[i][j] !== 0) {
          let w = weightDist.generate();
          if (weightType === "negative") {
            w = -w;
          } else if (weightType === "mixed") {
            if (Math.random() < 0.5) w = -w;
          }
          weightMatrix[i][j] = w;
          // For undirected graphs, mirror the weight
          if (!directed && adjMatrix[j][i] !== 0) {
            weightMatrix[j][i] = w;
          }
        }
      }
    }

    return {
      n: n,
      directed: directed,
      adjMatrix: adjMatrix,
      weightMatrix: weightMatrix
    };
  },

  regenerateWeights(graphData, weightType, weightP) {
    const n = graphData.n;
    if (n <= 1) return;

    const weightDist = new window.FarryDistribution(weightP);
    const wm = [];
    for (let i = 0; i < n; i++) {
      wm.push(new Array(n).fill(0));
    }

    for (let i = 0; i < n; i++) {
      for (let j = 0; j < n; j++) {
        if (graphData.adjMatrix[i][j] !== 0) {
          let w = weightDist.generate();
          if (weightType === 'negative') {
            w = -w;
          } else if (weightType === 'mixed') {
            if (Math.random() < 0.5) w = -w;
          }
          wm[i][j] = w;
          if (!graphData.directed && graphData.adjMatrix[j][i] !== 0) {
            wm[j][i] = w;
          }
        }
      }
    }

    graphData.weightMatrix = wm;
  }
};
