#include "gridview.h"
#include "gridscene.h"
#include <QWheelEvent>
#include <QMouseEvent>

GridView::GridView(QWidget* parent) : QGraphicsView(parent), scaleFactorTotal(1.0) {
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(NoDrag);
}

// constants for limits
static constexpr double MIN_SCALE = 0.2;
static constexpr double MAX_SCALE = 5.0;

void GridView::wheelEvent(QWheelEvent* event) {
    double factor = (event->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    double newScale = scaleFactorTotal * factor;

    // clamp
    if (newScale < MIN_SCALE || newScale > MAX_SCALE)
        return;

    scale(factor, factor);
    scaleFactorTotal = newScale;
}

void GridView::mouseMoveEvent(QMouseEvent* event) {
    if (!lastPanPoint.isNull()) {
        QPointF delta = mapToScene(lastPanPoint) - mapToScene(event->pos());
        translate(delta.x(), delta.y());
        lastPanPoint = event->pos();
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GridView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
        lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}
