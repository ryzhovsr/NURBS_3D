#pragma once

#include "Curve3d/Curve.h"

class MergeCurves
{
public:
    Curve attachCurves(const Curve &curve1, const Curve &curve2, bool fixateStartEndPoints);

    std::vector<Curve> attachBSplines(std::vector<Curve> &bezierCurves);

private:
    // Рекурсивные методы рассчёта производных, предназначенных для дальнейшего соединения кривых Безье
    Point3D calcDerivLeftBezierCurveForMerger(const std::vector<Point3D> &points, int currentIndex, int startIndex);
    Point3D calcNegativeDerivLeftBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex); // Возвращает отрицательную производную для левой кривой
    Point3D calcDerivRightBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex);

};

