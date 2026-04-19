#include "mainwindow.h"
#include "graphwidget.h"
#include "graphanalysis.h"
#include "shimbel.h"
#include "routesearch.h"
#include "bicomp.h"
#include "dijkstraneg.h"
#include "bellmanford.h"
#include "maxflow.h"
#include "mincostflow.h"
#include "spanningtreecount.h"
#include "mstprim.h"
#include "vertexcover.h"

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
#include <random>

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
    auto* tab7 = new QWidget;
    auto* tab8 = new QWidget;

    setupTab1(tab1);
    setupTab2(tab2);
    setupTab3(tab3);
    setupTab4(tab4);
    setupTab5(tab5);
    setupTab6(tab6);
    setupTab7(tab7);
    setupTab8(tab8);

    tabs_->addTab(tab1, "Граф и Анализ");
    tabs_->addTab(tab2, "Метод Шимбелла");
    tabs_->addTab(tab3, "Поиск маршрута");
    tabs_->addTab(tab4, "Точки сочленения");
    tabs_->addTab(tab5, "Дейкстра (отр. веса)");
    tabs_->addTab(tab6, "Сравнение алгоритмов");
    tabs_->addTab(tab7, "Поток");
    tabs_->addTab(tab8, "Лаб 4");

    connect(tabs_, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    setWindowTitle("Поток");
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

    auto* generateDAGBtn = new QPushButton("Сгенерировать DAG");
    connect(generateDAGBtn, &QPushButton::clicked, this, &MainWindow::onGenerateDAG);
    row2->addWidget(generateDAGBtn);
    layout->addLayout(row2);

    // --- Row 3: Regenerate weights buttons ---
    auto* row3 = new QHBoxLayout;
    row3->addWidget(new QLabel("Перегенерировать веса:"));

    auto* regenPosBtn = new QPushButton("Положительные");
    connect(regenPosBtn, &QPushButton::clicked, this, &MainWindow::onRegenWeightsPositive);
    row3->addWidget(regenPosBtn);

    auto* regenNegBtn = new QPushButton("Отрицательные");
    connect(regenNegBtn, &QPushButton::clicked, this, &MainWindow::onRegenWeightsNegative);
    row3->addWidget(regenNegBtn);

    auto* regenMixBtn = new QPushButton("Смешанные");
    connect(regenMixBtn, &QPushButton::clicked, this, &MainWindow::onRegenWeightsMixed);
    row3->addWidget(regenMixBtn);

    row3->addStretch();
    layout->addLayout(row3);

    // --- Row 4: Rearrange directed/undirected buttons ---
    auto* row4 = new QHBoxLayout;
    row4->addWidget(new QLabel("Перестроить граф:"));

    auto* rearrangeDirBtn = new QPushButton("Ориентированный");
    connect(rearrangeDirBtn, &QPushButton::clicked, this, &MainWindow::onRearrangeDirected);
    row4->addWidget(rearrangeDirBtn);

    auto* rearrangeUndirBtn = new QPushButton("Неориентированный");
    connect(rearrangeUndirBtn, &QPushButton::clicked, this, &MainWindow::onRearrangeUndirected);
    row4->addWidget(rearrangeUndirBtn);

    row4->addStretch();
    layout->addLayout(row4);

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
    wLay->addWidget(new QLabel("Матрица весов"));
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
    pathLength_->setRange(0, 50);
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

    // Store original matrices for directed/undirected rearrangement
    originalAdjMatrix_    = graph_.adjMatrix;
    originalWeightMatrix_ = graph_.weightMatrix;

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
    dijkStagesTable_->clear();
    dijkStagesTable_->setRowCount(0);
    dijkStagesTable_->setColumnCount(0);
    dijkWeightTable_->clear();
    dijkWeightTable_->setRowCount(0);
    dijkWeightTable_->setColumnCount(0);
    cmpText_->clear();

    // Clear Tab 7 (Поток)
    capacityTable_->clear();  capacityTable_->setRowCount(0);  capacityTable_->setColumnCount(0);
    costTable_->clear();      costTable_->setRowCount(0);      costTable_->setColumnCount(0);
    maxFlowText_->clear();
    maxFlowTable_->clear();   maxFlowTable_->setRowCount(0);   maxFlowTable_->setColumnCount(0);
    minCostFlowText_->clear();
    minCostFlowTable_->clear(); minCostFlowTable_->setRowCount(0); minCostFlowTable_->setColumnCount(0);
    hasFlowNetwork_ = false;
    lastMaxFlow_ = 0;

    // Clear Tab 8 (Лаб 4)
    lab4Task1Text_->clear();
    lab4Task1Table_->clear();    lab4Task1Table_->setRowCount(0);    lab4Task1Table_->setColumnCount(0);
    lab4Task2Text_->clear();
    lab4Task2Table_->clear();    lab4Task2Table_->setRowCount(0);    lab4Task2Table_->setColumnCount(0);
    lab4Task3Text_->clear();
    lab4Task3Table_->clear();    lab4Task3Table_->setRowCount(0);    lab4Task3Table_->setColumnCount(0);
    mstAdjMatrix_.clear();
    hasMST_ = false;
    vcMSTRadio_->setEnabled(false);
    vcFullGraphRadio_->setChecked(true);

    // Display matrices
    displayMatrix(adjTable_,    graph_.adjMatrix, 0, MatrixMode::Adjacency);
    displayMatrix(weightTable_, graph_.weightMatrix, 0, MatrixMode::Weighted);

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
    for (int i = 0; i < n; ++i) {
        text += QString("  v%1 = %2\n").arg(i).arg(res.eccentricities[i]);
    }

    text += "\nЦентр графа: { ";
    for (size_t i = 0; i < res.center.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.center[i]);
    }
    text += " }\n";

    text += QString("Диаметр: %1\n").arg(res.diameter);

    text += "Диаметральные вершины: { ";
    for (size_t i = 0; i < res.diametralVertices.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.diametralVertices[i]);
    }
    text += " }";

    analysisText_->setText(text);

    // Update spin-boxes on Tab 2, Tab 3, and Tab 5
    pathLength_->setRange(0, std::max(1, n - 1));
    fromVertex_->setRange(0, n - 1);
    toVertex_->setRange(0, n - 1);
    dijkSrcVertex_->setRange(0, n - 1);
    dijkDstVertex_->setRange(0, std::max(0, n - 1));
    if (n > 1) dijkDstVertex_->setValue(1);

    // Update Tab 7 spin boxes
    flowSrcVertex_->setRange(0, std::max(0, n - 1));
    flowSinkVertex_->setRange(0, std::max(0, n - 1));
    if (n > 1) flowSinkVertex_->setValue(n - 1);

    // Update Tab 5 weight type label
    if (positiveRadio_->isChecked())
        dijkWeightTypeLabel_->setText("Тип весов: Положительные");
    else if (negativeRadio_->isChecked())
        dijkWeightTypeLabel_->setText("Тип весов: Отрицательные");
    else
        dijkWeightTypeLabel_->setText("Тип весов: Смешанные");
}

// -----------------------------------------------------------------------
//  Slot: Generate DAG (richer connectivity for flow algorithms)
// -----------------------------------------------------------------------
void MainWindow::onGenerateDAG()
{
    const int n        = vertexCount_->value();
    const double p     = paramP_->value();
    const double wp    = weightParamP_->value();

    const bool dir = directedCheck_->isChecked();

    WeightType wt = WeightType::Positive;
    if (negativeRadio_->isChecked()) wt = WeightType::Negative;
    else if (mixedRadio_->isChecked()) wt = WeightType::Mixed;

    graph_    = GraphGenerator::generateDAG(n, p, dir, wt, wp);
    hasGraph_ = true;

    // Store original matrices for directed/undirected rearrangement
    originalAdjMatrix_    = graph_.adjMatrix;
    originalWeightMatrix_ = graph_.weightMatrix;

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
    dijkStagesTable_->clear();
    dijkStagesTable_->setRowCount(0);
    dijkStagesTable_->setColumnCount(0);
    dijkWeightTable_->clear();
    dijkWeightTable_->setRowCount(0);
    dijkWeightTable_->setColumnCount(0);
    cmpText_->clear();

    // Clear Tab 7 (Поток)
    capacityTable_->clear();  capacityTable_->setRowCount(0);  capacityTable_->setColumnCount(0);
    costTable_->clear();      costTable_->setRowCount(0);      costTable_->setColumnCount(0);
    maxFlowText_->clear();
    maxFlowTable_->clear();   maxFlowTable_->setRowCount(0);   maxFlowTable_->setColumnCount(0);
    minCostFlowText_->clear();
    minCostFlowTable_->clear(); minCostFlowTable_->setRowCount(0); minCostFlowTable_->setColumnCount(0);
    hasFlowNetwork_ = false;
    lastMaxFlow_ = 0;

    // Clear Tab 8 (Лаб 4)
    lab4Task1Text_->clear();
    lab4Task1Table_->clear();    lab4Task1Table_->setRowCount(0);    lab4Task1Table_->setColumnCount(0);
    lab4Task2Text_->clear();
    lab4Task2Table_->clear();    lab4Task2Table_->setRowCount(0);    lab4Task2Table_->setColumnCount(0);
    lab4Task3Text_->clear();
    lab4Task3Table_->clear();    lab4Task3Table_->setRowCount(0);    lab4Task3Table_->setColumnCount(0);
    mstAdjMatrix_.clear();
    hasMST_ = false;
    vcMSTRadio_->setEnabled(false);
    vcFullGraphRadio_->setChecked(true);

    // Display matrices
    displayMatrix(adjTable_,    graph_.adjMatrix, 0, MatrixMode::Adjacency);
    displayMatrix(weightTable_, graph_.weightMatrix, 0, MatrixMode::Weighted);

    // --- Analysis ---
    auto res = GraphAnalysis::analyze(graph_.adjMatrix, dir);

    QString text;

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
    for (int i = 0; i < n; ++i) {
        text += QString("  v%1 = %2\n").arg(i).arg(res.eccentricities[i]);
    }

    text += "\nЦентр графа: { ";
    for (size_t i = 0; i < res.center.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.center[i]);
    }
    text += " }\n";

    text += QString("Диаметр: %1\n").arg(res.diameter);

    text += "Диаметральные вершины: { ";
    for (size_t i = 0; i < res.diametralVertices.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.diametralVertices[i]);
    }
    text += " }";

    analysisText_->setText(text);

    // Update spin-boxes on other tabs
    pathLength_->setRange(0, std::max(1, n - 1));
    fromVertex_->setRange(0, n - 1);
    toVertex_->setRange(0, n - 1);
    dijkSrcVertex_->setRange(0, n - 1);
    dijkDstVertex_->setRange(0, std::max(0, n - 1));
    if (n > 1) dijkDstVertex_->setValue(1);

    flowSrcVertex_->setRange(0, std::max(0, n - 1));
    flowSinkVertex_->setRange(0, std::max(0, n - 1));
    if (n > 1) flowSinkVertex_->setValue(n - 1);

    dijkWeightTypeLabel_->setText("Тип весов: Положительные");
    if (negativeRadio_->isChecked())
        dijkWeightTypeLabel_->setText("Тип весов: Отрицательные");
    else if (mixedRadio_->isChecked())
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
        displayMatrix(routeTable_, graph_.adjMatrix, 0, MatrixMode::Adjacency);
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
    displayMatrix(routeTable_, graph_.adjMatrix, 0, MatrixMode::Adjacency);
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
                               int sentinel,
                               MatrixMode mode,
                               bool editable)
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
            if (editable) {
                text = QString::number(matrix[i][j]);
            } else if (sentinel > 0 && matrix[i][j] >= sentinel) {
                text = QString::fromUtf8("∞");
            } else if (sentinel < 0 && matrix[i][j] <= sentinel) {
                text = QString::fromUtf8("∞");
            } else if (i != j && matrix[i][j] == 0 && mode == MatrixMode::Weighted) {
                text = QString::fromUtf8("∞");
            } else if (i != j && matrix[i][j] == 0 && mode == MatrixMode::Adjacency) {
                text = QStringLiteral("-");
            } else {
                text = QString::number(matrix[i][j]);
            }

            auto* item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignCenter);
            if (!editable || i == j)
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
    dijkText_->setMaximumHeight(120);
    layout->addWidget(dijkText_);

    auto* splitter = new QSplitter(Qt::Vertical);

    auto* stagesGroup = new QWidget;
    auto* stagesLay = new QVBoxLayout(stagesGroup);
    stagesLay->setContentsMargins(0, 0, 0, 0);
    stagesLay->addWidget(new QLabel("Таблица Дейкстры (по этапам)"));
    dijkStagesTable_ = new QTableWidget;
    stagesLay->addWidget(dijkStagesTable_);
    splitter->addWidget(stagesGroup);

    auto* weightGroup = new QWidget;
    auto* weightLay = new QVBoxLayout(weightGroup);
    weightLay->setContentsMargins(0, 0, 0, 0);
    weightLay->addWidget(new QLabel("Весовая матрица (путь выделен)"));
    dijkWeightTable_ = new QTableWidget;
    weightLay->addWidget(dijkWeightTable_);
    splitter->addWidget(weightGroup);

    layout->addWidget(splitter, 1);
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
        displayMatrix(bicompTable_, graph_.adjMatrix, 0, MatrixMode::Adjacency);
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
    displayMatrix(bicompTable_, adj, 0, MatrixMode::Adjacency);
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
    text += QString("Количество итераций (Дейкстра): %1\n").arg(res.iterations);

    if (res.hasNegativeCycle) {
        text += "⚠ Обнаружен отрицательный цикл!\n";
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

    // --- Display Dijkstra stages table ---
    {
        const int numStages = static_cast<int>(res.stages.size());
        dijkStagesTable_->clear();
        dijkStagesTable_->setRowCount(numStages);
        dijkStagesTable_->setColumnCount(n + 1);  // +1 for processed vertex column

        QStringList headers;
        headers << "Обр. верш.";
        for (int i = 0; i < n; ++i)
            headers << QString("d[%1]").arg(i);
        dijkStagesTable_->setHorizontalHeaderLabels(headers);

        QStringList rowHeaders;
        for (int s = 0; s < numStages; ++s)
            rowHeaders << QString("Этап %1").arg(s);
        dijkStagesTable_->setVerticalHeaderLabels(rowHeaders);

        for (int s = 0; s < numStages; ++s) {
            // Processed vertex column
            auto* vItem = new QTableWidgetItem(QString("v%1").arg(res.stages[s].processedVertex));
            vItem->setTextAlignment(Qt::AlignCenter);
            vItem->setFlags(vItem->flags() & ~Qt::ItemIsEditable);
            vItem->setBackground(QColor(200, 220, 255));
            dijkStagesTable_->setItem(s, 0, vItem);

            // Distance values
            for (int i = 0; i < n; ++i) {
                QString dStr = (res.stages[s].dist[i] >= INT_MAX / 2)
                                   ? QString::fromUtf8("∞")
                                   : QString::number(res.stages[s].dist[i]);
                auto* item = new QTableWidgetItem(dStr);
                item->setTextAlignment(Qt::AlignCenter);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                // Highlight the processed vertex column
                if (i == res.stages[s].processedVertex)
                    item->setBackground(QColor(200, 220, 255));
                dijkStagesTable_->setItem(s, i + 1, item);
            }
        }
        dijkStagesTable_->resizeColumnsToContents();
    }

    // Display weight matrix with path highlighted
    displayMatrix(dijkWeightTable_, graph_.weightMatrix, 0, MatrixMode::Weighted);
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
//  Slots: Regenerate weights
// -----------------------------------------------------------------------
void MainWindow::onRegenWeightsPositive() { doRegenerateWeights(WeightType::Positive); }
void MainWindow::onRegenWeightsNegative() { doRegenerateWeights(WeightType::Negative); }
void MainWindow::onRegenWeightsMixed()    { doRegenerateWeights(WeightType::Mixed); }

void MainWindow::doRegenerateWeights(WeightType wType)
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    const double wp = weightParamP_->value();
    GraphGenerator::regenerateWeights(graph_, wType, wp);

    // Update graph visualization
    graphWidget_->setGraph(graph_);

    // Refresh weight matrix display
    displayMatrix(weightTable_, graph_.weightMatrix, 0, MatrixMode::Weighted);

    // Clear stale results on other tabs
    minTable_->clear();  minTable_->setRowCount(0);  minTable_->setColumnCount(0);
    maxTable_->clear();  maxTable_->setRowCount(0);  maxTable_->setColumnCount(0);
    dijkText_->clear();
    dijkStagesTable_->clear();  dijkStagesTable_->setRowCount(0);  dijkStagesTable_->setColumnCount(0);
    dijkWeightTable_->clear();  dijkWeightTable_->setRowCount(0);  dijkWeightTable_->setColumnCount(0);
    cmpText_->clear();

    // Clear Tab 7 (Поток)
    capacityTable_->clear();  capacityTable_->setRowCount(0);  capacityTable_->setColumnCount(0);
    costTable_->clear();      costTable_->setRowCount(0);      costTable_->setColumnCount(0);
    maxFlowText_->clear();
    maxFlowTable_->clear();   maxFlowTable_->setRowCount(0);   maxFlowTable_->setColumnCount(0);
    minCostFlowText_->clear();
    minCostFlowTable_->clear(); minCostFlowTable_->setRowCount(0); minCostFlowTable_->setColumnCount(0);
    hasFlowNetwork_ = false;
    lastMaxFlow_ = 0;

    // Update weight type label on Dijkstra tab
    if (wType == WeightType::Positive)
        dijkWeightTypeLabel_->setText("Тип весов: Положительные");
    else if (wType == WeightType::Negative)
        dijkWeightTypeLabel_->setText("Тип весов: Отрицательные");
    else
        dijkWeightTypeLabel_->setText("Тип весов: Смешанные");
}

// -----------------------------------------------------------------------
//  Slots: Rearrange graph as directed / undirected
// -----------------------------------------------------------------------
void MainWindow::onRearrangeDirected()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    // Restore original directed matrices
    graph_.adjMatrix    = originalAdjMatrix_;
    graph_.weightMatrix = originalWeightMatrix_;
    graph_.directed     = true;
    directedCheck_->setChecked(true);

    // Refresh display
    graphWidget_->setGraph(graph_);
    displayMatrix(adjTable_,    graph_.adjMatrix,    0, MatrixMode::Adjacency);
    displayMatrix(weightTable_, graph_.weightMatrix, 0, MatrixMode::Weighted);

    // Re-run analysis with directed flag
    const int n = graph_.n;
    auto res = GraphAnalysis::analyze(graph_.adjMatrix, true);
    QString text;
    text += "Эксцентриситеты:\n";
    for (int i = 0; i < n; ++i)
        text += QString("  v%1 = %2\n").arg(i).arg(res.eccentricities[i]);
    text += "\nЦентр графа: { ";
    for (size_t i = 0; i < res.center.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.center[i]);
    }
    text += " }\n";
    text += QString("Диаметр: %1\n").arg(res.diameter);
    text += "Диаметральные вершины: { ";
    for (size_t i = 0; i < res.diametralVertices.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.diametralVertices[i]);
    }
    text += " }";
    analysisText_->setText(text);

    // Clear downstream results
    minTable_->clear();  minTable_->setRowCount(0);  minTable_->setColumnCount(0);
    maxTable_->clear();  maxTable_->setRowCount(0);  maxTable_->setColumnCount(0);
    dijkText_->clear();
    dijkStagesTable_->clear();  dijkStagesTable_->setRowCount(0);  dijkStagesTable_->setColumnCount(0);
    dijkWeightTable_->clear();  dijkWeightTable_->setRowCount(0);  dijkWeightTable_->setColumnCount(0);
    cmpText_->clear();
    capacityTable_->clear();  capacityTable_->setRowCount(0);  capacityTable_->setColumnCount(0);
    costTable_->clear();      costTable_->setRowCount(0);      costTable_->setColumnCount(0);
    maxFlowText_->clear();
    maxFlowTable_->clear();   maxFlowTable_->setRowCount(0);   maxFlowTable_->setColumnCount(0);
    minCostFlowText_->clear();
    minCostFlowTable_->clear(); minCostFlowTable_->setRowCount(0); minCostFlowTable_->setColumnCount(0);
    hasFlowNetwork_ = false;
    lastMaxFlow_ = 0;
}

void MainWindow::onRearrangeUndirected()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    const int n = graph_.n;

    // Build undirected acyclic graph from original edges.
    // Add each original edge as undirected only if it doesn't create a cycle.
    std::vector<std::vector<int>> newAdj(n, std::vector<int>(n, 0));
    std::vector<std::vector<int>> newWeight(n, std::vector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && originalAdjMatrix_[i][j] && !newAdj[i][j]) {
                // Check if i and j are already connected undirectedly
                if (!GraphGenerator::canReachUndirected(newAdj, i, j)) {
                    newAdj[i][j] = 1;
                    newAdj[j][i] = 1;
                    int w = originalWeightMatrix_[i][j] != 0 ? originalWeightMatrix_[i][j]
                                                             : originalWeightMatrix_[j][i];
                    newWeight[i][j] = w;
                    newWeight[j][i] = w;
                }
            }
        }
    }

    graph_.adjMatrix    = newAdj;
    graph_.weightMatrix = newWeight;
    graph_.directed = false;
    directedCheck_->setChecked(false);

    // Refresh display
    graphWidget_->setGraph(graph_);
    displayMatrix(adjTable_,    graph_.adjMatrix,    0, MatrixMode::Adjacency);
    displayMatrix(weightTable_, graph_.weightMatrix, 0, MatrixMode::Weighted);

    // Re-run analysis with undirected flag
    auto res = GraphAnalysis::analyze(graph_.adjMatrix, false);
    QString text;
    text += "Эксцентриситеты:\n";
    for (int i = 0; i < n; ++i)
        text += QString("  v%1 = %2\n").arg(i).arg(res.eccentricities[i]);
    text += "\nЦентр графа: { ";
    for (size_t i = 0; i < res.center.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.center[i]);
    }
    text += " }\n";
    text += QString("Диаметр: %1\n").arg(res.diameter);
    text += "Диаметральные вершины: { ";
    for (size_t i = 0; i < res.diametralVertices.size(); ++i) {
        if (i) text += ", ";
        text += QString::number(res.diametralVertices[i]);
    }
    text += " }";
    analysisText_->setText(text);

    // Clear downstream results
    minTable_->clear();  minTable_->setRowCount(0);  minTable_->setColumnCount(0);
    maxTable_->clear();  maxTable_->setRowCount(0);  maxTable_->setColumnCount(0);
    dijkText_->clear();
    dijkStagesTable_->clear();  dijkStagesTable_->setRowCount(0);  dijkStagesTable_->setColumnCount(0);
    dijkWeightTable_->clear();  dijkWeightTable_->setRowCount(0);  dijkWeightTable_->setColumnCount(0);
    cmpText_->clear();
    capacityTable_->clear();  capacityTable_->setRowCount(0);  capacityTable_->setColumnCount(0);
    costTable_->clear();      costTable_->setRowCount(0);      costTable_->setColumnCount(0);
    maxFlowText_->clear();
    maxFlowTable_->clear();   maxFlowTable_->setRowCount(0);   maxFlowTable_->setColumnCount(0);
    minCostFlowText_->clear();
    minCostFlowTable_->clear(); minCostFlowTable_->setRowCount(0); minCostFlowTable_->setColumnCount(0);
    hasFlowNetwork_ = false;
    lastMaxFlow_ = 0;
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
    case 6:
    case 7:
        graphWidget_->clearHighlights();
        break;
    default:
        break;
    }
}

// -----------------------------------------------------------------------
//  Tab 7 — Поток (Network Flows)
// -----------------------------------------------------------------------
void MainWindow::setupTab7(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);

    // ===== ЗАДАНИЕ 1: Генерация сети =====
    auto* task1 = new QGroupBox("Задание 1: Генерация сети");
    auto* task1Layout = new QVBoxLayout(task1);

    auto* row1 = new QHBoxLayout;
    row1->addWidget(new QLabel("Исток:"));
    flowSrcVertex_ = new QSpinBox;
    flowSrcVertex_->setRange(0, 0);
    row1->addWidget(flowSrcVertex_);

    row1->addSpacing(12);
    row1->addWidget(new QLabel("Сток:"));
    flowSinkVertex_ = new QSpinBox;
    flowSinkVertex_->setRange(0, 0);
    row1->addWidget(flowSinkVertex_);
    row1->addStretch();
    task1Layout->addLayout(row1);

    auto* row2 = new QHBoxLayout;
    row2->addWidget(new QLabel("Макс. пропускная способность:"));
    maxCapacity_ = new QSpinBox;
    maxCapacity_->setRange(1, 100);
    maxCapacity_->setValue(20);
    row2->addWidget(maxCapacity_);

    row2->addSpacing(12);
    row2->addWidget(new QLabel("Макс. стоимость:"));
    maxCost_ = new QSpinBox;
    maxCost_->setRange(1, 100);
    maxCost_->setValue(10);
    row2->addWidget(maxCost_);

    row2->addSpacing(12);
    auto* genNetBtn = new QPushButton("Сгенерировать сеть");
    connect(genNetBtn, &QPushButton::clicked, this, &MainWindow::onGenerateFlowNetwork);
    row2->addWidget(genNetBtn);
    row2->addStretch();
    task1Layout->addLayout(row2);

    auto* matricesSplitter = new QSplitter(Qt::Horizontal);
    auto* capWidget = new QWidget;
    auto* capLayout = new QVBoxLayout(capWidget);
    capLayout->setContentsMargins(0, 0, 0, 0);
    capLayout->addWidget(new QLabel("Матрица пропускных способностей"));
    capacityTable_ = new QTableWidget;
    capLayout->addWidget(capacityTable_, 1);
    matricesSplitter->addWidget(capWidget);

    auto* costWidget = new QWidget;
    auto* costLayout = new QVBoxLayout(costWidget);
    costLayout->setContentsMargins(0, 0, 0, 0);
    costLayout->addWidget(new QLabel("Матрица стоимостей"));
    costTable_ = new QTableWidget;
    costLayout->addWidget(costTable_, 1);
    matricesSplitter->addWidget(costWidget);

    task1Layout->addWidget(matricesSplitter, 1);
    layout->addWidget(task1, 2);

    // ===== ЗАДАНИЕ 2: Максимальный поток =====
    auto* task2 = new QGroupBox("Задание 2: Максимальный поток (Форд-Фалкерсон)");
    auto* task2Layout = new QVBoxLayout(task2);

    auto* maxFlowBtn = new QPushButton("Найти макс. поток");
    connect(maxFlowBtn, &QPushButton::clicked, this, &MainWindow::onFindMaxFlow);
    task2Layout->addWidget(maxFlowBtn);

    maxFlowText_ = new QTextEdit;
    maxFlowText_->setReadOnly(true);
    maxFlowText_->setMaximumHeight(150);
    task2Layout->addWidget(maxFlowText_);

    task2Layout->addWidget(new QLabel("Матрица потока"));
    maxFlowTable_ = new QTableWidget;
    task2Layout->addWidget(maxFlowTable_, 1);

    layout->addWidget(task2, 2);

    // ===== ЗАДАНИЕ 3: Поток минимальной стоимости =====
    auto* task3 = new QGroupBox("Задание 3: Поток минимальной стоимости");
    auto* task3Layout = new QVBoxLayout(task3);

    auto* minCostBtn = new QPushButton("Найти мин. стоимость потока");
    connect(minCostBtn, &QPushButton::clicked, this, &MainWindow::onFindMinCostFlow);
    task3Layout->addWidget(minCostBtn);

    minCostFlowText_ = new QTextEdit;
    minCostFlowText_->setReadOnly(true);
    minCostFlowText_->setMaximumHeight(150);
    task3Layout->addWidget(minCostFlowText_);

    task3Layout->addWidget(new QLabel("Матрица потока (мин. стоимость)"));
    minCostFlowTable_ = new QTableWidget;
    task3Layout->addWidget(minCostFlowTable_, 1);

    layout->addWidget(task3, 2);
}

// -----------------------------------------------------------------------
//  Slot: Generate flow network (Task 1)
// -----------------------------------------------------------------------
void MainWindow::onGenerateFlowNetwork()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }
    if (!graph_.directed) {
        QMessageBox::warning(this, "Внимание",
            "Для задачи о потоках граф должен быть ориентированным.\n"
            "Пожалуйста, сгенерируйте ориентированный граф.");
        return;
    }

    const int src = flowSrcVertex_->value();
    const int snk = flowSinkVertex_->value();
    if (src == snk) {
        QMessageBox::warning(this, "Внимание", "Исток и сток не должны совпадать!");
        return;
    }

    const int n = graph_.n;
    const int maxCap = maxCapacity_->value();
    const int maxCst = maxCost_->value();

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> capDist(1, maxCap);
    std::uniform_int_distribution<int> costDist(1, maxCst);

    capacityMatrix_.assign(n, std::vector<int>(n, 0));
    costMatrix_.assign(n, std::vector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (graph_.adjMatrix[i][j] == 1) {
                capacityMatrix_[i][j] = capDist(rng);
                costMatrix_[i][j] = costDist(rng);
            }
        }
    }

    hasFlowNetwork_ = true;
    lastMaxFlow_ = 0;

    displayMatrix(capacityTable_, capacityMatrix_, 0, MatrixMode::Default, true);
    displayMatrix(costTable_, costMatrix_, 0, MatrixMode::Default, true);

    // Clear previous results
    maxFlowText_->clear();
    maxFlowTable_->clear();   maxFlowTable_->setRowCount(0);   maxFlowTable_->setColumnCount(0);
    minCostFlowText_->clear();
    minCostFlowTable_->clear(); minCostFlowTable_->setRowCount(0); minCostFlowTable_->setColumnCount(0);
}

// -----------------------------------------------------------------------
//  Slot: Find maximum flow (Task 2)
// -----------------------------------------------------------------------
void MainWindow::onFindMaxFlow()
{
    if (!hasFlowNetwork_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте сеть потоков!");
        return;
    }

    const int src = flowSrcVertex_->value();
    const int snk = flowSinkVertex_->value();

    // Read user-edited capacity values back from the table
    const int n = graph_.n;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (auto* item = capacityTable_->item(i, j)) {
                bool ok;
                int v = item->text().toInt(&ok);
                capacityMatrix_[i][j] = (ok && v >= 0) ? v : 0;
            }
        }
    }

    auto result = MaxFlow::solve(capacityMatrix_, src, snk);
    lastMaxFlow_ = result.maxFlow;

    QString text;
    text += QString("Максимальный поток: %1\n").arg(result.maxFlow);
    text += QString("Количество увеличивающих путей: %1\n\n").arg(result.iterations);

    for (int i = 0; i < result.iterations; ++i) {
        text += QString("Путь %1: ").arg(i + 1);
        const auto& path = result.augmentingPaths[i];
        const auto& edgeTypes = result.pathEdgeTypes[i];
        for (size_t j = 0; j < path.size(); ++j) {
            if (j > 0) {
                if (edgeTypes[j - 1])
                    text += QString::fromUtf8(" →(обр.) ");
                else
                    text += " → ";
            }
            text += QString::number(path[j]);
        }
        text += QString("  (поток: %1)\n").arg(result.pathFlows[i]);
    }

    maxFlowText_->setText(text);
    displayMatrix(maxFlowTable_, result.flowMatrix);

    // Clear min cost flow results since max flow may have changed
    minCostFlowText_->clear();
    minCostFlowTable_->clear(); minCostFlowTable_->setRowCount(0); minCostFlowTable_->setColumnCount(0);
}

// -----------------------------------------------------------------------
//  Slot: Find minimum cost flow (Task 3)
// -----------------------------------------------------------------------
void MainWindow::onFindMinCostFlow()
{
    if (!hasFlowNetwork_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте сеть потоков!");
        return;
    }

    const int src = flowSrcVertex_->value();
    const int snk = flowSinkVertex_->value();

    // Read user-edited capacity and cost values back from the tables
    const int n = graph_.n;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (auto* item = capacityTable_->item(i, j)) {
                bool ok;
                int v = item->text().toInt(&ok);
                capacityMatrix_[i][j] = (ok && v >= 0) ? v : 0;
            }
            if (auto* item = costTable_->item(i, j)) {
                bool ok;
                int v = item->text().toInt(&ok);
                costMatrix_[i][j] = (ok && v >= 0) ? v : 0;
            }
        }
    }

    // If max flow hasn't been computed yet, compute it now
    if (lastMaxFlow_ == 0) {
        auto mfResult = MaxFlow::solve(capacityMatrix_, src, snk);
        lastMaxFlow_ = mfResult.maxFlow;

        if (lastMaxFlow_ == 0) {
            minCostFlowText_->setText(
                "Максимальный поток = 0 (нет пути из истока в сток).\n"
                "Поток минимальной стоимости не определён.");
            return;
        }
    }

    const int desiredFlow = (2 * lastMaxFlow_) / 3;

    if (desiredFlow == 0) {
        minCostFlowText_->setText(
            QString("Максимальный поток: %1\n"
                    "F = ⌊2/3 × %1⌋ = 0\n"
                    "Требуемый поток = 0, стоимость = 0.")
                .arg(lastMaxFlow_));
        return;
    }

    auto result = MinCostFlow::solve(capacityMatrix_, costMatrix_, src, snk, desiredFlow);

    QString text;
    text += QString("Максимальный поток: %1\n").arg(lastMaxFlow_);
    text += QString("Требуемый поток F = ⌊2/3 × %1⌋ = %2\n").arg(lastMaxFlow_).arg(desiredFlow);
    text += QString("Достигнутый поток: %1\n").arg(result.flowValue);
    text += QString("Минимальная стоимость: %1\n").arg(result.totalCost);

    if (!result.feasible) {
        text += "\n⚠ Невозможно отправить поток требуемой величины!\n";
    }

    text += QString("\nКоличество итераций: %1\n\n").arg(result.iterations);

    for (int i = 0; i < result.iterations; ++i) {
        text += QString("Итерация %1: ").arg(i + 1);
        const auto& path = result.augmentingPaths[i];
        for (size_t j = 0; j < path.size(); ++j) {
            if (j) text += " → ";
            text += QString::number(path[j]);
        }
        text += QString("  (поток: %1, стоимость пути: %2)\n")
                    .arg(result.pathFlows[i])
                    .arg(result.pathCosts[i]);
    }

    minCostFlowText_->setText(text);
    displayMatrix(minCostFlowTable_, result.flowMatrix);
}

// -----------------------------------------------------------------------
//  Tab 8 — Лаб 4 (Spanning tree count, MST+Prüfer, Min Vertex Cover)
// -----------------------------------------------------------------------
void MainWindow::setupTab8(QWidget* tab)
{
    auto* layout = new QVBoxLayout(tab);

    // ===== ЗАДАНИЕ 1: Число остовных деревьев =====
    auto* task1 = new QGroupBox("Задание 1: Число остовных деревьев (теорема Кирхгофа)");
    auto* task1Layout = new QVBoxLayout(task1);

    auto* countBtn = new QPushButton("Посчитать число остовных деревьев");
    connect(countBtn, &QPushButton::clicked, this, &MainWindow::onLab4CountTrees);
    task1Layout->addWidget(countBtn);

    lab4Task1Text_ = new QTextEdit;
    lab4Task1Text_->setReadOnly(true);
    lab4Task1Text_->setMaximumHeight(80);
    task1Layout->addWidget(lab4Task1Text_);

    task1Layout->addWidget(new QLabel("Матрица Кирхгофа"));
    lab4Task1Table_ = new QTableWidget;
    task1Layout->addWidget(lab4Task1Table_, 1);

    layout->addWidget(task1, 2);

    // ===== ЗАДАНИЕ 2-c: МОД через Прима + код Прюфера =====
    auto* task2 = new QGroupBox("Задание 2-c: МОД (алгоритм Прима) + код Прюфера");
    auto* task2Layout = new QVBoxLayout(task2);

    auto* mstBtn = new QPushButton("Построить МОД и закодировать");
    connect(mstBtn, &QPushButton::clicked, this, &MainWindow::onLab4BuildMST);
    task2Layout->addWidget(mstBtn);

    lab4Task2Text_ = new QTextEdit;
    lab4Task2Text_->setReadOnly(true);
    lab4Task2Text_->setMaximumHeight(120);
    task2Layout->addWidget(lab4Task2Text_);

    task2Layout->addWidget(new QLabel("Матрица весов (рёбра МОД выделены зелёным)"));
    lab4Task2Table_ = new QTableWidget;
    task2Layout->addWidget(lab4Task2Table_, 1);

    layout->addWidget(task2, 2);

    // ===== ЗАДАНИЕ 3-f: Минимальное вершинное покрытие =====
    auto* task3 = new QGroupBox("Задание 3-f: Минимальное вершинное покрытие (2-приближение)");
    auto* task3Layout = new QVBoxLayout(task3);

    auto* sourceRow = new QHBoxLayout;
    sourceRow->addWidget(new QLabel("Применить к:"));
    vcFullGraphRadio_ = new QRadioButton("Исходный граф");
    vcFullGraphRadio_->setChecked(true);
    vcMSTRadio_ = new QRadioButton("МОД");
    vcMSTRadio_->setEnabled(false);
    sourceRow->addWidget(vcFullGraphRadio_);
    sourceRow->addWidget(vcMSTRadio_);
    sourceRow->addStretch();
    task3Layout->addLayout(sourceRow);

    auto* vcBtn = new QPushButton("Найти минимальное вершинное покрытие");
    connect(vcBtn, &QPushButton::clicked, this, &MainWindow::onLab4MinVertexCover);
    task3Layout->addWidget(vcBtn);

    lab4Task3Text_ = new QTextEdit;
    lab4Task3Text_->setReadOnly(true);
    lab4Task3Text_->setMaximumHeight(100);
    task3Layout->addWidget(lab4Task3Text_);

    task3Layout->addWidget(new QLabel("Матрица смежности (вершины покрытия выделены)"));
    lab4Task3Table_ = new QTableWidget;
    task3Layout->addWidget(lab4Task3Table_, 1);

    layout->addWidget(task3, 2);
}

// Helper: symmetrize adj + weight matrices in-place
static void symmetrize(std::vector<std::vector<int>>& adj,
                        std::vector<std::vector<int>>& w)
{
    const int n = static_cast<int>(adj.size());
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j) {
            if (adj[i][j] || adj[j][i]) {
                adj[i][j] = adj[j][i] = 1;
                int wv = (w[i][j] != 0) ? w[i][j] : w[j][i];
                w[i][j] = w[j][i] = wv;
            }
        }
}

// -----------------------------------------------------------------------
//  Slot: Count spanning trees (Task 1)
// -----------------------------------------------------------------------
void MainWindow::onLab4CountTrees()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    auto adj = graph_.adjMatrix;
    auto w   = graph_.weightMatrix;
    if (graph_.directed) symmetrize(adj, w);

    const int n = graph_.n;

    // Count undirected edges to detect tree
    int edgeCount = 0;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (adj[i][j]) ++edgeCount;

    auto res = SpanningTreeCount::solve(adj);

    QString text;
    if (graph_.directed)
        text += "Граф рассматривается как неориентированный.\n";
    text += QString("Число остовных деревьев: %1\n").arg(res.count);

    lab4Task1Text_->setText(text);
    displayMatrix(lab4Task1Table_, res.laplacian);
    graphWidget_->clearHighlights();
}

// -----------------------------------------------------------------------
//  Slot: Build MST + Prüfer code (Task 2c)
// -----------------------------------------------------------------------
void MainWindow::onLab4BuildMST()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    auto adj = graph_.adjMatrix;
    auto w   = graph_.weightMatrix;
    if (graph_.directed) symmetrize(adj, w);

    auto res = MSTPrim::solve(adj, w);

    QString text;
    if (graph_.directed)
        text += "Граф рассматривается как неориентированный.\n";

    if (!res.connected) {
        text += "Граф несвязный — МОД не существует.\n";
        lab4Task2Text_->setText(text);
        return;
    }

    text += QString("Суммарный вес МОД: %1\n").arg(res.totalWeight);

    // MST edges
    text += "Рёбра МОД: ";
    for (int k = 0; k < (int)res.edges.size(); ++k) {
        if (k) text += ", ";
        text += QString("(%1, %2, w=%3)")
                    .arg(res.edges[k].first)
                    .arg(res.edges[k].second)
                    .arg(res.weights[k]);
    }
    text += "\n";

    // Prüfer code
    if (graph_.n <= 2) {
        text += "Код Прюфера: [] (граф из 2 вершин)\n";
    } else {
        text += "Код Прюфера A = [";
        for (int k = 0; k < (int)res.pruferCode.size(); ++k) {
            if (k) text += ", ";
            text += QString::number(res.pruferCode[k]);
        }
        text += "]\n";
    }
    text += "Веса рёбер W = [";
    for (int k = 0; k < (int)res.pruferWeights.size(); ++k) {
        if (k) text += ", ";
        text += QString::number(res.pruferWeights[k]);
    }
    text += "]\n";
    text += QString("Обратное декодирование: %1").arg(res.roundTripOk ? "ОК ✓" : "ОШИБКА ✗");

    lab4Task2Text_->setText(text);

    // Display weight matrix; highlight MST edges
    displayMatrix(lab4Task2Table_, w, 0, MatrixMode::Weighted);
    std::vector<std::pair<int,int>> mstEdgePairs;
    for (auto& [u, v] : res.edges) {
        mstEdgePairs.push_back({u, v});
        mstEdgePairs.push_back({v, u});
    }
    highlightPath(lab4Task2Table_, mstEdgePairs);

    // Canvas highlight
    std::set<std::pair<int,int>> mstSet;
    for (auto& [u, v] : res.edges)
        mstSet.insert({std::min(u,v), std::max(u,v)});
    graphWidget_->highlightMSTEdges(mstSet);

    // Save MST adjacency matrix for task 3f
    const int n = graph_.n;
    mstAdjMatrix_.assign(n, std::vector<int>(n, 0));
    for (int k = 0; k < (int)res.edges.size(); ++k) {
        int u = res.edges[k].first, v = res.edges[k].second;
        mstAdjMatrix_[u][v] = mstAdjMatrix_[v][u] = res.weights[k];
    }
    hasMST_ = true;
    vcMSTRadio_->setEnabled(true);
}

// -----------------------------------------------------------------------
//  Slot: Min vertex cover (Task 3f)
// -----------------------------------------------------------------------
void MainWindow::onLab4MinVertexCover()
{
    if (!hasGraph_) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте граф!");
        return;
    }

    const bool usesMST = vcMSTRadio_->isChecked();
    if (usesMST && !hasMST_) {
        QMessageBox::warning(this, "Внимание", "Сначала постройте МОД (Задание 2)!");
        return;
    }

    std::vector<std::vector<int>> adj;
    if (usesMST) {
        adj = mstAdjMatrix_;
    } else {
        adj = graph_.adjMatrix;
        auto w = graph_.weightMatrix;
        if (graph_.directed) symmetrize(adj, w);
    }

    auto res = VertexCover::solve(adj);

    QString text;
    if (usesMST)
        text += "Применяется к: МОД\n";
    else if (graph_.directed)
        text += "Применяется к: исходный граф (рассматривается как неориентированный)\n";
    else
        text += "Применяется к: исходный граф\n";

    text += "S = { ";
    for (int k = 0; k < (int)res.cover.size(); ++k) {
        if (k) text += ", ";
        text += QString::number(res.cover[k]);
    }
    text += " }\n";
    text += QString("|S| = %1  (|S| ≤ 2|T|)\n").arg(res.cover.size());

    text += "Выбранные рёбра: ";
    for (int k = 0; k < (int)res.pickedEdges.size(); ++k) {
        if (k) text += ", ";
        text += QString("(%1, %2)").arg(res.pickedEdges[k].first).arg(res.pickedEdges[k].second);
    }

    lab4Task3Text_->setText(text);

    displayMatrix(lab4Task3Table_, adj, 0, MatrixMode::Adjacency);
    highlightCells(lab4Task3Table_, res.cover, QColor(200, 150, 240));

    graphWidget_->highlightVertexCover(res.cover);
}
