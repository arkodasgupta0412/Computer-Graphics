#include "mainwindow.h"
#include "gridscene.h"
#include "gridview.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QSlider>
#include <QDebug>
#include <QStatusBar>

static QBrush kPolarBrush = QBrush(QColor(220, 20, 60));
static QBrush kMidBrush   = QBrush(QColor(0, 90, 255));

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    scene = new GridScene(this);
    view  = new GridView(this);
    view->setScene(scene);
    scene->setSceneRect(-500, -500, 1000, 1000);

    auto *inputRow = new QHBoxLayout;
    auto *radiusLabel = new QLabel("Radius:");
    radiusInput = new QLineEdit();
    radiusInput->setValidator(new QIntValidator(1, 1000, this));
    radiusInput->setPlaceholderText("Enter radius");
    radiusInput->setMaximumWidth(100);

    radiusSlider = new QSlider(Qt::Horizontal, this);
    radiusSlider->setRange(0, 40);
    radiusSlider->setTickInterval(5);
    radiusSlider->setTickPosition(QSlider::TicksBelow);
    radiusSlider->setFixedWidth(200);
    radiusSlider->setValue(10);

    inputRow->addWidget(radiusLabel);
    inputRow->addWidget(radiusInput);
    inputRow->addSpacing(12);
    inputRow->addWidget(new QLabel("Live radius:"));
    inputRow->addWidget(radiusSlider);
    inputRow->addStretch();

    auto *btnRow = new QHBoxLayout;
    auto *btnPolar = new QPushButton("Draw Circle (Polar)");
    auto *btnMid   = new QPushButton("Draw Circle (Midpoint)");
    auto *btnCart  = new QPushButton("Draw Circle (Cartesian)");
    auto *btnClear = new QPushButton("Clear");
    auto *btnPerf  = new QPushButton("Compare Times");

    btnRow->addWidget(btnPolar);
    btnRow->addWidget(btnMid);
    btnRow->addWidget(btnCart);
    btnRow->addWidget(btnPerf);
    btnRow->addStretch();
    btnRow->addWidget(btnClear);

    mainLayout->addLayout(inputRow);
    mainLayout->addWidget(view);
    mainLayout->addLayout(btnRow);
    setCentralWidget(central);

    connect(scene, &GridScene::cellClicked, this, &MainWindow::onCellClicked);
    connect(radiusInput, &QLineEdit::textChanged, this, &MainWindow::onRadiusChanged);
    connect(btnPolar, &QPushButton::clicked, this, &MainWindow::drawCirclePolar);
    connect(btnMid,   &QPushButton::clicked, this, &MainWindow::drawCircleMidpoint);
    connect(btnCart,  &QPushButton::clicked, this, &MainWindow::drawCircleCartesian);
    connect(btnPerf,  &QPushButton::clicked, this, &MainWindow::compareExecutionTimes);
    connect(btnClear, &QPushButton::clicked, scene, &GridScene::clearCells);
    connect(radiusSlider, &QSlider::valueChanged, this, &MainWindow::onRadiusSliderChanged);

    animTimer.setSingleShot(false);
    connect(&animTimer, &QTimer::timeout, this, &MainWindow::stepAnimation);

    setStatus("Click to select center point, then enter radius and click draw.");
}

MainWindow::~MainWindow() {}

void MainWindow::onRadiusSliderChanged(int value)
{
    if (!updatingRadiusUI) {
        updatingRadiusUI = true;
        radiusInput->setText(QString::number(value));
        updatingRadiusUI = false;
    }
    animTimer.stop();
    if (!haveCenter) {
        setStatus(QString("Radius: %1 (select a center by clicking the grid)").arg(value));
        return;
    }
    drawCircleImmediate(value, kMidBrush);
    setStatus(QString("Radius: %1").arg(value));
}

void MainWindow::drawCircleImmediate(int r, const QBrush& brush)
{
    scene->clearCells();
    scene->paintCell(centerCell, QBrush(Qt::blue));
    const auto pts = buildMidpointFrames(centerCell, r);
    for (const QPoint& p : pts) scene->paintCell(p, brush);
}

void MainWindow::setStatus(const QString& s) { statusBar()->showMessage(s, 5000); }

void MainWindow::onCellClicked(const QPoint& cell) {
    centerCell = cell;
    haveCenter = true;
    scene->clearCells();
    scene->paintCell(centerCell, QBrush(Qt::blue));
    setStatus(QString("Center set at (%1,%2). Enter radius and click draw.").arg(cell.x()).arg(cell.y()));
}

void MainWindow::onRadiusChanged() {
    const QString text = radiusInput->text();
    if (text.isEmpty()) return;
    const int radius = text.toInt();
    if (!updatingRadiusUI && radiusSlider) {
        updatingRadiusUI = true;
        radiusSlider->setValue(qBound(radiusSlider->minimum(), radius, radiusSlider->maximum()));
        updatingRadiusUI = false;
    }
    setStatus(QString("Radius: %1. Use slider for live redraw.").arg(radius));
}

int MainWindow::currentRadius() const {
    const QString text = radiusInput->text();
    if (text.isEmpty()) return 10;
    return text.toInt();
}

QVector<QPoint> MainWindow::eightSymmetry(const QPoint& c, int x, int y) {
    return {
        {c.x() + x, c.y() + y},
        {c.x() - x, c.y() + y},
        {c.x() + x, c.y() - y},
        {c.x() - x, c.y() - y},
        {c.x() + y, c.y() + x},
        {c.x() - y, c.y() + x},
        {c.x() + y, c.y() - x},
        {c.x() - y, c.y() - x}
    };
}

QVector<QPoint> MainWindow::buildPolarFrames(const QPoint& c, int r) {
    QVector<QPoint> frames;
    const double dtheta = 1.0 / qMax(1, r);
    for (double t = 0.0; t <= M_PI/4.0 + 1e-9; t += dtheta) {
        const int x = int(qRound(r * qCos(t)));
        const int y = int(qRound(r * qSin(t)));
        frames += eightSymmetry(c, x, y);
    }
    return frames;
}

QVector<QPoint> MainWindow::buildMidpointFrames(const QPoint& c, int r) {
    QVector<QPoint> frames;
    int x = 0, y = r, p = 1 - r;
    while (x <= y) {
        frames += eightSymmetry(c, x, y);
        ++x;
        if (p < 0) p += 2 * x + 1;
        else { --y; p += 2 * (x - y) + 1; }
    }
    return frames;
}

QVector<QPoint> MainWindow::buildCartesianFrames(const QPoint& c, int r) {
    QVector<QPoint> frames;
    for (int x = 0; x <= r; ++x) {
        const int y = int(qRound(qSqrt(double(r*r - x*x))));
        frames += eightSymmetry(c, x, y);
    }
    return frames;
}

void MainWindow::beginAnimation(const QVector<QPoint>& frames, const QBrush& brush, int msStep) {
    if (!haveCenter) { setStatus("Please select a center point first!"); return; }
    if (radiusInput->text().isEmpty()) { setStatus("Please enter a radius value!"); return; }
    animTimer.stop();
    animFrames = frames;
    animIndex  = 0;
    animBrush  = brush;
    scene->paintCell(centerCell, QBrush(Qt::blue));
    if (!animFrames.isEmpty()) animTimer.start(msStep);
}

void MainWindow::stepAnimation() {
    if (animIndex >= animFrames.size()) { animTimer.stop(); return; }
    scene->paintCell(animFrames[animIndex++], animBrush);
}

void MainWindow::drawCirclePolar() {
    const int r = currentRadius();
    auto frames = buildPolarFrames(centerCell, r);
    qDebug() << "Number of pixels coloured (Polar Method):" << frames.size();
    beginAnimation(frames, kPolarBrush, 15);
}

void MainWindow::drawCircleMidpoint() {
    const int r = currentRadius();
    auto frames = buildMidpointFrames(centerCell, r);
    qDebug() << "Number of pixels coloured (Midpoint Method):" << frames.size();
    beginAnimation(frames, kMidBrush, 10);
}

void MainWindow::drawCircleCartesian() {
    const int r = currentRadius();
    auto frames = buildCartesianFrames(centerCell, r);
    qDebug() << "Number of pixels coloured (Cartesian Method):" << frames.size();
    beginAnimation(frames, QBrush(QColor(10, 160, 10)), 12);
}

void MainWindow::compareExecutionTimes() {
    if (!haveCenter) { setStatus("Please select a center point first!"); return; }
    if (radiusInput->text().isEmpty()) { setStatus("Please enter a radius value!"); return; }
    const int r = currentRadius();
    const int iters = 200;
    QElapsedTimer t; qint64 tp=0, tm=0, tc=0;
    t.start(); for (int i=0;i<iters;++i) volatile auto v = buildPolarFrames(centerCell, r); tp = t.nsecsElapsed();
    t.restart(); for (int i=0;i<iters;++i) volatile auto v = buildMidpointFrames(centerCell, r); tm = t.nsecsElapsed();
    t.restart(); for (int i=0;i<iters;++i) volatile auto v = buildCartesianFrames(centerCell, r); tc = t.nsecsElapsed();
    setStatus(QString("Avg per iteration (ns): Polar %1 | Midpoint %2 | Cartesian %3").arg(tp/iters).arg(tm/iters).arg(tc/iters));
}
