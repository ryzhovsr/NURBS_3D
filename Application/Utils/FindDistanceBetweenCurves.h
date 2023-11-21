#pragma once

#include "Curve3d/Curve.h"

class FindDistanceBetweenCurves
{
public:
    // Возвращает максимальное расстояние между кривыми
    static double findMaxLenBetweenCurves(const Curve &curve1, const Curve &curve2);

    // Возвращает самые дальние точки двух кривых (для построения расстояния между ними)
    static std::pair<CurvePoint, CurvePoint> findFarthestPointsNURBS(const Curve &curve1, const Curve &curve2);

private:
    // Возвращает косинус между вектором производной в точке кривой (pointNURBS) и вектором, начинающимся от точки кривой и заканчивающимся в (point)
    static double cosBetweenVectors(const CurvePoint &pointNURBS, const Point3D &point);

    // Возвращает точку кривой с наименьшим/наибольшим расстоянием до точки (startingPoint) (Нормаль)
    static CurvePoint findPointNURBS(const Curve &curve, const Point3D &startingPoint, bool nearest);
    static CurvePoint findPointNURBS(const Curve &curve, double x, double y, double z, bool nearest);

    // Возвращает вектор с параметрами спанов реального узлового вектора
    static std::vector<double> calcPointsRealSpan(const std::vector<double> &nodalVector, int degree);

    // Сравнивает массив точек и возвращает самую ближайшую/дальнюю к исходной точке (startingPoint)
    static CurvePoint comparePoints(const std::vector<CurvePoint> &points, const std::vector<double> &cosines, const Point3D &startingPoint, bool nearest);
};
