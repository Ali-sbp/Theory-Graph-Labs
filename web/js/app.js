// app.js — Main controller: tab switching, event handlers
// Ports mainwindow.cpp slot logic to JavaScript

(function () {
  'use strict';

  // ---- State ----
  let graph = null;   // { n, directed, adjMatrix, weightMatrix }
  let hasGraph = false;
  let graphCanvas = null;

  // Original directed matrices (for directed/undirected rearrangement)
  let originalAdjMatrix = null;
  let originalWeightMatrix = null;

  // Flow network state (Tab 6)
  let capacityMatrix = null;
  let costMatrix = null;
  let hasFlowNetwork = false;
  let lastMaxFlow = 0;
  let lastMaxFlowResult = null;

  // ---- Helpers ----
  function $(id) { return document.getElementById(id); }
  function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)); }
  function intVal(id, fallback) { var v = parseInt($(id).value, 10); return isNaN(v) ? fallback : v; }
  function floatVal(id, fallback) { var v = parseFloat($(id).value); return isNaN(v) ? fallback : v; }

  function setInputRange(id, min, max) {
    const el = $(id);
    el.min = min;
    el.max = max;
    el.value = clamp(parseInt(el.value, 10) || min, min, max);
  }

  // ---- Tab switching ----
  function setupTabs() {
    const btns = document.querySelectorAll('.tab-btn');
    const panels = document.querySelectorAll('.tab-panel');

    btns.forEach(btn => {
      btn.addEventListener('click', () => {
        const tab = parseInt(btn.dataset.tab, 10);

        btns.forEach(b => b.classList.remove('active'));
        panels.forEach(p => p.classList.remove('active'));

        btn.classList.add('active');
        document.getElementById('panel-' + tab).classList.add('active');

        // Clear highlights on plain-graph tabs (matching onTabChanged)
        if (tab === 0 || tab === 1 || tab === 5 || tab === 6) {
          if (graphCanvas) graphCanvas.clearHighlights();
        }

        // Swap edge labels for flow tab
        if (graphCanvas) {
          if (tab === 6 && hasFlowNetwork && capacityMatrix) {
            graphCanvas.setEdgeLabels(capacityMatrix);
          } else {
            graphCanvas.clearEdgeLabels();
            graphCanvas.clearFlowLabels();
          }
        }
      });
    });
  }

  // ---- Tab 0: Generate ----
  function onGenerate() {
    const n = clamp(intVal('vertexCount', 6), 1, 100);
    const p = clamp(floatVal('paramP', 0.5), 0.01, 0.99);
    const wp = clamp(floatVal('weightParamP', 0.4), 0.01, 0.99);
    const dir = $('directedCheck').checked;
    const wtRadio = document.querySelector('input[name="weightType"]:checked');
    const wt = wtRadio ? wtRadio.value : 'positive';

    graph = GraphGenerator.generate(n, p, dir, wt, wp);
    hasGraph = true;

    // Store original matrices for directed/undirected rearrangement
    originalAdjMatrix = graph.adjMatrix.map(r => r.slice());
    originalWeightMatrix = graph.weightMatrix.map(r => r.slice());

    // Update graph canvas
    graphCanvas.setGraph(graph);

    // Clear stale results
    $('minMatrix').innerHTML = '';
    $('maxMatrix').innerHTML = '';
    $('routeMatrix').innerHTML = '';
    $('routeText').textContent = '';
    $('bicompText').textContent = '';
    $('bicompMatrix').innerHTML = '';
    $('dijkText').textContent = '';
    $('dijkStagesTable').innerHTML = '';
    $('dijkMatrix').innerHTML = '';
    $('cmpText').textContent = '';
    $('cmpTable').innerHTML = '';

    // Clear Tab 6 (Поток)
    $('capacityMatrix').innerHTML = '';
    $('costMatrixDisplay').innerHTML = '';
    $('maxFlowText').textContent = '';
    $('maxFlowMatrix').innerHTML = '';
    $('minCostFlowText').textContent = '';
    $('minCostFlowMatrix').innerHTML = '';
    hasFlowNetwork = false;
    lastMaxFlow = 0;

    // Display matrices
    UIHelpers.displayMatrix('adjMatrix', graph.adjMatrix, 0, 'adjacency');
    UIHelpers.displayMatrix('weightMatrix', graph.weightMatrix, 0, 'weighted');

    // ---- Analysis ----
    const res = GraphAnalysis.analyze(graph.adjMatrix, dir);
    let text = '';

    if (n === 1) {
      text += 'Граф состоит из одной вершины (без рёбер).\n\n';
      text += 'Эксцентриситет: v0 = 0\n';
      text += 'Центр графа: { 0 }\n';
      text += 'Диаметр: 0\n';
      text += 'Диаметральные вершины: { 0 }';
      $('analysisText').textContent = text;
      setInputRange('fromVertex', 0, 0);
      setInputRange('toVertex', 0, 0);
      setInputRange('pathLength', 1, 1);
      return;
    }

    text += 'Эксцентриситеты:\n';
    for (let i = 0; i < n; i++) {
      text += '  v' + i + ' = ' + res.eccentricities[i] + '\n';
    }

    text += '\nЦентр графа: { ' + res.center.join(', ') + ' }\n';

    text += 'Диаметр: ' + res.diameter + '\n';
    text += 'Диаметральные вершины: { ' + res.diametralVertices.join(', ') + ' }';

    $('analysisText').textContent = text;

    // Update ranges on other tabs
    setInputRange('pathLength', 0, Math.max(1, n - 1));
    setInputRange('fromVertex', 0, n - 1);
    setInputRange('toVertex', 0, n - 1);
    setInputRange('dijkSrc', 0, n - 1);
    setInputRange('dijkDst', 0, Math.max(0, n - 1));
    if (n > 1) $('dijkDst').value = 1;

    // Update Tab 6 spin boxes
    setInputRange('flowSrc', 0, Math.max(0, n - 1));
    setInputRange('flowSink', 0, Math.max(0, n - 1));
    if (n > 1) $('flowSink').value = n - 1;

    // Update weight type label on Tab 4
    const labels = { positive: 'Положительные', negative: 'Отрицательные', mixed: 'Смешанные' };
    $('dijkWeightTypeLabel').textContent = 'Тип весов: ' + (labels[wt] || wt);
  }

  // ---- Tab 0: Generate DAG ----
  function onGenerateDAG() {
    const n = clamp(intVal('vertexCount', 6), 1, 100);
    const p = clamp(floatVal('paramP', 0.5), 0.01, 0.99);
    const wp = clamp(floatVal('weightParamP', 0.4), 0.01, 0.99);

    const dir = $('directedCheck').checked;

    const wtRadio = document.querySelector('input[name="weightType"]:checked');
    const wt = wtRadio ? wtRadio.value : 'positive';

    graph = GraphGenerator.generateDAG(n, p, dir, wt, wp);
    hasGraph = true;

    // Store original matrices for directed/undirected rearrangement
    originalAdjMatrix = graph.adjMatrix.map(r => r.slice());
    originalWeightMatrix = graph.weightMatrix.map(r => r.slice());

    graphCanvas.setGraph(graph);

    // Clear stale results (same as onGenerate)
    $('minMatrix').innerHTML = '';
    $('maxMatrix').innerHTML = '';
    $('routeMatrix').innerHTML = '';
    $('routeText').textContent = '';
    $('bicompText').textContent = '';
    $('bicompMatrix').innerHTML = '';
    $('dijkText').textContent = '';
    $('dijkStagesTable').innerHTML = '';
    $('dijkMatrix').innerHTML = '';
    $('cmpText').textContent = '';
    $('cmpTable').innerHTML = '';

    $('capacityMatrix').innerHTML = '';
    $('costMatrixDisplay').innerHTML = '';
    $('maxFlowText').textContent = '';
    $('maxFlowMatrix').innerHTML = '';
    $('minCostFlowText').textContent = '';
    $('minCostFlowMatrix').innerHTML = '';
    hasFlowNetwork = false;
    lastMaxFlow = 0;

    UIHelpers.displayMatrix('adjMatrix', graph.adjMatrix, 0, 'adjacency');
    UIHelpers.displayMatrix('weightMatrix', graph.weightMatrix, 0, 'weighted');

    const res = GraphAnalysis.analyze(graph.adjMatrix, dir);
    let text = '';

    if (n === 1) {
      text += 'Граф состоит из одной вершины (без рёбер).\n\n';
      text += 'Эксцентриситет: v0 = 0\n';
      text += 'Центр графа: { 0 }\n';
      text += 'Диаметр: 0\n';
      text += 'Диаметральные вершины: { 0 }';
      $('analysisText').textContent = text;
      setInputRange('fromVertex', 0, 0);
      setInputRange('toVertex', 0, 0);
      setInputRange('pathLength', 1, 1);
      return;
    }

    text += 'Эксцентриситеты:\n';
    for (let i = 0; i < n; i++) {
      text += '  v' + i + ' = ' + res.eccentricities[i] + '\n';
    }
    text += '\nЦентр графа: { ' + res.center.join(', ') + ' }\n';
    text += 'Диаметр: ' + res.diameter + '\n';
    text += 'Диаметральные вершины: { ' + res.diametralVertices.join(', ') + ' }';
    $('analysisText').textContent = text;

    setInputRange('pathLength', 0, Math.max(1, n - 1));
    setInputRange('fromVertex', 0, n - 1);
    setInputRange('toVertex', 0, n - 1);
    setInputRange('dijkSrc', 0, n - 1);
    setInputRange('dijkDst', 0, Math.max(0, n - 1));
    if (n > 1) $('dijkDst').value = 1;

    setInputRange('flowSrc', 0, Math.max(0, n - 1));
    setInputRange('flowSink', 0, Math.max(0, n - 1));
    if (n > 1) $('flowSink').value = n - 1;

    const labels = { positive: 'Положительные', negative: 'Отрицательные', mixed: 'Смешанные' };
    $('dijkWeightTypeLabel').textContent = 'Тип весов: ' + (labels[wt] || wt);
  }

  // ---- Tab 1: Shimbel ----
  function onShimbelCalculate() {
    if (!hasGraph) { alert('Сначала сгенерируйте граф!'); return; }

    const n = graph.n;
    const k = intVal('pathLength', 2);

    if (n <= 1) { alert('Граф содержит ≤ 1 вершины — метод Шимбелла не применим.'); return; }

    if (k >= n) {
      if (!confirm('Длина пути k = ' + k + ' ≥ кол-ва вершин n = ' + n +
        '.\nДля дерева из ' + n + ' вершин маршрутов такой длины скорее всего нет.\nПродолжить?'))
        return;
    }

    const minRes = Shimbel.minPath(graph.weightMatrix, k);
    const maxRes = Shimbel.maxPath(graph.weightMatrix, k);

    UIHelpers.displayMatrix('minMatrix', minRes, Shimbel.POS_INF);
    UIHelpers.displayMatrix('maxMatrix', maxRes, Shimbel.NEG_INF);
  }

  // ---- Tab 2: Route Search ----
  function onFindRoute() {
    if (!hasGraph) { alert('Сначала сгенерируйте граф!'); return; }

    const from = intVal('fromVertex', 0);
    const to = intVal('toVertex', 0);

    if (from === to) {
      $('routeText').textContent =
        'Вершина ' + from + ' = вершина ' + to + ' — тривиальный маршрут (длина 0, 1 путь).';
      UIHelpers.displayMatrix('routeMatrix', graph.adjMatrix, 0, 'adjacency');
      graphCanvas.highlightRoute([from]);
      return;
    }

    const res = RouteSearch.findRoute(graph.adjMatrix, from, to);
    let text = '';

    if (res.exists) {
      text += 'Маршрут из ' + from + ' в ' + to + ' существует.\n';
      text += 'Количество простых путей: ' + res.count + '\n';
      text += 'Путь: ' + res.path.join(' → ');
    } else {
      text = 'Маршрут из ' + from + ' в ' + to + ' не существует.\n' +
        '(Граф ориентированный — возможно, нет направленного пути.)';
    }
    $('routeText').textContent = text;

    UIHelpers.displayMatrix('routeMatrix', graph.adjMatrix, 0, 'adjacency');
    if (res.exists) {
      const edges = [];
      for (let i = 0; i + 1 < res.path.length; i++)
        edges.push([res.path[i], res.path[i + 1]]);
      UIHelpers.highlightPath('routeMatrix', edges);
      graphCanvas.highlightRoute(res.path);
    } else {
      graphCanvas.clearHighlights();
    }
  }

  // ---- Tab 3: BiComp ----
  function onFindBiComp() {
    if (!hasGraph) { alert('Сначала сгенерируйте граф!'); return; }

    const n = graph.n;
    if (n <= 1) {
      $('bicompText').textContent = 'Граф содержит ≤ 1 вершины — точек сочленения нет.';
      UIHelpers.displayMatrix('bicompMatrix', graph.adjMatrix, 0, 'adjacency');
      graphCanvas.clearHighlights();
      return;
    }

    // For directed graphs, treat as undirected
    let adj = graph.adjMatrix.map(row => row.slice());
    if (graph.directed) {
      for (let i = 0; i < n; i++)
        for (let j = i + 1; j < n; j++) {
          if (adj[i][j] || adj[j][i]) {
            adj[i][j] = 1;
            adj[j][i] = 1;
          }
        }
    }

    const res = BiComp.find(adj);
    let text = '';
    text += 'Количество итераций (BiComp): ' + res.iterations + '\n\n';

    if (res.articulationPoints.size === 0) {
      text += 'Точек сочленения нет.\n';
    } else {
      text += 'Точки сочленения: { ' + Array.from(res.articulationPoints).join(', ') + ' }\n';
    }

    text += '\nКоличество блоков (двусвязных компонент): ' + res.blocks.length + '\n';

    for (let b = 0; b < res.blocks.length; b++) {
      text += '  Блок ' + (b + 1) + ': рёбра { ';
      text += res.blocks[b].map(e => '(' + e[0] + ',' + e[1] + ')').join(', ');
      text += ' }\n';
    }

    $('bicompText').textContent = text;

    UIHelpers.displayMatrix('bicompMatrix', adj, 0, 'adjacency');
    UIHelpers.highlightCells('bicompMatrix', Array.from(res.articulationPoints), 'cell-highlight-red');

    graphCanvas.highlightBiComp(res.articulationPoints, res.blocks);
  }

  // ---- Tab 4: Dijkstra ----
  function onDijkstraCalculate() {
    if (!hasGraph) { alert('Сначала сгенерируйте граф!'); return; }

    const n = graph.n;
    if (n <= 1) {
      $('dijkText').textContent = 'Граф содержит ≤ 1 вершины — алгоритм Дейкстры не применим.';
      return;
    }

    const src = intVal('dijkSrc', 0);
    const dst = intVal('dijkDst', 0);
    const INF = 1e9;

    const res = DijkstraNeg.solve(graph.weightMatrix, src, graph.directed);
    let text = '';
    text += 'Количество итераций (Дейкстра): ' + res.iterations + '\n';

    if (res.hasNegativeCycle) {
      text += '⚠ Обнаружен отрицательный цикл!\n';
    }

    const path = DijkstraNeg.reconstructPath(res.parent, src, dst);
    text += '\n';

    if (src === dst) {
      text += 'Путь из ' + src + ' в ' + dst + ': тривиальный (длина 0).';
    } else if (path.length === 0) {
      text += 'Путь из ' + src + ' в ' + dst + ' не существует.';
    } else {
      const d = res.dist[dst] >= INF ? '∞' : res.dist[dst];
      text += 'Кратчайший путь из ' + src + ' в ' + dst + ' (длина = ' + d + '):\n';
      text += path.join(' → ');
    }

    $('dijkText').textContent = text;

    // --- Dijkstra stages table ---
    {
      let html = '<table class="matrix-table"><thead><tr>';
      html += '<th>Этап</th><th>Обр. верш.</th>';
      for (let i = 0; i < n; i++) html += '<th>d[' + i + ']</th>';
      html += '</tr></thead><tbody>';

      for (let s = 0; s < res.stages.length; s++) {
        html += '<tr>';
        html += '<td>' + s + '</td>';
        html += '<td class="cell-highlight-blue">v' + res.stages[s].vertex + '</td>';
        for (let i = 0; i < n; i++) {
          const dVal = res.stages[s].dist[i] >= INF ? '∞' : res.stages[s].dist[i];
          const cls = (i === res.stages[s].vertex) ? ' class="cell-highlight-blue"' : '';
          html += '<td' + cls + '>' + dVal + '</td>';
        }
        html += '</tr>';
      }
      html += '</tbody></table>';
      $('dijkStagesTable').innerHTML = html;
    }

    UIHelpers.displayMatrix('dijkMatrix', graph.weightMatrix, 0, 'weighted');
    if (path.length > 0) {
      const edges = [];
      for (let i = 0; i + 1 < path.length; i++)
        edges.push([path[i], path[i + 1]]);
      UIHelpers.highlightPath('dijkMatrix', edges);
      graphCanvas.highlightDijkstraPath(path);
    } else {
      graphCanvas.clearHighlights();
    }
  }

  // ---- Regenerate weights ----
  function onRegenerateWeights(weightType) {
    if (!hasGraph) { alert('Сначала сгенерируйте граф!'); return; }

    const wp = clamp(floatVal('weightParamP', 0.4), 0.01, 0.99);
    GraphGenerator.regenerateWeights(graph, weightType, wp);

    // Update canvas and weight matrix display
    graphCanvas.setGraph(graph);
    UIHelpers.displayMatrix('weightMatrix', graph.weightMatrix, 0, 'weighted');

    // Clear stale results
    $('minMatrix').innerHTML = '';
    $('maxMatrix').innerHTML = '';
    $('dijkText').textContent = '';
    $('dijkStagesTable').innerHTML = '';
    $('dijkMatrix').innerHTML = '';
    $('cmpText').textContent = '';
    $('cmpTable').innerHTML = '';

    // Clear Tab 6 (Поток)
    $('capacityMatrix').innerHTML = '';
    $('costMatrixDisplay').innerHTML = '';
    $('maxFlowText').textContent = '';
    $('maxFlowMatrix').innerHTML = '';
    $('minCostFlowText').textContent = '';
    $('minCostFlowMatrix').innerHTML = '';
    hasFlowNetwork = false;
    lastMaxFlow = 0;

    // Update weight type label
    const labels = { positive: 'Положительные', negative: 'Отрицательные', mixed: 'Смешанные' };
    $('dijkWeightTypeLabel').textContent = 'Тип весов: ' + (labels[weightType] || weightType);
  }

  // ---- Rearrange directed / undirected ----
  function onRearrangeDirected() {
    if (!hasGraph) { alert('Сначала сгенерируйте граф!'); return; }

    // Restore original directed matrices
    graph.adjMatrix = originalAdjMatrix.map(r => r.slice());
    graph.weightMatrix = originalWeightMatrix.map(r => r.slice());
    graph.directed = true;
    $('directedCheck').checked = true;

    // Refresh display
    graphCanvas.setGraph(graph);
    UIHelpers.displayMatrix('adjMatrix', graph.adjMatrix, 0, 'adjacency');
    UIHelpers.displayMatrix('weightMatrix', graph.weightMatrix, 0, 'weighted');

    // Re-run analysis
    const n = graph.n;
    const res = GraphAnalysis.analyze(graph.adjMatrix, true);
    let text = 'Эксцентриситеты:\n';
    for (let i = 0; i < n; i++) text += '  v' + i + ' = ' + res.eccentricities[i] + '\n';
    text += '\nЦентр графа: { ' + res.center.join(', ') + ' }\n';
    text += 'Диаметр: ' + res.diameter + '\n';
    text += 'Диаметральные вершины: { ' + res.diametralVertices.join(', ') + ' }';
    $('analysisText').textContent = text;

    // Clear downstream results
    $('minMatrix').innerHTML = '';
    $('maxMatrix').innerHTML = '';
    $('dijkText').textContent = '';
    $('dijkStagesTable').innerHTML = '';
    $('dijkMatrix').innerHTML = '';
    $('cmpText').textContent = '';
    $('cmpTable').innerHTML = '';
    $('capacityMatrix').innerHTML = '';
    $('costMatrixDisplay').innerHTML = '';
    $('maxFlowText').textContent = '';
    $('maxFlowMatrix').innerHTML = '';
    $('minCostFlowText').textContent = '';
    $('minCostFlowMatrix').innerHTML = '';
    hasFlowNetwork = false;
    lastMaxFlow = 0;
  }

  function onRearrangeUndirected() {
    if (!hasGraph) { alert('Сначала сгенерируйте граф!'); return; }

    const n = graph.n;

    // Build undirected acyclic graph from original edges.
    // Add each original edge as undirected only if it doesn't create a cycle.
    const newAdj = Array.from({length: n}, () => new Array(n).fill(0));
    const newWeight = Array.from({length: n}, () => new Array(n).fill(0));

    for (let i = 0; i < n; i++) {
      for (let j = 0; j < n; j++) {
        if (i !== j && originalAdjMatrix[i][j] && !newAdj[i][j]) {
          // Check if i and j are already connected undirectedly
          if (!GraphGenerator._canReachUndirected(newAdj, i, j)) {
            newAdj[i][j] = 1;
            newAdj[j][i] = 1;
            const w = originalWeightMatrix[i][j] !== 0 ? originalWeightMatrix[i][j]
                                                       : originalWeightMatrix[j][i];
            newWeight[i][j] = w;
            newWeight[j][i] = w;
          }
        }
      }
    }

    graph.adjMatrix = newAdj;
    graph.weightMatrix = newWeight;
    graph.directed = false;
    $('directedCheck').checked = false;

    // Refresh display
    graphCanvas.setGraph(graph);
    UIHelpers.displayMatrix('adjMatrix', graph.adjMatrix, 0, 'adjacency');
    UIHelpers.displayMatrix('weightMatrix', graph.weightMatrix, 0, 'weighted');

    // Re-run analysis
    const res = GraphAnalysis.analyze(graph.adjMatrix, false);
    let text = 'Эксцентриситеты:\n';
    for (let i = 0; i < n; i++) text += '  v' + i + ' = ' + res.eccentricities[i] + '\n';
    text += '\nЦентр графа: { ' + res.center.join(', ') + ' }\n';
    text += 'Диаметр: ' + res.diameter + '\n';
    text += 'Диаметральные вершины: { ' + res.diametralVertices.join(', ') + ' }';
    $('analysisText').textContent = text;

    // Clear downstream results
    $('minMatrix').innerHTML = '';
    $('maxMatrix').innerHTML = '';
    $('dijkText').textContent = '';
    $('dijkStagesTable').innerHTML = '';
    $('dijkMatrix').innerHTML = '';
    $('cmpText').textContent = '';
    $('cmpTable').innerHTML = '';
    $('capacityMatrix').innerHTML = '';
    $('costMatrixDisplay').innerHTML = '';
    $('maxFlowText').textContent = '';
    $('maxFlowMatrix').innerHTML = '';
    $('minCostFlowText').textContent = '';
    $('minCostFlowMatrix').innerHTML = '';
    hasFlowNetwork = false;
    lastMaxFlow = 0;
  }

  // ---- Tab 5: Comparison ----
  function onRunComparison() {
    const minN = intVal('cmpMinN', 5);
    const maxN = intVal('cmpMaxN', 50);
    const step = intVal('cmpStep', 5);

    if (minN > maxN) { alert('n (от) должно быть ≤ n (до).'); return; }

    const rows = [];
    for (let n = minN; n <= maxN; n += step) {
      const g = GraphGenerator.generate(n, 0.5, false, 'positive', 0.4);
      const biRes = BiComp.find(g.adjMatrix);
      const djRes = DijkstraNeg.solve(g.weightMatrix, 0, false);
      rows.push({ n: n, bicompIter: biRes.iterations, dijkstraIter: djRes.iterations });
    }

    // Build table
    let html = '<table class="matrix-table"><thead><tr>';
    html += '<th>n (вершин)</th><th>BiComp (итер.)</th><th>Дейкстра (итер.)</th>';
    html += '</tr></thead><tbody>';
    for (const r of rows) {
      html += '<tr><td>' + r.n + '</td><td>' + r.bicompIter + '</td><td>' + r.dijkstraIter + '</td></tr>';
    }
    html += '</tbody></table>';
    $('cmpTable').innerHTML = html;

    // Summary
    let text = 'Сравнение количества итераций:\n';
    text += 'Диапазон: n = ' + minN + '..' + maxN + ', шаг = ' + step + '\n';
    if (rows.length > 0) {
      text += 'BiComp: от ' + rows[0].bicompIter + ' до ' + rows[rows.length - 1].bicompIter + ' итераций\n';
      text += 'Дейкстра: от ' + rows[0].dijkstraIter + ' до ' + rows[rows.length - 1].dijkstraIter + ' итераций\n';
    }
    $('cmpText').textContent = text;
  }

  // ---- Tab 6: Flow Network ----

  function onGenerateFlowNetwork() {
    if (!hasGraph) { alert('Сначала сгенерируйте граф!'); return; }
    if (!graph.directed) {
      alert('Для задачи о потоках граф должен быть ориентированным.\nПожалуйста, сгенерируйте ориентированный граф.');
      return;
    }

    const src = intVal('flowSrc', 0);
    const snk = intVal('flowSink', 0);
    if (src === snk) { alert('Исток и сток не должны совпадать!'); return; }

    const n = graph.n;
    const maxCap = clamp(intVal('maxCapInput', 20), 1, 100);
    const maxCst = clamp(intVal('maxCostInput', 10), 1, 100);

    capacityMatrix = Array.from({ length: n }, () => new Array(n).fill(0));
    costMatrix = Array.from({ length: n }, () => new Array(n).fill(0));

    for (let i = 0; i < n; i++) {
      for (let j = 0; j < n; j++) {
        if (graph.adjMatrix[i][j] === 1) {
          capacityMatrix[i][j] = Math.floor(Math.random() * maxCap) + 1;
          costMatrix[i][j] = Math.floor(Math.random() * maxCst) + 1;
        }
      }
    }

    hasFlowNetwork = true;
    lastMaxFlow = 0;
    lastMaxFlowResult = null;

    UIHelpers.displayEditableMatrix('capacityMatrix', capacityMatrix, 0, 'weighted');
    UIHelpers.displayEditableMatrix('costMatrixDisplay', costMatrix, 0, 'weighted');

    // Show capacity labels on visual graph
    if (graphCanvas) {
      graphCanvas.clearFlowLabels();
      graphCanvas.setEdgeLabels(capacityMatrix);
    }

    // Clear previous results
    $('maxFlowText').textContent = '';
    $('maxFlowMatrix').innerHTML = '';
    $('minCostFlowText').textContent = '';
    $('minCostFlowMatrix').innerHTML = '';
  }

  function onFindMaxFlow() {
    if (!hasFlowNetwork) { alert('Сначала сгенерируйте сеть потоков!'); return; }

    const src = intVal('flowSrc', 0);
    const snk = intVal('flowSink', 0);

    // Read back user-edited values
    capacityMatrix = UIHelpers.readMatrixFromTable('capacityMatrix', graph.n);

    const result = MaxFlow.solve(capacityMatrix, src, snk);
    lastMaxFlow = result.maxFlow;
    lastMaxFlowResult = result;

    let html = 'Максимальный поток: ' + result.maxFlow + '\n';
    html += 'Количество увеличивающих путей: ' + result.iterations + '\n\n';

    for (let i = 0; i < result.iterations; i++) {
      const path = result.augmentingPaths[i];
      const types = result.pathEdgeTypes[i];
      let pathStr = String(path[0]);
      for (let j = 1; j < path.length; j++) {
        pathStr += types[j - 1] ? ' &rarr;(обр.) ' : ' &rarr; ';
        pathStr += path[j];
      }
      html += '<span class="flow-path-line" data-path-index="' + i + '">'
           + 'Путь ' + (i + 1) + ': ' + pathStr
           + '  (поток: ' + result.pathFlows[i] + ')'
           + '</span>\n';
    }

    $('maxFlowText').innerHTML = html;
    UIHelpers.displayMatrix('maxFlowMatrix', result.flowMatrix);

    // Show flow/capacity labels on sink edges in visual graph
    if (graphCanvas) {
      graphCanvas.setFlowLabels(result.flowMatrix, capacityMatrix, snk);
    }

    // Attach hover handlers to path lines
    document.querySelectorAll('#maxFlowText .flow-path-line').forEach(function (span) {
      span.addEventListener('mouseenter', function () {
        onFlowPathHover(parseInt(this.dataset.pathIndex, 10));
      });
      span.addEventListener('mouseleave', onFlowPathUnhover);
    });

    // Clear min cost flow results
    $('minCostFlowText').textContent = '';
    $('minCostFlowMatrix').innerHTML = '';
  }

  function onFindMinCostFlow() {
    if (!hasFlowNetwork) { alert('Сначала сгенерируйте сеть потоков!'); return; }

    const src = intVal('flowSrc', 0);
    const snk = intVal('flowSink', 0);

    // Read back user-edited values
    capacityMatrix = UIHelpers.readMatrixFromTable('capacityMatrix', graph.n);
    costMatrix = UIHelpers.readMatrixFromTable('costMatrixDisplay', graph.n);

    // Compute max flow if not done yet
    if (lastMaxFlow === 0) {
      const mfResult = MaxFlow.solve(capacityMatrix, src, snk);
      lastMaxFlow = mfResult.maxFlow;

      if (lastMaxFlow === 0) {
        $('minCostFlowText').textContent =
          'Максимальный поток = 0 (нет пути из истока в сток).\n' +
          'Поток минимальной стоимости не определён.';
        return;
      }
    }

    const desiredFlow = Math.floor((2 * lastMaxFlow) / 3);

    if (desiredFlow === 0) {
      $('minCostFlowText').textContent =
        'Максимальный поток: ' + lastMaxFlow + '\n' +
        'F = \u230a2/3 \u00d7 ' + lastMaxFlow + '\u230b = 0\n' +
        'Требуемый поток = 0, стоимость = 0.';
      return;
    }

    const result = MinCostFlow.solve(capacityMatrix, costMatrix, src, snk, desiredFlow);

    let text = 'Максимальный поток: ' + lastMaxFlow + '\n';
    text += 'Требуемый поток F = \u230a2/3 \u00d7 ' + lastMaxFlow + '\u230b = ' + desiredFlow + '\n';
    text += 'Достигнутый поток: ' + result.flowValue + '\n';
    text += 'Минимальная стоимость: ' + result.totalCost + '\n';

    if (!result.feasible) {
      text += '\n\u26a0 Невозможно отправить поток требуемой величины!\n';
    }

    text += '\nКоличество итераций: ' + result.iterations + '\n\n';

    for (let i = 0; i < result.iterations; i++) {
      text += 'Итерация ' + (i + 1) + ': ' + result.augmentingPaths[i].join(' \u2192 ');
      text += '  (поток: ' + result.pathFlows[i] + ', стоимость пути: ' + result.pathCosts[i] + ')\n';
    }

    $('minCostFlowText').textContent = text;
    UIHelpers.displayMatrix('minCostFlowMatrix', result.flowMatrix);
  }

  // ---- Flow path hover handlers ----

  function onFlowPathHover(pathIndex) {
    if (!lastMaxFlowResult || pathIndex >= lastMaxFlowResult.augmentingPaths.length) return;
    const path = lastMaxFlowResult.augmentingPaths[pathIndex];
    const edgeTypes = lastMaxFlowResult.pathEdgeTypes[pathIndex];

    // Highlight edges in the capacity matrix
    const container = document.getElementById('capacityMatrix');
    if (container) {
      for (let k = 0; k + 1 < path.length; k++) {
        const u = path[k], v = path[k + 1];
        let cell;
        if (edgeTypes[k]) {
          // Backward edge: highlight the original direction (v→u) in orange
          cell = container.querySelector('td[data-row="' + v + '"][data-col="' + u + '"]');
          if (cell) cell.classList.add('cell-highlight-orange');
        } else {
          // Forward edge: highlight u→v in green
          cell = container.querySelector('td[data-row="' + u + '"][data-col="' + v + '"]');
          if (cell) cell.classList.add('cell-highlight-green');
        }
      }
    }

    // Highlight on visual graph
    if (graphCanvas) {
      graphCanvas.highlightFlowPath(path, edgeTypes);
    }
  }

  function onFlowPathUnhover() {
    // Remove all highlights from capacity matrix
    const container = document.getElementById('capacityMatrix');
    if (container) {
      container.querySelectorAll('.cell-highlight-green').forEach(function (el) {
        el.classList.remove('cell-highlight-green');
      });
      container.querySelectorAll('.cell-highlight-orange').forEach(function (el) {
        el.classList.remove('cell-highlight-orange');
      });
    }

    // Clear visual graph highlights
    if (graphCanvas) {
      graphCanvas.clearHighlights();
      // Restore edge labels if we have a flow network
      if (hasFlowNetwork && capacityMatrix) {
        graphCanvas.setEdgeLabels(capacityMatrix);
      }
    }
  }

  // ---- Init ----
  document.addEventListener('DOMContentLoaded', function () {
    setupTabs();

    graphCanvas = new GraphCanvas(document.getElementById('graph-canvas'));

    $('generateBtn').addEventListener('click', onGenerate);
    $('generateDAGBtn').addEventListener('click', onGenerateDAG);
    $('shimbelBtn').addEventListener('click', onShimbelCalculate);
    $('findRouteBtn').addEventListener('click', onFindRoute);
    $('bicompBtn').addEventListener('click', onFindBiComp);
    $('dijkBtn').addEventListener('click', onDijkstraCalculate);
    $('cmpBtn').addEventListener('click', onRunComparison);
    $('regenPosBtn').addEventListener('click', function () { onRegenerateWeights('positive'); });
    $('regenNegBtn').addEventListener('click', function () { onRegenerateWeights('negative'); });
    $('regenMixBtn').addEventListener('click', function () { onRegenerateWeights('mixed'); });
    $('rearrangeDirBtn').addEventListener('click', onRearrangeDirected);
    $('rearrangeUndirBtn').addEventListener('click', onRearrangeUndirected);
    $('genFlowNetworkBtn').addEventListener('click', onGenerateFlowNetwork);
    $('maxFlowBtn').addEventListener('click', onFindMaxFlow);
    $('minCostFlowBtn').addEventListener('click', onFindMinCostFlow);
  });

})();
