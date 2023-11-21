#pragma once

#include "IApproxAlgs.h"

// Класс реализован по паттерну "Стратегия"
class ResourceApproxAlgs
{
public:
    ResourceApproxAlgs(IApproxAlgs* approxAlgorithm);
    ~ResourceApproxAlgs();

    void setStrategy(IApproxAlgs* approxAlgorithm); // Установить стратегию аппроксимации

    Curve approximateCurve(const Curve &curve, int degreeApprox);

private:
    IApproxAlgs* _approxAlgorithm;
};

