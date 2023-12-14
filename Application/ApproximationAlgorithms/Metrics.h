#pragma once

#include "Curve3d/Curve.h"

class Metrics
{
public:
    Metrics();

    // Рассчитывает кривизну кривой. Кривизна рассчитывается методом правых прямоугольников
    static double calcCurveCurvature(const std::vector<CurvePoint> &curvePoints);
    static double calcCurveCurvature(const Curve &curve);

    // Рассчитывает метрику Хаусдорфа между двух кривых
    static double calcHausdorffMetric(const Curve &Curve1, const Curve &Curve2);

    // Рассчитывает квадратичную разность между двух кривых
    static double calcQuadraticDifference(const Curve &Curve1, const Curve &Curve2);

    // Рассчитывает точки двух кривых метрики Хаусдорфа
    static std::pair<CurvePoint, CurvePoint> calcPointsHausdorffMetric(const Curve &Curve1, const Curve &Curve2);
};
