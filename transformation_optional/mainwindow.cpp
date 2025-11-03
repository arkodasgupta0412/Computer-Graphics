#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gridscene.h"
#include "gridview.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new GridScene(this))
    , isDrawing(false)
{
    ui->setupUi(this);

    ui->grid->setScene(scene);
    ui->grid->setFixedSize(ui->grid->width(), ui->grid->height());
    ui->grid->centerOn(0, 0);
    scene->setCellSize(10);

    // Connect all buttons to their slots
    connect(ui->drawPolygon, &QPushButton::clicked, this, &MainWindow::drawPolygon);
    connect(ui->restoreOriginal, &QPushButton::clicked, this, &MainWindow::restoreOriginal);
    connect(ui->reflectX, &QPushButton::clicked, this, &MainWindow::reflectX);
    connect(ui->reflectY, &QPushButton::clicked, this, &MainWindow::reflectY);
    connect(ui->reflectArbitraryLine, &QPushButton::clicked, this, &MainWindow::onReflectArbitraryLine);
    connect(ui->rotateAboutPoint, &QPushButton::clicked, this, &MainWindow::onRotateAboutPoint);
    connect(ui->transform, &QPushButton::clicked, this, &MainWindow::applyTransform);
    connect(ui->clear, &QPushButton::clicked, this, &MainWindow::clear);
    connect(scene, &GridScene::leftClick, this, &MainWindow::onCellClicked);

    originalCells.clear();
    originalCellsF.clear();
    currentCellsF.clear();

    // Set ranges for all spin boxes
    ui->spinBoxA->setRange(-100.0, 100.0);
    ui->spinBoxB->setRange(-100.0, 100.0);
    ui->spinBoxC->setRange(-1000.0, 1000.0);
    ui->spinBoxPivotX->setRange(-1000.0, 1000.0);
    ui->spinBoxPivotY->setRange(-1000.0, 1000.0);
    ui->spinBoxX_scale->setRange(-100.0, 100.0);
    ui->spinBoxY_scale->setRange(-100.0, 100.0);
    ui->spinBox_rotate->setRange(-100.0, 100.0);
    ui->spinBoxX_translate->setRange(-100.0, 100.0);
    ui->spinBoxY_translate->setRange(-100.0, 100.0);
    ui->spinBoxX_shear->setRange(-100.0, 100.0);
    ui->spinBoxY_shear->setRange(-100.0, 100.0);

    ui->spinBoxA->setDecimals(2);
    ui->spinBoxB->setDecimals(2);
    ui->spinBoxC->setDecimals(2);
    ui->spinBoxPivotX->setDecimals(2);
    ui->spinBoxPivotY->setDecimals(2);
    ui->spinBoxX_scale->setDecimals(2);
    ui->spinBoxY_scale->setDecimals(2);
    ui->spinBox_rotate->setDecimals(2);
    ui->spinBoxX_translate->setDecimals(2);
    ui->spinBoxY_translate->setDecimals(2);
    ui->spinBoxX_shear->setDecimals(2);
    ui->spinBoxY_shear->setDecimals(2);
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
    for (const QPointF &p : floatCells) {
        out.append(QPoint(qRound(p.x()), qRound(p.y())));
    }
    return out;
}

void MainWindow::redrawFromFloatCells() {
    scene->clearCells();
    QList<QPoint> pts = roundedCells(currentCellsF);
    // paint vertices
    for (const QPoint &p : pts) scene->paintCell(p, QBrush(Qt::blue));
    // draw outline
    if (pts.size() >= 2) {
        for (int i = 0; i < pts.size() - 1; ++i)
            bresenhamCells(pts[i], pts[i+1]);
        bresenhamCells(pts.last(), pts.first());
    }
    scene->update();
}

void MainWindow::drawPolygon()
{
    if (originalCells.size() < 2) {
        QMessageBox::warning(this, "Warning", "Please select at least 2 vertices first!");
        return;
    }
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
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        scene->paintCell(QPoint(x1, y1), QBrush(Qt::blue));
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

void MainWindow::restoreOriginal()
{
    if (originalCellsF.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No original polygon to restore!");
        return;
    }
    currentCellsF = originalCellsF;
    redrawFromFloatCells();
}


// ==================== HOMOGENEOUS COORDINATE FUNCTIONS ====================
QVector<QVector<double>> MainWindow::multiplyMatrices(
    const QVector<QVector<double>> &m1,
    const QVector<QVector<double>> &m2)
{
    int rows1 = m1.size();
    int cols1 = m1[0].size();
    int cols2 = m2[0].size();

    QVector<QVector<double>> result(rows1, QVector<double>(cols2, 0.0));

    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols2; j++) {
            for (int k = 0; k < cols1; k++) {
                result[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }

    return result;
}

QPointF MainWindow::transformPoint(const QPointF &point, const QVector<QVector<double>> &matrix)
{
    // Homogeneous coordinates [x, y, 1]
    double x = point.x();
    double y = point.y();

    double newX = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2];
    double newY = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2];
    double w = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2];

    return QPointF(newX / w, newY / w);
}

QVector<QVector<double>> MainWindow::getTranslationMatrix(double tx, double ty)
{
    return {
        {1.0, 0.0, tx},
        {0.0, 1.0, -ty},
        {0.0, 0.0, 1.0}
    };
}

QVector<QVector<double>> MainWindow::getScalingMatrix(double sx, double sy)
{
    return {
        {sx, 0.0, 0.0},
        {0.0, sy, 0.0},
        {0.0, 0.0, 1.0}
    };
}

QVector<QVector<double>> MainWindow::getRotationMatrix(double angle)
{
    double rad = qDegreesToRadians(angle);
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);

    return {
        {cosA, -sinA, 0.0},
        {sinA, cosA, 0.0},
        {0.0, 0.0, 1.0}
    };
}

QVector<QVector<double>> MainWindow::getShearMatrix(double shx, double shy)
{
    return {
        {1.0, -shx, 0.0},
        {-shy, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    };
}

QVector<QVector<double>> MainWindow::getReflectionXMatrix()
{
    return {
        {1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0},
        {0.0, 0.0, 1.0}
    };
}

QVector<QVector<double>> MainWindow::getReflectionYMatrix()
{
    return {
        {-1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    };
}

QVector<QVector<double>> MainWindow::getReflectionLineMatrix(double a, double b, double c)
{
    // Reflection about line ax + by + c = 0
    // Normalize the line equation
    double norm = std::sqrt(a * a + b * b);
    if (norm == 0) return {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};

    a /= norm;
    b /= -norm;
    c /= norm;

    double a2 = a * a;
    double b2 = b * b;
    double ab = a * b;

    return {
        {b2 - a2, -2.0 * ab, -2.0 * a * c},
        {-2.0 * ab, a2 - b2, -2.0 * b * c},
        {0.0, 0.0, 1.0}
    };
}

QVector<QVector<double>> MainWindow::getRotationAboutPointMatrix(double angle, double px, double py)
{
    // Rotation about arbitrary point (px, py)
    // Composite transformation: T(px, py) * R(angle) * T(-px, -py)

    auto translate1 = getTranslationMatrix(px, -py);
    auto rotate = getRotationMatrix(angle);
    auto translate2 = getTranslationMatrix(px, py);

    // Multiply in correct order: T2 * R * T1
    auto temp = multiplyMatrices(rotate, translate1);
    return multiplyMatrices(translate2, temp);
}

// ==================== TRANSFORMATION SLOTS ====================

void MainWindow::reflectX()
{
    if (currentCellsF.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please draw a polygon first!");
        return;
    }

    auto matrix = getReflectionXMatrix();

    for (int i = 0; i < currentCellsF.size(); i++) {
        currentCellsF[i] = transformPoint(currentCellsF[i], matrix);
    }

    redrawFromFloatCells();
}

void MainWindow::reflectY()
{
    if (currentCellsF.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please draw a polygon first!");
        return;
    }

    auto matrix = getReflectionYMatrix();

    for (int i = 0; i < currentCellsF.size(); i++) {
        currentCellsF[i] = transformPoint(currentCellsF[i], matrix);
    }

    redrawFromFloatCells();
}

void MainWindow::onReflectArbitraryLine()
{
    if (currentCellsF.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please draw a polygon first!");
        return;
    }

    // Get line equation parameters: Ax + By + C = 0
    double a = ui->spinBoxA->value();
    double b = ui->spinBoxB->value();
    double c = ui->spinBoxC->value();

    if (a == 0 && b == 0) {
        QMessageBox::warning(this, "Warning", "Invalid line equation! A and B cannot both be zero.");
        return;
    }

    auto matrix = getReflectionLineMatrix(a, b, c);

    for (int i = 0; i < currentCellsF.size(); i++) {
        currentCellsF[i] = transformPoint(currentCellsF[i], matrix);
    }

    redrawFromFloatCells();
}

void MainWindow::onRotateAboutPoint()
{
    if (currentCellsF.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please draw a polygon first!");
        return;
    }

    // Get rotation parameters
    double angle = ui->spinBox_rotate->value();
    double px = ui->spinBoxPivotX->value();
    double py = ui->spinBoxPivotY->value();

    auto matrix = getRotationAboutPointMatrix(angle, px, py);

    for (int i = 0; i < currentCellsF.size(); i++) {
        currentCellsF[i] = transformPoint(currentCellsF[i], matrix);
    }

    redrawFromFloatCells();
}

void MainWindow::applyTransform()
{
    if (currentCellsF.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please draw a polygon first!");
        return;
    }

    // Parameters
    double tx = ui->spinBoxX_translate->value();
    double ty = ui->spinBoxY_translate->value();
    double angle = ui->spinBox_rotate->value();

    double sx = 1.0 + ui->spinBoxX_scale->value() / 100.0;
    double sy = 1.0 + ui->spinBoxY_scale->value() / 100.0;

    double shx = ui->spinBoxX_shear->value() / 10.0;
    double shy = ui->spinBoxY_shear->value() / 10.0;

    // Start with identity
    QVector<QVector<double>> compositeMatrix = {
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    };

    // Get screen center (pivot for scaling)
    double cx = ui->grid->width() / (2.0 * scene->getCellSize());
    double cy = ui->grid->height() / (2.0 * scene->getCellSize());

    // ---- Apply transforms in order ----  scale -> shear -> rotate -> translate
    if (sx != 1.0 || sy != 1.0) {
        auto T1 = getTranslationMatrix(-cx, -cy);
        auto S  = getScalingMatrix(sx, sy);
        auto T2 = getTranslationMatrix(cx, cy);

        auto temp = multiplyMatrices(S, T1);
        auto scaleMatrix = multiplyMatrices(T2, temp);

        compositeMatrix = multiplyMatrices(compositeMatrix, scaleMatrix);
    }

    if (shx != 0.0 || shy != 0.0) {
        compositeMatrix = multiplyMatrices(compositeMatrix, getShearMatrix(shx, shy));
    }

    if (angle != 0.0) {
        compositeMatrix = multiplyMatrices(compositeMatrix, getRotationMatrix(angle));
    }

    if (tx != 0.0 || ty != 0.0) {
        compositeMatrix = multiplyMatrices(compositeMatrix, getTranslationMatrix(tx, ty));
    }

    // Apply transformation
    for (int i = 0; i < currentCellsF.size(); i++) {
        currentCellsF[i] = transformPoint(currentCellsF[i], compositeMatrix);
    }

    redrawFromFloatCells();
}


void MainWindow::clear()
{
    scene->clearCells();

    originalCells.clear();
    originalCellsF.clear();
    currentCellsF.clear();

    isDrawing = false;

    // Reset all UI fields
    ui->spinBoxA->setValue(0);
    ui->spinBoxB->setValue(0);
    ui->spinBoxC->setValue(0);
    ui->spinBoxPivotX->setValue(0);
    ui->spinBoxPivotY->setValue(0);
    ui->spinBoxX_scale->setValue(0);
    ui->spinBoxY_scale->setValue(0);
    ui->spinBox_rotate->setValue(0);
    ui->spinBoxX_translate->setValue(0);
    ui->spinBoxY_translate->setValue(0);
    ui->spinBoxX_shear->setValue(0);
    ui->spinBoxY_shear->setValue(0);

    scene->update();
}
