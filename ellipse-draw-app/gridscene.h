#ifndef GRIDSCENE_H
#define GRIDSCENE_H

#include <QGraphicsScene>
#include <QPoint>
#include <QVector>
#include <QPair>
#include <QPainter>
#include <QBrush>

class GridScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit GridScene(QObject* parent = nullptr);

    void toggleCell(const QPoint& cell);
    void paintCell(const QPoint& cell, const QBrush& brush);
    void clearCells();

signals:
    void cellClicked(const QPoint& cell);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    QVector<QPair<QPoint, QBrush>> coloredCells;
    const int cellSize = 5;
};

#endif // GRIDSCENE_H
