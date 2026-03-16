/**
 * UIHelpers — DOM utility functions for matrix display and highlighting.
 */
(function () {
  'use strict';

  const UIHelpers = {
    /**
     * Display a 2D matrix as an HTML table inside the given container.
     * @param {string} containerId — id of the wrapper div
     * @param {number[][]} matrix — 2D array of numbers
     * @param {number} sentinel — if > 0, values >= sentinel render as "∞";
     *                             if < 0, values <= sentinel render as "∞"
     */
    displayMatrix(containerId, matrix, sentinel) {
      if (sentinel === undefined) sentinel = 0;
      const container = document.getElementById(containerId);
      if (!container) return;
      container.innerHTML = '';

      if (!matrix || matrix.length === 0) {
        container.textContent = '(пусто)';
        return;
      }

      const n = matrix.length;
      const wrapper = document.createElement('div');
      wrapper.className = 'matrix-wrapper';

      const table = document.createElement('table');
      table.className = 'matrix-table';

      // Header row
      const thead = document.createElement('thead');
      const headerRow = document.createElement('tr');
      const corner = document.createElement('th');
      corner.textContent = '';
      headerRow.appendChild(corner);
      for (let j = 0; j < n; j++) {
        const th = document.createElement('th');
        th.textContent = String(j);
        headerRow.appendChild(th);
      }
      thead.appendChild(headerRow);
      table.appendChild(thead);

      // Data rows
      const tbody = document.createElement('tbody');
      for (let i = 0; i < n; i++) {
        const tr = document.createElement('tr');
        const rowHeader = document.createElement('th');
        rowHeader.textContent = String(i);
        tr.appendChild(rowHeader);
        for (let j = 0; j < n; j++) {
          const td = document.createElement('td');
          td.setAttribute('data-row', String(i));
          td.setAttribute('data-col', String(j));
          const val = matrix[i][j];
          let display;
          if (sentinel > 0 && val >= sentinel) {
            display = '\u221E';
          } else if (sentinel < 0 && val <= sentinel) {
            display = '\u221E';
          } else {
            display = String(val);
          }
          td.textContent = display;
          tr.appendChild(td);
        }
        tbody.appendChild(tr);
      }
      table.appendChild(tbody);
      wrapper.appendChild(table);
      container.appendChild(wrapper);
    },

    /**
     * Highlight path edges in a matrix table.
     * @param {string} containerId
     * @param {number[][]} edges — array of [from, to] pairs
     */
    highlightPath(containerId, edges) {
      const container = document.getElementById(containerId);
      if (!container) return;
      for (const [from, to] of edges) {
        const cell = container.querySelector(
          'td[data-row="' + from + '"][data-col="' + to + '"]'
        );
        if (cell) {
          cell.classList.add('cell-highlight-green');
        }
      }
    },

    /**
     * Highlight entire rows and columns for given vertices.
     * @param {string} containerId
     * @param {number[]} vertices
     * @param {string} cssClass — e.g. 'cell-highlight-red'
     */
    highlightCells(containerId, vertices, cssClass) {
      const container = document.getElementById(containerId);
      if (!container) return;
      const cells = container.querySelectorAll('td[data-row][data-col]');
      const vSet = new Set(vertices.map(String));
      cells.forEach(function (td) {
        const r = td.getAttribute('data-row');
        const c = td.getAttribute('data-col');
        if (vSet.has(r) || vSet.has(c)) {
          td.classList.add(cssClass);
        }
      });
    },

    /**
     * Show a warning message to the user.
     * @param {string} message
     */
    showWarning(message) {
      alert(message);
    }
  };

  window.UIHelpers = UIHelpers;
})();
