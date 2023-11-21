#include "MergeCurves.h"
#include "Utils/MathUtils.h"
#include "Utils/IMatrixOperations.h"
#include "Curve3D/CalcCurve.h"

// Рекурсивные методы рассчёта производных, предназначенных для дальнейшего соединения кривых Безье
Point3D calcDerivLeftBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex)
{
    if(startIndex == currentIndex)
        return points[currentIndex];
    else
        return calcDerivLeftBezierCurveForMerger(points, currentIndex + 1, startIndex) - calcDerivLeftBezierCurveForMerger(points, currentIndex, startIndex - 1);
}

Point3D calcNegativeDerivLeftBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex)
{
    if(startIndex == currentIndex)
        return -1 * points[currentIndex];
    else
        return calcNegativeDerivLeftBezierCurveForMerger(points, currentIndex + 1, startIndex)  - calcNegativeDerivLeftBezierCurveForMerger(points, currentIndex, startIndex - 1);
}

Point3D calcDerivRightBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex)
{
    if(startIndex == currentIndex)
        return points[currentIndex];
    else
        return calcDerivRightBezierCurveForMerger(points, currentIndex, startIndex + 1) - calcDerivRightBezierCurveForMerger(points, currentIndex - 1, startIndex);
}

Curve MergeCurves::attachTwoBezierCurves(const Curve &curve1, const Curve &curve2, bool fixateStartEndPoints)
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
    //std::vector<double> weights(controlPointsNewCurve.size(), 1);

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

    // Для сплайна из новых точек - было 11 и станет 11 контрольных точек
    std::vector<Point3D> answer(newControlPointsFirstCurve.size() * 2 - 1);
    std::vector<double> weights(answer.size(), 1);

    for (size_t i = 0; i < newControlPointsFirstCurve.size(); ++i)
    {
        answer[i] = newControlPointsFirstCurve[i];
        answer[newControlPointsFirstCurve.size() - 1 + i] = newControlPointsSecondCurve[i];
    }


    for (int i = numControlPoints - 1; i >= 0 ; --i) // Находим новые производные для первой кривой
    {
        derivsFirstCurve[numControlPoints - i - 1] = calcDerivLeftBezierCurveForMerger(newControlPointsFirstCurve, i, startIndexFirstCurve);
        negDerivsFirstCurve[numControlPoints - i - 1] = calcNegativeDerivLeftBezierCurveForMerger(newControlPointsFirstCurve, i, startIndexFirstCurve);
    }

    for (int i = 0; i < numControlPoints; ++i)   // Находим новые производные для второй кривой
        derivsSecondCurve[i] = calcDerivRightBezierCurveForMerger(newControlPointsSecondCurve, i, startIndexSecondCurve);

    for (int i = 0; i < numControlPoints; ++i)
    {
        if (abs(derivsFirstCurve[i].x - derivsSecondCurve[i].x) > 0.001 || abs(derivsFirstCurve[i].y - derivsSecondCurve[i].y) > 0.001
            || abs(derivsFirstCurve[i].z - derivsSecondCurve[i].z) > 0.001)
            qDebug() << "Error! attachCurvesUsualMethod: derivsFirstCurve[i] != derivsSecondCurve[i]";
    }

    return Curve(answer, weights, curve1.getDegree(), static_cast<int>(curve1.getCurvePoints().size()));
}

std::vector<Curve> MergeCurves::attachAllBezierCurves(std::vector<Curve>& bezierCurves)
{
    // Находим базисные функции у первой кривой в начальной точке
    double parameter = 0;
    int span = CalcCurve::findSpanForParameter(parameter, bezierCurves[0].getNodalVector(), bezierCurves[0].getDegree());
    std::vector<std::vector<double>> basisFuncsAndTheirDerivs = CalcCurve::calcBasisFuncsAndTheirDerivs(bezierCurves[0].getNodalVector(), parameter, span, bezierCurves[0].getDegree());

    double N_000 = basisFuncsAndTheirDerivs[0][0], N_010 = basisFuncsAndTheirDerivs[0][1], N_020 = basisFuncsAndTheirDerivs[0][2]; // НУЛЕВЫЕ ПРОИЗВОДНЫЕ = сама функция
    double N_100 = basisFuncsAndTheirDerivs[1][0], N_110 = basisFuncsAndTheirDerivs[1][1], N_120 = basisFuncsAndTheirDerivs[1][2]; // ПЕРВЫЕ ПРОИЗВОДНЫЕ
    double N_200 = basisFuncsAndTheirDerivs[2][0], N_210 = basisFuncsAndTheirDerivs[2][1], N_220 = basisFuncsAndTheirDerivs[2][2]; // ВТОРЫЕ ПРОИЗВОДНЫЕ

    // Находим базисные функции у первой кривой в конечной точке
    parameter = 1;
    span = CalcCurve::findSpanForParameter(parameter, bezierCurves[0].getNodalVector(), bezierCurves[0].getDegree());
    basisFuncsAndTheirDerivs = CalcCurve::calcBasisFuncsAndTheirDerivs(bezierCurves[0].getNodalVector(), parameter, span, bezierCurves[0].getDegree());

    double N_001 = basisFuncsAndTheirDerivs[0][0], N_011 = basisFuncsAndTheirDerivs[0][1], N_021 = basisFuncsAndTheirDerivs[0][2]; // НУЛЕВЫЕ ПРОИЗВОДНЫЕ = сама функция
    double N_101 = basisFuncsAndTheirDerivs[1][0], N_111 = basisFuncsAndTheirDerivs[1][1], N_121 = basisFuncsAndTheirDerivs[1][2]; // ПЕРВЫЕ ПРОИЗВОДНЫЕ
    double N_201 = basisFuncsAndTheirDerivs[2][0], N_211 = basisFuncsAndTheirDerivs[2][1], N_221 = basisFuncsAndTheirDerivs[2][2]; // ВТОРЫЕ ПРОИЗВОДНЫЕ

    const double SIZE_MATRIX = 15; // Почему 15 - разобраться

    std::vector<std::vector<double>> coefficients(SIZE_MATRIX, std::vector<double>(SIZE_MATRIX));

    bool fixStartPoint = true;  // Фиксация первой граничной точки
    bool fixEndPoint = true;    // Фиксация последней граничной точки

    bool fixFirstDivStartPoint = false; // Фиксация первой производной первой граничной точки
    bool fixFirstDivEndPoint = false;   // Фиксация первой производной последней граничной точки

    const size_t EPSILON_COUNT = bezierCurves.size() * bezierCurves[0].getControlPoints().size();

    // Заполняем матрицу коэффициентами
    for (size_t i = 0; i != EPSILON_COUNT; ++i) // Заполняем двойками по главной диагонали
    {
        coefficients[i][i] = 2;
    }

   if (fixStartPoint == false)
   {
        coefficients[0][9] = N_001; coefficients[0][10] = N_101; coefficients[0][11] = N_201;
   }
   if (fixFirstDivStartPoint == false)
   {
        coefficients[1][9] = N_011; coefficients[1][10] = N_111; coefficients[1][11] = N_211;
   }

   coefficients[2][9] = N_021; coefficients[2][10] = N_121; coefficients[2][11] = N_221;

   coefficients[3][9] = -N_000; coefficients[3][10] = -N_100; coefficients[3][11] = -N_200; coefficients[3][12] = N_001; coefficients[3][13] = N_101; coefficients[3][14] = N_201;
   coefficients[4][9] = -N_010; coefficients[4][10] = -N_110; coefficients[4][11] = -N_210; coefficients[4][12] = N_011; coefficients[4][13] = N_111; coefficients[4][14] = N_211;
   coefficients[5][9] = -N_020; coefficients[5][10] = -N_120; coefficients[5][11] = -N_220; coefficients[5][12] = N_021; coefficients[5][13] = N_121; coefficients[5][14] = N_221;

   coefficients[6][12] = -N_000; coefficients[6][13] = -N_100; coefficients[6][14] = -N_200;

   if (fixFirstDivEndPoint == false)
   {
        coefficients[7][12] = -N_010; coefficients[7][13] = -N_110; coefficients[7][14] = -N_210;
   }
   if (fixEndPoint == false)
   {
        coefficients[8][12] = -N_020; coefficients[8][13] = -N_120; coefficients[8][14] = -N_220;
   }

   coefficients[9][0] = N_001; coefficients[9][1] = N_011; coefficients[9][2] = N_021; coefficients[9][3] = -N_000; coefficients[9][4] = -N_010; coefficients[9][5] = -N_020;
   coefficients[10][0] = N_101; coefficients[10][1] = N_111; coefficients[10][2] = N_121; coefficients[10][3] = -N_100; coefficients[10][4] = -N_110; coefficients[10][5] = -N_120;
   coefficients[11][0] = N_201; coefficients[11][1] = N_211; coefficients[11][2] = N_221; coefficients[11][3] = -N_200; coefficients[11][4] = -N_210; coefficients[11][5] = -N_220;

   coefficients[12][3] = N_001; coefficients[12][4] = N_011; coefficients[12][5] = N_021; coefficients[12][6] = -N_000; coefficients[12][7] = -N_010; coefficients[12][8] = -N_020;
   coefficients[13][3] = N_101; coefficients[13][4] = N_111; coefficients[13][5] = N_121; coefficients[13][6] = -N_100; coefficients[13][7] = -N_110; coefficients[13][8] = -N_120;
   coefficients[14][3] = N_201; coefficients[14][4] = N_211; coefficients[14][5] = N_221; coefficients[14][6] = -N_200; coefficients[14][7] = -N_210; coefficients[14][8] = -N_220;

   const size_t COUNT_BEZIER_CURVES = bezierCurves.size();
   std::vector<std::vector<Point3D>> controlPointsBezierCurves (COUNT_BEZIER_CURVES, std::vector<Point3D>());

   for (size_t i = 0; i != COUNT_BEZIER_CURVES; ++i)
   {
        controlPointsBezierCurves[i] = bezierCurves[i].getControlPoints();
   }

   std::vector<Point3D> freeMembers(SIZE_MATRIX);

   freeMembers[9] = controlPointsBezierCurves[0][0] * -N_001 - controlPointsBezierCurves[0][1] * N_011 - controlPointsBezierCurves[0][2] * N_021 + controlPointsBezierCurves[1][0] * N_000 + controlPointsBezierCurves[1][1] * N_010 + controlPointsBezierCurves[1][2] * N_020;
   freeMembers[10] = controlPointsBezierCurves[0][0] * -N_101 - controlPointsBezierCurves[0][1] * N_111 - controlPointsBezierCurves[0][2] * N_121 + controlPointsBezierCurves[1][0] * N_100 + controlPointsBezierCurves[1][1] * N_110 + controlPointsBezierCurves[1][2] * N_120;
   freeMembers[11] = controlPointsBezierCurves[0][0] * -N_201 - controlPointsBezierCurves[0][1] * N_211 - controlPointsBezierCurves[0][2] * N_221 + controlPointsBezierCurves[1][0] * N_200 + controlPointsBezierCurves[1][1] * N_210 + controlPointsBezierCurves[1][2] * N_220;

   freeMembers[12] = controlPointsBezierCurves[1][0] * -N_001 - controlPointsBezierCurves[1][1] * N_011 - controlPointsBezierCurves[1][2] * N_021 + controlPointsBezierCurves[2][0] * N_000 + controlPointsBezierCurves[2][1] * N_010 + controlPointsBezierCurves[2][2] * N_020;
   freeMembers[13] = controlPointsBezierCurves[1][0] * -N_101 - controlPointsBezierCurves[1][1] * N_111 - controlPointsBezierCurves[1][2] * N_121 + controlPointsBezierCurves[2][0] * N_100 + controlPointsBezierCurves[2][1] * N_110 + controlPointsBezierCurves[2][2] * N_120;
   freeMembers[14] = controlPointsBezierCurves[1][0] * -N_201 - controlPointsBezierCurves[1][1] * N_211 - controlPointsBezierCurves[1][2] * N_221 + controlPointsBezierCurves[2][0] * N_200 + controlPointsBezierCurves[2][1] * N_210 + controlPointsBezierCurves[2][2] * N_220;

   auto operation = IMatrixOperations::GetMatrixOperationsClass(OperationClass::eigen); // Создаём указатель на интерфейс операций СЛАУ

   if (operation == nullptr)
   {
        qDebug() << "Error! attachBSplines: oper == nullptr";;
        return {};
   }

   std::vector<Point3D> solution = operation->solveEquation(coefficients, freeMembers); // Решаем СЛАУ
   // int rankMatrixA = operation->getMatrixRank(coefficients);
   // double detMatrixA = operation->getMatrixDet(coefficients);

   int tempCounter = 0;

   // Регулируем котрольные точки Безье кривых для сопряжения
   for (size_t i = 0; i != COUNT_BEZIER_CURVES; ++i)
   {
        for (size_t j = 0; j != controlPointsBezierCurves[i].size(); ++j)
        {
            controlPointsBezierCurves[i][j] += solution[tempCounter++];
        }
   }

   std::vector<Curve> newBezierCurves;
   const std::vector<double> WEIGHTS(controlPointsBezierCurves[0].size(), 1);   // Весовые коэффициенты (у всех кривых Безье одинаковые)
   const int CURVE_NUM_POINTS = 61;   // Кол-во точек, из которых будет состоять кривая
   const int DEGREE = bezierCurves[0].getDegree(); // Степень кривой (у всех кривых Безье одинаковые)

   for (size_t i = 0; i != COUNT_BEZIER_CURVES; ++i) // Создаём новые кривые Безье и добавляем в вектор, чтобы функция возвратила его
   {
        Curve tempCurve(controlPointsBezierCurves[i], WEIGHTS, DEGREE, CURVE_NUM_POINTS);
        newBezierCurves.push_back(tempCurve);
   }

   return newBezierCurves;
}
