#include "mainwindow.h"
#include "graphwidget.h"
#include "graphanalysis.h"
#include "shimbel.h"
#include "routesearch.h"
#include "bicomp.h"
#include "dijkstraneg.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>

#include <climits>

// -----------------------------------------------------------------------
//  Constructor
// -----------------------------------------------------------------------
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    tabs_ = new QTabWidget;
    graphWidget_ = new GraphWidget;

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(tabs_);
    splitter->addWidget(graphWidget_);
    splitter->setStretchFactor(0, 3);   // tabs ~60%
    splitter->setStretchFactor(1, 2);   // graph ~40%
    setCentralWidget(splitter);

    auto* tab1 = new QWidget;
    auto* tab2 = new QWidget;
    auto* tab3 = new QWidget;
    auto* tab4 = new QWidget;
    auto* tab5 = new QWidget;
    auto* tab6 = new QWidget;

    setupTab1(tab1);
    setupTab2(tab2);
    setupTab3(tab3);
    setupTab4(tab4);
    setupTab5(tab5);
    setupTab6(tab6);

    tabs_->addTab(tab1, "Граф и Анализ");
    tabs_->addTab(tab2, "Метод Шимбелла");
    tabs_->addTab(tab3, "Поиск маршрута");
    tabs_->addTab(tab4, "Точки сочленения");
    tabs_->addTab(tab5, "Дейкстра (отр. веса)");
    tabs_->addTab(tab6, "Сравнение алгоритмов");

    connect(tabs_, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    setWindowTitle("Лаб. 1-2 — Теория графов");
    resize(1400, 800);
}

// -----------------------------------------------------------------------
//  Tab 1 — Graph generation + Task 2 analysis
// -----------------------------------------------------------------------
void MainWindow::setupTab1(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);

    // --- Row 1: main parameters ---
    auto* row1 = new QHBoxLayout;

    row1->addWidget(new QLabel("Кол-во вершин:"));
    vertexCount_ = new QSpinBox;
    vertexCount_->setRange(1, 100);
    vertexCount_->setValue(6);
    row1->addWidget(vertexCount_);

    row1->addSpacing(12);
    row1->addWidget(new QLabel("p (степени):"));
    paramP_ = new QDoubleSpinBox;
    paramP_->setRange(0.01, 0.99);
    paramP_->setSingleStep(0.05);
    paramP_->setValue(0.50);
    row1->addWidget(paramP_);

    row1->addSpacing(12);
    row1->addWidget(new QLabel("p (веса):"));
    weightParamP_ = new QDoubleSpinBox;
    weightParamP_->setRange(0.01, 0.99);
    weightParamP_->setSingleStep(0.05);
    weightParamP_->setValue(0.40);
    row1->addWidget(weightParamP_);

    row1->addSpacing(12);
    directedCheck_ = new QCheckBox("Ориентированный");
    row1->addWidget(directedCheck_);

    row1->addStretch();
    layout->addLayout(row1);

    // --- Row 2: weight type + Generate button ---
    auto* row2 = new QHBoxLayout;

    row2->addWidget(new QLabel("Веса:"));
    positiveRadio_ = new QRadioButton("Положительные");
    negativeRadio_ = new QRadioButton("Отрицательные");
    mixedRadio_    = new QRadioButton("Смешанные");
    positiveRadio_->setChecked(true);
    row2->addWidget(positiveRadio_);
    row2->addWidget(negativeRadio_);
    row2->addWidget(mixedRadio_);
    row2->addStretch();

    auto* generateBtn = new QPushButton("Сгенерировать");
    connect(generateBtn, &QPushButton::clicked, this, &MainWindow::onGenerate);
    row2->addWidget(generateBtn);
    layout->addLayout(row2);

    // --- Matrices side-by-side ---
    auto* splitter = new QSplitter(Qt::Horizontal);

    auto* adjGroup = new QWidget;
    auto* adjLay   = new QVBoxLayout(adjGroup);
    adjLay->setContentsMargins(0, 0, 0, 0);
    adjLay->addWidget(new QLabel("Матрица смежности (невзвешенная)"));
    adjTable_ = new QTableWidget;
    adjLay->addWidget(adjTable_);
    splitter->addWidget(adjGroup);

    auto* wGroup = new QWidget;
    auto* wLay   = new QVBoxLayout(wGroup);
    wLay->setContentsMargins(0, 0, 0, 0);
    wLay->addWidget(new QLabel("Матрица смежности (взвешенная)"));
    weightTable_ = new QTableWidget;
    wLay->addWidget(weightTable_);
    splitter->addWidget(wGroup);

    layout->addWidget(splitter, 1);

    // --- Analysis results ---
    auto* analysisGroup = new QGroupBox("Анализ (Задание 2)");
    auto* aLay = new QVBoxLayout(analysisGroup);
    analysisText_ = new QTextEdit;
    analysisText_->setReadOnly(true);
    analysisText_->setMaximumHeight(160);
    aLay->addWidget(analysisText_);
    layout->addWidget(analysisGroup);
}

// -----------------------------------------------------------------------
//  Tab 2 — Shimbel method
// -----------------------------------------------------------------------
void MainWindow::setupTab2(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);

    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Длина пути (k):"));
    pathLength_ = new QSpinBox;
    pathLength_->setRange(1, 50);
    pathLength_->setValue(2);
    row->addWidget(pathLength_);

    auto* calcBtn = new QPushButton("Рассчитать");
    connect(calcBtn, &QPushButton::clicked, this, &MainWindow::onShimbelCalculate);
    row->addWidget(calcBtn);
    row->addStretch();
    layout->addLayout(row);

    auto* splitter = new QSplitter(Qt::Horizontal);

    auto* minGroup = new QWidget;
    auto* minLay   = new QVBoxLayout(minGroup);
    minLay->setContentsMargins(0, 0, 0, 0);
    minLay->addWidget(new QLabel("Матрица минимальных путей"));
    minTable_ = new QTableWidget;
    minLay->addWidget(minTable_);
    splitter->addWidget(minGroup);

    auto* maxGroup = new QWidget;
    auto* maxLay   = new QVBoxLayout(maxGroup);
    maxLay->setContentsMargins(0, 0, 0, 0);
    maxLay->addWidget(new QLabel("Матрица максимальных путей"));
    maxTable_ = new QTableWidget;
    maxLay->addWidget(maxTable_);
    splitter->addWidget(maxGroup);

    layout->addWidget(splitter, 1);
}

// -----------------------------------------------------------------------
//  Tab 3 — Route search
// -----------------------------------------------------------------------
void MainWindow::setupTab3(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);

    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("Из вершины:"));
    fromVertex_ = new QSpinBox;
    fromVertex_->setRange(0, 0);
    row->addWidget(fromVertex_);

    row->addSpacing(12);
    row->addWidget(new QLabel("В вершину:"));
    toVertex_ = new QSpinBox;
    toVertex_->setRange(0, 0);
    row->addWidget(toVertex_);

    auto* findBtn = new QPushButton("Найти маршрут");
    connect(findBtn, &QPushButton::clicked, this, &MainWindow::onFindRoute);
    row->addWidget(findBtn);
    row->addStretch();
    layout->addLayout(row);

    routeText_ = new QTextEdit;
    routeText_->setReadOnly(true);
    routeText_->setMaximumHeight(100);
    layout->addWidget(routeText_);

    layout->addWidget(new QLabel("Матрица смежности (маршрут выделен)"));
    routeTable_ = new QTableWidget;
    layout->addWidget(routeTable_, 1);
}

// -----------------------------------------------------------------------
//  Slot: Generate graph
// -----------------------------------------------------------------------
void MainWindow::onGenerate()
{
    const int n        = vertexCount_->value();
    const double p     = paramP_->value();
    const double wp    = weightParamP_->value();
    const bool dir     = directedCheck_->isChecked();

    WeightType wt = WeightType::Positive;
    if (negativeRadio_->isChecked()) wt = WeightType::Negative;
    else if (mixedRadio_->isChecked()) wt = WeightType::Mixed;

    graph_    = GraphGenerator::generate(n, p, dir, wt, wp);
    hasGraph_ = true;

    // Update graph visualization
    graphWidget_->setGraph(graph_);

    // Clear stale results on other tabs
    minTable_->clear();
    minTable_->setRowCount(0);
    minTable_->setColumnCount(0);
    maxTable_->clear();
    maxTable_->setRowCount(0);
    maxTable_->setColumnCount(0);
    routeTable_->clear();
    routeTable_->setRowCount(0);
    routeTable_->setColumnCount(0);
    routeText_->clear();
    bicompText_->clear();
    bicompTable_->clear();
    bicompTable_->setRowCount(0);
    bicompTable_->setColumnCount(0);
    dijkText_->clear();
    dijkWeightTable_->clear();
    dijkWeightTable_->setRowCount(0);
    dijkWeightTable_->setColumnCount(0);
    cmpText_->clear();

    // Display matrices
    displayMatrix(adjTable_,    graph_.adjMatrix);
    displayMatrix(weightTable_, graph_.weightMatrix);

    // --- Task 2: Analysis ---
    auto res = GraphAnalysis::analyze(graph_.adjMatrix, dir);

    QString text;

    // Special case: single vertex
    if (n == 1) {
        text += "Граф состоит из одной вершины (без рёбер).\n\n";
        text += "Эксцентриситет: v0 = 0\n";
        text += "Центр графа: { 0 }\n";
        text += "Диаметр: 0\n";
        text += "Диаметральные вершины: { 0 }";
        analysisText_->setText(text);
        fromVertex_->setRange(0, 0);
        toVertex_->setRange(0, 0);
        pathLength_->setRange(1, 1);
        return;
    }

    text += "Эксцентриситеты:\n";
    int unreachableCount = 0;
    for (int i = 0; i < n; ++i) {
        QString ecc = (res.eccentricities[i] == INT_MAX)
                          ? QString::fromUtf8("∞")
                          : QString::number(res.eccentricities[i]);
        text += QString("  v%1 = %2\n").arg(i).arg(ecc);
        if (res.eccentricities[i] == INT_MAX) ++unreachableCount;
    }

    if (unreachableCount > 0) {
        text += QString("\n⚠ %1 вершин(а) имеют эксц. = ∞ "
                        "(не все вершины достижимы; граф ориентированный).\n")
                    .arg(unreachableCount);
    }

    text += "\nЦентр графа: { ";
    for (size_t i = 0; i < res.center.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.center[i]);
    }
    text += " }\n";

    QString diam = (res.diameter == INT_MAX)
                       ? QString::fromUtf8("∞")
                       : QString::number(res.diameter);
    text += QString("Диаметр: %1\n").arg(diam);

    text += "Диаметральные вершины: { ";
    for (size_t i = 0; i < res.diametralVertices.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.diametralVertices[i]);
    }
    text += " }";

    analysisText_->setText(text);

    // Update spin-boxes on Tab 2, Tab 3, and Tab 5
    pathLength_->setRange(1, std::max(1, n - 1));
    fromVertex_->setRange(0, n - 1);
    toVertex_->setRange(0, n - 1);
    dijkSrcVertex_->setRange(0, n - 1);
    dijkDstVertex_->setRange(0, std::max(0, n - 1));
    if (n > 1) dijkDstVertex_->setValue(1);

    // Update Tab 5 weight type label
    if (positiveRadio_->isChecked())
        dijkWeightTypeLabel_->setText("Тип весов: Положительные");
    else if (negativeRadio_->isChecked())
        dijkWeightTypeLabel_->setText("Тип весов: Отрицательные");
    else
        dijkWeightTypeLabel_->setText("Тип весов: Смешанные");
}

// -----------------------------------------------------------------------
//  Slot: Shimbel calculation
// -----------------------------------------------------------------------
void MainWindow::onShimbelCalculate()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    const int n = graph_.n;
    const int k = pathLength_->value();

    if (n <= 1) {
        QMessageBox::information(this, "Информация",
            "Граф содержит ≤ 1 вершины — метод Шимбелла не применим.");
        return;
    }

    if (k >= n) {
        auto reply = QMessageBox::question(this, "Предупреждение",
            QString("Длина пути k = %1 ≥ кол-ва вершин n = %2.\n"
                    "Для дерева из %2 вершин маршрутов такой длины скорее всего нет.\n"
                    "Продолжить?")
                .arg(k).arg(n),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
    }

    auto minRes = Shimbel::minPath(graph_.weightMatrix, k);
    auto maxRes = Shimbel::maxPath(graph_.weightMatrix, k);

    displayMatrix(minTable_, minRes,  INT_MAX / 2);   // POS_INF sentinel
    displayMatrix(maxTable_, maxRes,  INT_MIN / 2);   // NEG_INF sentinel
}

// -----------------------------------------------------------------------
//  Slot: Find route
// -----------------------------------------------------------------------
void MainWindow::onFindRoute()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    const int from = fromVertex_->value();
    const int to   = toVertex_->value();

    if (from == to) {
        routeText_->setText(
            QString("Вершина %1 = вершина %2 — тривиальный маршрут (длина 0, 1 путь).")
                .arg(from).arg(to));
        displayMatrix(routeTable_, graph_.adjMatrix);
        graphWidget_->highlightRoute({from});
        return;
    }

    auto res = RouteSearch::findRoute(graph_.adjMatrix, from, to);

    QString text;
    if (res.exists) {
        text += QString("Маршрут из %1 в %2 существует.\n").arg(from).arg(to);
        text += QString("Количество простых путей: %1\n").arg(res.count);
        text += "Путь: ";
        for (size_t i = 0; i < res.path.size(); ++i) {
            if (i) text += QString::fromUtf8(" → ");
            text += QString::number(res.path[i]);
        }
    } else {
        text = QString("Маршрут из %1 в %2 не существует.\n"
                       "(Граф ориентированный — возможно, нет направленного пути.)")
                   .arg(from).arg(to);
    }
    routeText_->setText(text);

    // Display adjacency matrix with highlighted path edges
    displayMatrix(routeTable_, graph_.adjMatrix);
    if (res.exists) {
        std::vector<std::pair<int,int>> edges;
        for (size_t i = 0; i + 1 < res.path.size(); ++i)
            edges.push_back({res.path[i], res.path[i + 1]});
        highlightPath(routeTable_, edges);
        graphWidget_->highlightRoute(res.path);
    } else {
        graphWidget_->clearHighlights();
    }
}

// -----------------------------------------------------------------------
//  Helpers
// -----------------------------------------------------------------------
void MainWindow::displayMatrix(QTableWidget* table,
                               const std::vector<std::vector<int>>& matrix,
                               int sentinel)
{
    const int n = static_cast<int>(matrix.size());
    table->clear();
    table->setRowCount(n);
    table->setColumnCount(n);

    QStringList headers;
    for (int i = 0; i < n; ++i)
        headers << QString::number(i);
    table->setHorizontalHeaderLabels(headers);
    table->setVerticalHeaderLabels(headers);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            QString text;
            if (sentinel > 0 && matrix[i][j] >= sentinel)
                text = QString::fromUtf8("∞");
            else if (sentinel < 0 && matrix[i][j] <= sentinel)
                text = QString::fromUtf8("∞");
            else
                text = QString::number(matrix[i][j]);

            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            table->setItem(i, j, item);
        }
    }
    table->resizeColumnsToContents();
}

void MainWindow::highlightPath(QTableWidget* table,
                               const std::vector<std::pair<int,int>>& edges)
{
    const QColor color(144, 238, 144);   // light green
    for (auto& [from, to] : edges) {
        if (auto* item = table->item(from, to))
            item->setBackground(color);
    }
}

void MainWindow::highlightCells(QTableWidget* table,
                                const std::vector<int>& vertices,
                                const QColor& color)
{
    for (int v : vertices) {
        // Highlight the entire row and column header area by coloring diagonal
        if (auto* item = table->item(v, v))
            item->setBackground(color);
        // Highlight entire row
        for (int j = 0; j < table->columnCount(); ++j)
            if (auto* item = table->item(v, j))
                item->setBackground(color);
        // Highlight entire column
        for (int i = 0; i < table->rowCount(); ++i)
            if (auto* item = table->item(i, v))
                item->setBackground(color);
    }
}

// -----------------------------------------------------------------------
//  Tab 4 — BiComp (articulation points)
// -----------------------------------------------------------------------
void MainWindow::setupTab4(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);

    auto* row = new QHBoxLayout;
    auto* findBtn = new QPushButton("Найти точки сочленения");
    connect(findBtn, &QPushButton::clicked, this, &MainWindow::onFindBiComp);
    row->addWidget(findBtn);
    row->addStretch();
    layout->addLayout(row);

    bicompText_ = new QTextEdit;
    bicompText_->setReadOnly(true);
    bicompText_->setMaximumHeight(200);
    layout->addWidget(bicompText_);

    layout->addWidget(new QLabel("Матрица смежности (точки сочленения выделены)"));
    bicompTable_ = new QTableWidget;
    layout->addWidget(bicompTable_, 1);
}

// -----------------------------------------------------------------------
//  Tab 5 — Dijkstra for negative weights
// -----------------------------------------------------------------------
void MainWindow::setupTab5(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);

    auto* row1 = new QHBoxLayout;
    dijkWeightTypeLabel_ = new QLabel("Тип весов: (сначала сгенерируйте граф)");
    row1->addWidget(dijkWeightTypeLabel_);
    row1->addStretch();
    layout->addLayout(row1);

    auto* row2 = new QHBoxLayout;
    row2->addWidget(new QLabel("Из вершины:"));
    dijkSrcVertex_ = new QSpinBox;
    dijkSrcVertex_->setRange(0, 0);
    row2->addWidget(dijkSrcVertex_);

    row2->addSpacing(12);
    row2->addWidget(new QLabel("В вершину:"));
    dijkDstVertex_ = new QSpinBox;
    dijkDstVertex_->setRange(0, 0);
    row2->addWidget(dijkDstVertex_);

    auto* calcBtn = new QPushButton("Рассчитать Дейкстру");
    connect(calcBtn, &QPushButton::clicked, this, &MainWindow::onDijkstraCalculate);
    row2->addWidget(calcBtn);
    row2->addStretch();
    layout->addLayout(row2);

    dijkText_ = new QTextEdit;
    dijkText_->setReadOnly(true);
    dijkText_->setMaximumHeight(180);
    layout->addWidget(dijkText_);

    layout->addWidget(new QLabel("Весовая матрица (путь выделен)"));
    dijkWeightTable_ = new QTableWidget;
    layout->addWidget(dijkWeightTable_, 1);
}

// -----------------------------------------------------------------------
//  Tab 6 — Speed comparison
// -----------------------------------------------------------------------
void MainWindow::setupTab6(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);

    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel("n от:"));
    cmpMinN_ = new QSpinBox;
    cmpMinN_->setRange(2, 500);
    cmpMinN_->setValue(5);
    row->addWidget(cmpMinN_);

    row->addSpacing(8);
    row->addWidget(new QLabel("до:"));
    cmpMaxN_ = new QSpinBox;
    cmpMaxN_->setRange(2, 500);
    cmpMaxN_->setValue(50);
    row->addWidget(cmpMaxN_);

    row->addSpacing(8);
    row->addWidget(new QLabel("шаг:"));
    cmpStep_ = new QSpinBox;
    cmpStep_->setRange(1, 100);
    cmpStep_->setValue(5);
    row->addWidget(cmpStep_);

    auto* runBtn = new QPushButton("Запустить сравнение");
    connect(runBtn, &QPushButton::clicked, this, &MainWindow::onRunComparison);
    row->addWidget(runBtn);
    row->addStretch();
    layout->addLayout(row);

    cmpText_ = new QTextEdit;
    cmpText_->setReadOnly(true);
    cmpText_->setMaximumHeight(120);
    layout->addWidget(cmpText_);

    cmpTable_ = new QTableWidget;
    layout->addWidget(cmpTable_, 1);
}

// -----------------------------------------------------------------------
//  Slot: Find articulation points (BiComp)
// -----------------------------------------------------------------------
void MainWindow::onFindBiComp()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    const int n = graph_.n;
    if (n <= 1) {
        bicompText_->setText("Граф содержит ≤ 1 вершины — точек сочленения нет.");
        displayMatrix(bicompTable_, graph_.adjMatrix);
        graphWidget_->clearHighlights();
        return;
    }

    // For directed graphs, treat as undirected for BiComp
    auto adj = graph_.adjMatrix;
    if (graph_.directed) {
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) {
                if (adj[i][j] || adj[j][i]) {
                    adj[i][j] = 1;
                    adj[j][i] = 1;
                }
            }
    }

    auto res = BiComp::find(adj);

    QString text;
    text += QString("Количество итераций (BiComp): %1\n\n").arg(res.iterations);

    if (res.articulationPoints.empty()) {
        text += "Точек сочленения нет.\n";
    } else {
        text += "Точки сочленения: { ";
        bool first = true;
        for (int v : res.articulationPoints) {
            if (!first) text += ", ";
            text += QString::number(v);
            first = false;
        }
        text += " }\n";
    }

    text += QString("\nКоличество блоков (двусвязных компонент): %1\n")
                .arg(static_cast<int>(res.blocks.size()));

    for (size_t b = 0; b < res.blocks.size(); ++b) {
        text += QString("  Блок %1: рёбра { ").arg(b + 1);
        for (size_t e = 0; e < res.blocks[b].size(); ++e) {
            if (e) text += ", ";
            text += QString("(%1,%2)")
                        .arg(res.blocks[b][e].first)
                        .arg(res.blocks[b][e].second);
        }
        text += " }\n";
    }

    bicompText_->setText(text);

    // Display matrix with highlighted articulation points
    displayMatrix(bicompTable_, adj);
    std::vector<int> artVec(res.articulationPoints.begin(),
                            res.articulationPoints.end());
    highlightCells(bicompTable_, artVec, QColor(255, 200, 200));

    // Update graph visualization
    graphWidget_->highlightBiComp(res.articulationPoints, res.blocks);
}

// -----------------------------------------------------------------------
//  Slot: Dijkstra for negative weights
// -----------------------------------------------------------------------
void MainWindow::onDijkstraCalculate()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    const int n = graph_.n;
    if (n <= 1) {
        dijkText_->setText("Граф содержит ≤ 1 вершины — алгоритм Дейкстры не применим.");
        return;
    }

    const int src = dijkSrcVertex_->value();
    const int dst = dijkDstVertex_->value();

    // Use the same weight matrix generated in Tab 1
    auto res = DijkstraNeg::solve(graph_.weightMatrix, src, graph_.directed);

    QString text;
    text += QString("Количество итераций (Дейкстра): %1\n\n").arg(res.iterations);

    if (res.hasNegativeCycle) {
        text += "⚠ Обнаружен отрицательный цикл!\n";
    }

    // Distance vector
    text += "Вектор расстояний от вершины " + QString::number(src) + ":\n";
    for (int i = 0; i < n; ++i) {
        QString d = (res.dist[i] >= INT_MAX / 2)
                        ? QString::fromUtf8("∞")
                        : QString::number(res.dist[i]);
        text += QString("  d[%1] = %2\n").arg(i).arg(d);
    }

    // Shortest path
    auto path = DijkstraNeg::reconstructPath(res.parent, src, dst);
    text += "\n";
    if (src == dst) {
        text += QString("Путь из %1 в %2: тривиальный (длина 0).").arg(src).arg(dst);
    } else if (path.empty()) {
        text += QString("Путь из %1 в %2 не существует.").arg(src).arg(dst);
    } else {
        QString d = (res.dist[dst] >= INT_MAX / 2)
                        ? QString::fromUtf8("∞")
                        : QString::number(res.dist[dst]);
        text += QString("Кратчайший путь из %1 в %2 (длина = %3):\n")
                    .arg(src).arg(dst).arg(d);
        for (size_t i = 0; i < path.size(); ++i) {
            if (i) text += QString::fromUtf8(" → ");
            text += QString::number(path[i]);
        }
    }

    dijkText_->setText(text);

    // Display weight matrix with path highlighted
    displayMatrix(dijkWeightTable_, graph_.weightMatrix);
    if (!path.empty()) {
        std::vector<std::pair<int,int>> edges;
        for (size_t i = 0; i + 1 < path.size(); ++i)
            edges.push_back({path[i], path[i + 1]});
        highlightPath(dijkWeightTable_, edges);
        graphWidget_->highlightDijkstraPath(path);
    } else {
        graphWidget_->clearHighlights();
    }
}

// -----------------------------------------------------------------------
//  Slot: Run speed comparison
// -----------------------------------------------------------------------
void MainWindow::onRunComparison()
{
    const int minN = cmpMinN_->value();
    const int maxN = cmpMaxN_->value();
    const int step = cmpStep_->value();

    if (minN > maxN) {
        QMessageBox::warning(this, "Внимание", "n (от) должно быть ≤ n (до).");
        return;
    }

    struct Row { int n; int bicompIter; int dijkstraIter; };
    std::vector<Row> rows;

    for (int n = minN; n <= maxN; n += step) {
        auto g = GraphGenerator::generate(n, 0.5, false, WeightType::Positive, 0.4);

        // BiComp
        auto biRes = BiComp::find(g.adjMatrix);

        // Dijkstra (from vertex 0)
        auto djRes = DijkstraNeg::solve(g.weightMatrix, 0, false);

        rows.push_back({n, biRes.iterations, djRes.iterations});
    }

    // Fill table
    cmpTable_->clear();
    cmpTable_->setRowCount(static_cast<int>(rows.size()));
    cmpTable_->setColumnCount(3);
    cmpTable_->setHorizontalHeaderLabels(
        {"n (вершин)", "BiComp (итер.)", "Дейкстра (итер.)"});

    for (int r = 0; r < static_cast<int>(rows.size()); ++r) {
        auto setItem = [&](int col, const QString& txt) {
            auto* item = new QTableWidgetItem(txt);
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            cmpTable_->setItem(r, col, item);
        };
        setItem(0, QString::number(rows[r].n));
        setItem(1, QString::number(rows[r].bicompIter));
        setItem(2, QString::number(rows[r].dijkstraIter));
    }
    cmpTable_->resizeColumnsToContents();

    // Summary
    QString text;
    text += "Сравнение количества итераций:\n";
    text += QString("Диапазон: n = %1..%2, шаг = %3\n")
                .arg(minN).arg(maxN).arg(step);
    if (!rows.empty()) {
        text += QString("BiComp: от %1 до %2 итераций\n")
                    .arg(rows.front().bicompIter)
                    .arg(rows.back().bicompIter);
        text += QString("Дейкстра: от %1 до %2 итераций\n")
                    .arg(rows.front().dijkstraIter)
                    .arg(rows.back().dijkstraIter);
    }
    cmpText_->setText(text);
}

// -----------------------------------------------------------------------
//  Slot: Tab changed — update graph widget highlights
// -----------------------------------------------------------------------
void MainWindow::onTabChanged(int index)
{
    // Tabs 0 (Граф), 1 (Шимбелл), 5 (Сравнение) — just show the plain graph
    // Tabs 2, 3, 4 — keep highlights set by their algorithm slots
    switch (index) {
    case 0:
    case 1:
    case 5:
        graphWidget_->clearHighlights();
        break;
    default:
        break;
    }
}
