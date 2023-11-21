#include "AlgBasedIntegralNorm.h"
#include <QDebug>
#include "Utils/IMatrixOperations.h"
#include "Curve3d/CalcCurve.h"

Curve AlgBasedIntegralNorm::approximateCurve(const Curve &curve, int degreeApprox) const
{
    int newNumControlPoints = degreeApprox + 1;

    auto newPoints = calcNewControlsPointsIntegralsMethod(curve, newNumControlPoints, degreeApprox); // Находим новые контрольные точки кривой

    std::vector<Point3D> controlPointsNewCurve(newNumControlPoints);
    controlPointsNewCurve[0] = curve.getControlPoints()[0];
    controlPointsNewCurve[controlPointsNewCurve.size() - 1] = curve.getControlPoints()[curve.getControlPoints().size() - 1];

    for (int i = 0; i < newPoints.size(); ++i)
    {
        controlPointsNewCurve[1 + i].x = (newPoints[i][0]);
        controlPointsNewCurve[1 + i].y = (newPoints[i][1]);
        controlPointsNewCurve[1 + i].z = (newPoints[i][2]);
    }

    std::vector<double> newWeights (newNumControlPoints, 1);

    return  Curve(controlPointsNewCurve, newWeights, degreeApprox, static_cast<int>(curve.getCurvePoints().size()));
}

std::vector<std::vector<double>> AlgBasedIntegralNorm::calcNewControlsPointsIntegralsMethod(const Curve &originalCurve, int newNumControlPoints, int newDegreeCurve) const
{
    const double NUMBER_INTEGRALS = 200; // Количество разбиений на интегралы
    const double DELTA = 1 / NUMBER_INTEGRALS;
    const int NUM_ROWS = newNumControlPoints; // Кол-во строк матриц
    const int NUM_COLS = newNumControlPoints; // Кол-во строк столбцов матриц

    std::vector<double> newNodalVector = createUniformNodalVector(newNumControlPoints, newDegreeCurve);

    std::vector<std::vector<double>> integrals(NUM_ROWS, std::vector<double> (NUM_COLS));      // Матрица интегралов
    std::vector<std::vector<double>> basisFunctions(NUM_ROWS, std::vector<double> (NUM_COLS)); // Матрица базисных функций и их производных
    std::vector<std::vector<double>> integralsSecondDeriv(NUM_ROWS, std::vector<double> (NUM_COLS)); // Матрица интегралов вторых производных
    std::vector<std::vector<double>> c_N(NUM_ROWS - 2, std::vector<double> (2));
    CurvePoint pointOriginalCurve;   // Точка первоначальной кривой

    for (int i = 0; i < NUMBER_INTEGRALS + 1; ++i)
    {
        double realPoint = (i / NUMBER_INTEGRALS); // Точка реальной часли узлового вектора
        int span = CalcCurve::findSpanForParameter(realPoint, newNodalVector, newDegreeCurve);
        basisFunctions = CalcCurve::calcBasisFuncsAndTheirDerivs(newNodalVector, realPoint, span, newDegreeCurve);

        for (size_t rows = 0; rows < NUM_ROWS; ++rows)
        {
            for (size_t cols = 1; cols < NUM_COLS - 1; ++cols)
            {
                integrals[rows][cols] += basisFunctions[0][rows] * basisFunctions[0][cols] * DELTA;
                integralsSecondDeriv[rows][cols] += basisFunctions[2][rows] * basisFunctions[2][cols] * DELTA;
            }
        }

        CalcCurve::calcCurvePointAndDerivs(originalCurve, pointOriginalCurve, realPoint);
        int count = 1;

        for (auto &point: c_N)
        {
            point[0] += pointOriginalCurve.x * basisFunctions[0][count] * DELTA;
            point[1] += pointOriginalCurve.y * basisFunctions[0][count] * DELTA;
            ++count;
        }
    }

    const double PRECISION = 0;
    std::vector<std::vector<double>> coefficients(newNumControlPoints - 2, std::vector<double> (newNumControlPoints - 2)); // Матрица коэффициентов

    for (int rows = 0; rows < coefficients.size(); ++rows) // Заполняем матрицу коээфициентов
    {
        for (int cols = 0; cols < coefficients[0].size(); ++cols)
            coefficients[rows][cols] = 2 * PRECISION * integralsSecondDeriv[1 + cols][1 + rows] - 2 * integrals[1 + cols][1 + rows];
    }

    std::vector<std::vector<double>> freeMembers(newNumControlPoints - 2, std::vector<double> (2)); // Матрица свободных членов
    std::vector<double> startPointOriginalCurve { originalCurve.getControlPoints()[0].x, originalCurve.getControlPoints()[0].y, originalCurve.getControlPoints()[0].z };       // Начальная точка оригинальной кривой
    std::vector<double> endPointOriginalCurve { originalCurve.getControlPoints().back().x, originalCurve.getControlPoints().back().y, originalCurve.getControlPoints().back().z }; // Конечная точка оригинальной кривой

    for (int rows = 0; rows < freeMembers.size(); ++rows)
    {
        freeMembers[rows][0] = startPointOriginalCurve[0] * (-2) * PRECISION * integralsSecondDeriv[0][1 + rows] - endPointOriginalCurve[0] * 2 * PRECISION * integralsSecondDeriv[newNumControlPoints - 1][1 + rows] -
                               2 * c_N[rows][0] + startPointOriginalCurve[0] * 2 * integrals[0][1 + rows] + endPointOriginalCurve[0] * 2 * integrals[newNumControlPoints - 1][1 + rows];
        freeMembers[rows][1] = startPointOriginalCurve[1] * (-2) * PRECISION * integralsSecondDeriv[0][1 + rows] - endPointOriginalCurve[1] * 2 * PRECISION * integralsSecondDeriv[newNumControlPoints - 1][1 + rows] -
                               2 * c_N[rows][1] + startPointOriginalCurve[1] * 2 * integrals[0][1 + rows] + endPointOriginalCurve[1] * 2 * integrals[newNumControlPoints - 1][1 + rows];
    }

    auto operation = IMatrixOperations::GetMatrixOperationsClass(OperationClass::eigen); // Создаём указатель на интерфейс операций СЛАУ

    if (operation == nullptr)
    {
        qDebug() << "Error! calcNewControlsPointsIntegralsMethod: oper == nullptr";;
        return {};
    }

    auto newPoints = operation->solveEquation(coefficients, freeMembers); // Решаем СЛАУ и получаем координаты новых точек кривой

    return newPoints;
}

std::vector<double> AlgBasedIntegralNorm::createUniformNodalVector(int numControlPoints, int degreeCurve)
{
    int numKnots = numControlPoints + degreeCurve + 1;
    int numRealRangeKnots = numControlPoints - degreeCurve + 1;

    // Рассчитываем начало и конец реального диапазона
    int realRangeStart = degreeCurve;
    int realRangeEnd = numKnots - degreeCurve - 1;

    std::vector<double> nodalVector(numKnots);

    // Равномерно заполняем узловой веткор
    double step = 1 / static_cast<double>(numRealRangeKnots - 1); // Шаг в реальном диапазоне

    for (int i = realRangeStart + 1; i < realRangeEnd; ++i)    // Заполняем реальный диапазон
        nodalVector[i] = nodalVector[i - 1] + step;

    for (size_t i = realRangeEnd; i < nodalVector.size(); ++i) // Заполняем последние параметры единицами
        nodalVector[i] = 1;

    return nodalVector;
}
