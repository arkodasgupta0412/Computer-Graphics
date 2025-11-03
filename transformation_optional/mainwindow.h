#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QPoint>
#include <QPointF>
#include <QVector>
#include "gridscene.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void drawPolygon();
    void restoreOriginal();
    void reflectX();
    void reflectY();
    void onReflectArbitraryLine();
    void onRotateAboutPoint();
    void applyTransform();
    void clear();
    void onCellClicked(const QPoint& cell);

private:
    void drawLines();
    void bresenhamCells(const QPoint& p1, const QPoint& p2);

    // Helpers
    QList<QPoint> roundedCells(const QList<QPointF>& floatCells) const;
    void redrawFromFloatCells();

    // Homogeneous coordinate transformation functions
    QVector<QVector<double>> multiplyMatrices(const QVector<QVector<double>> &m1, const QVector<QVector<double>> &m2);
    QPointF transformPoint(const QPointF &point, const QVector<QVector<double>> &matrix);

    // Transformation matrices (3x3 homogeneous coordinates)
    QVector<QVector<double>> getTranslationMatrix(double tx, double ty);
    QVector<QVector<double>> getScalingMatrix(double sx, double sy);
    QVector<QVector<double>> getRotationMatrix(double angle);
    QVector<QVector<double>> getShearMatrix(double shx, double shy);
    QVector<QVector<double>> getReflectionXMatrix();
    QVector<QVector<double>> getReflectionYMatrix();
    QVector<QVector<double>> getReflectionLineMatrix(double a, double b, double c);
    QVector<QVector<double>> getRotationAboutPointMatrix(double angle, double px, double py);

    Ui::MainWindow *ui;
    GridScene *scene;

    // Store both integer cell coordinates and floating coords for transforms
    QList<QPoint> originalCells;
    QList<QPointF> originalCellsF;
    QList<QPointF> currentCellsF;
    bool isDrawing;
};

#endif // MAINWINDOW_H
