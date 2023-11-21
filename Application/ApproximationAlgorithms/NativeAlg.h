#pragma once

#include "IApproxAlgs.h"

class NativeAlg : public IApproxAlgs
{
public:
    // Аппрроксимирует кривую до степени degreeApprox
    Curve approximateCurve(const Curve &curve, int degreeApprox) const override;

    // Возвращает кривую с пониженной степенью с заданной точностью (нативный алгоритм аппроксимации)
    Curve approximateCurveForEpsilon(const Curve &curve, double epsilon) const;
};
