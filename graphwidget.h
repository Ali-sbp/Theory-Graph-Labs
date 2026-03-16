#pragma once

#include <QWidget>
#include <QPointF>
#include <QColor>
#include <vector>
#include <set>

struct GraphData;

class GraphWidget : public QWidget {
    Q_OBJECT
public:
    enum class HighlightMode {
        None,           // Tabs 1, 2, 6 — just draw the graph
        Route,          // Tab 3 — highlight path vertices + edges
        BiComp,         // Tab 4 — articulation points + colored blocks
        DijkstraPath    // Tab 5 — highlight shortest path
    };

    explicit GraphWidget(QWidget* parent = nullptr);

    void setGraph(const GraphData& graph);
    void clearHighlights();

    void highlightRoute(const std::vector<int>& path);
    void highlightBiComp(const std::set<int>& articulationPoints,
                         const std::vector<std::vector<std::pair<int,int>>>& blocks);
    void highlightDijkstraPath(const std::vector<int>& path);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void computeLayout();
    QPointF mapToPixel(const QPointF& normalized) const;
    double vertexRadius() const;
    void drawEdge(QPainter& p, int from, int to, const QPen& pen);
    void drawArrowhead(QPainter& p, QPointF from, QPointF to, double radius);
    void drawVertex(QPainter& p, int v, const QBrush& fill);

    // Graph data
    int n_ = 0;
    bool directed_ = false;
    std::vector<std::vector<int>> adj_;
    std::vector<std::vector<int>> weights_;

    // Layout — normalized [0,1] positions
    std::vector<QPointF> positions_;
    static constexpr double kPadding = 40.0;
    static constexpr double kBaseRadius = 18.0;

    // Highlight state
    HighlightMode mode_ = HighlightMode::None;
    std::vector<int> highlightedPath_;
    std::set<int> highlightedVertices_;
    std::set<int> articulationPoints_;
    std::vector<std::vector<std::pair<int,int>>> blocks_;
    std::set<std::pair<int,int>> highlightedEdges_;

    static const QColor& defaultVertexColor();
    static const QColor& routeVertexColor();
    static const QColor& articulationColor();
    static const QColor& dijkstraVertexColor();
    static const std::vector<QColor>& blockColors();
};
