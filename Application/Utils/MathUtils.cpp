#include "MathUtils.h"
#include <QtMath>

double MathUtils::calcRadiusVectorLength(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
}

double MathUtils::calcRadiusVectorLength(const Point3D &point)
{
    return calcRadiusVectorLength(point.x, point.y, point.z);
}

double MathUtils::calcVectorLenght(double x1, double y1, double z1, double x2, double y2, double z2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 -z1, 2));
}

double MathUtils::calcVectorLenght(const Point3D &point1, const Point3D &point2)
{
    return calcVectorLenght(point1.x, point1.y, point1.z, point2.x, point2.y, point2.z);
}

double MathUtils::calcVectorLenght(const CurvePoint &point1, const Point3D &point2)
{
    return calcVectorLenght(point1.x, point1.y, point1.z, point2.x, point2.y, point2.z);
}

double MathUtils::calcVectorLenght(const Point3D &point1, double x2, double y2, double z2)
{
    return calcVectorLenght(point1.x, point1.y, point1.z, x2, y2, z2);
}

double MathUtils::calcAngleBetweenVectors(const Point3D &vecStart1, const Point3D &vecEnd1, const Point3D &vecStart2, const Point3D &vecEnd2)
{
    double x1 = vecEnd1.x - vecStart1.x;
    double y1 = vecEnd1.y - vecStart1.y;
    double z1 = vecEnd1.z - vecStart1.z;
    double x2 = vecEnd2.x - vecStart2.x;
    double y2 = vecEnd2.y - vecStart2.y;
    double z2 = vecEnd2.z - vecStart2.z;
    double numerator = x1 * x2 + y1 * y2 + z1 * z2;
    double vecLen1 = calcVectorLenght(vecStart1, vecEnd1);
    double vecLen2 = calcVectorLenght(vecStart2, vecEnd2);
    double cos = numerator / (vecLen1 * vecLen2);
    return acos(cos) * 180 / M_PI;
}

int MathUtils::calcCombWithoutRepetition(int n, int k)
{
    return k == 0 || k == n ? 1 : calcCombWithoutRepetition(n - 1, k - 1) * n / k;
}

std::pair<std::vector<CurvePoint>, std::vector<CurvePoint>> MathUtils::moveCurve(const Curve &curve, double length)
{
    int numRealRangePoints = static_cast<int>(curve.getCurvePoints().size());
    std::vector<CurvePoint> movePointsNURBS(numRealRangePoints); // Точки NURBS со сдвигом длиной length
    std::vector<CurvePoint> reverseMovePointsNURBS(numRealRangePoints); // Точки NURBS со сдвигом в противоположную сторону длиной length

    for (size_t i = 0; i < curve.getCurvePoints().size(); ++i)
    {
        CurvePoint curvePoint = curve.getCurvePoints()[i];
        double rotatedX = curvePoint.derivs[1].x * cos(M_PI / 2) - curvePoint.derivs[1].y * sin(M_PI / 2);
        double rotatedY = curvePoint.derivs[1].x * sin(M_PI / 2) + curvePoint.derivs[1].y * cos(M_PI / 2);
        double rotatedZ = curvePoint.derivs[1].x * sin(M_PI / 2) + curvePoint.derivs[1].y * cos(M_PI / 2); // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок
        Point3D rotatedPoint =  Point3D(rotatedX + curvePoint.x, rotatedY + curvePoint.y, rotatedZ + curvePoint.z);  // Точка кривой со повотором на 90 градусов от начала коор-т

        double x = rotatedPoint.x - curvePoint.x;
        double y = rotatedPoint.y - curvePoint.y;
        double z = rotatedPoint.z - curvePoint.z; // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок
        double vecLen = MathUtils::calcRadiusVectorLength(x, y, z);
        x *= length / vecLen;
        y *= length / vecLen;
        z *= length / vecLen; // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок

        movePointsNURBS[i].x = x + curvePoint.x;
        movePointsNURBS[i].y = y + curvePoint.y;
        movePointsNURBS[i].z = z + curvePoint.z; // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок

        rotatedX = curvePoint.derivs[1].x * cos(-M_PI / 2) - curvePoint.derivs[1].y * sin(-M_PI / 2); // Точка кривой со сдвигом на -90 градусов от начала коор-т
        rotatedY = curvePoint.derivs[1].x * sin(-M_PI / 2) + curvePoint.derivs[1].y * cos(-M_PI / 2);
        rotatedZ = curvePoint.derivs[1].x * sin(-M_PI / 2) + curvePoint.derivs[1].y * cos(-M_PI / 2); // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок

        Point3D reverseRotatedPoints = Point3D(rotatedX + curvePoint.x, rotatedY + curvePoint.y, rotatedZ + curvePoint.z);

        x = reverseRotatedPoints.x - curvePoint.x;
        y = reverseRotatedPoints.y - curvePoint.y;
        z = reverseRotatedPoints.z - curvePoint.z; // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок
        vecLen = MathUtils::calcRadiusVectorLength(x, y, z);
        x *= length / vecLen;
        y *= length / vecLen;
        z *= length / vecLen; // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок

        reverseMovePointsNURBS[i].x = x + curvePoint.x;
        reverseMovePointsNURBS[i].y = y + curvePoint.y;
        reverseMovePointsNURBS[i].z = z + curvePoint.z; // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок
    }

    return std::pair<std::vector<CurvePoint>, std::vector<CurvePoint>> (movePointsNURBS, reverseMovePointsNURBS);
}
