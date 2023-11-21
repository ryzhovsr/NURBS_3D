#include "FindDistanceBetweenCurves.h"
#include "MathUtils.h"
#include "Curve3d/CalcCurve.h"

double FindDistanceBetweenCurves::findMaxLenBetweenCurves(const Curve &curve1, const Curve &curve2)
{
    double maxLength = 0, minCosine = 1;

    for (size_t i = 0; i < curve2.getCurvePoints().size(); ++i) // Находим наибольшее расстояние между кривыми
    {
        // Находим ближайшую точку исходной кривой к точке аппроксимирующей кривой
        Point3D startingPoint = Point3D(curve2.getCurvePoints()[i].x, curve2.getCurvePoints()[i].y, curve2.getCurvePoints()[i].z);

        CurvePoint nearestPoint = findPointNURBS(curve1, startingPoint, false);
        double tempLength = MathUtils::calcVectorLenght(nearestPoint, startingPoint);
        double tempCosine = cosBetweenVectors(nearestPoint, startingPoint);

        const double RIGHT_ANGLE = 0.1;    // Допустимое значение прямого угла

        if (RIGHT_ANGLE > minCosine + tempCosine) // Если разница косинусов очень маленькая
        {
            if (maxLength < tempLength)
            {
                maxLength = tempLength;
                minCosine = tempCosine;
            }
        } // Иначе сравниваем косинусы
        else if ((minCosine > tempCosine) || // Если новый косинус угла меньше предыдущего
                 (minCosine == tempCosine && maxLength < tempLength))      // Или ищем наибольшую длину
        {
            maxLength = tempLength;
            minCosine = tempCosine;
        }
    }

    return maxLength;
}

std::pair<CurvePoint, CurvePoint> FindDistanceBetweenCurves::findFarthestPointsNURBS(const Curve &curve1, const Curve &curve2)
{
    double maxLength = 0;
    std::pair<CurvePoint, CurvePoint> farthestPoints;

    for (size_t i = 0; i < curve2.getCurvePoints().size(); ++i)
    {
        Point3D startingPoint = Point3D(curve2.getCurvePoints()[i].x, curve2.getCurvePoints()[i].y, curve2.getCurvePoints()[i].z);

        CurvePoint farthestPoint = findPointNURBS(curve1, startingPoint, true);
        double tempLength = MathUtils::calcVectorLenght(farthestPoint, startingPoint);
        double cos = cosBetweenVectors(farthestPoint, startingPoint);

        if (0.1 > cos && maxLength < tempLength)
        { // Если угол вектора расстояния до кривой ~90 градусов и его длина больше прошлой длины
            maxLength = tempLength;
            farthestPoints.first = farthestPoint;
            farthestPoints.second = curve2.getCurvePoints()[i];
        }
    }

    return farthestPoints;
}

double FindDistanceBetweenCurves::cosBetweenVectors(const CurvePoint& pointNURBS, const Point3D& point)
{
    Point3D vec1(pointNURBS.x - point.x, pointNURBS.y - point.y, pointNURBS.z - point.z);
    Point3D vec2(pointNURBS.derivs[1].x, pointNURBS.derivs[1].y, pointNURBS.derivs[1].z);
    double numerator = vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
    double vecLen1 = sqrt(pow(vec1.x, 2) + pow(vec1.y, 2) + pow(vec1.z, 2));
    double vecLen2 = sqrt(pow(vec2.x, 2) + pow(vec2.y, 2) + pow(vec2.z, 2));
    double denominator = vecLen1 * vecLen2;

    if (denominator == 0)
        return 0;
    else
        return std::abs(numerator / denominator);
}

CurvePoint FindDistanceBetweenCurves::findPointNURBS(const Curve &curve, const Point3D& startingPoint, bool nearest)
{
    int degree = curve.getDegree();
    std::vector<Point3D> controlPoints = curve.getControlPoints();
    std::vector<double> weight = curve.getWeights();
    std::vector<double> nodalVector = curve.getNodalVector();
    int numRealRangeKnots = static_cast<int>(controlPoints.size()) - degree + 1;  // Кол-во узлов реального диапазона узл. вектора
    std::vector<CurvePoint> foundPoints(numRealRangeKnots - 1);   // Массив ближайших/дальних точек из разных спанов

    std::vector<double> pointsRealSpans = calcPointsRealSpan(nodalVector, degree);    // Спаны реального диапазона узлового вектора
    std::vector<double> cosines(pointsRealSpans.size() - 1, 1);    // Массив косинусов для каждой точки

    for (size_t i = 0; i < pointsRealSpans.size() - 1; ++i) // Итерируемся по спанам, начиная с нулевого
    {
        const double LEN_CURRENT_SPAN = pointsRealSpans[i + 1] - pointsRealSpans[i];
        foundPoints[i].parameter = LEN_CURRENT_SPAN / 2 + pointsRealSpans[i]; // Берём среднее текущего спана
        CalcCurve::calcCurvePointAndDerivs(curve, foundPoints[i], foundPoints[i].parameter);

        double k = 1;   // Коэффициент, регулирующий шаг до следующей точки
        const double DELTA = 0.001 * LEN_CURRENT_SPAN;  // Основное допустимое значение для поиска точки
        const double RIGHT_ANGLE = 0.01;    // Допустимое значение прямого угла
        int counter = 0;

        while (cosines[i] > RIGHT_ANGLE)     // Пока косинус не будет меньше допуст. знач. (~90 градусов)
        {
            double x = foundPoints[i].x - startingPoint.x;
            double y = foundPoints[i].y - startingPoint.y;
            double z = foundPoints[i].z - startingPoint.z;  // Это неправильно, т.к. этот код для 2Д, а эта переменная - заглушка, чтобы не было ошибок
            double numerator = x * foundPoints[i].derivs[1].x + y * foundPoints[i].derivs[1].y + z * foundPoints[i].derivs[1].z;
            double denominator = x * foundPoints[i].derivs[2].x + y * foundPoints[i].derivs[2].y + z * foundPoints[i].derivs[2].z +
                                 pow(MathUtils::calcRadiusVectorLength(foundPoints[i].derivs[1]), 2);

            double pointRealNew = foundPoints[i].parameter - k * numerator / denominator; // Новая точка реального диапазона

            if (abs(pointRealNew - foundPoints[i].parameter) <= DELTA) // Основное условие остановки алгоритма по допуст. знач
            {
                // Проверка на выход из диапазона
                if (pointRealNew < pointsRealSpans[i])
                    pointRealNew = pointsRealSpans[i];
                else
                    pointRealNew = pointsRealSpans[i + 1];

                CalcCurve::calcCurvePointAndDerivs(curve, foundPoints[i], pointRealNew);
                cosines[i] = cosBetweenVectors(foundPoints[i], startingPoint);
                break;
            }

            // Если новая точка выходит за пределы текущего интервала узл. вектора
            if (pointRealNew < pointsRealSpans[i] || pointRealNew > pointsRealSpans[i + 1])
            {
                if (abs(pointRealNew - foundPoints[i].parameter) <= DELTA) // Если основное усл. ост. алг. выполняется
                { // Берём граничные значения спана
                    if (pointRealNew < pointsRealSpans[i])
                        pointRealNew = pointsRealSpans[i];
                    else
                        pointRealNew = pointsRealSpans[i + 1];

                    CalcCurve::calcCurvePointAndDerivs(curve, foundPoints[i], pointRealNew);
                    cosines[i] = cosBetweenVectors(foundPoints[i], startingPoint);
                    break;
                }
                else
                { // Иначе уменьшаем шаг для след. итерации
                    counter = 0;
                    k /= 2;
                    continue;
                }
            }

            CalcCurve::calcCurvePointAndDerivs(curve, foundPoints[i], pointRealNew);
            cosines[i] = cosBetweenVectors(foundPoints[i], startingPoint);

            ++counter;
            if (counter == 5) // Чтоб не было бесконечного цикла
            {
                k /= 1.1;
                counter = 0;
            }
        }
    }

    return comparePoints(foundPoints, cosines, startingPoint, nearest);
}

CurvePoint FindDistanceBetweenCurves::findPointNURBS(const Curve& curve, double x, double y, double z, bool nearest)
{
    return findPointNURBS(curve, Point3D(x, y, z), nearest);
}

std::vector<double> FindDistanceBetweenCurves::calcPointsRealSpan(const std::vector<double> &nodalVector, int degree)
{
    int numKnots = static_cast<int>(nodalVector.size());
    double pointStart = nodalVector[degree];
    const double pointEnd = nodalVector[numKnots - degree - 1];
    std::vector<double> pointsRealSpan;

    for (int i = 1; pointStart < pointEnd; ++i)
    {
        pointsRealSpan.push_back(pointStart);
        pointStart = nodalVector[degree + i];
    }

    pointsRealSpan.push_back(pointEnd);
    return pointsRealSpan;
}

CurvePoint FindDistanceBetweenCurves::comparePoints(const std::vector<CurvePoint>& points, const std::vector<double> &cosines, const Point3D& startingPoint, bool nearest)
{
    CurvePoint resultPoint;    // Искомая точка
    resultPoint = points[0];   // Присваиваем первую точку для дальнейшего сравнения

    if (points.size() == 1)    // Если в массиве только одна точка, возвращаем её
        return resultPoint;

    double resultVecLen = MathUtils::calcVectorLenght(startingPoint, resultPoint.x, resultPoint.y, resultPoint.z);
    double resultCos = cosines[0];
    const double RIGHT_ANGLE = 0.1;    // Допустимое значение прямого угла

    for (size_t i = 1; i < points.size(); ++i)
    {
        double tempVecLen = MathUtils::calcVectorLenght(startingPoint, points[i].x, points[i].y, points[i].z);
        double tempCos = cosines[i];

        if (RIGHT_ANGLE > resultCos + tempCos) // Если разница косинусов очень маленькая
        { // Будем сравнивать длину в зависимости от (nearest)
            if ((nearest && tempVecLen < resultVecLen) || // Ищем наименьшую длину
                (tempVecLen > resultVecLen))         // Или ищем наибольшую длину в зависимости от nearest
            {
                resultVecLen = tempVecLen;
                resultCos = tempCos;
                resultPoint = points[i];
            }
        } // Иначе сравниваем косинусы
        else if ((resultCos > tempCos) || // Если новый косинус угла меньше предыдущего
                 (resultCos == tempCos && // Или косинусы равны, то в зависимости от (nearest)
                  ((nearest && resultVecLen < tempVecLen) // Ищем наименьшую длину
                   || resultVecLen > tempVecLen)))        // Или ищем наибольшую длину
        {
            resultVecLen = tempVecLen;
            resultCos = tempCos;
            resultPoint = points[i];
        }
    }

    return resultPoint;
}
