#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QVector>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QMap>
#include <QBrush>
#include <QSet>

class GridScene;
class GridView;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Edge {
    int ymax;
    float xofymin;
    float slopeinverse;

    bool operator<(const Edge& other) const {
        return xofymin < other.xofymin;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Algorithm { Flood, Boundary, Scanline };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCellClicked(const QPoint& cell);
    void onSeedSelected(const QPoint& cell);
    void onAlgorithmChanged(int idx);
    void onConnectivityChanged(int idx);
    void onDrawLineClicked();
    void onFillClicked();
    void stepFillAnimation();
    void onClearClicked();
    void onResetFillClicked();
    void onBoundaryColorChanged(int idx);
    void onFillColorChanged(int idx);

    void handleLeftClick(const QPoint &cell);
    void handleRightClick(const QPoint &cell);

    void startFloodFill();
    void startBoundaryFill();
    void startScanlineFill();

    void stepFill();
    void resetFill();
    void clearScene();

private:
    Ui::MainWindow *ui;
    GridScene *scene;
    GridView *view;

    QComboBox *boundaryColorCombo;
    QComboBox *fillColorCombo;
    QComboBox *connCombo;
    QComboBox *algCombo;
    QMap<QString, QBrush> colorMap;

    QBrush boundaryBrush;
    QBrush fillBrush;
    QBrush seedBrush = QBrush(Qt::blue);

    QPushButton *drawLineBtn;
    QPushButton *fillBtn;
    QPushButton *resetBtn;
    QPushButton *clearBtn;

    QTimer *fillTimer;
    QVector<QPoint> fillQueue;
    QSet<QPoint> visited;

    QVector<QPoint> selectedPoints;
    QPoint seedPoint;
    bool haveSeed = false;

    Algorithm currentAlgorithm;

    int fillAnimIndex = 0;

    bool isCellPaintedWith(const QPoint& cell, const QBrush& brush) const;

    void styleUi();
    QVector<QPoint> computeBresenhamLine(const QPoint& p1, const QPoint& p2) const;

    bool isEightConnected() const;

    QVector<QPoint> floodFillPoints(const QPoint& seed, bool eightConnected, const QBrush& boundaryBrush, const QBrush& fillBrush);
    QVector<QPoint> boundaryFillPoints(const QPoint& seed, bool eightConnected, const QBrush& boundaryBrush, const QBrush& fillBrush);
    QVector<QPoint> scanlineFillPoints(const QBrush& boundaryBrush, const QBrush& fillBrush);

    void addBoundaryPoint(const QPoint &cell);
};

#endif // MAINWINDOW_H
