#ifndef GRIDSCENE_H
#define GRIDSCENE_H

#include <QGraphicsScene>
#include <QPoint>

class GridScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit GridScene(QObject* parent = nullptr);

    void paintCell(const QPoint& cell, const QBrush& brush);
    void toggleCell(const QPoint& cell);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    int cellSize = 10;
    QList<QPair<QPoint, QBrush>> coloredCells;

signals:
    void cellClicked(QPoint cell);

public slots:
    void clearCells();

};

#endif // GRIDSCENE_H
