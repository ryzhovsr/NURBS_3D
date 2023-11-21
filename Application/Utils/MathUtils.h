#pragma once

#include "Curve3d/Curve.h"
#include <vector>
#include "Curve3d/Point3d.h"

class MathUtils
{
public:
    // Возвращет длину радиус-вектора
    static double calcRadiusVectorLength(double x, double y, double z);
    static double calcRadiusVectorLength(const Point3D &point);

    // Возвращет длину вектора
    static double calcVectorLenght(double x1, double y1, double z1, double x2, double y2, double z2);
    static double calcVectorLenght(const Point3D &point1, const Point3D &point2);
    static double calcVectorLenght(const CurvePoint &point1, const Point3D &point2);
    static double calcVectorLenght(const Point3D &point1, double x2, double y2, double z2);

    // Возвращает угол между двумя векторами
    static double calcAngleBetweenVectors(const Point3D &vecStart1, const Point3D &vecEnd1, const Point3D &vecStart2, const Point3D &vecEnd2);

    // Рассчитывает сочетания без повторений из n по k
    static int calcCombWithoutRepetition(int n, int k);

    // Возвращает пару векторов с точками кривой, отдалённых от исходной кривой на длину (len) сверху и снизу
    static std::pair<std::vector<CurvePoint>, std::vector<CurvePoint>> moveCurve(const Curve &curve, double length);

private:
    MathUtils();
};
