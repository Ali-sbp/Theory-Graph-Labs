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

      // Eccentricity = max distance to reachable vertices (hops, not weights)
      let maxDist = 0;
      for (let v = 0; v < n; v++) {
        if (v !== src && dist[v] !== -1) {
          if (dist[v] > maxDist) {
            maxDist = dist[v];
          }
        }
      }

      eccentricities[src] = maxDist;
    }

    // Diameter: max eccentricity
    let diameter = 0;
    for (let i = 0; i < n; i++) {
      if (eccentricities[i] > diameter) {
        diameter = eccentricities[i];
      }
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
    for (let i = 0; i < n; i++) {
      if (eccentricities[i] === diameter) {
        diametralVertices.push(i);
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
