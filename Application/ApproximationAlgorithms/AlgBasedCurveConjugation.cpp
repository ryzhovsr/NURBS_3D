#include "AlgBasedCurveConjugation.h"
#include <QDebug>
#include "Curve3d/CalcCurve.h"
#include "Utils/MathUtils.h"
#include "Utils/IMatrixOperations.h"

Curve AlgBasedCurveConjugation::approximateCurve(const Curve &curve, int degreeApprox) const
{
     // Исходная степень кривой
    int initialDegree = curve.getDegree();

    if (curve.getControlPoints().size() - initialDegree != 2)
        qDebug() << "Error! AlgBasedCurveConjugation: степень кривой должна быть на 2 меньше кол-ва контрольных точек!";

    Curve tempCurve = curve;
    int numParameters = static_cast<int>(tempCurve.getCurvePoints().size());

    addNodalPoints(tempCurve);

    std::vector<Point3D> tempControlPoints = tempCurve.getControlPoints();

    std::vector<Point3D> controlPointsBezier_1(tempControlPoints.begin(), tempControlPoints.end() -  initialDegree);
    std::vector<Point3D> controlPointsBezier_2(tempControlPoints.begin() + initialDegree, tempControlPoints.end());
    std::vector<double> newWeights(controlPointsBezier_1.size(), 1);

    Curve bezier1(controlPointsBezier_1, newWeights, initialDegree, numParameters);
    Curve bezier2(controlPointsBezier_2, newWeights, initialDegree, numParameters);

    reductBezierCurveDegree(bezier1);
    reductBezierCurveDegree(bezier2);
    tempCurve = attachCurve(bezier1, bezier2, true);

    Curve resCurve = tempCurve;

    for (int currentDegree = tempCurve.getDegree(); currentDegree > degreeApprox; --currentDegree)
    {
        // Добавляем кратный узел в кривую
        tempCurve = redefineControlPointsNodalVectorCurve(tempCurve);

        tempControlPoints = tempCurve.getControlPoints();
        std::vector<Point3D> tempControlPoints_1(tempControlPoints.begin(), tempControlPoints.end() -  tempCurve.getDegree());
        std::vector<Point3D> tempControlPoints_2(tempControlPoints.begin() + tempCurve.getDegree(), tempControlPoints.end());
        std::vector<double> tempWeights(controlPointsBezier_1.size(), 1);
        Curve tempBezier_1(tempControlPoints_1, tempWeights, currentDegree, numParameters);
        Curve tempBezier_2(tempControlPoints_2, tempWeights, currentDegree, numParameters);

        reductBezierCurveDegree(tempBezier_1);
        reductBezierCurveDegree(tempBezier_2);
        tempCurve = attachCurve(tempBezier_1, tempBezier_2, true);
        resCurve = tempCurve;
    }

    return resCurve;
}

void AlgBasedCurveConjugation::addNodalPoints(Curve &curve) const
{
    int degree = curve.getDegree();
    std::vector<Point3D> controlPoints = curve.getControlPoints();
    int controlPointsSize = static_cast<int>(controlPoints.size());
    int numRealRangeKnots = controlPointsSize - degree + 1; // Кол-во узлов реального диапазона узл. вектора
    std::vector<double> nodalVector = curve.getNodalVector();
    int numKnots = static_cast<int>(nodalVector.size());

    int numNewNodes = (degree - 1) * (numRealRangeKnots - 2);   // "-1" не берём уже имеющиеся узлы; "-2" не берём граничные узлы равные 0 и 1
    std::vector<double> newNodesNodalVector;    // Новые узлы для нового узлового вектора


    // Начало и конец реального диапазона узл. вектора
    int realRangeStart = degree;
    int realRangeEnd = numKnots - degree - 1;
    int numNotBoundaryNodals = 0; // Количество не граничных узлов (не равных 0 и 1)

    for (int i = realRangeStart; i < realRangeEnd; ++i)
    {
        if (nodalVector[i] != 0 && nodalVector[i] != 1) // Пропускаем границы реального диапазона
            ++numNotBoundaryNodals;
    }

    if (numNotBoundaryNodals == 0)
    {
        qDebug() << "Error! addNodalPoints: numNotBoundaryNodals == 0";
        return;
    }

    for (int i = realRangeStart; i < realRangeEnd; ++i)
    {
        int counter = 0;  // Кол-во добавлений нового узла

        while (counter < numNewNodes / numNotBoundaryNodals)
        {
            if (nodalVector[i] == 0 || nodalVector[i] == 1) // Пропускаем границы реального диапазона
                break;

            newNodesNodalVector.push_back(nodalVector[i]);
            ++counter;
        }
    }

    int numOldControlPoints = static_cast<int>(curve.getControlPoints().size());    // Коли-во контрольных точек до вставки
    int maxIndexNewNodes = static_cast<int>(newNodesNodalVector.size() - 1);        // Максимальный индекс вектора newNodesNodalVector

    std::vector<Point3D> newControlPoints(numOldControlPoints + newNodesNodalVector.size());    // Новые контрольные точки
    std::vector<double> newNodalVector(nodalVector.size() + newNodesNodalVector.size());        // Новый узловой вектор

    double a = CalcCurve::findSpanForParameter(newNodesNodalVector[0], nodalVector, degree);
    double b = CalcCurve::findSpanForParameter(newNodesNodalVector[maxIndexNewNodes], nodalVector, degree) + 1;

    for (int i = 0; i < a - degree + 1; ++i)
        newControlPoints[i] = controlPoints[i];

    for (int i = b - 1; i < numOldControlPoints; ++i)
        newControlPoints[i + maxIndexNewNodes + 1] = controlPoints[i];

    for (int i = 0; i < a + 1; ++i)
        newNodalVector[i] = nodalVector[i];

    for (int i = b + degree; i < numOldControlPoints + degree + 1; ++i)
        newNodalVector[i + maxIndexNewNodes + 1] = nodalVector[i];

    int ind = b + degree - 1;
    int k = b + degree + maxIndexNewNodes;

    for (int i = maxIndexNewNodes; i > -1; --i)
    {
        while ((newNodesNodalVector[i] <= nodalVector[ind]) && (ind > a))
        {
            newControlPoints[k - degree - 1] = controlPoints[ind - degree - 1];
            newNodalVector[k] = nodalVector[ind];
            --k, --ind;
        }

        newControlPoints[k - degree - 1] = newControlPoints[k - degree];

        for (int j = 1; j < degree + 1; ++j)
        {
            int temp = k - degree + j;
            double alpha = newNodalVector[k + j] - newNodesNodalVector[i];

            if (alpha == 0)
                newControlPoints[temp - 1] = newControlPoints[temp];
            else
            {
                alpha /= (newNodalVector[k + j] - nodalVector[ind - degree + j]);
                newControlPoints[temp - 1].x = (alpha * newControlPoints[temp - 1].x + (1 - alpha) * newControlPoints[temp].x);
                newControlPoints[temp - 1].y = (alpha * newControlPoints[temp - 1].y + (1 - alpha) * newControlPoints[temp].y);
                newControlPoints[temp - 1].z = (alpha * newControlPoints[temp - 1].z + (1 - alpha) * newControlPoints[temp].z);
            }
        }

        newNodalVector[k] = newNodesNodalVector[i];
        --k;
    }

    curve = redefineCurve(newControlPoints, newNodalVector, degree, static_cast<int>(curve.getCurvePoints().size()));
}

Curve AlgBasedCurveConjugation::redefineCurve(const std::vector<Point3D>& controlPoints, const std::vector<double>& nodalVector, int degree, int numParameters) const
{
    std::vector<double> weights(controlPoints.size(), 1);

    Curve newCurve(controlPoints, weights, degree, numParameters);

    if (!nodalVector.empty()) // Если новый узловой вектор пустой, то заполним его равномерно
    {
        newCurve.setNodalVector(nodalVector);
        newCurve.calcCurve();
    }

    return newCurve;
}

void AlgBasedCurveConjugation::reductBezierCurveDegree(Curve &curve) const
{
    std::vector<Point3D> controlPoints = curve.getControlPoints();
    int degree = curve.getDegree();

    if (controlPoints.size() - 1 != degree) // Проверка что сплайн является кривой Безье
    {
        qDebug() << "Error! reductBezierCurveDegree: максимальный индекс массива не равен степени _controlPoints.size() != _degree";
        return;
    }

    if (degree <= 2)
    {
        qDebug() << "Error! reductBezierCurveDegree: порядок исходного сплайна не может быть меньше 3";
        return;
    }

    int numOldControlPoints = static_cast<int>(controlPoints.size());
    std::vector<Point3D> newControlPoints(numOldControlPoints - 1); // Новые контрольные точки

    // Присваиваем коор-ты первой и последней старых контрольных точек
    newControlPoints[0] = controlPoints[0];
    newControlPoints[numOldControlPoints - 2] = controlPoints[numOldControlPoints - 1];

    int meadle = (degree - 1) / 2;

    // Заполняем новые контрольные точки
    for (int i = 1; i < meadle + 1; ++i)
        newControlPoints[i] = (controlPoints[i] - (i / static_cast<double>(degree)) * newControlPoints[i - 1]) / (1 - (i / static_cast<double>(degree)));

    for (int i = degree - 2; i > meadle; --i)
        newControlPoints[i] = (controlPoints[i + 1] - (1 - (i + 1) / static_cast<double>(degree)) * newControlPoints[i + 1]) / ((i + 1) / static_cast<double>(degree));

    if (degree % 2 != 0) // Для нечётной степени вычисляем центральную контрольную точку
    {
        Point3D left = (controlPoints[meadle] - (meadle / static_cast<double>(degree)) * newControlPoints[meadle - 1]) / (1 - (meadle / static_cast<double>(degree)));
        Point3D right = (controlPoints[meadle + 1] - (1 - (meadle + 1) / static_cast<double>(degree)) * newControlPoints[meadle + 1]) / ((meadle + 1) / static_cast<double>(degree));

        newControlPoints[meadle] = (left + right) * 0.5;
    }

    curve = redefineCurve(newControlPoints, {}, --degree, static_cast<int>(curve.getCurvePoints().size()));
}

Curve AlgBasedCurveConjugation::attachCurve(const Curve &curve1, const Curve &curve2, bool fixateStartEndPoints) const
{
    std::vector<Point3D> controlPointsFirstCurve = curve1.getControlPoints();
    std::vector<Point3D> controlPointsSecondCurve = curve2.getControlPoints();
    int numControlPoints = 0; // Кол-во контрольных точек

    if (curve1.getControlPoints().size() != curve2.getControlPoints().size()) // Проверка на одинаковое кол-во контрольных точек кривых (степень кривых Безье должны быть равны)
        qDebug() << "Error! attachCurvesUsualMethod: кривые Безье имеют разное кол-во контрольных точек";
    else
        numControlPoints = static_cast<int>(curve1.getControlPoints().size());

    std::vector<Point3D> derivsFirstCurve(numControlPoints);    // Производные первой кривой
    std::vector<Point3D> negDerivsFirstCurve(numControlPoints); // Отрицательные дельты контрольных точек первой кривой
    int startIndexFirstCurve = numControlPoints - 1;          // Стартовый индекс начинается с конца

    for (int i = numControlPoints - 1; i >= 0 ; --i) // Находим дельты для первой кривой
    {
        derivsFirstCurve[numControlPoints - i - 1] = calcDerivLeftBezierCurveForMerger(controlPointsFirstCurve, i, startIndexFirstCurve);
        negDerivsFirstCurve[numControlPoints - i - 1] = calcNegativeDerivLeftBezierCurveForMerger(controlPointsFirstCurve, i, startIndexFirstCurve);
    }

    std::vector<Point3D> derivsSecondCurve(numControlPoints);   // Производные второй кривой
    int startIndexSecondCurve = 0;    // Стартовый индекс начинается с начала

    for (int i = 0; i < numControlPoints; ++i)    // Находим дельты для второй кривой
        derivsSecondCurve[i] = calcDerivRightBezierCurveForMerger(controlPointsSecondCurve, i, startIndexSecondCurve);

    std::vector<std::vector<double>> coefficients(numControlPoints * 3, std::vector<double> (numControlPoints * 3)); // Матрица коэффициентов

    // Заполняем матрицу коэффициентов
    for (int i = 0; i < numControlPoints * 2; ++i)
        coefficients[i][i] = 2;

    for (int i = 0; i < numControlPoints; ++i)
    {
        int resNum = 0;
        int counter = 0;

        for (int r = 0; r <= i; ++r)
        {
            resNum = pow(-1, r - i) * MathUtils::calcCombWithoutRepetition(i, r);
            coefficients[numControlPoints * 2 + i][startIndexFirstCurve - i + counter] = resNum;
            coefficients[numControlPoints * 2 + i][startIndexFirstCurve + i - counter + 1] = pow(-1, i % 2? 0 : 1) * resNum;

            coefficients[startIndexFirstCurve - i + counter][numControlPoints * 2 + i] = resNum;
            coefficients[startIndexFirstCurve + i - counter + 1][numControlPoints * 2 + i] = pow(-1, i % 2? 0 : 1) * resNum;
            ++counter;
        }
    }

    if (fixateStartEndPoints) // Если начальная и конечная точка фиксированная
    { // Изменяем матрицу коэффициентов
        coefficients[0][0] = 1;
        coefficients[numControlPoints * 2 - 1][numControlPoints * 2 - 1] = 1;
        coefficients[0][coefficients[0].size() - 1] = 0;
        coefficients[numControlPoints * 2 - 1][coefficients[0].size() - 1] = 0;
    }

    std::vector<Point3D> freeMembers(numControlPoints * 3); // Матрица свободных членов
    int counter = 0;

    for (int i = numControlPoints * 2; i < numControlPoints * 3; ++ i) // Заполняем матрицу свободных членов
    {
        freeMembers[i] = negDerivsFirstCurve[counter] + derivsSecondCurve[counter];
        ++counter;
    }

    auto operation = IMatrixOperations::GetMatrixOperationsClass(OperationClass::eigen); // Создаём указатель на интерфейс операций СЛАУ

    if (operation == nullptr)
        qDebug () << "Error! attachCurvesUsualMethod: oper == nullptr";;

    std::vector<Point3D> solution = operation->solveEquation(coefficients, freeMembers); // Решаем СЛАУ

    std::vector<Point3D> tempPoints(numControlPoints); // Временные точки для рассчёта новых контрольных точек
    std::vector<Point3D> controlPointsNewCurve(numControlPoints);

    for (int i = 0; i < numControlPoints; ++i)
    {
        tempPoints[i] = (controlPointsFirstCurve[i] + solution[i]);
    }

    controlPointsNewCurve[0] = tempPoints[0];
    counter = 1;

    while (counter != numControlPoints)
    {
        for (int i = 0; i < numControlPoints - counter; ++i)
            tempPoints[i] =  -1 * tempPoints[i] + 2 * tempPoints[i + 1];

        controlPointsNewCurve[counter] = tempPoints[0];
        ++counter;
    }

    // Найдём новые точки двух кривых по отдельности
    std::vector<Point3D> newControlPointsFirstCurve(numControlPoints);
    std::vector<Point3D> newControlPointsSecondCurve(numControlPoints);

    for (int i = 0; i < numControlPoints; ++i)
    {
        newControlPointsFirstCurve[i] = (controlPointsFirstCurve[i] + solution[i]);

        newControlPointsSecondCurve[i] = (controlPointsSecondCurve[i] + solution[numControlPoints + i]);
    }

    //this->_controlPoints = newControlPoints_P; // Если нужно, можем присвоить новые точки текущим кривым
    //anotherCurve._controlPoints=newControlPoints_Q;

    for (int i = numControlPoints - 1; i >= 0 ; --i) // Находим новые производные для первой кривой
    {
        derivsFirstCurve[numControlPoints - i - 1] = calcDerivLeftBezierCurveForMerger(newControlPointsFirstCurve, i, startIndexFirstCurve);
        negDerivsFirstCurve[numControlPoints - i - 1] = calcNegativeDerivLeftBezierCurveForMerger(newControlPointsFirstCurve, i, startIndexFirstCurve);
    }

    for (int i = 0; i < numControlPoints; ++i)   // Находим новые производные для второй кривой
        derivsSecondCurve[i] = calcDerivRightBezierCurveForMerger(newControlPointsSecondCurve, i, startIndexSecondCurve);

    for (int i = 0; i < numControlPoints; ++i)
    {
        if (abs(derivsFirstCurve[i].x - derivsSecondCurve[i].x) > 0.001 || abs(derivsFirstCurve[i].y - derivsSecondCurve[i].y) > 0.001)
            qDebug() << "Error! attachCurvesUsualMethod: derivsFirstCurve[i] != derivsSecondCurve[i]";
    }

    return Curve(controlPointsNewCurve, curve1.getWeights(), curve1.getDegree(), static_cast<int>(curve1.getCurvePoints().size()));
}

Point3D AlgBasedCurveConjugation::calcDerivLeftBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex) const
{
    if(startIndex == currentIndex)
        return points[currentIndex];
    else
        return calcDerivLeftBezierCurveForMerger(points, currentIndex + 1, startIndex) - calcDerivLeftBezierCurveForMerger(points, currentIndex, startIndex - 1);
}

Point3D AlgBasedCurveConjugation::calcNegativeDerivLeftBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex) const
{
    if(startIndex == currentIndex)
        return -1 * points[currentIndex];
    else
        return calcNegativeDerivLeftBezierCurveForMerger(points, currentIndex + 1, startIndex)  - calcNegativeDerivLeftBezierCurveForMerger(points, currentIndex, startIndex - 1);
}

Point3D AlgBasedCurveConjugation::calcDerivRightBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex) const
{
    if(startIndex == currentIndex)
        return points[currentIndex];
    else
        return calcDerivRightBezierCurveForMerger(points, currentIndex, startIndex + 1) - calcDerivRightBezierCurveForMerger(points, currentIndex - 1, startIndex);
}

Curve AlgBasedCurveConjugation::redefineControlPointsNodalVectorCurve(Curve &curve) const
{
    std::vector<double> oldNodalVector = curve.getNodalVector();
    std::vector<Point3D> oldControlPoints = curve.getControlPoints();


    int newDegreeCurve = curve.getDegree();
    std::vector<double> newNodes(newDegreeCurve, 0.5); // Новый вектор узловых точек, который будет добавлен в текущий
    std::vector<Point3D> newControlPoints(curve.getControlPoints().size() + newDegreeCurve);
    std::vector<double> newNodalVector(curve.getNodalVector().size() + newNodes.size());

    int numOldControlPoints = static_cast<int>(oldControlPoints.size());    // Коли-во контрольных точек до вставки
    int maxIndexNewNodes = static_cast<int>(newNodes.size()) - 1;

    double a = CalcCurve::findSpanForParameter(newNodes[0], oldNodalVector, newDegreeCurve);
    double b = CalcCurve::findSpanForParameter(newNodes[maxIndexNewNodes], oldNodalVector, newDegreeCurve) + 1;

    for (int i = 0; i < a - newDegreeCurve + 1; ++i)
        newControlPoints[i] = oldControlPoints[i];

    for (int i = b - 1; i < numOldControlPoints; ++i)
        newControlPoints[i + maxIndexNewNodes + 1] = oldControlPoints[i];

    for (int i = 0; i < a + 1; ++i)
        newNodalVector[i] = oldNodalVector[i];

    for (int i = b + newDegreeCurve; i < numOldControlPoints + newDegreeCurve + 1; ++i)
        newNodalVector[i + maxIndexNewNodes + 1] = oldNodalVector[i];

    int ind = b + newDegreeCurve - 1;
    int k = b + newDegreeCurve + maxIndexNewNodes;

    for (int i = maxIndexNewNodes; i > -1; --i)
    {
        while ((newNodes[i] <= oldNodalVector[ind]) && (ind > a))
        {
            newControlPoints[k - newDegreeCurve - 1] = oldControlPoints[ind - newDegreeCurve - 1];
            newNodalVector[k] = oldNodalVector[ind];
            --k, --ind;
        }

        newControlPoints[k - newDegreeCurve - 1] = newControlPoints[k - newDegreeCurve];

        for (int j = 1; j < newDegreeCurve + 1; ++j)
        {
            int temp = k - newDegreeCurve + j;
            double alpha = newNodalVector[k + j] - newNodes[i];

            if (alpha == 0)
                newControlPoints[temp - 1] = newControlPoints[temp];
            else
            {
                alpha /= (newNodalVector[k + j] - oldNodalVector[ind - newDegreeCurve + j]);
                newControlPoints[temp - 1].x = (alpha * newControlPoints[temp - 1].x + (1 - alpha) * newControlPoints[temp].x);
                newControlPoints[temp - 1].y = (alpha * newControlPoints[temp - 1].y + (1 - alpha) * newControlPoints[temp].y);
                newControlPoints[temp - 1].z = (alpha * newControlPoints[temp - 1].z + (1 - alpha) * newControlPoints[temp].z);
            }
        }

        newNodalVector[k] = newNodes[i];
        --k;
    }

    return redefineCurve(newControlPoints, newNodalVector, newDegreeCurve, static_cast<int>(curve.getCurvePoints().size()));
}
