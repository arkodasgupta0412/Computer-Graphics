#include "mainwindow.h"
#include "gridscene.h"
#include "gridview.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QElapsedTimer>
#include <QDebug>
#include <QLabel>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget *central = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;

    scene = new GridScene(this);
    view = new GridView;
    view->setScene(scene);
    scene->setSceneRect(-500, -500, 1000, 1000);

    labelP1 = new QLabel("P1: ( , )");
    labelP2 = new QLabel("P2: ( , )");

    btnDrawDDA = new QPushButton("DDA Line");
    btnDrawBres = new QPushButton("Bresenham Line");
    btnClear = new QPushButton("Clear");
    btnCompare = new QPushButton("Compare");

    QHBoxLayout *controls = new QHBoxLayout;
    controls->addWidget(labelP1);
    controls->addWidget(labelP2);
    controls->addWidget(btnDrawDDA);
    controls->addWidget(btnDrawBres);
    controls->addWidget(btnClear);
    controls->addWidget(btnCompare);

    mainLayout->addWidget(view);
    mainLayout->addLayout(controls);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    connect(scene, &GridScene::cellClicked, this, &MainWindow::onCellClicked);
    connect(btnDrawDDA, &QPushButton::clicked, this, &MainWindow::drawLineDDA);
    connect(btnDrawBres, &QPushButton::clicked, this, &MainWindow::drawLineBresenham);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::clearGrid);
    connect(btnCompare, &QPushButton::clicked, this, &MainWindow::compareAlgorithms);
}

void MainWindow::onCellClicked(QPoint pos) {
    if (!hasFirstPoint) {
        point1 = pos;
        labelP1->setText(QString("P1: (%1, %2)").arg(pos.x()).arg(pos.y()));
        hasFirstPoint = true;
        scene->paintCell(point1, QBrush(Qt::black));
    } else {
        point2 = pos;
        labelP2->setText(QString("P2: (%1, %2)").arg(pos.x()).arg(pos.y()));
        hasFirstPoint = false;
        scene->paintCell(point2, QBrush(Qt::black));
    }
}

QVector<QPoint> MainWindow::computeDDALine(QPoint p1, QPoint p2) {
    QVector<QPoint> points;

    int dx = p2.x() - p1.x();
    int dy = p2.y() - p1.y();
    int steps = std::max(abs(dx), abs(dy));
    float x_inc = dx / (float) steps;
    float y_inc = dy / (float) steps;

    float x = p1.x();
    float y = p1.y();
    for (int i = 0; i <= steps; ++i) {
        points.append(QPoint((int)x, (int)y));
        x += x_inc;
        y += y_inc;
    }

    return points;
}

QVector<QPoint> MainWindow::computeBresenhamLine(QPoint p1, QPoint p2) {
    QVector<QPoint> points;

    int x1 = p1.x(), y1 = p1.y();
    int x2 = p2.x(), y2 = p2.y();

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int x = x1, y = y1;
    while (true) {
        points.append(QPoint(x, y));
        if (x == x2 && y == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
    return points;
}

void MainWindow::drawLineDDA() {
    auto points = computeDDALine(point1, point2);
    for (auto &pt : points)
        scene->paintCell(pt, QBrush(Qt::red));
}

void MainWindow::drawLineBresenham() {
    auto points = computeBresenhamLine(point1, point2);
    for (auto &pt : points)
        scene->paintCell(pt, QBrush(Qt::blue));
}

qreal MainWindow::computeAvgDDAtime(int num_iters) {
    qint64 totalTimeDDA = 0;

    // qDebug() << "DDA time each iter:\n";
    for (int iter=0; iter < num_iters; iter++) {
        QElapsedTimer timer;

        timer.start();
        volatile auto ddaPoints = computeDDALine(point1, point2);
        // qDebug() << "iter " << iter+1 << ": " << timer.nsecsElapsed() << "ns\n";
        totalTimeDDA += timer.nsecsElapsed();
    }

    // qDebug() << "DDA total time for 50 iters: " << totalTimeDDA << "ns\n";

    return totalTimeDDA / num_iters;
}

qreal MainWindow::computeAvgBresenhamtime(int num_iters) {
    qint64 totalTimeBresenham = 0;

    // qDebug() << "Bresenham time each iter:\n";
    for (int iter=0; iter < num_iters; iter++) {
        QElapsedTimer timer;

        timer.start();
        volatile auto bresPoints = computeBresenhamLine(point1, point2);
        // qDebug() << "iter " << iter+1 << ": " << timer.nsecsElapsed() << "ns\n";
        totalTimeBresenham += timer.nsecsElapsed();
    }

    // qDebug() << "Bresenham total time for 50 iters: " << totalTimeBresenham << "ns\n";
    return totalTimeBresenham / num_iters;
}

void MainWindow::compareAlgorithms() {
    qreal avgDDAtime = computeAvgDDAtime(10);
    qreal avgBresenhamtime = computeAvgBresenhamtime(10);

    qDebug() << "Average time (DDA):" << avgDDAtime << "ns";
    qDebug() << "Average time (Bresenham):" << avgBresenhamtime << "ns";

    if (avgDDAtime > avgBresenhamtime) {
        qDebug() << "Bresenham is faster by " << avgDDAtime - avgBresenhamtime << "ns";
    }
    else if (avgDDAtime < avgBresenhamtime) {
        qDebug() << "DDA is faster by " << avgBresenhamtime - avgDDAtime << "ns";
    }
    else {
        qDebug() << "Both performed equally";
    }
}

void MainWindow::clearGrid() {
    if (line) {
        scene->removeItem(line);
        delete line;
        line = nullptr;
    }

    scene->clearCells();
    labelP1->setText("P1: ( , )");
    labelP2->setText("P2: ( , )");
}

