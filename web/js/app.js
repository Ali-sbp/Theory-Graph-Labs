// app.js — Main controller: tab switching, event handlers
// Ports mainwindow.cpp slot logic to JavaScript

(function () {
  'use strict';

  // ---- State ----
  let graph = null;   // { n, directed, adjMatrix, weightMatrix }
  let hasGraph = false;
  let graphCanvas = null;

  // ---- Helpers ----
  function $(id) { return document.getElementById(id); }
  function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)); }
  function intVal(id, fallback) { return parseInt($(id).value, 10) || fallback; }
  function floatVal(id, fallback) { return parseFloat($(id).value) || fallback; }

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
        if (tab === 0 || tab === 1 || tab === 5) {
          if (graphCanvas) graphCanvas.clearHighlights();
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
    $('dijkMatrix').innerHTML = '';
    $('cmpText').textContent = '';
    $('cmpTable').innerHTML = '';

    // Display matrices
    UIHelpers.displayMatrix('adjMatrix', graph.adjMatrix);
    UIHelpers.displayMatrix('weightMatrix', graph.weightMatrix);

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
    let unreachableCount = 0;
    for (let i = 0; i < n; i++) {
      const ecc = res.eccentricities[i] === Infinity ? '∞' : res.eccentricities[i];
      text += '  v' + i + ' = ' + ecc + '\n';
      if (res.eccentricities[i] === Infinity) unreachableCount++;
    }

    if (unreachableCount > 0) {
      text += '\n⚠ ' + unreachableCount + ' вершин(а) имеют эксц. = ∞ (не все вершины достижимы; граф ориентированный).\n';
    }

    text += '\nЦентр графа: { ' + res.center.join(', ') + ' }\n';

    const diam = res.diameter === Infinity ? '∞' : res.diameter;
    text += 'Диаметр: ' + diam + '\n';
    text += 'Диаметральные вершины: { ' + res.diametralVertices.join(', ') + ' }';

    $('analysisText').textContent = text;

    // Update ranges on other tabs
    setInputRange('pathLength', 1, Math.max(1, n - 1));
    setInputRange('fromVertex', 0, n - 1);
    setInputRange('toVertex', 0, n - 1);
    setInputRange('dijkSrc', 0, n - 1);
    setInputRange('dijkDst', 0, Math.max(0, n - 1));
    if (n > 1) $('dijkDst').value = 1;

    // Update weight type label on Tab 4
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
      UIHelpers.displayMatrix('routeMatrix', graph.adjMatrix);
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

    UIHelpers.displayMatrix('routeMatrix', graph.adjMatrix);
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
      UIHelpers.displayMatrix('bicompMatrix', graph.adjMatrix);
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

    UIHelpers.displayMatrix('bicompMatrix', adj);
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
    text += 'Количество итераций (Дейкстра): ' + res.iterations + '\n\n';

    if (res.hasNegativeCycle) {
      text += '⚠ Обнаружен отрицательный цикл!\n';
    }

    text += 'Вектор расстояний от вершины ' + src + ':\n';
    for (let i = 0; i < n; i++) {
      const d = res.dist[i] >= INF ? '∞' : res.dist[i];
      text += '  d[' + i + '] = ' + d + '\n';
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

    UIHelpers.displayMatrix('dijkMatrix', graph.weightMatrix);
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

  // ---- Init ----
  document.addEventListener('DOMContentLoaded', function () {
    setupTabs();

    graphCanvas = new GraphCanvas(document.getElementById('graph-canvas'));

    $('generateBtn').addEventListener('click', onGenerate);
    $('shimbelBtn').addEventListener('click', onShimbelCalculate);
    $('findRouteBtn').addEventListener('click', onFindRoute);
    $('bicompBtn').addEventListener('click', onFindBiComp);
    $('dijkBtn').addEventListener('click', onDijkstraCalculate);
    $('cmpBtn').addEventListener('click', onRunComparison);
  });

})();
