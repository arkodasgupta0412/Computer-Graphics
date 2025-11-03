#ifndef GRIDWIDGET_H
#define GRIDWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QVector>

class GridWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GridWidget(QWidget *parent = nullptr);

    void setPoints(QPoint p1, QPoint p2); // for drawing line

signals:
    void sendMousePosition(QPoint pos);
    void mousePressed(QPoint pos);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QPoint point1, point2;
    bool hasTwoPoints = false;
};

#endif // GRIDWIDGET_H
