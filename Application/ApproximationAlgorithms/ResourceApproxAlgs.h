#pragma once

#include "IApproxAlgs.h"

// Класс реализован по паттерну "Стратегия"
class ResourceApproxAlgs
{
public:
    ResourceApproxAlgs(IApproxAlgs* approxAlgorithm);
    ~ResourceApproxAlgs();

    // Установить стратегию аппроксимации
    void setStrategy(IApproxAlgs* approxAlgorithm);

    Curve approximateCurve(const Curve &curve, int degreeApprox);

private:
    IApproxAlgs* _approxAlgorithm;
};

