#include "Metrics.h"
#include "Utils/MathUtils.h"
#include <QDebug>
#include "Utils/FindDistanceBetweenCurves.h"

double Metrics::calcCurveCurvature(const std::vector<CurvePoint> &curvePoints)
{
    Point3D secondDeriv;
    double square = 0, height = 0;  // Общая площадь и высота прямоугольника соответственно
    double split = curvePoints[1].parameter; // Основнаие прямоугольника

    for (int i = 0; i < curvePoints.size() - 1; ++i)
    {
        secondDeriv.x = (curvePoints[i].derivs[2].x * curvePoints[i].derivs[2].x);
        secondDeriv.y = (curvePoints[i].derivs[2].y * curvePoints[i].derivs[2].y);
        secondDeriv.z = (curvePoints[i].derivs[2].z * curvePoints[i].derivs[2].z);
        height = sqrt(secondDeriv.x * secondDeriv.x + secondDeriv.y * secondDeriv.y + secondDeriv.z * secondDeriv.z);
        square += height * split;
    }

    return square;
}

double Metrics::calcCurveCurvature(const Curve &curve)
{
    return calcCurveCurvature(curve.getCurvePoints());
}

double Metrics::calcHausdorffMetric(const Curve &Curve1, const Curve &Curve2)
{
    double distanceFromFirstCurve = FindDistanceBetweenCurves::findMaxLenBetweenCurves(Curve1, Curve2);    // Расстояние от первой кривой ко второй кривой
    double distanceFromSecondCurve = FindDistanceBetweenCurves::findMaxLenBetweenCurves(Curve2, Curve1);   // Расстояние от второй кривой к первой кривой

    return distanceFromFirstCurve > distanceFromSecondCurve ? distanceFromFirstCurve : distanceFromSecondCurve;
}

double Metrics::calcQuadraticDifference(const Curve &Curve1, const Curve &Curve2)
{
    double x = 0, y = 0;
    std::vector<CurvePoint> curvePoints1 = Curve1.getCurvePoints();
    std::vector<CurvePoint> curvePoints2 = Curve2.getCurvePoints();

    if (curvePoints1.size() != curvePoints2.size())
        qDebug() << "Error! calcQuadraticDifference: curvePoints1.size() != curvePoints2.size()!";

    for (size_t i = 0; i < curvePoints1.size(); ++i)
    {
        x += pow((curvePoints1[i].x - curvePoints2[i].x), 2);
        y += pow((curvePoints1[i].y - curvePoints2[i].y), 2);
    }

    return sqrt(x * x + y * y);
}

std::pair<CurvePoint, CurvePoint> Metrics::calcPointsHausdorffMetric(const Curve &Curve1, const Curve &Curve2)
{
    std::pair<CurvePoint, CurvePoint> farthestPoints_1 = FindDistanceBetweenCurves::findFarthestPointsNURBS(Curve1, Curve2);
    std::pair<CurvePoint, CurvePoint> farthestPoints_2 = FindDistanceBetweenCurves::findFarthestPointsNURBS(Curve2, Curve1);

    double dist_1 = MathUtils::calcVectorLenght(farthestPoints_1.first.x, farthestPoints_1.first.y, farthestPoints_1.first.z, farthestPoints_1.second.x, farthestPoints_1.second.y, farthestPoints_1.second.z);
    double dist_2 = MathUtils::calcVectorLenght(farthestPoints_2.first.x, farthestPoints_2.first.y, farthestPoints_2.first.z, farthestPoints_2.second.x, farthestPoints_2.second.y, farthestPoints_2.second.z);

    return dist_1 > dist_2 ? farthestPoints_1 : farthestPoints_2;
}
