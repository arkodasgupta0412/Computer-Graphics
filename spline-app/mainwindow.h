#pragma once
#include <QMainWindow>
#include <QPoint>
#include <QVector>
#include <QBrush>
#include <QTimer>

class GridScene;
class GridView;
class QLabel;
class QPushButton;
class QSlider;
class QCheckBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCellClicked(const QPoint& cell);
    void onDrawClicked();
    void onAnimateClicked();
    void onClearClicked();
    void onNewCurveClicked();
    void onUndoPointClicked();
    void onStepsChanged(int v);
    void stepAnimation();

private:
    void setStatus(const QString& s);
    void repaintAll();
    void drawControlPolygon();
    void drawBezierImmediate();
    void buildBezierSamplePoints();
    void bresenhamLine(const QPoint& a, const QPoint& b, const QBrush& brush);

    GridScene* scene = nullptr;
    GridView*  view  = nullptr;

    QVector<QPoint> controlPts;
    QVector<QPoint> bezierPts;
    int segmentCount = 100;

    QBrush ctrlBrush = QBrush(Qt::blue);
    QBrush polyBrush = QBrush(QColor(180,180,180));
    QBrush curveBrush = QBrush(QColor(128,0,128));

    QTimer animTimer;
    int animIndex = 0;

    QLabel* lblSteps = nullptr;
    QSlider* sldSteps = nullptr;
    QPushButton* btnDraw = nullptr;
    QPushButton* btnAnimate = nullptr;
    QPushButton* btnClear = nullptr;
    QPushButton* btnNew = nullptr;
    QPushButton* btnUndo = nullptr;
    QCheckBox* chkShowPoly = nullptr;
};
