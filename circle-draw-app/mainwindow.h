#pragma once
#include <QMainWindow>
#include <QPoint>
#include <QVector>
#include <QBrush>
#include <QTimer>

class GridScene;
class GridView;
class QLineEdit;
class QSlider;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCellClicked(const QPoint& cell);
    void onRadiusChanged();
    void onRadiusSliderChanged(int value);
    void drawCirclePolar();
    void drawCircleMidpoint();
    void drawCircleCartesian();
    void compareExecutionTimes();
    void stepAnimation();

private:
    void setStatus(const QString& s);
    int  currentRadius() const;

    QVector<QPoint> eightSymmetry(const QPoint& c, int x, int y);
    QVector<QPoint> buildPolarFrames(const QPoint& c, int r);
    QVector<QPoint> buildMidpointFrames(const QPoint& c, int r);
    QVector<QPoint> buildCartesianFrames(const QPoint& c, int r);

    void beginAnimation(const QVector<QPoint>& frames, const QBrush& brush, int msStep);
    void drawCircleImmediate(int r, const QBrush& brush);

    GridScene* scene{nullptr};
    GridView*  view{nullptr};

    QLineEdit* radiusInput{nullptr};
    QSlider*   radiusSlider{nullptr};

    QPoint centerCell{0,0};
    bool   haveCenter{false};

    QTimer animTimer;
    QVector<QPoint> animFrames;
    int animIndex{0};
    QBrush animBrush;

    bool updatingRadiusUI{false};
};
