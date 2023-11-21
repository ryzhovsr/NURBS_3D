#pragma once

#include "Curve3d/Curve.h"

class MergeCurves
{
public:
    Curve attachCurves(const Curve &curve1, const Curve &curve2, bool fixateStartEndPoints);

    std::vector<Curve> attachBSplines(std::vector<Curve> &bezierCurves);
};

