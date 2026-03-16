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
    void onShimbelCalculate();
    void onFindRoute();
    void onFindBiComp();
    void onDijkstraCalculate();
    void onRunComparison();
    void onTabChanged(int index);

private:
    void setupTab1(QWidget* tab);
    void setupTab2(QWidget* tab);
    void setupTab3(QWidget* tab);
    void setupTab4(QWidget* tab);
    void setupTab5(QWidget* tab);
    void setupTab6(QWidget* tab);

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
    QTableWidget*    dijkWeightTable_;

    // ---- Tab 6 widgets (Comparison) ----
    QSpinBox*        cmpMinN_;
    QSpinBox*        cmpMaxN_;
    QSpinBox*        cmpStep_;
    QTextEdit*       cmpText_;
    QTableWidget*    cmpTable_;

    // ---- Graph visualization ----
    QTabWidget*  tabs_;
    GraphWidget* graphWidget_;

    // ---- State ----
    GraphData graph_;
    bool      hasGraph_ = false;
};
