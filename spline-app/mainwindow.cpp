#include "mainwindow.h"
#include "gridscene.h"
#include "gridview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QStatusBar>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    scene = new GridScene(this);
    view  = new GridView(this);
    view->setScene(scene);
    scene->setSceneRect(-500, -500, 1000, 1000);

    auto *topRow = new QHBoxLayout;
    chkShowPoly = new QCheckBox("Show control graph", this);
    chkShowPoly->setChecked(false);
    // lblSteps = new QLabel("Segments: 100", this);
    // sldSteps = new QSlider(Qt::Horizontal, this);
    // sldSteps->setRange(10, 400);
    // sldSteps->setValue(100);
    // sldSteps->setFixedWidth(240);
    topRow->addWidget(chkShowPoly);
    topRow->addSpacing(12);
    topRow->addWidget(lblSteps);
    topRow->addWidget(sldSteps);
    topRow->addStretch();

    auto *btnRow = new QHBoxLayout;
    btnDraw = new QPushButton("Draw", this);
    btnAnimate = new QPushButton("Animate", this);
    btnUndo = new QPushButton("Undo Point", this);
    btnNew = new QPushButton("New Curve", this);
    btnClear = new QPushButton("Clear", this);
    btnRow->addWidget(btnDraw);
    btnRow->addWidget(btnAnimate);
    btnRow->addWidget(btnUndo);
    btnRow->addWidget(btnNew);
    btnRow->addStretch();
    btnRow->addWidget(btnClear);

    mainLayout->addLayout(topRow);
    mainLayout->addWidget(view);
    mainLayout->addLayout(btnRow);
    setCentralWidget(central);

    connect(scene, &GridScene::cellClicked, this, &MainWindow::onCellClicked);
    connect(btnDraw, &QPushButton::clicked, this, &MainWindow::onDrawClicked);
    connect(btnAnimate, &QPushButton::clicked, this, &MainWindow::onAnimateClicked);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(btnNew, &QPushButton::clicked, this, &MainWindow::onNewCurveClicked);
    connect(btnUndo, &QPushButton::clicked, this, &MainWindow::onUndoPointClicked);
    // connect(sldSteps, &QSlider::valueChanged, this, &MainWindow::onStepsChanged);
    connect(&animTimer, &QTimer::timeout, this, &MainWindow::stepAnimation);

    setStatus("Click 4 control points on the grid. Then Draw or Animate.");
}

MainWindow::~MainWindow() {}

void MainWindow::setStatus(const QString& s) { statusBar()->showMessage(s, 3000); }

void MainWindow::onCellClicked(const QPoint& cell) {
    if (animTimer.isActive()) return;
    if (controlPts.size() < 4) {
        controlPts.append(cell);
        repaintAll();
        if (controlPts.size() < 4)
            setStatus(QString("P%1 set at (%2,%3)").arg(controlPts.size()).arg(cell.x()).arg(cell.y()));
        else
            setStatus("All 4 control points set. Press Draw or Animate.");
    }
}

void MainWindow::onDrawClicked() {
    if (controlPts.size() != 4) { setStatus("Select 4 control points first."); return; }
    animTimer.stop();
    buildBezierSamplePoints();
    drawBezierImmediate();
}

void MainWindow::onAnimateClicked() {
    if (controlPts.size() != 4) { setStatus("Select 4 control points first."); return; }
    buildBezierSamplePoints();
    scene->clearCells();
    repaintAll();
    animIndex = 1;
    animTimer.start(10);
}

void MainWindow::onClearClicked() {
    animTimer.stop();
    scene->clearCells();
    controlPts.clear();
    bezierPts.clear();
    setStatus("Cleared.");
}

void MainWindow::onNewCurveClicked() {
    animTimer.stop();
    bezierPts.clear();
    controlPts.clear();
    scene->clearCells();
    setStatus("Start placing 4 control points.");
}

void MainWindow::onUndoPointClicked() {
    if (animTimer.isActive()) return;
    if (!controlPts.isEmpty()) {
        controlPts.removeLast();
        bezierPts.clear();
        repaintAll();
        setStatus("Undid last point.");
    }
}

void MainWindow::onStepsChanged(int v) {
    segmentCount = v;
    lblSteps->setText(QString("Segments: %1").arg(v));
    if (controlPts.size() == 4 && !animTimer.isActive()) {
        buildBezierSamplePoints();
        drawBezierImmediate();
    }
}

void MainWindow::stepAnimation() {
    if (animIndex >= bezierPts.size()) { animTimer.stop(); return; }
    bresenhamLine(bezierPts[animIndex-1], bezierPts[animIndex], curveBrush);
    animIndex++;
}

void MainWindow::repaintAll() {
    scene->clearCells();
    for (const auto& p : controlPts) scene->paintCell(p, ctrlBrush);
    if (chkShowPoly->isChecked() && controlPts.size() >= 2) {
        for (int i = 0; i+1 < controlPts.size(); ++i) bresenhamLine(controlPts[i], controlPts[i+1], polyBrush);
    }
    if (!bezierPts.isEmpty()) {
        for (int i = 1; i < bezierPts.size(); ++i) bresenhamLine(bezierPts[i-1], bezierPts[i], curveBrush);
    }
}

void MainWindow::drawControlPolygon() {
    if (controlPts.size() >= 2)
        for (int i = 0; i+1 < controlPts.size(); ++i) bresenhamLine(controlPts[i], controlPts[i+1], polyBrush);
}

void MainWindow::drawBezierImmediate() {
    scene->clearCells();
    for (const auto& p : controlPts) scene->paintCell(p, ctrlBrush);
    if (chkShowPoly->isChecked()) drawControlPolygon();
    for (int i = 1; i < bezierPts.size(); ++i) bresenhamLine(bezierPts[i-1], bezierPts[i], curveBrush);
}

void MainWindow::buildBezierSamplePoints() {
    bezierPts.clear();
    if (controlPts.size() != 4) return;
    QPointF P0(controlPts[0]), P1(controlPts[1]), P2(controlPts[2]), P3(controlPts[3]);
    int N = qMax(2, segmentCount);
    for (int i = 0; i <= N; ++i) {
        double t = double(i) / double(N);
        double u = 1.0 - t;
        double b0 = u*u*u;
        double b1 = 3*u*u*t;
        double b2 = 3*u*t*t;
        double b3 = t*t*t;
        double x = b0*P0.x() + b1*P1.x() + b2*P2.x() + b3*P3.x();
        double y = b0*P0.y() + b1*P1.y() + b2*P2.y() + b3*P3.y();
        bezierPts.append(QPoint(qRound(x), qRound(y)));
    }
    QVector<QPoint> connected;
    for (int i = 1; i < bezierPts.size(); ++i) {
        connected.append(bezierPts[i-1]);
        connected.append(bezierPts[i]);
    }
}

void MainWindow::bresenhamLine(const QPoint& a, const QPoint& b, const QBrush& brush) {
    int x1 = a.x(), y1 = a.y(), x2 = b.x(), y2 = b.y();
    int dx = qAbs(x2 - x1), dy = qAbs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        scene->paintCell(QPoint(x1, y1), brush);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx)  { err += dx; y1 += sy; }
    }
}
