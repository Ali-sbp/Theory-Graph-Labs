window.GraphAnalysis = {
  analyze(adjMatrix) {
    const n = adjMatrix.length;
    if (n === 0) {
      return { eccentricities: [], center: [], diameter: 0, diametralVertices: [] };
    }

    const eccentricities = new Array(n);

    // BFS from each vertex
    for (let src = 0; src < n; src++) {
      const dist = new Array(n).fill(-1);
      dist[src] = 0;
      const queue = [src];
      let head = 0;

      while (head < queue.length) {
        const u = queue[head++];
        for (let v = 0; v < n; v++) {
          if (adjMatrix[u][v] !== 0 && dist[v] === -1) {
            dist[v] = dist[u] + 1;
            queue.push(v);
          }
        }
      }

      let maxDist = 0;
      let allReachable = true;
      for (let v = 0; v < n; v++) {
        if (v !== src) {
          if (dist[v] === -1) {
            allReachable = false;
            break;
          }
          if (dist[v] > maxDist) {
            maxDist = dist[v];
          }
        }
      }

      eccentricities[src] = allReachable ? maxDist : Infinity;
    }

    // Diameter: max finite eccentricity, or Infinity if all are infinite
    let diameter = 0;
    let hasFinite = false;
    for (let i = 0; i < n; i++) {
      if (eccentricities[i] !== Infinity) {
        hasFinite = true;
        if (eccentricities[i] > diameter) {
          diameter = eccentricities[i];
        }
      }
    }
    if (!hasFinite) {
      diameter = Infinity;
    }

    // Center: vertices with minimum eccentricity
    let minEcc = Infinity;
    for (let i = 0; i < n; i++) {
      if (eccentricities[i] < minEcc) {
        minEcc = eccentricities[i];
      }
    }
    const center = [];
    for (let i = 0; i < n; i++) {
      if (eccentricities[i] === minEcc) {
        center.push(i);
      }
    }

    // Diametral vertices: vertices whose eccentricity equals the diameter
    const diametralVertices = [];
    if (diameter !== Infinity) {
      for (let i = 0; i < n; i++) {
        if (eccentricities[i] === diameter) {
          diametralVertices.push(i);
        }
      }
    }

    return {
      eccentricities: eccentricities,
      center: center,
      diameter: diameter,
      diametralVertices: diametralVertices
    };
  }
};
