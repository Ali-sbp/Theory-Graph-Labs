/**
 * GraphCanvas — Canvas2D graph visualization widget.
 * Port of the Qt GraphWidget.
 */
(function () {
  'use strict';

  const BLOCK_COLORS = [
    '#4CAF50', '#2196F3', '#FF9800', '#9C27B0', '#00BCD4',
    '#F44336', '#FFEB3B', '#795548', '#E91E63', '#3F51B5'
  ];

  const PADDING = 40;

  const HighlightMode = {
    None: 0,
    Route: 1,
    BiComp: 2,
    DijkstraPath: 3,
    FlowPath: 4,
    MSTEdges: 5,
    VertexCover: 6
  };

  class GraphCanvas {
    constructor(canvasElement) {
      this.canvas = canvasElement;
      this.ctx = canvasElement.getContext('2d');
      this.n = 0;
      this.directed = false;
      this.adj = null;
      this.weights = null;
      this.positions = [];
      this.mode = HighlightMode.None;
      this.highlightedVertices = new Set();
      this.highlightedEdges = new Set();
      this.articulationPoints = [];
      this.blocks = [];
      this.edgeLabels = null;
      this.flowLabels = null;
      this.forceUndirectedView = false;
      this.setupResize();
      this.draw();
    }

    /* ---- Public API ---- */

    setGraph(data) {
      this.n = data.n || 0;
      this.directed = !!data.directed;
      this.adj = data.adjMatrix || null;
      this.weights = data.weightMatrix || null;
      this.mode = HighlightMode.None;
      this.highlightedVertices = new Set();
      this.highlightedEdges = new Set();
      this.articulationPoints = [];
      this.blocks = [];
      this.edgeLabels = null;
      this.flowLabels = null;
      this.computeLayout();
      this.draw();
    }

    setForceUndirectedView(v) { this.forceUndirectedView = v; this.draw(); }

    clearHighlights() {
      this.mode = HighlightMode.None;
      this.highlightedVertices = new Set();
      this.highlightedEdges = new Set();
      this.articulationPoints = [];
      this.blocks = [];
      this.draw();
    }

    highlightRoute(path) {
      this.mode = HighlightMode.Route;
      this.highlightedVertices = new Set(path);
      this.highlightedEdges = new Set();
      for (let i = 0; i + 1 < path.length; i++) {
        this.highlightedEdges.add(path[i] + ',' + path[i + 1]);
      }
      this.draw();
    }

    highlightBiComp(articulationPoints, blocks) {
      this.mode = HighlightMode.BiComp;
      this.articulationPoints = Array.from(articulationPoints || []);
      this.blocks = blocks || [];
      this.highlightedVertices = new Set();
      this.highlightedEdges = new Set();
      this.draw();
    }

    highlightDijkstraPath(path) {
      this.mode = HighlightMode.DijkstraPath;
      this.highlightedVertices = new Set(path);
      this.highlightedEdges = new Set();
      for (let i = 0; i + 1 < path.length; i++) {
        this.highlightedEdges.add(path[i] + ',' + path[i + 1]);
      }
      this.draw();
    }

    highlightMSTEdges(mstEdgeSet) {
      this.mode = HighlightMode.MSTEdges;
      this.highlightedEdges = mstEdgeSet; // Set of 'u,v' strings (u < v)
      this.highlightedVertices = new Set();
      this.articulationPoints = [];
      this.blocks = [];
      this.draw();
    }

    highlightVertexCover(cover, pickedEdges, removedEdges, mstEdgeSet = null) {
      this.mode = HighlightMode.VertexCover;
      this.highlightedVertices = new Set(cover);
      this.highlightedEdges = new Set();
      this.vcPickedEdges = new Set((pickedEdges || []).map(([a, b]) => Math.min(a, b) + ',' + Math.max(a, b)));
      this.vcRemovedEdges = new Set((removedEdges || []).map(([a, b]) => Math.min(a, b) + ',' + Math.max(a, b)));
      this.vcMSTEdges = mstEdgeSet;
      this.articulationPoints = [];
      this.blocks = [];
      this.draw();
    }

    highlightFlowPath(path, isBackwardArray) {
      this.mode = HighlightMode.FlowPath;
      this.highlightedVertices = new Set(path);
      this.highlightedEdges = new Set();
      for (let i = 0; i + 1 < path.length; i++) {
        if (isBackwardArray[i]) {
          // Backward edge: the original edge is path[i+1]→path[i]
          this.highlightedEdges.add(path[i + 1] + ',' + path[i]);
        } else {
          this.highlightedEdges.add(path[i] + ',' + path[i + 1]);
        }
      }
      this.draw();
    }

    setEdgeLabels(labels) {
      this.edgeLabels = labels;
      this.draw();
    }

    clearEdgeLabels() {
      this.edgeLabels = null;
      this.draw();
    }

    setFlowLabels(flowMatrix, capacityMatrix, sink) {
      const n = flowMatrix.length;
      this.flowLabels = Array.from({ length: n }, function () { return new Array(n).fill(null); });
      for (let u = 0; u < n; u++) {
        if (flowMatrix[u][sink] > 0) {
          this.flowLabels[u][sink] = flowMatrix[u][sink] + '/' + capacityMatrix[u][sink];
        }
      }
      this.draw();
    }

    clearFlowLabels() {
      this.flowLabels = null;
      this.draw();
    }

    /* ---- Layout ---- */

    computeLayout() {
      this.positions = [];
      for (let i = 0; i < this.n; i++) {
        const angle = (2 * Math.PI * i) / this.n - Math.PI / 2;
        this.positions.push({
          x: 0.5 + 0.4 * Math.cos(angle),
          y: 0.5 + 0.4 * Math.sin(angle)
        });
      }
    }

    mapToPixel(nx, ny, w, h) {
      return {
        x: PADDING + nx * (w - 2 * PADDING),
        y: PADDING + ny * (h - 2 * PADDING)
      };
    }

    vertexRadius(w, h) {
      if (this.n <= 1) return 18;
      const circleR = 0.4 * Math.min(w - 2 * PADDING, h - 2 * PADDING);
      const r = Math.min(18, circleR * Math.sin(Math.PI / this.n) * 0.8);
      return Math.max(8, r);
    }

    /* ---- Drawing ---- */

    draw() {
      const container = this.canvas.parentElement;
      if (!container) return;
      const dpr = window.devicePixelRatio || 1;
      const cw = container.clientWidth;
      const ch = container.clientHeight;
      this.canvas.width = cw * dpr;
      this.canvas.height = ch * dpr;
      this.canvas.style.width = cw + 'px';
      this.canvas.style.height = ch + 'px';
      const ctx = this.ctx;
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

      // Clear
      ctx.fillStyle = '#16162a';
      ctx.fillRect(0, 0, cw, ch);

      if (this.n <= 0) {
        ctx.fillStyle = '#555';
        ctx.font = '16px -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText('Граф не сгенерирован', cw / 2, ch / 2);
        return;
      }

      const vr = this.vertexRadius(cw, ch);

      // Build edge-to-block map for BiComp
      const edgeBlockMap = new Map();
      if (this.mode === HighlightMode.BiComp && this.blocks.length > 0) {
        for (let bi = 0; bi < this.blocks.length; bi++) {
          const block = this.blocks[bi];
          for (const [u, v] of block) {
            edgeBlockMap.set(u + ',' + v, bi);
            edgeBlockMap.set(v + ',' + u, bi);
          }
        }
      }

      // Pixel positions
      const px = [];
      for (let i = 0; i < this.n; i++) {
        px.push(this.mapToPixel(this.positions[i].x, this.positions[i].y, cw, ch));
      }

      // ---- Draw edges ----
      const treatAsUndirected = !this.directed || this.forceUndirectedView;
      for (let i = 0; i < this.n; i++) {
        for (let j = 0; j < this.n; j++) {
          if (treatAsUndirected) {
            if (j <= i) continue;
            if (!this.adj[i][j] && !this.adj[j][i]) continue;
          } else {
            if (!this.adj[i][j]) continue;
          }

          const edgeKey = i + ',' + j;
          const bidirectional = this.directed && !this.forceUndirectedView && this.adj[i][j] && this.adj[j][i];

          // Determine style
          let color = '#8888aa';
          let width = 1.5;
          let dash = [];
          if (this.mode === HighlightMode.Route && this.highlightedEdges.has(edgeKey)) {
            color = '#64DC64';
            width = 3;
          } else if (this.mode === HighlightMode.DijkstraPath && this.highlightedEdges.has(edgeKey)) {
            color = '#FFB43C';
            width = 3;
          } else if (this.mode === HighlightMode.FlowPath && this.highlightedEdges.has(edgeKey)) {
            color = '#FF4444';
            width = 3;
          } else if (this.mode === HighlightMode.BiComp && edgeBlockMap.has(edgeKey)) {
            const bi = edgeBlockMap.get(edgeKey);
            color = BLOCK_COLORS[bi % BLOCK_COLORS.length];
            width = 2.5;
          } else if (this.mode === HighlightMode.MSTEdges) {
            const normKey = Math.min(i, j) + ',' + Math.max(i, j);
            if (this.highlightedEdges.has(normKey)) {
              color = '#64DC64';
              width = 3;
            } else {
              color = '#444460';
              width = 1;
            }
          } else if (this.mode === HighlightMode.VertexCover) {
            const normKey = Math.min(i, j) + ',' + Math.max(i, j);
            if (this.vcPickedEdges && this.vcPickedEdges.has(normKey)) {
              color = 'rgb(170,100,220)';
              width = 3;
            } else if (this.vcRemovedEdges && this.vcRemovedEdges.has(normKey)) {
              color = this.vcMSTEdges ? '#64DC64' : '#FFD700';
              width = 1.5;
              dash = [6, 4];
            } else if (this.vcMSTEdges && !this.vcMSTEdges.has(normKey)) {
              color = '#444460';
              width = 1;
            }
          }

          ctx.strokeStyle = color;
          ctx.lineWidth = width;

          const x1 = px[i].x, y1 = px[i].y;
          const x2 = px[j].x, y2 = px[j].y;

          if (bidirectional) {
            // Curved edge
            const dx = x2 - x1;
            const dy = y2 - y1;
            const len = Math.sqrt(dx * dx + dy * dy) || 1;
            const nx = -dy / len;
            const ny = dx / len;
            const off = 8;
            const mx = (x1 + x2) / 2 + nx * off;
            const my = (y1 + y2) / 2 + ny * off;

            ctx.setLineDash(dash);
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.quadraticCurveTo(mx, my, x2, y2);
            ctx.stroke();
            ctx.setLineDash([]);

            if (this.directed && !this.forceUndirectedView) {
              this.drawArrowCurved(ctx, x1, y1, mx, my, x2, y2, vr, color);
            }

            // Weight / capacity / flow label
            if (this.n <= 20) {
              const t = 0.5;
              const lx = (1 - t) * (1 - t) * x1 + 2 * (1 - t) * t * mx + t * t * x2;
              const ly = (1 - t) * (1 - t) * y1 + 2 * (1 - t) * t * my + t * t * y2;
              if (this.flowLabels && this.flowLabels[i][j]) {
                this.drawWeightLabel(ctx, lx, ly, this.flowLabels[i][j]);
              } else if (this.edgeLabels) {
                this.drawWeightLabel(ctx, lx, ly, this.edgeLabels[i][j]);
              } else if (this.weights) {
                this.drawWeightLabel(ctx, lx, ly, this.weights[i][j]);
              }
            }
          } else {
            // Straight edge
            ctx.setLineDash(dash);
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();
            ctx.setLineDash([]);

            if (this.directed && !this.forceUndirectedView) {
              this.drawArrowStraight(ctx, x1, y1, x2, y2, vr, color);
            }

            // Weight / capacity / flow label
            if (this.n <= 20) {
              const lx = (x1 + x2) / 2;
              const ly = (y1 + y2) / 2;
              if (this.flowLabels && this.flowLabels[i][j]) {
                this.drawWeightLabel(ctx, lx, ly, this.flowLabels[i][j]);
              } else if (this.edgeLabels) {
                this.drawWeightLabel(ctx, lx, ly, this.edgeLabels[i][j]);
              } else if (this.weights) {
                this.drawWeightLabel(ctx, lx, ly, this.weights[i][j] || this.weights[j][i]);
              }
            }
          }
        }
      }

      // ---- Draw vertices ----
      for (let i = 0; i < this.n; i++) {
        let fill = 'rgb(173,216,230)';
        if (this.mode === HighlightMode.Route && this.highlightedVertices.has(i)) {
          fill = 'rgb(100,220,100)';
        } else if (this.mode === HighlightMode.BiComp && this.articulationPoints.indexOf(i) !== -1) {
          fill = 'rgb(255,100,100)';
        } else if (this.mode === HighlightMode.DijkstraPath && this.highlightedVertices.has(i)) {
          fill = 'rgb(255,180,60)';
        } else if (this.mode === HighlightMode.FlowPath && this.highlightedVertices.has(i)) {
          fill = 'rgb(255,100,100)';
        } else if (this.mode === HighlightMode.VertexCover && this.highlightedVertices.has(i)) {
          fill = 'rgb(170,100,220)';
        }

        ctx.beginPath();
        ctx.arc(px[i].x, px[i].y, vr, 0, 2 * Math.PI);
        ctx.fillStyle = fill;
        ctx.fill();
        ctx.strokeStyle = '#aaa';
        ctx.lineWidth = 1.5;
        ctx.stroke();

        // Label
        ctx.fillStyle = '#111';
        ctx.font = 'bold ' + Math.max(9, vr * 0.9) + 'px -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(String(i), px[i].x, px[i].y);
      }
    }

    drawArrowStraight(ctx, x1, y1, x2, y2, vr, color) {
      const dx = x2 - x1;
      const dy = y2 - y1;
      const len = Math.sqrt(dx * dx + dy * dy) || 1;
      const ux = dx / len;
      const uy = dy / len;
      // Tip at edge of target circle
      const tx = x2 - ux * vr;
      const ty = y2 - uy * vr;
      const arrLen = Math.min(12, vr * 0.8);
      const angle = 0.45;

      ctx.fillStyle = color;
      ctx.beginPath();
      ctx.moveTo(tx, ty);
      ctx.lineTo(
        tx - arrLen * Math.cos(Math.atan2(uy, ux) - angle),
        ty - arrLen * Math.sin(Math.atan2(uy, ux) - angle)
      );
      ctx.lineTo(
        tx - arrLen * Math.cos(Math.atan2(uy, ux) + angle),
        ty - arrLen * Math.sin(Math.atan2(uy, ux) + angle)
      );
      ctx.closePath();
      ctx.fill();
    }

    drawArrowCurved(ctx, x1, y1, cx, cy, x2, y2, vr, color) {
      // Tangent at t=1 for quadratic bezier: 2*(1-t)*(C-P0) + 2*t*(P1-C) at t~1 => direction P1-C
      const dx = x2 - cx;
      const dy = y2 - cy;
      const len = Math.sqrt(dx * dx + dy * dy) || 1;
      const ux = dx / len;
      const uy = dy / len;
      const tx = x2 - ux * vr;
      const ty = y2 - uy * vr;
      const arrLen = Math.min(12, vr * 0.8);
      const angle = 0.45;
      const base = Math.atan2(uy, ux);

      ctx.fillStyle = color;
      ctx.beginPath();
      ctx.moveTo(tx, ty);
      ctx.lineTo(tx - arrLen * Math.cos(base - angle), ty - arrLen * Math.sin(base - angle));
      ctx.lineTo(tx - arrLen * Math.cos(base + angle), ty - arrLen * Math.sin(base + angle));
      ctx.closePath();
      ctx.fill();
    }

    drawWeightLabel(ctx, x, y, w) {
      if (w === undefined || w === null) return;
      const text = String(w);
      ctx.font = '11px -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif';
      const tm = ctx.measureText(text);
      const pad = 3;
      ctx.fillStyle = 'rgba(22,22,42,0.85)';
      ctx.fillRect(x - tm.width / 2 - pad, y - 7 - pad, tm.width + 2 * pad, 14 + 2 * pad);
      ctx.fillStyle = '#ccc';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText(text, x, y);
    }

    /* ---- Resize ---- */

    setupResize() {
      const parent = this.canvas.parentElement;
      if (!parent) return;
      const ro = new ResizeObserver(() => {
        this.draw();
      });
      ro.observe(parent);
    }
  }

  window.GraphCanvas = GraphCanvas;
})();
