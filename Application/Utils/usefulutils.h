#pragma once

#include "Curve3D/Curve.h"

class UsefulUtils
{
public:

    // Переводит вектор кривых Безье в одну кривую NURBS
    static Curve bezierCurvesToNURBSCurve(const std::vector<Curve> &bezierCurves, int degree, int curveNumPoints);

    // Проверка непрерывности всех точек в одной в кривой
    static void checkAllCurveBreaks(const Curve& curve);

    // Проверяет конкретную точки в кривой на непрерывность
    static void checkCurveBreakPoint(Curve& curve, double parametr);

    // Проверка непрерывности двух Безье кривых
    static void checkContinuityTwoCurves(const Curve& firstBezierCurve, const Curve& secondBezierCurve);

    // Разбивает NURBS кривую на кривые Безье
    static std::vector<Curve> splittingСurveIntoBezierCurves(const Curve& originalCurve);

    // Запись точек кривой в файл
    static void outNURBSPoints(const Curve& curve);
};


