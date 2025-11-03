#include "gridview.h"
#include "gridscene.h"
#include <QWheelEvent>
#include <QMouseEvent>

GridView::GridView(QWidget* parent) : QGraphicsView(parent) {
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(NoDrag);
}

void GridView::wheelEvent(QWheelEvent* event) {
    double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0)
        scale(scaleFactor, scaleFactor);
    else
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
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

