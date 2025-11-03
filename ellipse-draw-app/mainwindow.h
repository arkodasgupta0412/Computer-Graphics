#pragma once
#include <QMainWindow>
#include <QPoint>
#include <QVector>
#include <QTimer>
#include <QLineEdit>
#include <QBrush>

class GridScene;
class GridView;
class QSlider;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCellClicked(const QPoint& cell);
    void onAxesChanged();
    void onASliderChanged(int value);
    void onBSliderChanged(int value);
    void drawEllipsePolar();
    void drawEllipseMidpoint();
    void compareExecutionTimes();
    void stepAnimation();

private:
    int  currentA() const;
    int  currentB() const;
    void setStatus(const QString& s);
    void beginAnimation(const QVector<QPoint>& frames, const QBrush& brush, int msStep);
    void drawEllipseImmediate(int a, int b);
    static QVector<QPoint> fourSymmetry(const QPoint& c, int x, int y);
    QVector<QPoint> buildPolarFrames(const QPoint& c, int a, int b);
    QVector<QPoint> buildMidpointFrames(const QPoint& c, int a, int b);

private:
    Ui::MainWindow *ui;

    GridScene* scene = nullptr;
    GridView*  view  = nullptr;

    QLineEdit* aInput = nullptr;
    QLineEdit* bInput = nullptr;
    QSlider*   aSlider = nullptr;
    QSlider*   bSlider = nullptr;

    bool   haveCenter = false;
    QPoint centerCell;

    QTimer animTimer;
    QVector<QPoint> animFrames;
    int   animIndex = 0;
    QBrush animBrush = Qt::blue;

    bool updatingAxisUI = false;
};
