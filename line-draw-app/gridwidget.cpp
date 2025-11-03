#include "gridwidget.h"
#include <QPainter>
#include <QMouseEvent>

GridWidget::GridWidget(QWidget *parent)
    : QWidget(parent) {
    setMouseTracking(true);
}

void GridWidget::setPoints(QPoint p1, QPoint p2) {
    point1 = p1;
    point2 = p2;
    hasTwoPoints = true;
    update();
}

void GridWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);

    // Background
    painter.fillRect(rect(), Qt::white);

    int gridSize = 20;
    painter.setPen(QPen(Qt::lightGray, 1));

    // Grid lines
    for (int x = 0; x < width(); x += gridSize)
        painter.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += gridSize)
        painter.drawLine(0, y, width(), y);

    // Axes
    QPen axisPen(Qt::black, 2);
    painter.setPen(axisPen);
    painter.drawLine(0, height()/2, width(), height()/2);
    painter.drawLine(width()/2, 0, width()/2, height());

    // Draw selected points
    painter.setPen(QPen(Qt::red, 4));
    if (point1 != QPoint())
        painter.drawPoint(point1);
    if (point2 != QPoint())
        painter.drawPoint(point2);

    // Draw line if two points are selected
    if (hasTwoPoints) {
        painter.setPen(QPen(Qt::blue, 2));
        painter.drawLine(point1, point2);
    }
}

void GridWidget::mouseMoveEvent(QMouseEvent *event) {
    emit sendMousePosition(event->pos());
}

void GridWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        emit mousePressed(event->pos());
}
