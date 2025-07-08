#include "Metrics.h"
#include "Utils/MathUtils.h"
#include <QDebug>
#include "Utils/FindDistanceBetweenCurves.h"

double Metrics::calcCurveCurvature(const std::vector<CurvePoint> &curvePoints)
{
    double sum = 0;

    for (int i = 0; i != curvePoints.size(); ++i)
    {
        const std::vector<Point3D>& derivs = curvePoints[i].derivs;
        double numerator = derivs[1].cross(derivs[2]).magnitude();
        double denomenator = std::pow(derivs[1].magnitude(), 3);
        sum += numerator / denomenator * curvePoints[i].parameter;
    }

    return sum;
}

double Metrics::calcCurveCurvature(const Curve &curve)
{
    return calcCurveCurvature(curve.getCurvePoints());
}

double Metrics::calcNewCurveCurvature(const Curve &curve)
{
    double curvature = 0;

    // Рассчитывает определитель 2 на 2
    auto calcDelim = [](double leftUpper, double rightUpper, double leftDown, double rightDown) -> double
    {
        return leftUpper * leftDown - rightUpper * rightDown;
    };

    for (const auto& point: curve.getCurvePoints())
    {
        auto firstDeriv = point.derivs[1];
        auto secondDeriv = point.derivs[2];

        double firstDelimeter = calcDelim(firstDeriv.y, firstDeriv.z, secondDeriv.y, secondDeriv.z);
        double secondDelimeter = calcDelim(firstDeriv.z, firstDeriv.x, secondDeriv.z, secondDeriv.x);
        double thirdDelimeter = calcDelim(firstDeriv.x, firstDeriv.y, secondDeriv.x, secondDeriv.y);

        // Значения в квадрате
        double firstDelimiterSq = firstDelimeter * firstDelimeter;
        double secondDelimeterSq = secondDelimeter * secondDelimeter;
        double thirdDelimeterSq = thirdDelimeter * thirdDelimeter;

        double numerator = sqrt(firstDelimiterSq + secondDelimeterSq + thirdDelimeterSq);

        // Вектор в знаминателе
        double vecDenominator = sqrt(firstDeriv.x * firstDeriv.x + firstDeriv.y * firstDeriv.x + firstDeriv.z * firstDeriv.x);
        double denominator = vecDenominator * vecDenominator * vecDenominator;

        curvature += numerator / denominator;
    }

    return curvature;
}

double Metrics::calcTorsion(const Curve& iCurve)
{
    double sum = 0;
    const auto& curvePoints = iCurve.getCurvePoints();

    for (int i = 0; i != curvePoints.size(); ++i)
    {
        const std::vector<Point3D>& derivs = curvePoints[i].derivs;
        double numerator = derivs[1].cross(derivs[2]).dot(derivs[3]);
        double denomenator = std::pow((derivs[1].cross(derivs[2])).magnitude(), 2);
        sum += numerator / denomenator * curvePoints[i].parameter;
    }

    return sum;
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
