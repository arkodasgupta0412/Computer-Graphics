#ifndef GRIDVIEW_H
#define GRIDVIEW_H

#include <QGraphicsView>
#include <QPoint>

class GridView : public QGraphicsView {
    Q_OBJECT
public:
    explicit GridView(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QPoint lastPanPoint;
};

#endif // GRIDVIEW_H
