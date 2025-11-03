#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QPoint>
#include <QPointF>
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
    void translate();
    void rotate();
    void scale();
    void shear();

    // Matrices
    QVector<QVector<double>> getReflectionXMatrix();
    QVector<QVector<double>> getReflectionYMatrix();
    QVector<QVector<double>> getReflectionLineMatrix(double a, double b, double c);
    QVector<QVector<double>> getRotationAboutPointMatrix(double angle, double px, double py);

    // Actions
    void onReflectArbitraryLine();
    void onRotateAboutPoint();
    void clear();
    void onCellClicked(const QPoint& cell);

private:
    void drawLines();
    void bresenhamCells(const QPoint& p1, const QPoint& p2);

    // Helpers
    QList<QPoint> roundedCells(const QList<QPointF>& floatCells) const;
    void redrawFromFloatCells();

    // Matrix/transform helpers (all homogeneous 3x3, row-major)
    QList<QPointF> applyTransformMatrix(const QList<QPointF>& points, const QVector<QVector<double>>& mat);
    QPointF transformPoint(const QPointF& p, const QVector<QVector<double>>& mat) const;
    QVector<QVector<double>> multiplyMatrices(const QVector<QVector<double>>& A, const QVector<QVector<double>>& B) const;
    QVector<QVector<double>> getTranslationMatrix(double tx, double ty) const;
    QVector<QVector<double>> getRotationMatrix(double angleDegrees) const;

    Ui::MainWindow *ui;
    GridScene *scene;

    QList<QPoint> originalCells;
    QList<QPointF> originalCellsF;
    QList<QPointF> currentCellsF;
    bool isDrawing;
};
#endif // MAINWINDOW_H
