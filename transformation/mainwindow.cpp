#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gridscene.h"
#include "gridview.h"

#include <QMouseEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QtMath>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new GridScene(this))
    , isDrawing(false)
{
    ui->setupUi(this);

    // --- View / Scene setup ---
    ui->grid->setScene(scene);

    // Remove fixed size so layouts can resize the view
    // ui->grid->setFixedSize(ui->grid->width(), ui->grid->height());  // <-- remove this
    ui->grid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // A large scene rect so you can draw freely around the center
    scene->setSceneRect(-1500, -1000, 3000, 2000);
    ui->grid->centerOn(0, 0);
    ui->grid->setAlignment(Qt::AlignCenter);
    scene->setCellSize(10);

    // Core action buttons wired
    connect(ui->drawPolygon, &QPushButton::clicked, this, &MainWindow::drawPolygon);
    connect(ui->restoreOriginal, &QPushButton::clicked, this, &MainWindow::restoreOriginal);
    connect(ui->reflectX, &QPushButton::clicked, this, &MainWindow::reflectX);
    connect(ui->reflectY, &QPushButton::clicked, this, &MainWindow::reflectY);
    connect(ui->translate, &QPushButton::clicked, this, &MainWindow::translate);
    connect(ui->rotate, &QPushButton::clicked, this, &MainWindow::rotate);
    connect(ui->scale, &QPushButton::clicked, this, &MainWindow::scale);
    connect(ui->shear, &QPushButton::clicked, this, &MainWindow::shear);
    connect(ui->clear, &QPushButton::clicked, this, &MainWindow::clear);
    connect(ui->reflectArbitraryLine, &QPushButton::clicked, this, &MainWindow::onReflectArbitraryLine);
    connect(ui->rotateAboutPoint, &QPushButton::clicked, this, &MainWindow::onRotateAboutPoint);

    // (Optional) Try to auto-wire the special-case buttons if they exist.
    // If you already connected these in Designer, this is harmless.
    if (auto btn = findChild<QPushButton*>("rotateAboutPointButton"))
        connect(btn, &QPushButton::clicked, this, &MainWindow::onRotateAboutPoint);
    if (auto btn = findChild<QPushButton*>("btnRotateAbout"))
        connect(btn, &QPushButton::clicked, this, &MainWindow::onRotateAboutPoint);
    if (auto btn = findChild<QPushButton*>("reflectLineButton"))
        connect(btn, &QPushButton::clicked, this, &MainWindow::onReflectArbitraryLine);
    if (auto btn = findChild<QPushButton*>("btnReflectLine"))
        connect(btn, &QPushButton::clicked, this, &MainWindow::onReflectArbitraryLine);

    // Scene click hookup
    connect(scene, &GridScene::leftClick, this, &MainWindow::onCellClicked);

    originalCells.clear();
    originalCellsF.clear();
    currentCellsF.clear();

    // Ranges
    ui->spinBoxX_translate->setRange(-1000.0, 1000.0);
    ui->spinBoxY_translate->setRange(-1000.0, 1000.0);
    ui->spinBox_rotate->setRange(-360.0, 360.0);
    ui->spinBoxX_scale->setRange(-10.0, 10.0);
    ui->spinBoxY_scale->setRange(-10.0, 10.0);
    ui->spinBoxX_shear->setRange(-10.0, 10.0);
    ui->spinBoxY_shear->setRange(-10.0, 10.0);
    ui->spinBoxA->setRange(-100.0, 100.0);
    ui->spinBoxB->setRange(-100.0, 100.0);
    ui->spinBoxC->setRange(-1000.0, 1000.0);
    ui->spinBoxPivotX->setRange(-1000.0, 1000.0);
    ui->spinBoxPivotY->setRange(-1000.0, 1000.0);

    // Steps/decimals
    ui->spinBoxX_translate->setSingleStep(0.1);
    ui->spinBoxY_translate->setSingleStep(0.1);
    ui->spinBox_rotate->setSingleStep(0.1);
    ui->spinBoxX_scale->setSingleStep(0.1);
    ui->spinBoxY_scale->setSingleStep(0.1);
    ui->spinBoxX_shear->setSingleStep(3);
    ui->spinBoxY_shear->setSingleStep(3);
    ui->spinBoxX_translate->setDecimals(2);
    ui->spinBoxY_translate->setDecimals(2);
    ui->spinBox_rotate->setDecimals(2);
    ui->spinBoxX_scale->setDecimals(2);
    ui->spinBoxY_scale->setDecimals(2);
    ui->spinBoxX_shear->setDecimals(3);
    ui->spinBoxY_shear->setDecimals(3);
}

MainWindow::~MainWindow()
{
    delete scene;
    delete ui;
}

void MainWindow::onCellClicked(const QPoint& cell)
{
    if (!isDrawing) {
        scene->clearCells();
        originalCells.clear();
        originalCellsF.clear();
        currentCellsF.clear();
        isDrawing = true;
    }
    originalCells.append(cell);
    originalCellsF.append(QPointF(cell));
    currentCellsF = originalCellsF;
    scene->paintCell(cell, QBrush(Qt::blue));
    scene->update();
}

QList<QPoint> MainWindow::roundedCells(const QList<QPointF>& floatCells) const {
    QList<QPoint> out;
    out.reserve(floatCells.size());
    for (const QPointF &p : floatCells) out.append(QPoint(qRound(p.x()), qRound(p.y())));
    return out;
}

void MainWindow::redrawFromFloatCells() {
    scene->clearCells();
    const QList<QPoint> pts = roundedCells(currentCellsF);
    for (const QPoint &p : pts) scene->paintCell(p, QBrush(Qt::blue));
    if (pts.size() >= 2) {
        for (int i = 0; i < pts.size() - 1; ++i) bresenhamCells(pts[i], pts[i+1]);
        bresenhamCells(pts.last(), pts.first());
    }
    scene->update();
}

void MainWindow::drawPolygon()
{
    if (originalCells.size() < 2) return;
    currentCellsF = originalCellsF;
    redrawFromFloatCells();
}

void MainWindow::drawLines()
{
    redrawFromFloatCells();
}

void MainWindow::bresenhamCells(const QPoint& p1, const QPoint& p2)
{
    int x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();
    int dx = std::abs(x2 - x1), dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        scene->paintCell(QPoint(x1, y1), QBrush(Qt::blue));
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
}

void MainWindow::restoreOriginal()
{
    currentCellsF = originalCellsF;
    redrawFromFloatCells();
}

void MainWindow::reflectX()
{
    for (QPointF &p : currentCellsF) p.setY(-p.y());
    redrawFromFloatCells();
}

void MainWindow::reflectY()
{
    for (QPointF &p : currentCellsF) p.setX(-p.x());
    redrawFromFloatCells();
}

void MainWindow::translate()
{
    // UI is in math y-up; screen is y-down → invert dy
    const qreal dx = static_cast<qreal>(ui->spinBoxX_translate->value());
    const qreal dy = static_cast<qreal>(ui->spinBoxY_translate->value());
    for (QPointF &p : currentCellsF) p += QPointF(dx, -dy);
    redrawFromFloatCells();
}

void MainWindow::rotate()
{
    // Keep behavior users already see as correct: use -angle on screen coords
    const qreal angle = static_cast<qreal>(ui->spinBox_rotate->value());
    const qreal rad   = qDegreesToRadians(-angle);
    const qreal c = std::cos(rad);
    const qreal s = std::sin(rad);
    for (QPointF &p : currentCellsF) {
        const qreal x = p.x(), y = p.y();
        p.setX(x * c - y * s);
        p.setY(x * s + y * c);
    }
    redrawFromFloatCells();
}

// --- Matrix helpers ---
QList<QPointF> MainWindow::applyTransformMatrix(const QList<QPointF>& points,
                                                const QVector<QVector<double>>& mat)
{
    QList<QPointF> result;
    result.reserve(points.size());
    for (const QPointF& p : points) {
        const double x = p.x(), y = p.y();
        const double nx = mat[0][0]*x + mat[0][1]*y + mat[0][2];
        const double ny = mat[1][0]*x + mat[1][1]*y + mat[1][2];
        const double nw = mat[2][0]*x + mat[2][1]*y + mat[2][2];
        result.append( (nw != 0.0 && nw != 1.0) ? QPointF(nx/nw, ny/nw) : QPointF(nx, ny) );
    }
    return result;
}

void MainWindow::scale()
{
    const double sx = static_cast<double>(ui->spinBoxX_scale->value());
    const double sy = static_cast<double>(ui->spinBoxY_scale->value());
    const QVector<QVector<double>> S = {
        {sx, 0.0, 0.0},
        {0.0, sy, 0.0},
        {0.0, 0.0, 1.0}
    };
    currentCellsF = applyTransformMatrix(currentCellsF, S);
    redrawFromFloatCells();
}

void MainWindow::shear()
{
    // Keep your existing UI convention (positive is "math y-up")
    const qreal shx = static_cast<qreal>(ui->spinBoxX_shear->value()) / 10.0;
    const qreal shy = static_cast<qreal>(ui->spinBoxY_shear->value()) / 10.0;
    for (QPointF &p : currentCellsF) {
        const qreal x = p.x(), y = p.y();
        p.setX(x + y * -shx);
        p.setY(x * -shy + y);
    }
    redrawFromFloatCells();
}

QVector<QVector<double>> MainWindow::getReflectionXMatrix()
{
    return { {1.0, 0.0, 0.0},
            {0.0,-1.0, 0.0},
            {0.0, 0.0, 1.0} };
}

QVector<QVector<double>> MainWindow::getReflectionYMatrix()
{
    return { {-1.0, 0.0, 0.0},
            { 0.0, 1.0, 0.0},
            { 0.0, 0.0, 1.0} };
}

QVector<QVector<double>> MainWindow::getReflectionLineMatrix(double a, double b, double c)
{
    // Expect a,b,c from UI in math y-up. Convert to screen y-down: (a, -b, c)
    double ad = a, bd = -b, cd = c;

    const double norm = std::sqrt(ad*ad + bd*bd);
    if (norm == 0.0) return { {1,0,0},{0,1,0},{0,0,1} };

    ad /= norm; bd /= norm; cd /= norm;
    return {
        {1.0 - 2.0*ad*ad,   -2.0*ad*bd,     -2.0*ad*cd},
        {  -2.0*ad*bd,    1.0 - 2.0*bd*bd,  -2.0*bd*cd},
        {0.0,               0.0,             1.0}
    };
}

QVector<QVector<double>> MainWindow::getRotationAboutPointMatrix(double angle, double px, double py)
{
    // UI pivot is in math y-up; convert to screen y-down
    const double px_d = px;
    const double py_d = -py;

    // Composite on screen coords: T(px_d, py_d) * R(angle) * T(-px_d, -py_d)
    const auto T1 = getTranslationMatrix(-px_d, -py_d);
    const auto R  = getRotationMatrix(angle);   // uses -angle internally to match your rotate()
    const auto T2 = getTranslationMatrix( px_d,  py_d);
    return multiplyMatrices(T2, multiplyMatrices(R, T1));
}

void MainWindow::clear()
{
    scene->clearCells();
    originalCells.clear();
    originalCellsF.clear();
    currentCellsF.clear();
    isDrawing = false;

    ui->spinBoxX_translate->setValue(0);
    ui->spinBoxY_translate->setValue(0);
    ui->spinBox_rotate->setValue(0);
    ui->spinBoxX_scale->setValue(0);
    ui->spinBoxY_scale->setValue(0);
    ui->spinBoxX_shear->setValue(0);
    ui->spinBoxY_shear->setValue(0);
    scene->update();
}

void MainWindow::onReflectArbitraryLine()
{
    if (currentCellsF.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please draw a polygon first!");
        return;
    }

    const double a = ui->spinBoxA->value();
    const double b = ui->spinBoxB->value();
    const double c = ui->spinBoxC->value();

    if (a == 0.0 && b == 0.0) {
        QMessageBox::warning(this, "Warning", "Invalid line! A and B cannot both be zero.");
        return;
    }

    const auto M = getReflectionLineMatrix(a, b, c); // conversion handled inside
    for (int i = 0; i < currentCellsF.size(); ++i)
        currentCellsF[i] = transformPoint(currentCellsF[i], M);

    redrawFromFloatCells();
}

void MainWindow::onRotateAboutPoint()
{
    if (currentCellsF.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please draw a polygon first!");
        return;
    }

    const double angle = ui->spinBox_rotate->value();
    const double px = ui->spinBoxPivotX->value();
    const double py = ui->spinBoxPivotY->value();

    const auto M = getRotationAboutPointMatrix(angle, px, py); // UI→screen conversion inside
    for (int i = 0; i < currentCellsF.size(); ++i)
        currentCellsF[i] = transformPoint(currentCellsF[i], M);

    redrawFromFloatCells();
}

QPointF MainWindow::transformPoint(const QPointF& p, const QVector<QVector<double>>& mat) const
{
    const double x = p.x(), y = p.y();
    const double nx = mat[0][0]*x + mat[0][1]*y + mat[0][2];
    const double ny = mat[1][0]*x + mat[1][1]*y + mat[1][2];
    const double nw = mat[2][0]*x + mat[2][1]*y + mat[2][2];
    return (nw != 0.0 && nw != 1.0) ? QPointF(nx/nw, ny/nw) : QPointF(nx, ny);
}

QVector<QVector<double>> MainWindow::multiplyMatrices(const QVector<QVector<double>>& A,
                                                      const QVector<QVector<double>>& B) const
{
    QVector<QVector<double>> C(3, QVector<double>(3, 0.0));
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            for (int k = 0; k < 3; ++k)
                C[r][c] += A[r][k] * B[k][c];
    return C;
}

QVector<QVector<double>> MainWindow::getTranslationMatrix(double tx, double ty) const
{
    return { {1.0, 0.0, tx},
            {0.0, 1.0, ty},
            {0.0, 0.0, 1.0} };
}

QVector<QVector<double>> MainWindow::getRotationMatrix(double angleDegrees) const
{
    // Matches your existing rotate(): uses -angle on screen coords
    const double rad = qDegreesToRadians(-angleDegrees);
    const double c = std::cos(rad);
    const double s = std::sin(rad);
    return { { c, -s, 0.0},
            { s,  c, 0.0},
            {0.0, 0.0, 1.0} };
}
