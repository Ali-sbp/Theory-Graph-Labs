#pragma once

#include <QMainWindow>
#include "graphgenerator.h"

class GraphWidget;

QT_BEGIN_NAMESPACE
class QTabWidget;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QRadioButton;
class QPushButton;
class QTableWidget;
class QTextEdit;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onGenerate();
    void onGenerateDAG();
    void onShimbelCalculate();
    void onFindRoute();
    void onFindBiComp();
    void onDijkstraCalculate();
    void onRunComparison();
    void onTabChanged(int index);
    void onRegenWeightsPositive();
    void onRegenWeightsNegative();
    void onRegenWeightsMixed();
    void onRearrangeDirected();
    void onRearrangeUndirected();
    void onGenerateFlowNetwork();
    void onFindMaxFlow();
    void onFindMinCostFlow();

private:
    void setupTab1(QWidget* tab);
    void setupTab2(QWidget* tab);
    void setupTab3(QWidget* tab);
    void setupTab4(QWidget* tab);
    void setupTab5(QWidget* tab);
    void setupTab6(QWidget* tab);
    void setupTab7(QWidget* tab);

    enum class MatrixMode { Default, Adjacency, Weighted };

    // sentinel: values >= sentinel (if sentinel>0) shown as "∞",
    //           values <= sentinel (if sentinel<0) shown as "-∞"
    // mode: Adjacency → off-diagonal zeros shown as "-"
    //        Weighted  → off-diagonal zeros shown as "∞"
    void displayMatrix(QTableWidget* table,
                       const std::vector<std::vector<int>>& matrix,
                       int sentinel = 0,
                       MatrixMode mode = MatrixMode::Default);
    void highlightPath(QTableWidget* table,
                       const std::vector<std::pair<int,int>>& edges);
    void highlightCells(QTableWidget* table,
                        const std::vector<int>& vertices, const QColor& color);
    void doRegenerateWeights(WeightType wType);

    // ---- Tab 1 widgets ----
    QSpinBox*        vertexCount_;
    QDoubleSpinBox*  paramP_;
    QDoubleSpinBox*  weightParamP_;
    QCheckBox*       directedCheck_;
    QRadioButton*    positiveRadio_;
    QRadioButton*    negativeRadio_;
    QRadioButton*    mixedRadio_;
    QTableWidget*    adjTable_;
    QTableWidget*    weightTable_;
    QTextEdit*       analysisText_;

    // ---- Tab 2 widgets ----
    QSpinBox*        pathLength_;
    QTableWidget*    minTable_;
    QTableWidget*    maxTable_;

    // ---- Tab 3 widgets ----
    QSpinBox*        fromVertex_;
    QSpinBox*        toVertex_;
    QTextEdit*       routeText_;
    QTableWidget*    routeTable_;

    // ---- Tab 4 widgets (BiComp) ----
    QTextEdit*       bicompText_;
    QTableWidget*    bicompTable_;

    // ---- Tab 5 widgets (Dijkstra neg) ----
    QSpinBox*        dijkSrcVertex_;
    QSpinBox*        dijkDstVertex_;
    QLabel*          dijkWeightTypeLabel_;
    QTextEdit*       dijkText_;
    QTableWidget*    dijkStagesTable_;
    QTableWidget*    dijkWeightTable_;

    // ---- Tab 6 widgets (Comparison) ----
    QSpinBox*        cmpMinN_;
    QSpinBox*        cmpMaxN_;
    QSpinBox*        cmpStep_;
    QTextEdit*       cmpText_;
    QTableWidget*    cmpTable_;

    // ---- Tab 7 widgets (Поток) ----
    // Task 1
    QSpinBox*     flowSrcVertex_;
    QSpinBox*     flowSinkVertex_;
    QSpinBox*     maxCapacity_;
    QSpinBox*     maxCost_;
    QTableWidget* capacityTable_;
    QTableWidget* costTable_;
    // Task 2
    QTextEdit*    maxFlowText_;
    QTableWidget* maxFlowTable_;
    // Task 3
    QTextEdit*    minCostFlowText_;
    QTableWidget* minCostFlowTable_;

    // ---- Graph visualization ----
    QTabWidget*  tabs_;
    GraphWidget* graphWidget_;

    // ---- State ----
    GraphData graph_;
    bool      hasGraph_ = false;

    // ---- Original directed matrices (for directed/undirected rearrangement) ----
    std::vector<std::vector<int>> originalAdjMatrix_;
    std::vector<std::vector<int>> originalWeightMatrix_;

    // ---- Flow network state ----
    std::vector<std::vector<int>> capacityMatrix_;
    std::vector<std::vector<int>> costMatrix_;
    bool hasFlowNetwork_ = false;
    int  lastMaxFlow_ = 0;
};
