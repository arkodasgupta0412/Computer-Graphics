#include "mainwindow.h"
#include "ui_mainwindow.h"
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
#include <QStatusBar>
#include <QDebug>

static QBrush kPolarBrush = QBrush(QColor(220, 20, 60));
static QBrush kMidBrush   = QBrush(QColor(0, 90, 255));

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setStatusBar(new QStatusBar(this));

    QWidget *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    scene = new GridScene(this);
    view  = new GridView(this);
    view->setScene(scene);
    scene->setSceneRect(-500, -500, 1000, 1000);

    auto *inputRow = new QHBoxLayout;
    auto *aLabel = new QLabel("a:");
    aInput = new QLineEdit();
    aInput->setValidator(new QIntValidator(1, 1000, this));
    aInput->setMaximumWidth(80);

    aSlider = new QSlider(Qt::Horizontal, this);
    aSlider->setRange(1, 80);
    aSlider->setTickInterval(5);
    aSlider->setTickPosition(QSlider::TicksBelow);
    aSlider->setFixedWidth(200);
    aSlider->setValue(20);
    aInput->setText("20");

    auto *bLabel = new QLabel("b:");
    bInput = new QLineEdit();
    bInput->setValidator(new QIntValidator(1, 1000, this));
    bInput->setMaximumWidth(80);

    bSlider = new QSlider(Qt::Horizontal, this);
    bSlider->setRange(1, 80);
    bSlider->setTickInterval(5);
    bSlider->setTickPosition(QSlider::TicksBelow);
    bSlider->setFixedWidth(200);
    bSlider->setValue(10);
    bInput->setText("10");

    inputRow->addWidget(aLabel);
    inputRow->addWidget(aInput);
    inputRow->addSpacing(8);
    inputRow->addWidget(aSlider);
    inputRow->addSpacing(16);
    inputRow->addWidget(bLabel);
    inputRow->addWidget(bInput);
    inputRow->addSpacing(8);
    inputRow->addWidget(bSlider);
    inputRow->addStretch();

    auto *btnRow = new QHBoxLayout;
    auto *btnPolar = new QPushButton("Draw Ellipse (Polar)");
    auto *btnMid   = new QPushButton("Draw Ellipse (Midpoint)");
    auto *btnClear = new QPushButton("Clear");
    auto *btnPerf  = new QPushButton("Compare Times");

    btnRow->addWidget(btnPolar);
    btnRow->addWidget(btnMid);
    btnRow->addWidget(btnPerf);
    btnRow->addStretch();
    btnRow->addWidget(btnClear);

    mainLayout->addLayout(inputRow);
    mainLayout->addWidget(view);
    mainLayout->addLayout(btnRow);
    setCentralWidget(central);

    connect(scene, &GridScene::cellClicked, this, &MainWindow::onCellClicked);
    connect(aInput, &QLineEdit::textChanged, this, &MainWindow::onAxesChanged);
    connect(bInput, &QLineEdit::textChanged, this, &MainWindow::onAxesChanged);
    connect(aSlider, &QSlider::valueChanged, this, &MainWindow::onASliderChanged);
    connect(bSlider, &QSlider::valueChanged, this, &MainWindow::onBSliderChanged);
    connect(btnPolar, &QPushButton::clicked, this, &MainWindow::drawEllipsePolar);
    connect(btnMid,   &QPushButton::clicked, this, &MainWindow::drawEllipseMidpoint);
    connect(btnPerf,  &QPushButton::clicked, this, &MainWindow::compareExecutionTimes);
    connect(btnClear, &QPushButton::clicked, scene, &GridScene::clearCells);

    animTimer.setSingleShot(false);
    connect(&animTimer, &QTimer::timeout, this, &MainWindow::stepAnimation);

    setStatus("Click to select center, then slide a/b or click draw.");
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setStatus(const QString& s) { statusBar()->showMessage(s, 3000); }

void MainWindow::onCellClicked(const QPoint& cell) {
    centerCell = cell;
    haveCenter = true;
    scene->clearCells();
    scene->paintCell(centerCell, QBrush(Qt::blue));
    setStatus(QString("Center: (%1,%2)").arg(cell.x()).arg(cell.y()));
}

void MainWindow::onAxesChanged() {
    if (updatingAxisUI) return;
    QString aText = aInput->text(), bText = bInput->text();
    if (!aText.isEmpty()) aSlider->setValue(aText.toInt());
    if (!bText.isEmpty()) bSlider->setValue(bText.toInt());
}

void MainWindow::onASliderChanged(int value) {
    if (!updatingAxisUI) {
        updatingAxisUI = true;
        aInput->setText(QString::number(value));
        updatingAxisUI = false;
    }
    animTimer.stop();
    if (!haveCenter) { setStatus(QString("a=%1 (click to set center)").arg(value)); return; }
    drawEllipseImmediate(value, bSlider->value());
    setStatus(QString("a=%1, b=%2").arg(value).arg(bSlider->value()));
}

void MainWindow::onBSliderChanged(int value) {
    if (!updatingAxisUI) {
        updatingAxisUI = true;
        bInput->setText(QString::number(value));
        updatingAxisUI = false;
    }
    animTimer.stop();
    if (!haveCenter) { setStatus(QString("b=%1 (click to set center)").arg(value)); return; }
    drawEllipseImmediate(aSlider->value(), value);
    setStatus(QString("a=%1, b=%2").arg(aSlider->value()).arg(value));
}

int MainWindow::currentA() const {
    QString t = aInput->text();
    return t.isEmpty() ? 20 : t.toInt();
}

int MainWindow::currentB() const {
    QString t = bInput->text();
    return t.isEmpty() ? 10 : t.toInt();
}

QVector<QPoint> MainWindow::fourSymmetry(const QPoint& c, int x, int y) {
    return {
        {c.x() + x, c.y() + y},
        {c.x() - x, c.y() + y},
        {c.x() + x, c.y() - y},
        {c.x() - x, c.y() - y}
    };
}

QVector<QPoint> MainWindow::buildPolarFrames(const QPoint& c, int a, int b) {
    QVector<QPoint> frames;
    double dtheta = 1.0 / (a + b);
    for (double theta = 0.0; theta <= M_PI/2.0 + 1e-9; theta += dtheta) {
        int x = (int)qRound(a * qCos(theta));
        int y = (int)qRound(b * qSin(theta));
        frames += fourSymmetry(c, x, y);
    }
    return frames;
}

QVector<QPoint> MainWindow::buildMidpointFrames(const QPoint& c, int a, int b) {
    QVector<QPoint> frames;
    int x = 0, y = b;
    int a2 = a*a, b2 = b*b;
    double d1 = b2 - a2*b + 0.25*a2;

    while ((2*b2*x) <= (2*a2*y)) {
        frames += fourSymmetry(c, x, y);
        if (d1 < 0) d1 += b2*(2*x + 3);
        else { d1 += b2*(2*x + 3) + a2*(-2*y + 2); y--; }
        x++;
    }

    double d2 = b2*(x + 0.5)*(x + 0.5) + a2*(y - 1)*(y - 1) - a2*b2;
    while (y >= 0) {
        frames += fourSymmetry(c, x, y);
        if (d2 < 0) { d2 += b2*(2*x + 2) + a2*(-2*y + 3); x++; }
        else d2 += a2*(-2*y + 3);
        y--;
    }
    return frames;
}

void MainWindow::beginAnimation(const QVector<QPoint>& frames, const QBrush& brush, int msStep) {
    if (!haveCenter) { setStatus("Select center first"); return; }
    if (aInput->text().isEmpty() || bInput->text().isEmpty()) { setStatus("Enter a and b"); return; }
    animTimer.stop();
    animFrames = frames;
    animIndex  = 0;
    animBrush  = brush;
    scene->paintCell(centerCell, QBrush(Qt::blue));
    if (!animFrames.isEmpty()) animTimer.start(msStep);
}

void MainWindow::drawEllipseImmediate(int a, int b) {
    scene->clearCells();
    scene->paintCell(centerCell, QBrush(Qt::blue));
    const auto pts = buildMidpointFrames(centerCell, a, b);
    for (const QPoint& p : pts) scene->paintCell(p, kMidBrush);
}

void MainWindow::stepAnimation() {
    if (animIndex >= animFrames.size()) { animTimer.stop(); return; }
    scene->paintCell(animFrames[animIndex++], animBrush);
}

void MainWindow::drawEllipsePolar() {
    int a = currentA(), b = currentB();
    beginAnimation(buildPolarFrames(centerCell, a, b), kPolarBrush, 8);
}

void MainWindow::drawEllipseMidpoint() {
    int a = currentA(), b = currentB();
    beginAnimation(buildMidpointFrames(centerCell, a, b), kMidBrush, 4);
}

void MainWindow::compareExecutionTimes() {
    if (!haveCenter) { setStatus("Select center first"); return; }
    if (aInput->text().isEmpty() || bInput->text().isEmpty()) { setStatus("Enter a and b"); return; }

    int a = currentA(), b = currentB();
    const int iters = 200;
    QElapsedTimer t;
    qint64 tp = 0, tm = 0;

    t.start();
    for (int i=0;i<iters;++i) volatile auto v = buildPolarFrames(centerCell, a, b);
    tp = t.nsecsElapsed();

    t.restart();
    for (int i=0;i<iters;++i) volatile auto v = buildMidpointFrames(centerCell, a, b);
    tm = t.nsecsElapsed();

    setStatus(QString("Avg ns: Polar %1 | Midpoint %2").arg(tp/iters).arg(tm/iters));
}
