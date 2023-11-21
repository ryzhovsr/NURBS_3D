#include "MergeCurves.h"
#include "Utils/MathUtils.h"
#include "Utils/IMatrixOperations.h"
#include "Curve3D/CalcCurve.h"

Curve MergeCurves::attachCurves(const Curve &curve1, const Curve &curve2, bool fixateStartEndPoints)
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

std::vector<Curve> MergeCurves::attachBSplines(std::vector<Curve>& bezierCurves)
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

    std::vector<std::vector<double>> matrix_a3(SIZE_MATRIX, std::vector<double>(SIZE_MATRIX));
    std::vector<Point3D> matrix_b3(SIZE_MATRIX);
    std::vector<std::vector<double>> x4(SIZE_MATRIX, std::vector<double>(2));
    std::vector<std::vector<double>> x4_1(SIZE_MATRIX, std::vector<double>(2));

    bool n00 = true; // "=1" == Первая (левая) Гранич. Точка -- ЗАКРЕПЛЕНА ТОЧКА
    bool n10 = false; // "=1" == Первая (левая) Гранич. Точка -- ЗАКРЕПЛЕНА ПЕРВАЯ ПРОИЗВОДНАЯ

    bool n02 = true; // "=1" == Последняя (правая) Гранич. Точка -- ЗАКРЕПЛЕНА ТОЧКА
    bool n12 = false; // "=1" == Последняя (правая) Гранич. Точка -- ЗАКРЕПЛЕНА ПЕРВАЯ ПРОИЗВОДНАЯ

   // Заполняем матрицу коэффициентами
   matrix_a3[0][0] = 2; matrix_a3[0][9] = N_001; matrix_a3[0][10] = N_101; matrix_a3[0][11] = N_201;

   if (n00 == true)
   {
        matrix_a3[0][9] = 0; matrix_a3[0][10] = 0; matrix_a3[0][11] = 0;
   }

   matrix_a3[1][1] = 2; matrix_a3[1][9] = N_011; matrix_a3[1][10] = N_111; matrix_a3[1][11] = N_211;

   if (n10 == true)
   {
        matrix_a3[1][9] = 0; matrix_a3[1][10] = 0; matrix_a3[1][11] = 0;
   }

   matrix_a3[2][2] = 2; matrix_a3[2][9] = N_021; matrix_a3[2][10] = N_121; matrix_a3[2][11] = N_221;

   matrix_a3[3][3] = 2; matrix_a3[3][9] = -N_000; matrix_a3[3][10] = -N_100; matrix_a3[3][11] = -N_200; matrix_a3[3][12] = N_001; matrix_a3[3][13] = N_101; matrix_a3[3][14] = N_201;
   matrix_a3[4][4] = 2; matrix_a3[4][9] = -N_010; matrix_a3[4][10] = -N_110; matrix_a3[4][11] = -N_210; matrix_a3[4][12] = N_011; matrix_a3[4][13] = N_111; matrix_a3[4][14] = N_211;
   matrix_a3[5][5] = 2; matrix_a3[5][9] = -N_020; matrix_a3[5][10] = -N_120; matrix_a3[5][11] = -N_220; matrix_a3[5][12] = N_021; matrix_a3[5][13] = N_121; matrix_a3[5][14] = N_221;

   matrix_a3[6][6] = 2; matrix_a3[6][12] = -N_000; matrix_a3[6][13] = -N_100; matrix_a3[6][14] = -N_200;
   matrix_a3[7][7] = 2; matrix_a3[7][12] = -N_010; matrix_a3[7][13] = -N_110; matrix_a3[7][14] = -N_210;

   if (n12 == true)
   {
        matrix_a3[7][12] = 0; matrix_a3[7][13] = 0; matrix_a3[7][14] = 0;
   }

   matrix_a3[8][8] = 2; matrix_a3[8][11] = -N_020; matrix_a3[8][12] = -N_120; matrix_a3[8][14] = -N_220;

   if (n02 == true)
   {
        matrix_a3[8][12] = 0; matrix_a3[8][13] = 0; matrix_a3[8][14] = 0;
   }

   matrix_a3[9][0] = N_001; matrix_a3[9][1] = N_011; matrix_a3[9][2] = N_021; matrix_a3[9][3] = -N_000; matrix_a3[9][4] = -N_010; matrix_a3[9][5] = -N_020;
   matrix_a3[10][0] = N_101; matrix_a3[10][1] = N_111; matrix_a3[10][2] = N_121; matrix_a3[10][3] = -N_100; matrix_a3[10][4] = -N_110; matrix_a3[10][5] = -N_120;
   matrix_a3[11][0] = N_201; matrix_a3[11][1] = N_211; matrix_a3[11][2] = N_221; matrix_a3[11][3] = -N_200; matrix_a3[11][4] = -N_210; matrix_a3[11][5] = -N_220;

   matrix_a3[12][3] = N_001; matrix_a3[12][4] = N_011; matrix_a3[12][5] = N_021; matrix_a3[12][6] = -N_000; matrix_a3[12][7] = -N_010; matrix_a3[12][8] = -N_020;
   matrix_a3[13][3] = N_101; matrix_a3[13][4] = N_111; matrix_a3[13][5] = N_121; matrix_a3[13][6] = -N_100; matrix_a3[13][7] = -N_110; matrix_a3[13][8] = -N_120;
   matrix_a3[14][3] = N_201; matrix_a3[14][4] = N_211; matrix_a3[14][5] = N_221; matrix_a3[14][6] = -N_200; matrix_a3[14][7] = -N_210; matrix_a3[14][8] = -N_220;

   const int COUNT_BSPLINES = static_cast<int>(bezierCurves.size());
   std::vector<std::vector<Point3D>> controlPointsBSplines (COUNT_BSPLINES, std::vector<Point3D>());

   for (int i = 0; i != COUNT_BSPLINES; ++i)
   {
        controlPointsBSplines[i] = bezierCurves[i].getControlPoints();
   }

   matrix_b3[9] = controlPointsBSplines[0][0] * -N_001 - controlPointsBSplines[0][1] * N_011 - controlPointsBSplines[0][2] * N_021 + controlPointsBSplines[1][0] * N_000 + controlPointsBSplines[1][1] * N_010 + controlPointsBSplines[1][2] * N_020;
   matrix_b3[10] = controlPointsBSplines[0][0] * -N_101 - controlPointsBSplines[0][1] * N_111 - controlPointsBSplines[0][2] * N_121 + controlPointsBSplines[1][0] * N_100 + controlPointsBSplines[1][1] * N_110 + controlPointsBSplines[1][2] * N_120;
   matrix_b3[11] = controlPointsBSplines[0][0] * -N_201 - controlPointsBSplines[0][1] * N_211 - controlPointsBSplines[0][2] * N_221 + controlPointsBSplines[1][0] * N_200 + controlPointsBSplines[1][1] * N_210 + controlPointsBSplines[1][2] * N_220;

   matrix_b3[12] = controlPointsBSplines[1][0] * -N_001 - controlPointsBSplines[1][1] * N_011 - controlPointsBSplines[1][2] * N_021 + controlPointsBSplines[2][0] * N_000 + controlPointsBSplines[2][1] * N_010 + controlPointsBSplines[2][2] * N_020;
   matrix_b3[13] = controlPointsBSplines[1][0] * -N_101 - controlPointsBSplines[1][1] * N_111 - controlPointsBSplines[1][2] * N_121 + controlPointsBSplines[2][0] * N_100 + controlPointsBSplines[2][1] * N_110 + controlPointsBSplines[2][2] * N_120;
   matrix_b3[14] = controlPointsBSplines[1][0] * -N_201 - controlPointsBSplines[1][1] * N_211 - controlPointsBSplines[1][2] * N_221 + controlPointsBSplines[2][0] * N_200 + controlPointsBSplines[2][1] * N_210 + controlPointsBSplines[2][2] * N_220;

   auto operation = IMatrixOperations::GetMatrixOperationsClass(OperationClass::eigen); // Создаём указатель на интерфейс операций СЛАУ

   if (operation == nullptr)
   {
        qDebug() << "Error! attachBSplines: oper == nullptr";;
        return {};
   }

   std::vector<Point3D> ans = operation->solveEquation(matrix_a3, matrix_b3); // Решаем СЛАУ
   // int rankMatrixA = operation->getMatrixRank(matrix_a3);
   // double detMatrixA = operation->getMatrixDet(matrix_a3);

   int tempCounter = 0;

   for (int i = 0; i != COUNT_BSPLINES; ++i)
   {
        for (size_t j = 0; j != controlPointsBSplines[i].size(); ++j)
        {
            controlPointsBSplines[i][j] += ans[tempCounter++];
        }
   }

   std::vector<Curve> newBsplines;
   const std::vector<double> weightsBezierCurves(controlPointsBSplines[0].size(), 1);   // Весовые коэффициенты контрольных точек

   for (int i = 0; i != COUNT_BSPLINES; ++i)
   {
        Curve temp1(controlPointsBSplines[i], weightsBezierCurves, bezierCurves[0].getDegree(), 61);
        newBsplines.push_back(temp1);
   }

   return newBsplines;
}

/*
std::vector<Curve> MergeCurves::attachBSplines(std::vector<Curve>& bezierCurves)
{
   // Находим базисные функции у первой кривой в начальной точке
   double parameter = 0;
   int span = CalcCurve::findSpanForParameter(parameter, bezierCurves[0].getNodalVector(), bezierCurves[0].getDegree());
   std::vector<std::vector<double>> basisFuncsAndTheirDerivs = CalcCurve::calcBasisFuncsAndTheirDerivs(bezierCurves[0].getNodalVector(), parameter, span, bezierCurves[0].getDegree());

   double N_000 = basisFuncsAndTheirDerivs[0][0], N_010 = basisFuncsAndTheirDerivs[0][1], N_020 = basisFuncsAndTheirDerivs[0][2]; // НУЛЕВЫЕ ПРОИЗВОДНЫЕ = сама функция
   double N_100 = basisFuncsAndTheirDerivs[1][0], N_110 = basisFuncsAndTheirDerivs[1][1], N_120 = basisFuncsAndTheirDerivs[1][2]; // ПЕРВЫЕ ПРОИЗВОДНЫЕ
   //double N_200 = basisFuncsAndTheirDerivs[2][0], N_210 = basisFuncsAndTheirDerivs[2][1], N_220 = basisFuncsAndTheirDerivs[2][2]; // ВТОРЫЕ ПРОИЗВОДНЫЕ

   // Находим базисные функции у первой кривой в конечной точке
   parameter = 1;
   span = CalcCurve::findSpanForParameter(parameter, bezierCurves[0].getNodalVector(), bezierCurves[0].getDegree());
   basisFuncsAndTheirDerivs = CalcCurve::calcBasisFuncsAndTheirDerivs(bezierCurves[0].getNodalVector(), parameter, span, bezierCurves[0].getDegree());

   double N_001 = basisFuncsAndTheirDerivs[0][0], N_011 = basisFuncsAndTheirDerivs[0][1], N_021 = basisFuncsAndTheirDerivs[0][2]; // НУЛЕВЫЕ ПРОИЗВОДНЫЕ = сама функция
   double N_101 = basisFuncsAndTheirDerivs[1][0], N_111 = basisFuncsAndTheirDerivs[1][1], N_121 = basisFuncsAndTheirDerivs[1][2]; // ПЕРВЫЕ ПРОИЗВОДНЫЕ
   //double N_201 = basisFuncsAndTheirDerivs[2][0], N_211 = basisFuncsAndTheirDerivs[2][1], N_221 = basisFuncsAndTheirDerivs[2][2]; // ВТОРЫЕ ПРОИЗВОДНЫЕ

   std::vector<std::vector<double>> matrix_a3(13, std::vector<double>(13));
   std::vector<std::vector<double>> matrix_b3(13, std::vector<double>(2));
   std::vector<std::vector<double>> x4(13, std::vector<double>(2));
   std::vector<std::vector<double>> x4_1(13, std::vector<double>(2));

   double n00 = 1; // "=1" == Первая (левая) Гранич. Точка -- ЗАКРЕПЛЕНА ТОЧКА
   double n10 = 1; // "=1" == Первая (левая) Гранич. Точка -- ЗАКРЕПЛЕНА ПЕРВАЯ ПРОИЗВОДНАЯ
   double n02 = 1; // "=1" == Последняя (правая) Гранич. Точка -- ЗАКРЕПЛЕНА ТОЧКА
   double n12 = 1; // "=1" == Последняя (правая) Гранич. Точка -- ЗАКРЕПЛЕНА ПЕРВАЯ ПРОИЗВОДНАЯ

   matrix_a3[0][0] = 2; matrix_a3[0][9] = N_001; matrix_a3[0][10] = N_101;

   if (n00 == 1)
   {
        matrix_a3[0][9] = 0; matrix_a3[0][10] = 0;
   }

   matrix_a3[1][1] = 2; matrix_a3[1][9] = N_011; matrix_a3[1][10] = N_111;

   if (n10==1)
   {
        matrix_a3[1][9] = 0; matrix_a3[1][10] = 0;
   }

   matrix_a3[2][2] = 2; matrix_a3[2][9] = N_021; matrix_a3[2][10] = N_121;

   matrix_a3[3][3] = 2; matrix_a3[3][9] = -N_000; matrix_a3[3][10] = -N_100; matrix_a3[3][11] = N_001; matrix_a3[3][12] = N_101;
   matrix_a3[4][4] = 2; matrix_a3[4][9] = -N_010; matrix_a3[4][10] = -N_110; matrix_a3[4][11] = N_011; matrix_a3[4][12] = N_111;
   matrix_a3[5][5] = 2; matrix_a3[5][9] = -N_020; matrix_a3[5][10] = -N_120; matrix_a3[5][11] = N_021; matrix_a3[5][12] = N_121;

   matrix_a3[6][6] = 2; matrix_a3[6][11] = -N_000; matrix_a3[6][12] = -N_100;
   matrix_a3[7][7] = 2; matrix_a3[7][11] = -N_010; matrix_a3[7][12] = -N_110;

   if (n12 == 1)
   {
        matrix_a3[7][12] = 0;
   }

   matrix_a3[8][8] = 2; matrix_a3[8][11] = -N_020; matrix_a3[8][12] = -N_120;

   if (n02 == 1)
   {
        matrix_a3[8][12] = 0;
   }

   matrix_a3[9][0] = N_001; matrix_a3[9][1] = N_011; matrix_a3[9][2] = N_021; matrix_a3[9][3] = -N_000; matrix_a3[9][4] = -N_010; matrix_a3[9][5] = -N_020;
   matrix_a3[10][0] = N_101; matrix_a3[10][1] = N_111; matrix_a3[10][2] = N_121; matrix_a3[10][3] = -N_100; matrix_a3[10][4] = -N_110; matrix_a3[10][5] = -N_120;

   matrix_a3[11][3] = N_001; matrix_a3[11][4] = N_011; matrix_a3[11][5] = N_021; matrix_a3[11][6] = -N_000; matrix_a3[11][7] = -N_010; matrix_a3[11][8] = -N_020;
   matrix_a3[12][3] = N_101; matrix_a3[12][4] = N_111; matrix_a3[12][5] = N_121; matrix_a3[12][6] = -N_100; matrix_a3[12][7] = -N_110; matrix_a3[12][8] = -N_120;

   std::vector<Point3D> contrPointsBsp_1 = bezierCurves[0].getControlPoints();
   std::vector<Point3D> contrPointsBsp_2 = bezierCurves[1].getControlPoints();
   std::vector<Point3D> contrPointsBsp_3 = bezierCurves[2].getControlPoints();

   // По своей сути это точка (координату z - не присваиваем) потом можно расширить вектор matrix_b3 до 3 элементов
   Point3D p1 = contrPointsBsp_1[0] * -N_001 - contrPointsBsp_1[1] * N_011 - contrPointsBsp_1[2] * N_021 + contrPointsBsp_2[0] * N_000 + contrPointsBsp_2[1] * N_010 + contrPointsBsp_2[2] * N_020;
   matrix_b3[9][0] = p1.x;
   matrix_b3[9][1] = p1.y;

   Point3D p2 = contrPointsBsp_1[0] * -N_101 - contrPointsBsp_1[1] * N_111 - contrPointsBsp_1[2] * N_121 + contrPointsBsp_2[0] * N_100 + contrPointsBsp_2[1] * N_110 + contrPointsBsp_2[2] * N_120;
   matrix_b3[10][0] = p2.x;
   matrix_b3[10][1] = p2.y;

   Point3D p3 = contrPointsBsp_2[0] * -N_001 - contrPointsBsp_2[1] * N_011 - contrPointsBsp_2[2] * N_021 + contrPointsBsp_3[0] * N_000 + contrPointsBsp_3[1] * N_010 + contrPointsBsp_3[2] * N_020;
   matrix_b3[11][0] = p3.x;
   matrix_b3[11][1] = p3.y;

   Point3D p4 = contrPointsBsp_2[0] * -N_101 - contrPointsBsp_2[1] * N_111 - contrPointsBsp_2[2] * N_121 + contrPointsBsp_3[0] * N_100 + contrPointsBsp_3[1] * N_110 + contrPointsBsp_3[2] * N_120;
   matrix_b3[12][1] = p4.y;

   auto operation = IMatrixOperations::GetMatrixOperationsClass(OperationClass::eigen); // Создаём указатель на интерфейс операций СЛАУ

   if (operation == nullptr)
   {
        qDebug() << "Error! attachBSplines: oper == nullptr";;
        return {};
   }

   auto ans = operation->solveEquation(matrix_a3, matrix_b3); // Решаем СЛАУ и получаем координаты новых точек кривой

   for (int i = 0; i < 3; ++i)
   {
        contrPointsBsp_1[i].x += ans[i][0];
        contrPointsBsp_1[i].y += ans[i][1];

        contrPointsBsp_2[i].x += ans[3 + i][0];
        contrPointsBsp_2[i].y += ans[3 + i][1];

        contrPointsBsp_3[i].x += ans[6 + i][0];
        contrPointsBsp_3[i].y += ans[6 + i][1];
   }

   std::vector<Curve> newBsplines;
   const std::vector<double> weightsBezierCurves(contrPointsBsp_1.size(), 1);   // Весовые коэффициенты контрольных точек

   Curve temp1(contrPointsBsp_1, weightsBezierCurves, bezierCurves[0].getDegree(), 61);
   newBsplines.push_back(temp1);

   Curve temp2(contrPointsBsp_2, weightsBezierCurves, bezierCurves[0].getDegree(), 61);
   newBsplines.push_back(temp2);

   Curve temp3(contrPointsBsp_3, weightsBezierCurves, bezierCurves[0].getDegree(), 61);
   newBsplines.push_back(temp3);

   return newBsplines;
}
*/

Point3D MergeCurves::calcDerivLeftBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex)
{
    if(startIndex == currentIndex)
        return points[currentIndex];
    else
        return calcDerivLeftBezierCurveForMerger(points, currentIndex + 1, startIndex) - calcDerivLeftBezierCurveForMerger(points, currentIndex, startIndex - 1);
}

Point3D MergeCurves::calcNegativeDerivLeftBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex)
{
    if(startIndex == currentIndex)
        return -1 * points[currentIndex];
    else
        return calcNegativeDerivLeftBezierCurveForMerger(points, currentIndex + 1, startIndex)  - calcNegativeDerivLeftBezierCurveForMerger(points, currentIndex, startIndex - 1);
}

Point3D MergeCurves::calcDerivRightBezierCurveForMerger(const std::vector<Point3D>& points, int currentIndex, int startIndex)
{
    if(startIndex == currentIndex)
        return points[currentIndex];
    else
        return calcDerivRightBezierCurveForMerger(points, currentIndex, startIndex + 1) - calcDerivRightBezierCurveForMerger(points, currentIndex - 1, startIndex);
}
