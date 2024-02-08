#include "UsefulUtils.h"
#include "Curve3D/CalcCurve.h"
#include "Utils/MathUtils.h"
#include <fstream>

// Переводит вектор кривых Безье в одну кривую NURBS
Curve UsefulUtils::bezierCurvesToNURBSCurve(const std::vector<Curve> &bezierCurves, int degree, int curveNumPoints)
{
    std::vector<Point3D> newControlPoints;
    // Для того, чтобы не было повторяющихся точек
    bool firstCheck = false;

    for (size_t curveCount = 0; curveCount != bezierCurves.size(); ++curveCount)
    {
        std::vector<Point3D> tempControlPoints = bezierCurves[curveCount].getControlPoints();

        for (size_t i = 0; i != tempControlPoints.size(); ++i)
        {
            if (firstCheck && i == 0)
            {
                continue;
            }

            if (curveCount == 0)
            {
                firstCheck = true;
            }

            newControlPoints.push_back(tempControlPoints[i]);
        }
    }

    return Curve(newControlPoints, std::vector<double> (newControlPoints.size(), 1), degree, curveNumPoints);
}

// Проверка непрерывности всех точек в одной в кривой
void UsefulUtils::checkAllCurveBreaks(const Curve& curve)
{
    std::vector<CurvePoint> curvePoint = curve.getCurvePoints();

    for (size_t i = 0; i < curvePoint.size(); ++i)
    {
        if (i == 0 || i == curvePoint.size() - 1)
            continue;

        CurvePoint leftPoint, rightPoint;

        double leftParameter = curvePoint[i].parameter - 0.0000000001;
        double rightParameter = curvePoint[i].parameter + 0.0000000001;

        CalcCurve::calcCurvePointAndDerivs(curve, leftPoint, leftParameter);
        CalcCurve::calcCurvePointAndDerivs(curve, rightPoint, rightParameter);

        for (int j = 0; j < curve.getCurvePoints()[0].derivs.size(); ++j)
        {
            double leftLength = sqrt(leftPoint.derivs[j].x * leftPoint.derivs[j].x + leftPoint.derivs[j].y * leftPoint.derivs[j].y + leftPoint.derivs[j].z * leftPoint.derivs[j].z);
            double rightLength = sqrt(rightPoint.derivs[j].x * rightPoint.derivs[j].x + rightPoint.derivs[j].y * rightPoint.derivs[j].y + rightPoint.derivs[j].z * rightPoint.derivs[j].z);
            double diff = abs(leftLength - rightLength);

            if (diff > 0.01)
            {
                qDebug() << "-----------------Разрыв!!------------------" << j << "-й производной, в точке " << curvePoint[i].parameter << " и diff = " << diff;
            }
        }
    }
}

// Проверяет конкретную точки в кривой на непрерывность
void UsefulUtils::checkCurveBreakPoint(Curve& curve, double parametr)
{
    CurvePoint leftPoint, rightPoint;
    double leftParameter = parametr - 1e-15;
    double rightParameter =  parametr + 1e-15;

    CalcCurve::calcCurvePointAndDerivs(curve, leftPoint, leftParameter);
    CalcCurve::calcCurvePointAndDerivs(curve, rightPoint, rightParameter);

    for (size_t i = 0; i != leftPoint.derivs.size(); ++i)
    {
        double leftLength = MathUtils::calcRadiusVectorLength(leftPoint.derivs[i]);
        double rightLength = MathUtils::calcRadiusVectorLength(rightPoint.derivs[i]);
        double diff = abs(leftLength - rightLength);

        qDebug() << "Левая кривая в " << parametr << " производная " << i << ":\t" << leftPoint.derivs[i].x << "\t" << leftPoint.derivs[i].y << "\t" << leftPoint.derivs[i].z;
        qDebug() << "Права кривая в " << parametr << " производная " << i << ":\t" << rightPoint.derivs[i].x << "\t" << rightPoint.derivs[i].y << "\t" << rightPoint.derivs[i].z;
        qDebug() << "Difference: " << diff;;

        if (diff > 0.01)
        {
            qDebug() << "-----------------Разрыв!!------------------" << i << "-й производной, в точке " << parametr << " и diff = " << diff;
        }

        qDebug() << "";
    }
}

// Проверка непрерывности двух Безье кривых
void UsefulUtils::checkContinuityTwoCurves(const Curve& firstBezierCurve, const Curve& secondBezierCurve)
{
    CurvePoint leftPoint, rightPoint;

    double leftParameter = 1 - 0.000001;
    double rightParameter =  0 + 0.000001;

    CalcCurve::calcCurvePointAndDerivs(firstBezierCurve, leftPoint, leftParameter);
    CalcCurve::calcCurvePointAndDerivs(secondBezierCurve, rightPoint, rightParameter);

    bool check = true;

    for (size_t i = 0; i < firstBezierCurve.getCurvePoints()[0].derivs.size(); ++i)
    {
        double leftLength = sqrt(leftPoint.derivs[i].x * leftPoint.derivs[i].x + leftPoint.derivs[i].y * leftPoint.derivs[i].y + leftPoint.derivs[i].z * leftPoint.derivs[i].z);
        double rightLength = sqrt(rightPoint.derivs[i].x * rightPoint.derivs[i].x + rightPoint.derivs[i].y * rightPoint.derivs[i].y + rightPoint.derivs[i].z * rightPoint.derivs[i].z);
        double diff = abs(leftLength - rightLength);

        qDebug() << "Левая кривая: производная " << i << ":\t" << leftPoint.derivs[i].x << "\t" << leftPoint.derivs[i].y << "\t" << leftPoint.derivs[i].z;
        qDebug() << "Права кривая: производная " << i << ":\t" << rightPoint.derivs[i].x << "\t" << rightPoint.derivs[i].y << "\t" << rightPoint.derivs[i].z;
        qDebug() << "Difference: " << diff << '\n';

        if (diff > 0.01)
        {
            qDebug() << "Разрыв " << i << "-й производной, в точке " << &leftPoint.derivs[i] << " и diff = " << diff;
            check = false;
        }
    }

    if (check)
        qDebug() << "Выполняется непрерывность между кривыми!\n";
}

// Разбивает NURBS кривую на кривые Безье
std::vector<Curve> UsefulUtils::splittingСurveIntoBezierCurves(const Curve& originalCurve)
{
    // Контрольные точки оригинальной кривой
    std::vector<Point3D> controlPointsOriginalCurve = originalCurve.getControlPoints();
    int degree = originalCurve.getDegree();

    // Число кривых Безье, на которое будем делить originalCurve
    size_t numberBezierCurves = controlPointsOriginalCurve.size() / degree;
    std::vector<Curve> bezierCurves;

    for (size_t i = 0; i != numberBezierCurves; ++i)
    {
        std::vector<Point3D> tempControlPoints;

        // Добавляем по частям контрольные точки оригинальной кривой для каждой отдельной Безье кривой
        for (size_t j = 0; j != degree + 1; ++j)
        {
            tempControlPoints.push_back(controlPointsOriginalCurve[j + i * degree]);
        }

        Curve tempBezierCurve(tempControlPoints, std::vector<double> (tempControlPoints.size(), 1), degree, 1001);
        bezierCurves.push_back(tempBezierCurve);
    }

    return bezierCurves;
}

// Запись точек кривой в файл
void UsefulUtils::outNURBSPoints(const Curve& curve)
{
    std::ofstream outFile("NURBSPoints.txt");
    if (outFile.is_open())
    {
        for (const auto& curvePoints: curve.getCurvePoints())
        {
            outFile << curvePoints.x << " " << curvePoints.y << " " << curvePoints.z << '\n';
        }
        outFile.close();
    }
    else
    {
        qDebug() << "outNURBSPoints. Unable to open file for writing!";
    }
}

