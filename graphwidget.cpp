#include "graphwidget.h"
#include "graphgenerator.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------------------
//  Static color accessors
// ---------------------------------------------------------------------------

const QColor& GraphWidget::defaultVertexColor()
{
    static QColor c(173, 216, 230);  // light blue
    return c;
}

const QColor& GraphWidget::routeVertexColor()
{
    static QColor c(100, 220, 100);  // green
    return c;
}

const QColor& GraphWidget::articulationColor()
{
    static QColor c(255, 100, 100);  // red
    return c;
}

const QColor& GraphWidget::dijkstraVertexColor()
{
    static QColor c(255, 180, 60);   // orange
    return c;
}

const std::vector<QColor>& GraphWidget::blockColors()
{
    static std::vector<QColor> colors = {
        QColor(76, 175, 80),    // green
        QColor(33, 150, 243),   // blue
        QColor(255, 152, 0),    // orange
        QColor(156, 39, 176),   // purple
        QColor(0, 188, 212),    // cyan
        QColor(244, 67, 54),    // red
        QColor(255, 235, 59),   // yellow
        QColor(121, 85, 72),    // brown
        QColor(233, 30, 99),    // pink
        QColor(63, 81, 181),    // indigo
    };
    return colors;
}

// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

GraphWidget::GraphWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(250, 250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background-color: white;");
}

// ---------------------------------------------------------------------------
//  Public interface
// ---------------------------------------------------------------------------

void GraphWidget::setGraph(const GraphData& graph)
{
    n_ = graph.n;
    directed_ = graph.directed;
    adj_ = graph.adjMatrix;
    weights_ = graph.weightMatrix;
    computeLayout();
    clearHighlights();
}

void GraphWidget::clearHighlights()
{
    mode_ = HighlightMode::None;
    highlightedPath_.clear();
    highlightedVertices_.clear();
    articulationPoints_.clear();
    blocks_.clear();
    highlightedEdges_.clear();
    update();
}

void GraphWidget::highlightRoute(const std::vector<int>& path)
{
    mode_ = HighlightMode::Route;
    highlightedPath_ = path;
    highlightedVertices_.clear();
    highlightedEdges_.clear();
    for (int v : path)
        highlightedVertices_.insert(v);
    for (size_t i = 0; i + 1 < path.size(); ++i)
        highlightedEdges_.insert({path[i], path[i + 1]});
    update();
}

void GraphWidget::highlightBiComp(const std::set<int>& aps,
                                   const std::vector<std::vector<std::pair<int,int>>>& blks)
{
    mode_ = HighlightMode::BiComp;
    articulationPoints_ = aps;
    blocks_ = blks;
    highlightedPath_.clear();
    highlightedVertices_.clear();
    highlightedEdges_.clear();
    update();
}

void GraphWidget::highlightDijkstraPath(const std::vector<int>& path)
{
    mode_ = HighlightMode::DijkstraPath;
    highlightedPath_ = path;
    highlightedVertices_.clear();
    highlightedEdges_.clear();
    for (int v : path)
        highlightedVertices_.insert(v);
    for (size_t i = 0; i + 1 < path.size(); ++i)
        highlightedEdges_.insert({path[i], path[i + 1]});
    update();
}

QSize GraphWidget::minimumSizeHint() const { return {250, 250}; }
QSize GraphWidget::sizeHint() const { return {400, 400}; }

// ---------------------------------------------------------------------------
//  Layout computation
// ---------------------------------------------------------------------------

void GraphWidget::computeLayout()
{
    positions_.resize(n_);
    if (n_ <= 0) return;
    if (n_ == 1) {
        positions_[0] = QPointF(0.5, 0.5);
        return;
    }
    for (int i = 0; i < n_; ++i) {
        double angle = 2.0 * M_PI * i / n_ - M_PI / 2.0;
        positions_[i] = QPointF(0.5 + 0.4 * std::cos(angle),
                                0.5 + 0.4 * std::sin(angle));
    }
}

QPointF GraphWidget::mapToPixel(const QPointF& normalized) const
{
    double w = width() - 2 * kPadding;
    double h = height() - 2 * kPadding;
    return QPointF(kPadding + normalized.x() * w,
                   kPadding + normalized.y() * h);
}

double GraphWidget::vertexRadius() const
{
    if (n_ <= 1) return kBaseRadius;
    // Shrink radius for large graphs so vertices don't overlap
    double circlePixelRadius = std::min(width(), height()) * 0.4 / 2.0;
    double maxFit = circlePixelRadius * std::sin(M_PI / n_) * 0.8;
    return std::min(kBaseRadius, std::max(8.0, maxFit));
}

// ---------------------------------------------------------------------------
//  Drawing helpers
// ---------------------------------------------------------------------------

void GraphWidget::drawArrowhead(QPainter& p, QPointF from, QPointF to, double radius)
{
    double dx = to.x() - from.x();
    double dy = to.y() - from.y();
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-6) return;

    // Normalize
    dx /= len;
    dy /= len;

    // Tip is at the edge of the target vertex circle
    QPointF tip(to.x() - dx * radius, to.y() - dy * radius);

    double arrowLen = std::min(12.0, radius * 0.8);
    double arrowAngle = 0.45;  // ~25 degrees

    QPointF left(tip.x() - arrowLen * (dx * std::cos(arrowAngle) - dy * std::sin(arrowAngle)),
                 tip.y() - arrowLen * (dy * std::cos(arrowAngle) + dx * std::sin(arrowAngle)));
    QPointF right(tip.x() - arrowLen * (dx * std::cos(arrowAngle) + dy * std::sin(arrowAngle)),
                  tip.y() - arrowLen * (dy * std::cos(arrowAngle) - dx * std::sin(arrowAngle)));

    QPainterPath arrow;
    arrow.moveTo(tip);
    arrow.lineTo(left);
    arrow.lineTo(right);
    arrow.closeSubpath();

    p.save();
    p.setBrush(p.pen().color());
    p.drawPath(arrow);
    p.restore();
}

void GraphWidget::drawEdge(QPainter& p, int from, int to, const QPen& pen)
{
    if (from < 0 || from >= n_ || to < 0 || to >= n_) return;

    QPointF pFrom = mapToPixel(positions_[from]);
    QPointF pTo   = mapToPixel(positions_[to]);
    double r = vertexRadius();

    bool bidirectional = directed_ && adj_[from][to] && adj_[to][from];

    p.setPen(pen);

    if (bidirectional) {
        // Curve the edge slightly so both directions are visible
        double dx = pTo.x() - pFrom.x();
        double dy = pTo.y() - pFrom.y();
        double len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-6) return;

        // Perpendicular offset
        double offX = -dy / len * 8.0;
        double offY =  dx / len * 8.0;

        QPointF mid((pFrom.x() + pTo.x()) / 2.0 + offX,
                    (pFrom.y() + pTo.y()) / 2.0 + offY);

        QPainterPath path;
        path.moveTo(pFrom);
        path.quadTo(mid, pTo);
        p.drawPath(path);

        if (directed_)
            drawArrowhead(p, mid, pTo, r);
    } else {
        p.drawLine(pFrom, pTo);
        if (directed_)
            drawArrowhead(p, pFrom, pTo, r);
    }

    // Weight label
    int w = weights_[from][to];
    if (w != 0 && n_ <= 20) {
        double dx = pTo.x() - pFrom.x();
        double dy = pTo.y() - pFrom.y();
        double len = std::sqrt(dx * dx + dy * dy);

        // Offset perpendicular to edge for readability
        double perpX = 0, perpY = 0;
        if (len > 1e-6) {
            perpX = -dy / len * 12.0;
            perpY =  dx / len * 12.0;
        }
        if (bidirectional) {
            perpX += -dy / len * 4.0;
            perpY +=  dx / len * 4.0;
        }

        QPointF mid((pFrom.x() + pTo.x()) / 2.0 + perpX,
                    (pFrom.y() + pTo.y()) / 2.0 + perpY);

        p.save();
        QFont f = p.font();
        f.setPointSize(8);
        p.setFont(f);

        QString label = QString::number(w);
        QRectF textRect = p.fontMetrics().boundingRect(label);
        textRect.moveCenter(mid);
        textRect.adjust(-2, -1, 2, 1);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 200));
        p.drawRoundedRect(textRect, 2, 2);

        p.setPen(Qt::darkGray);
        p.drawText(textRect, Qt::AlignCenter, label);
        p.restore();
    }
}

void GraphWidget::drawVertex(QPainter& p, int v, const QBrush& fill)
{
    QPointF center = mapToPixel(positions_[v]);
    double r = vertexRadius();

    p.setBrush(fill);
    p.setPen(QPen(Qt::black, 1.5));
    p.drawEllipse(center, r, r);

    // Label
    p.setPen(Qt::black);
    QFont f = p.font();
    f.setPointSize(r > 12 ? 10 : 8);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRectF(center.x() - r, center.y() - r, 2 * r, 2 * r),
               Qt::AlignCenter, QString::number(v));
}

// ---------------------------------------------------------------------------
//  Paint event
// ---------------------------------------------------------------------------

void GraphWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    p.fillRect(rect(), Qt::white);

    if (n_ <= 0) {
        p.setPen(Qt::gray);
        p.drawText(rect(), Qt::AlignCenter, "No graph generated");
        return;
    }

    // --- 1. Build edge color map for BiComp mode ---
    // Maps edge (min,max) → block index for coloring
    std::map<std::pair<int,int>, int> edgeBlockMap;
    if (mode_ == HighlightMode::BiComp) {
        for (size_t b = 0; b < blocks_.size(); ++b) {
            for (auto& [u, v] : blocks_[b]) {
                auto key = std::make_pair(std::min(u, v), std::max(u, v));
                edgeBlockMap[key] = static_cast<int>(b);
            }
        }
    }

    // --- 2. Draw edges ---
    QPen defaultEdgePen(Qt::darkGray, 1.5);
    QPen routeEdgePen(routeVertexColor(), 3.0);
    QPen dijkstraEdgePen(dijkstraVertexColor(), 3.0);

    auto drawEdgesForPair = [&](int i, int j) {
        if (!adj_[i][j]) return;

        if (mode_ == HighlightMode::Route || mode_ == HighlightMode::DijkstraPath) {
            bool isHighlighted = highlightedEdges_.count({i, j}) > 0;
            if (!directed_)
                isHighlighted = isHighlighted || highlightedEdges_.count({j, i}) > 0;

            QPen pen = isHighlighted
                ? (mode_ == HighlightMode::Route ? routeEdgePen : dijkstraEdgePen)
                : defaultEdgePen;
            drawEdge(p, i, j, pen);
        } else if (mode_ == HighlightMode::BiComp) {
            auto key = std::make_pair(std::min(i, j), std::max(i, j));
            auto it = edgeBlockMap.find(key);
            if (it != edgeBlockMap.end()) {
                const auto& colors = blockColors();
                QColor c = colors[it->second % colors.size()];
                drawEdge(p, i, j, QPen(c, 2.5));
            } else {
                drawEdge(p, i, j, defaultEdgePen);
            }
        } else {
            drawEdge(p, i, j, defaultEdgePen);
        }
    };

    if (directed_) {
        for (int i = 0; i < n_; ++i)
            for (int j = 0; j < n_; ++j)
                drawEdgesForPair(i, j);
    } else {
        for (int i = 0; i < n_; ++i)
            for (int j = i + 1; j < n_; ++j)
                drawEdgesForPair(i, j);
    }

    // --- 3. Draw vertices (on top of edges) ---
    for (int v = 0; v < n_; ++v) {
        QBrush fill;
        switch (mode_) {
        case HighlightMode::Route:
            fill = highlightedVertices_.count(v) ? QBrush(routeVertexColor())
                                                  : QBrush(defaultVertexColor());
            break;
        case HighlightMode::BiComp:
            fill = articulationPoints_.count(v) ? QBrush(articulationColor())
                                                 : QBrush(defaultVertexColor());
            break;
        case HighlightMode::DijkstraPath:
            fill = highlightedVertices_.count(v) ? QBrush(dijkstraVertexColor())
                                                  : QBrush(defaultVertexColor());
            break;
        default:
            fill = QBrush(defaultVertexColor());
            break;
        }
        drawVertex(p, v, fill);
    }
}
