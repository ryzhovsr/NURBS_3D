#pragma once

#include "Curve3D/Curve.h"

namespace UsefulUtils
{
    // Переводит вектор кривых Безье в одну кривую NURBS
    Curve bezierCurvesToNURBSCurve(const std::vector<Curve> &bezierCurves, int degree, int curveNumPoints);

    // Проверка непрерывности всех точек в одной в кривой
    void checkAllCurveBreaks(const Curve& curve);

    // Проверяет конкретную точки в кривой на непрерывность
    void checkCurveBreakPoint(Curve& curve, double parametr);

    // Проверка непрерывности двух Безье кривых
    void checkContinuityTwoCurves(const Curve& firstBezierCurve, const Curve& secondBezierCurve);

    // Разбивает NURBS кривую на кривые Безье
    std::vector<Curve> splittingСurveIntoBezierCurves(const Curve& originalCurve);

    // Запись точек кривой в файл
    void outNURBSPoints(const Curve& curve, const std::string& fileName);

    // Запись контрольных точек в файл
    void outControlPoints(const std::vector<Point3D>& controlPoints, const std::string& fileName);
};


