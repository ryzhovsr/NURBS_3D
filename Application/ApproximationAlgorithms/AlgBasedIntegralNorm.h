#pragma once

#include "IApproxAlgs.h"

class AlgBasedIntegralNorm : public IApproxAlgs
{
public:
    // Аппроксимирует кривую и создаёт из неё кривую Безье с заданным количеством точек
    Curve approximateCurve(const Curve &curve, int degreeApprox) const override;

private:
    // Находит новые контрольные точки кривой для метода аппроксимации кривой
    std::vector<std::vector<double>> calcNewControlsPointsIntegralsMethod(const Curve &originalCurve, int newNumControlPoints, int newDegreeCurve) const;

    // Создаёт равномерный узловой вектор
    static std::vector<double> createUniformNodalVector(int numControlPoints, int degreeCurve);
};

