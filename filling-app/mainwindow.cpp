#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gridscene.h"
#include "gridview.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDebug>
#include <QSet>
#include <queue>
#include <stack>
#include <cmath>

// Constructor & destructor
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), scene(new GridScene(this)), view(new GridView(scene, this)), fillTimer(new QTimer(this))
{
    QWidget *central = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    // --- Graphics view ---
    layout->addWidget(view);

    // --- Controls ---
    QHBoxLayout *controls = new QHBoxLayout;

    // Color map
    colorMap["Black"]  = QBrush(Qt::black);
    colorMap["Red"]    = QBrush(Qt::red);
    colorMap["Blue"]   = QBrush(Qt::blue);
    colorMap["Green"]  = QBrush(Qt::green);
    colorMap["Yellow"] = QBrush(Qt::yellow);

    // Boundary color dropdown
    QLabel *boundaryLabel = new QLabel("Boundary Color:");
    boundaryColorCombo = new QComboBox;
    boundaryColorCombo->addItems(colorMap.keys());
    boundaryColorCombo->setCurrentText("Blue");  // Set to Blue

    // Fill color dropdown
    QLabel *fillLabel = new QLabel("Fill Color:");
    fillColorCombo = new QComboBox;
    fillColorCombo->addItems(colorMap.keys());
    fillColorCombo->setCurrentText("Green");  // Set to Green

    boundaryBrush = colorMap[boundaryColorCombo->currentText()];
    fillBrush = colorMap[fillColorCombo->currentText()];

    controls->addWidget(boundaryLabel);
    controls->addWidget(boundaryColorCombo);
    controls->addWidget(fillLabel);
    controls->addWidget(fillColorCombo);

    // Buttons
    drawLineBtn = new QPushButton("Draw Line");
    algCombo = new QComboBox;
    algCombo->addItems({"Flood Fill", "Boundary Fill", "Scanline Fill"});
    fillBtn = new QPushButton("Fill");
    QLabel *connLabel = new QLabel("Connectivity:");
    connCombo = new QComboBox;
    connCombo->addItems({"4-connected", "8-connected"});
    connCombo->setCurrentIndex(0);
    resetBtn = new QPushButton("Reset Fill");
    clearBtn = new QPushButton("Clear All");

    controls->addWidget(drawLineBtn);
    controls->addWidget(algCombo);
    controls->addWidget(fillBtn);
    controls->addWidget(resetBtn);
    controls->addWidget(clearBtn);
    controls->addWidget(connLabel);
    controls->addWidget(connCombo);

    layout->addLayout(controls);
    central->setLayout(layout);
    setCentralWidget(central);

    // --- Connections ---
    connect(boundaryColorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onBoundaryColorChanged);
    connect(fillColorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFillColorChanged);
    connect(drawLineBtn, &QPushButton::clicked, this, &MainWindow::onDrawLineClicked);

    connect(scene, &GridScene::leftClick, this, &MainWindow::handleLeftClick);
    connect(scene, &GridScene::rightClick, this, &MainWindow::handleRightClick);

    connect(algCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAlgorithmChanged);
    connect(fillBtn, &QPushButton::clicked, this, &MainWindow::onFillClicked);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::resetFill);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearScene);

    connect(fillTimer, &QTimer::timeout, this, &MainWindow::stepFillAnimation);

    resize(900, 700);

    fillTimer->setInterval(10);
    currentAlgorithm = Flood;
    haveSeed = false;
    fillAnimIndex = 0;

    qDebug() << "Initial boundary color:" << boundaryColorCombo->currentText();
    qDebug() << "Initial fill color:" << fillColorCombo->currentText();
}


MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onBoundaryColorChanged(int idx) {
    QString colorName = boundaryColorCombo->itemText(idx);
    boundaryBrush = colorMap[colorName];

    // Repaint already drawn polygon boundary
    for (const QPoint &pt : selectedPoints) {
        scene->paintCell(pt, boundaryBrush);
    }

    qDebug() << "Boundary color changed to" << colorName;
}

void MainWindow::onFillColorChanged(int idx) {
    QString colorName = fillColorCombo->itemText(idx);
    fillBrush = colorMap[colorName];
    qDebug() << "Fill color changed to" << colorName;
}

void MainWindow::handleLeftClick(const QPoint &cell) {
    // Left click → add boundary point
    addBoundaryPoint(cell);
}

void MainWindow::handleRightClick(const QPoint &cell) {
    seedPoint = cell;
    haveSeed = true;
    qDebug() << "Seed set at" << seedPoint;
}

void MainWindow::addBoundaryPoint(const QPoint &cell) {
    selectedPoints.append(cell);
    scene->paintCell(cell, boundaryBrush);
}

void MainWindow::startFloodFill() {
    if (seedPoint.isNull()) {
        qDebug() << "No seed set!";
        return;
    }
    fillQueue = floodFillPoints(seedPoint, isEightConnected(), boundaryBrush, fillBrush);
    if (!fillQueue.isEmpty()) {
        fillTimer->start(5);
    }
}

void MainWindow::startBoundaryFill() {
    if (seedPoint.isNull()) {
        qDebug() << "No seed set!";
        return;
    }
    fillQueue = boundaryFillPoints(seedPoint, isEightConnected(), boundaryBrush, fillBrush);
    if (!fillQueue.isEmpty()) {
        fillTimer->start(5);
    }
}

void MainWindow::startScanlineFill() {
    if (seedPoint.isNull()) {
        qDebug() << "No seed set!";
        return;
    }
    fillQueue = scanlineFillPoints(boundaryBrush, fillBrush);
    if (!fillQueue.isEmpty()) {
        fillTimer->start(5);
    }
}

void MainWindow::stepFill() {
    if (fillQueue.isEmpty()) {
        fillTimer->stop();
        return;
    }
    QPoint p = fillQueue.takeFirst();
    scene->paintCell(p, fillBrush);
}

void MainWindow::resetFill() {
    fillTimer->stop();
    fillQueue.clear();
    visited.clear();
    seedPoint = QPoint();
    scene->clearCellsWithBrushes({fillBrush});
}

void MainWindow::clearScene() {
    fillTimer->stop();
    fillQueue.clear();
    visited.clear();
    seedPoint = QPoint();
    selectedPoints.clear();
    scene->clearCells();
    qDebug() << "Scene cleared";
}


// UI style
void MainWindow::styleUi() {
    setStyleSheet("QMainWindow { background-color: #111; }"
                  "QLabel, QComboBox, QPushButton { color: #EEE; }"
                  "QPushButton { background-color: #333; border: 1px solid #555; padding: 4px; }"
                  "QPushButton:hover { background-color: #444; }"
                  "QComboBox { background-color: #222; border: 1px solid #444; padding: 2px; }");
}

// Add point
void MainWindow::onCellClicked(const QPoint& cell) {
    selectedPoints.append(cell);
    scene->paintCell(cell, boundaryBrush);
}

// Set seed
void MainWindow::onSeedSelected(const QPoint& cell) {
    seedPoint = cell;
    haveSeed = true;
    scene->paintCell(cell, seedBrush);
}

// Algorithm selector
void MainWindow::onAlgorithmChanged(int idx) {
    if (idx == 0) {
        currentAlgorithm = Flood;
        qDebug() << "Algorithm set to Flood Fill";
    } else if (idx == 1) {
        currentAlgorithm = Boundary;
        qDebug() << "Algorithm set to Boundary Fill";
    } else if (idx == 2) {
        currentAlgorithm = Scanline;
        qDebug() << "Algorithm set to Scanline Fill";
    }
}

// Connectivity changed
void MainWindow::onConnectivityChanged(int idx) { Q_UNUSED(idx); }

// Connectivity getter
bool MainWindow::isEightConnected() const {
    return connCombo->currentIndex() == 1;
}

// Bresenham line
QVector<QPoint> MainWindow::computeBresenhamLine(const QPoint& p1, const QPoint& p2) const {
    QVector<QPoint> points;
    int x1 = p1.x(), y1 = p1.y();
    int x2 = p2.x(), y2 = p2.y();
    int dx = qAbs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = qAbs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    int x = x1, y = y1;
    while (true) {
        points.append(QPoint(x, y));
        if (x == x2 && y == y2) break;
        int e2 = err * 2;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx)  { err += dx; y += sy; }
    }
    return points;
}

// Draw latest Bresenham between last two selected points
void MainWindow::onDrawLineClicked() {
    if (selectedPoints.size() < 2) return;
    QPoint p1 = selectedPoints[selectedPoints.size()-2];
    QPoint p2 = selectedPoints[selectedPoints.size()-1];
    auto pts = computeBresenhamLine(p1, p2);
    for (auto &pt : pts) {
        scene->paintCell(pt, boundaryBrush);
        selectedPoints.append(pt);
    }
    qDebug() << "Drew line: " << pts.size() << " pixels";
}

// Helper: compute bounding rectangle in cell coordinates
static void computeBoundingBox(const QVector<QPoint>& pts, const QRectF& sceneRect,
                               int &minX, int &maxX, int &minY, int &maxY, int cellSize = 5) {
    if (!pts.isEmpty()) {
        minX = pts[0].x(); maxX = pts[0].x();
        minY = pts[0].y(); maxY = pts[0].y();
        for (const QPoint &p : pts) {
            minX = qMin(minX, p.x()); maxX = qMax(maxX, p.x());
            minY = qMin(minY, p.y()); maxY = qMax(maxY, p.y());
        }
        // Add some padding
        minX -= 2; maxX += 2;
        minY -= 2; maxY += 2;
    } else {
        // derive from scene rect
        minX = std::floor(sceneRect.left() / cellSize) - 2;
        maxX = std::ceil(sceneRect.right() / cellSize) + 2;
        minY = std::floor(sceneRect.top() / cellSize) - 2;
        maxY = std::ceil(sceneRect.bottom() / cellSize) + 2;
    }
}

// Flood fill (BFS)
QVector<QPoint> MainWindow::floodFillPoints(const QPoint& seed, bool eightConnected, const QBrush& boundaryBrush, const QBrush& fillBrush) {
    QVector<QPoint> filled;
    if (!scene) return filled;

    const int CELL_SIZE = 5;
    int minX, maxX, minY, maxY;
    computeBoundingBox(selectedPoints, scene->sceneRect(), minX, maxX, minY, maxY, CELL_SIZE);

    std::queue<QPoint> q;
    QSet<QPoint> visited;

    // Check if seed is valid
    if (seed.x() < minX || seed.x() > maxX || seed.y() < minY || seed.y() > maxY) {
        qDebug() << "Seed out of bounds";
        return filled;
    }

    // Get the starting color at seed point
    QBrush startBrush = scene->getCellBrush(seed);

    // If seed is already filled with target color, don't proceed
    if (scene->isCellPaintedWith(seed, fillBrush)) {
        qDebug() << "Seed is already filled";
        return filled;
    }

    // If seed color matches boundary color, don't proceed
    if (startBrush == boundaryBrush) {
        qDebug() << "Seed is on boundary, cannot fill";
        return filled;
    }

    q.push(seed);
    visited.insert(seed);

    while (!q.empty()) {
        QPoint p = q.front();
        q.pop();
        filled.append(p);

        QVector<QPoint> neighbors;
        neighbors << QPoint(p.x()+1, p.y()) << QPoint(p.x()-1, p.y())
                  << QPoint(p.x(), p.y()+1) << QPoint(p.x(), p.y()-1);

        if (eightConnected) {
            neighbors << QPoint(p.x()+1, p.y()+1) << QPoint(p.x()-1, p.y()-1)
            << QPoint(p.x()+1, p.y()-1) << QPoint(p.x()-1, p.y()+1);
        }

        for (const QPoint &n : neighbors) {
            if (n.x() < minX || n.x() > maxX || n.y() < minY || n.y() > maxY)
                continue;

            if (visited.contains(n))
                continue;

            // FLOOD FILL KEY DIFFERENCE: Stop at ANY color different from start color
            QBrush neighborBrush = scene->getCellBrush(n);
            if (neighborBrush != startBrush)  // Different from original area color
                continue;

            // Also don't cross already filled areas
            if (scene->isCellPaintedWith(n, fillBrush))
                continue;

            visited.insert(n);
            q.push(n);
        }
    }

    qDebug() << "Flood fill points computed:" << filled.size();
    return filled;
}

// Boundary fill (queue)
QVector<QPoint> MainWindow::boundaryFillPoints(const QPoint& seed, bool eightConnected, const QBrush& boundaryBrush, const QBrush& fillBrush) {
    QVector<QPoint> filled;
    if (!scene) return filled;

    const int CELL_SIZE = 5;
    int minX, maxX, minY, maxY;
    computeBoundingBox(selectedPoints, scene->sceneRect(), minX, maxX, minY, maxY, CELL_SIZE);

    std::queue<QPoint> q;
    QSet<QPoint> visited;

    // Check if seed is valid
    if (seed.x() < minX || seed.x() > maxX || seed.y() < minY || seed.y() > maxY) {
        qDebug() << "Seed out of bounds";
        return filled;
    }

    // If seed is on boundary, don't proceed
    if (scene->isCellPaintedWith(seed, boundaryBrush)) {
        qDebug() << "Seed is on boundary, cannot fill";
        return filled;
    }

    // If seed is already filled, don't proceed
    if (scene->isCellPaintedWith(seed, fillBrush)) {
        qDebug() << "Seed is already filled";
        return filled;
    }

    q.push(seed);
    visited.insert(seed);

    while (!q.empty()) {
        QPoint p = q.front();
        q.pop();

        // Skip boundary cells
        if (scene->isCellPaintedWith(p, boundaryBrush))
            continue;

        // Skip already filled cells
        if (scene->isCellPaintedWith(p, fillBrush))
            continue;

        filled.append(p);

        QVector<QPoint> neighbors;
        neighbors << QPoint(p.x()+1, p.y()) << QPoint(p.x()-1, p.y()) << QPoint(p.x(), p.y()+1) << QPoint(p.x(), p.y()-1);

        if (eightConnected) {
            neighbors << QPoint(p.x()+1, p.y()+1) << QPoint(p.x()-1, p.y()-1) << QPoint(p.x()+1, p.y()-1) << QPoint(p.x()-1, p.y()+1);
        }

        for (const QPoint &n : neighbors) {
            if (n.x() < minX || n.x() > maxX || n.y() < minY || n.y() > maxY)
                continue;

            if (!visited.contains(n) &&
                !scene->isCellPaintedWith(n, boundaryBrush) &&
                !scene->isCellPaintedWith(n, fillBrush)) {
                visited.insert(n);
                q.push(n);
            }
        }
    }

    qDebug() << "Boundary fill points computed:" << filled.size();
    return filled;
}


// Scanline Fill
QVector<QPoint> MainWindow::scanlineFillPoints(const QBrush& boundaryBrush, const QBrush& fillBrush) {
    QVector<QPoint> filled;
    if (!scene || selectedPoints.isEmpty()) {
        qDebug() << "No boundary points to fill";
        return filled;
    }

    QVector<QPoint> vertices;

    if (selectedPoints.size() < 3) {
        qDebug() << "Not enough points";
        return filled;
    }

    // Add first point
    vertices.append(selectedPoints[0]);

    for (int i = 1; i < selectedPoints.size() - 1; i++) {
        QPoint prev = selectedPoints[i - 1];
        QPoint curr = selectedPoints[i];
        QPoint next = selectedPoints[i + 1];

        // Calculate direction vectors
        int dx1 = curr.x() - prev.x();
        int dy1 = curr.y() - prev.y();
        int dx2 = next.x() - curr.x();
        int dy2 = next.y() - curr.y();

        int cross = dx1 * dy2 - dy1 * dx2;

        if (cross != 0) {
            vertices.append(curr);
        }
    }

    if (selectedPoints.last() != selectedPoints.first()) {
        vertices.append(selectedPoints.last());
    }

    int n = vertices.size();
    qDebug() << "Extracted" << n << "vertices from" << selectedPoints.size() << "boundary points";

    if (n < 3) {
        qDebug() << "Need at least 3 vertices for polygon";
        return filled;
    }

    // Find ymin and ymax
    int ymin = vertices[0].y();
    int ymax = vertices[0].y();

    for (const QPoint& pt : vertices) {
        ymin = std::min(ymin, pt.y());
        ymax = std::max(ymax, pt.y());
    }

    qDebug() << "ymin=" << ymin << "ymax=" << ymax;

    // Build Edge Table from vertices
    QVector<QVector<Edge>> edgeTable(ymax - ymin + 1);

    for (int i = 0; i < n; i++) {
        QPoint p1 = vertices[i];
        QPoint p2 = vertices[(i + 1) % n];

        // Skip horizontal edges
        if (p1.y() == p2.y()) {
            continue;
        }

        // Ensure p1 is the lower point
        if (p1.y() > p2.y()) {
            std::swap(p1, p2);
        }

        Edge edge;
        edge.ymax = p2.y();
        edge.xofymin = p1.x();
        edge.slopeinverse = float(p2.x() - p1.x()) / float(p2.y() - p1.y());

        int index = p1.y() - ymin;
        if (index >= 0 && index < edgeTable.size()) {
            edgeTable[index].push_back(edge);
        }
    }

    // Active Edge Table
    QVector<Edge> aet;
    QSet<QPoint> visitedFill;

    // Process each scanline
    for (int y = ymin; y <= ymax; y++) {
        int tableIdx = y - ymin;

        // Add new edges from ET to AET
        if (tableIdx >= 0 && tableIdx < edgeTable.size()) {
            for (const Edge& e : edgeTable[tableIdx]) {
                aet.push_back(e);
            }
        }

        // Remove edges where ymax == y
        aet.erase(std::remove_if(aet.begin(), aet.end(), [y](const Edge& e) { return e.ymax == y; }), aet.end());

        // Sort AET by x coordinate
        std::sort(aet.begin(), aet.end(), [](const Edge& a, const Edge& b) { return a.xofymin < b.xofymin; });

        // Fill between pairs
        for (int i = 0; i + 1 < aet.size(); i += 2) {
            int x1 = std::round(aet[i].xofymin);
            int x2 = std::round(aet[i + 1].xofymin);

            if (x1 > x2) std::swap(x1, x2);

            // Fill interior pixels
            for (int x = x1; x <= x2; x++) {
                QPoint pt(x, y);

                // Skip boundary pixels
                bool isBoundary = scene->isCellPaintedWith(pt, boundaryBrush);

                // Skip axis pixels
                // bool isAxis = (pt.x() == 0 || pt.y() == 0);

                // Skip already filled
                bool alreadyFilled = scene->isCellPaintedWith(pt, fillBrush);

                if (!isBoundary && !alreadyFilled && !visitedFill.contains(pt)) {
                    filled.append(pt);
                    visitedFill.insert(pt);
                }
            }
        }

        // Update x for next scanline
        for (Edge& e : aet) {
            e.xofymin += e.slopeinverse;
        }
    }

    qDebug() << "Scanline fill points computed:" << filled.size();
    return filled;
}


// On Fill button
void MainWindow::onFillClicked() {
    if (!haveSeed && currentAlgorithm != Scanline) {
        qDebug() << "Select a seed (right-click) before filling.";
        return;
    }

    bool eight = isEightConnected();
    fillQueue.clear();

    qDebug() << "Starting fill with algorithm:" << currentAlgorithm << "connectivity:" << (eight ? "8" : "4");

    switch (currentAlgorithm) {
    case Flood:
        fillQueue = floodFillPoints(seedPoint, eight, boundaryBrush, fillBrush);
        break;
    case Boundary:
        fillQueue = boundaryFillPoints(seedPoint, eight, boundaryBrush, fillBrush);
        break;
    case Scanline:
        fillQueue = scanlineFillPoints(boundaryBrush, fillBrush);
        break;
    }

    qDebug() << "Fill points computed:" << fillQueue.size();

    fillAnimIndex = 0;
    if (!fillQueue.isEmpty()) {
        fillTimer->start(5);
        qDebug() << "Animation started with" << fillQueue.size() << "points";
    } else {
        qDebug() << "No pixels to fill";
    }
}

// Paint one pixel per tick
void MainWindow::stepFillAnimation() {
    if (fillAnimIndex >= fillQueue.size()) {
        fillTimer->stop();
        qDebug() << "Filling completed. Total painted:" << fillQueue.size();
        return;
    }

    scene->paintCell(fillQueue[fillAnimIndex], fillBrush);
    fillAnimIndex++;

    // Update every 10 pixels for better performance
    if (fillAnimIndex % 10 == 0) {
        view->viewport()->update();
    }
}

// Clear everything
void MainWindow::onClearClicked() {
    selectedPoints.clear();
    seedPoint = QPoint();
    haveSeed = false;
    fillQueue.clear();
    fillTimer->stop();
    scene->clearCells();
    qDebug() << "Cleared scene.";
}

// Reset only fill & seed colors (keep polygon)
void MainWindow::onResetFillClicked() {
    seedPoint = QPoint();
    haveSeed = false;
    fillQueue.clear();
    fillTimer->stop();
    scene->clearCellsWithBrushes({ fillBrush, seedBrush });
    qDebug() << "Reset fill/seed colors.";
}

// Helper: check if scene has a cell painted with given brush
bool MainWindow::isCellPaintedWith(const QPoint& cell, const QBrush& brush) const {
    return scene->isCellPaintedWith(cell, brush);
}
