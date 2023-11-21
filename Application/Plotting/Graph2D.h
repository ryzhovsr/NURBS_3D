#pragma once

#include "qcustomplot.h"
#include "Curve3d/Curve.h"

class Graph2D
{
public:
    Graph2D(QCustomPlot *canvas, const QString &title = "") noexcept;

    // Рисует определяющий многоугольник кривой
    void drawDefiningPolygon(const std::vector<Point3D> &controlPoints, const QString &name = "", const QColor &color = QColor(0, 0, 0), const Qt::PenStyle &penStyle = Qt::SolidLine, double width = 1) const noexcept;

    // Рисует кривую
    void drawCurve(const Curve &curve, const QString &name = "", const QColor &color = QColor(0, 0, 0), const Qt::PenStyle &penStyle = Qt::SolidLine, double width = 3) const noexcept;
    void drawCurve(const std::vector<CurvePoint> &curvePoints, const QString  &name = "", const QColor &color = QColor(0, 0, 0), const Qt::PenStyle &penStyle = Qt::SolidLine, double width = 3) const noexcept;

    // Рисует две кривые
    void drawTwoCurves(const std::pair<std::vector<CurvePoint>, std::vector<CurvePoint>> &twoCurves) const noexcept;

    // Рисует годограф любой производной кривой
    void drawHodographDeriv(const Curve &curve, int derivativeOrder, const QString &name = "", const QColor &color = QColor(0, 0, 0), const Qt::PenStyle &penStyle = Qt::SolidLine, double width = 1.5) const noexcept;

    // Рисует точку
    void drawPoint(Point3D point, double width = 5, const QColor &color = QColor(0, 0, 0)) const noexcept;

    // Рисует линию
    void drawLine(QPointF point1, QPointF point2, const QColor &color = QColor(0, 0, 0), const Qt::PenStyle &penStyle = Qt::SolidLine, double width = 3.5) const noexcept;
    void drawLine(double x1, double y1, double x2, double y2, const QColor &color = QColor(0, 0, 0), const Qt::PenStyle &penStyle = Qt::SolidLine, double width = 3.5) const noexcept;

    // Рисует надпись
    void drawLable(QPointF point, const QString &label, double fontSize = 10) const noexcept;

    // Рисует стрелку "--->"
    void drawArrow(QPointF point1, QPointF point2) const noexcept;

    // Рисует двойную стрелку "<--->"
    void drawDoubleArrow(QPointF point1, QPointF point2) const noexcept;

    // Рисует касательную стрелку "--->" к кривой (вектор первой производной в точке)
    void drawTangentArrowToCurve(const CurvePoint &curvePoint) const noexcept;

    // Рисует касательную к кривой
    void drawTangentToCurve(const CurvePoint &curvePoint, const QColor &color = QColor(0, 0, 0), double width = 2.8) const noexcept;

    // Рисует перпендикуляры между кривыми
    void drawNormalsBetweenCurves(const Curve &curve1, const Curve &curve2, const QColor &color = QColor(0, 0, 0), int step = 4) const noexcept;

private:
    QCustomPlot *_canvas; // Указатель на окно для рисования
    QString _title;
};
