#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QGraphicsItem>
#include <QVector>

class GridScene;
class GridView;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onCellClicked(QPoint pos);
    QVector<QPoint> computeDDALine(QPoint p1, QPoint p2);
    QVector<QPoint> computeBresenhamLine(QPoint p1, QPoint p2);
    void drawLineDDA();
    void drawLineBresenham();
    qreal computeAvgDDAtime(int num_iters);
    qreal computeAvgBresenhamtime(int num_iters);
    void compareAlgorithms();
    void clearGrid();

private:
    GridScene* scene;
    GridView* view;
    QLabel* labelP1;
    QLabel* labelP2;
    QPushButton* btnDrawDDA;
    QPushButton* btnDrawBres;
    QPushButton* btnClear;
    QPushButton* btnCompare;

    QPoint point1, point2;
    bool hasFirstPoint = false;
    QGraphicsItem* line = nullptr;

    double avgTimeDDA = 0;
    double avgTimeBres = 0;
};

#endif // MAINWINDOW_H
