#pragma once

#include "Curve3D/Curve.h"

class IApproxAlgs
{
public:
    // Аппрроксимирует кривую до степени degreeApprox
    virtual Curve approximateCurve(const Curve &curve, int degreeApprox) const = 0;
    virtual ~IApproxAlgs() {}

    // Проверяет, что степень исходной кривой < степени аппроксимирующей кривой
    void checkDegreeApprox(const Curve &curve, int degreeApprox) const;
};

