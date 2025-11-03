#include "gridscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

GridScene::GridScene(QObject* parent) : QGraphicsScene(parent) {
    setSceneRect(-5000, -5000, 10000, 10000);
}

void GridScene::toggleCell(const QPoint& cell) {
    if (coloredCells.size() >= 2)
        coloredCells.clear();

    coloredCells.append(qMakePair(cell, QBrush(Qt::black)));
    update(QRectF(cell.x() * cellSize, cell.y() * cellSize, cellSize, cellSize));
}


void GridScene::paintCell(const QPoint& cell, const QBrush& brush) {
    for (int i = 0; i < coloredCells.size(); ++i) {
        if (coloredCells[i].first == cell) {
            coloredCells[i].second = brush;
            update(QRectF(cell.x() * cellSize, cell.y() * cellSize, cellSize, cellSize));
            return;
        }
    }

    coloredCells.append(qMakePair(cell, brush));
    update(QRectF(cell.x() * cellSize, cell.y() * cellSize, cellSize, cellSize));
}


void GridScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint cell(qFloor(event->scenePos().x() / cellSize), qFloor(event->scenePos().y() / cellSize));
        emit cellClicked(cell);
    }

    QGraphicsScene::mousePressEvent(event);
}


void GridScene::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    int left = std::floor(rect.left() / cellSize);
    int right = std::ceil(rect.right() / cellSize);
    int top = std::floor(rect.top() / cellSize);
    int bottom = std::ceil(rect.bottom() / cellSize);

    // grid lines
    QPen gridPen(Qt::lightGray);
    gridPen.setWidth(0);
    gridPen.setCosmetic(true);
    painter->setPen(gridPen);
    for (int x = left; x <= right; ++x)
        painter->drawLine(x * cellSize, rect.top(), x * cellSize, rect.bottom());
    for (int y = top; y <= bottom; ++y)
        painter->drawLine(rect.left(), y * cellSize, rect.right(), y * cellSize);

    // axes
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::black);

    // X-axis row (y = 0)
    for (int x = left; x <= right; ++x) {
        QRectF r(x * cellSize, 0, cellSize, cellSize);
        if (rect.intersects(r))
            painter->drawRect(r);
    }

    // Y-axis column (x = 0)
    for (int y = top; y <= bottom; ++y) {
        QRectF r(0, y * cellSize, cellSize, cellSize);
        if (rect.intersects(r))
            painter->drawRect(r);
    }


    // colored cells
    painter->setPen(Qt::NoPen);
    for (const QPair<QPoint, QBrush>& p : coloredCells) {
        QRectF r(p.first.x() * cellSize, p.first.y() * cellSize, cellSize, cellSize);
        painter->setBrush(p.second);
        if (rect.intersects(r))
            painter->drawRect(r);
    }
}


void GridScene::clearCells() {
    coloredCells.clear();
    update();
}
