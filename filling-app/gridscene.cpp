#include "gridscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <cmath>
#include <QDebug>

GridScene::GridScene(QObject *parent)
    : QGraphicsScene(parent) {
    setSceneRect(-5000, -5000, 10000, 10000);
}

QBrush GridScene::getCellBrush(const QPoint& cell) const {
    if (cell.x() == 0 || cell.y() == 0) {
        return QBrush(Qt::black);
    }

    if (cellItems.contains(cell)) {
        return cellItems[cell]->brush();
    }

    return QBrush();
}

void GridScene::paintCell(const QPoint& cell, const QBrush& brush) {
    if (!cellItems.contains(cell)) {
        auto *rect = addRect(cell.x() * cellSize, cell.y() * cellSize, cellSize, cellSize, Qt::NoPen, brush);
        cellItems[cell] = rect;
    } else {
        cellItems[cell]->setBrush(brush);
    }
}

void GridScene::clearCells() {
    for (auto *item : cellItems) removeItem(item);
    qDeleteAll(cellItems);
    cellItems.clear();
}

void GridScene::clearCellsWithBrushes(const QList<QBrush> &brushes) {
    QList<QPoint> toRemove;
    for (auto it = cellItems.begin(); it != cellItems.end(); ++it) {
        for (const auto &br : brushes) {
            if (it.value()->brush().color() == br.color()) {
                removeItem(it.value());
                delete it.value();
                toRemove.append(it.key());
                break;
            }
        }
    }
    for (auto &cell : toRemove) {
        cellItems.remove(cell);
    }
}

bool GridScene::isCellPaintedWith(const QPoint& cell, const QBrush& brush) const {
    if (cellItems.contains(cell)) {
        return cellItems[cell]->brush().color() == brush.color();
    }
    return false;
}

void GridScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QPoint cell(qFloor(event->scenePos().x() / cellSize),
                qFloor(event->scenePos().y() / cellSize));
    if (event->button() == Qt::LeftButton) {
        emit cellClicked(cell);
        emit leftClick(cell);
    } else if (event->button() == Qt::RightButton) {
        emit seedSelected(cell);
        emit rightClick(cell);
    }
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

    // axes (filled cell-wise, like drawline app)
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

    // painted cells
    painter->setPen(Qt::NoPen);
    for (auto it = cellItems.constBegin(); it != cellItems.constEnd(); ++it) {
        QRectF r(it.key().x() * cellSize, it.key().y() * cellSize, cellSize, cellSize);
        painter->setBrush(it.value()->brush());
        if (rect.intersects(r))
            painter->drawRect(r);
    }
}
