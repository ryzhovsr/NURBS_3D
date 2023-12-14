#pragma once

#include "Curve3d/Curve.h"

class MergeCurves
{
public:
    // Соединяет две кривые Безье
    Curve attachTwoBezierCurves(const Curve &curve1, const Curve &curve2, bool fixateStartEndPoints);

    // Соединяет все кривые Безье в векторе
    std::vector<Curve> attachAllBezierCurves(const Curve &bezierCurves);
};

