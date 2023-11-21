#pragma once

#include "Curve3D/Curve.h"

class IApproxAlgs
{
public:
    virtual Curve approximateCurve(const Curve &curve, int degreeApprox) const = 0; // Аппрроксимирует кривую до степени degreeApprox
    virtual ~IApproxAlgs() {}

    void checkDegreeApprox(const Curve &curve, int degreeApprox) const;   // Проверяет, что степень исходной кривой < степени аппроксимирующей кривой
};

