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
    // Находим базисные функции у первой кривой в конечной точке (базисные функции у всех кривых одинаковые)
    double parameter = 1;
    int span = CalcCurve::findSpanForParameter(parameter, bezierCurves[0].getNodalVector(), bezierCurves[0].getDegree());
    std::vector<std::vector<double>> basisFuncsAndTheirDerivs = CalcCurve::calcBasisFuncsAndTheirDerivs(bezierCurves[0].getNodalVector(), parameter, span, bezierCurves[0].getDegree());

    const size_t NUMBER_BASIS_FUNCS = basisFuncsAndTheirDerivs[0].size();                               // Количество базисных функций
    const size_t NUMBER_BEZIER_CURVES = bezierCurves.size();                                            // Количество Безье кривых
    const size_t NUMBER_EPSILONS = bezierCurves.size() * bezierCurves[0].getControlPoints().size();     // Количество эпсилон для СЛАУ
    const double MATRIX_SIZE = NUMBER_BASIS_FUNCS * (NUMBER_BEZIER_CURVES + NUMBER_BEZIER_CURVES - 1);  // Размер матрицы коэффициентов

    std::vector<std::vector<double>> coefficients(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));       // Матрица коэффициентов

    // Заполняем матрицу коэффициентами
    for (size_t i = 0; i != NUMBER_EPSILONS; ++i) // Заполняем двойками по главной диагонали
    {
        coefficients[i][i] = 2;
    }

    size_t reverseRow = basisFuncsAndTheirDerivs[0].size() * 2 - 1;
    size_t colBasisFunc = 0;

    for (size_t row = 0; row != basisFuncsAndTheirDerivs.size(); ++row) // Итерируемся по общему числу базисных функций
    {
        size_t rowBasisFunc = 0;
        double prevBasisFuncVal = basisFuncsAndTheirDerivs[rowBasisFunc][colBasisFunc];

        for (size_t col = NUMBER_EPSILONS; col != NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size(); ++col) // Итерируемся по общему числу производных базисных функций
        {
            double basisFuncVal = basisFuncsAndTheirDerivs[rowBasisFunc][colBasisFunc];
            coefficients[row][col] = basisFuncVal;

            if (basisFuncVal != 0)
            {
                if (prevBasisFuncVal < 0 && basisFuncVal < 0) // Если предыдущий был отрицательным и следующий тоже отрицательный
                {
                    basisFuncVal *= -1;
                }
                else if (prevBasisFuncVal > 0 && basisFuncVal > 0)
                {
                    basisFuncVal *= -1;
                }
                else if (prevBasisFuncVal == 0 && basisFuncVal > 0)
                {
                    basisFuncVal *= -1;
                }
            }

            coefficients[reverseRow][col] = basisFuncVal;
            prevBasisFuncVal = basisFuncVal;

            ++rowBasisFunc;
        }

        --reverseRow;
        ++colBasisFunc;
    }

    reverseRow = basisFuncsAndTheirDerivs[0].size() * 3 - 1;
    colBasisFunc = 0;

    for (size_t row = basisFuncsAndTheirDerivs.size(); row != basisFuncsAndTheirDerivs.size() * 2; ++row) // Итерируемся по общему числу базисных функций
    {
        size_t rowBasisFunc = 0;
        double prevBasisFuncVal = basisFuncsAndTheirDerivs[rowBasisFunc][colBasisFunc];

        for (size_t col = NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size(); col != NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size() * 2; ++col) // Итерируемся по общему числу производных базисных функций
        {
            double basisFuncVal = basisFuncsAndTheirDerivs[rowBasisFunc][colBasisFunc];
            coefficients[row][col] = basisFuncVal;

            if (basisFuncVal != 0)
            {
                if (prevBasisFuncVal < 0 && basisFuncVal < 0) // Если предыдущий был отрицательным и следующий тоже отрицательный
                {
                    basisFuncVal *= -1;
                }
                else if (prevBasisFuncVal > 0 && basisFuncVal > 0)
                {
                    basisFuncVal *= -1;
                }
                else if (prevBasisFuncVal == 0 && basisFuncVal > 0)
                {
                    basisFuncVal *= -1;
                }
            }

            prevBasisFuncVal = basisFuncVal;
            coefficients[reverseRow][col] = basisFuncVal;

            ++rowBasisFunc;
        }

        --reverseRow;
        ++colBasisFunc;
    }

    size_t rowBasisFunc = 0;

    for (size_t row = NUMBER_EPSILONS; row != NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size(); ++row) // Итерируемся по общему числу базисных функций
    {
        size_t reverseCol = basisFuncsAndTheirDerivs[0].size() * 2 - 1;
        size_t colBasisFunc = 0;
        double prevBasisFuncVal = basisFuncsAndTheirDerivs[rowBasisFunc][colBasisFunc];

        for (size_t col = 0; col !=  basisFuncsAndTheirDerivs.size(); ++col) // Итерируемся по общему числу производных базисных функций
        {
            double basisFuncVal = basisFuncsAndTheirDerivs[rowBasisFunc][colBasisFunc];
            coefficients[row][col] = basisFuncVal;

            if (basisFuncVal != 0) ///////////////////////////////////// COL != 0 УБРАТЬ ВОЗМОЖНО ОШИБКА
            {
                if (prevBasisFuncVal < 0 && basisFuncVal < 0 && col != 0) // Если предыдущий был отрицательным и следующий тоже отрицательный
                {
                    basisFuncVal *= -1;
                }
                else if (prevBasisFuncVal > 0 && basisFuncVal > 0)
                {
                    basisFuncVal *= -1;
                }
                else if (prevBasisFuncVal == 0 && basisFuncVal > 0)
                {
                    basisFuncVal *= -1;
                }
            }

            prevBasisFuncVal = basisFuncVal;
            coefficients[row][reverseCol] = basisFuncVal;

            ++colBasisFunc;
            --reverseCol;
        }

        ++rowBasisFunc;
    }

    rowBasisFunc = 0;

    for (size_t row = NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size(); row != NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size() * 2; ++row) // Итерируемся по общему числу базисных функций
    {
        size_t reverseCol = basisFuncsAndTheirDerivs[0].size() * 3 - 1;
        size_t colBasisFunc = 0;
        double prevBasisFuncVal = basisFuncsAndTheirDerivs[rowBasisFunc][colBasisFunc];

        for (size_t col = basisFuncsAndTheirDerivs[0].size(); col !=  basisFuncsAndTheirDerivs.size() * 2; ++col) // Итерируемся по общему числу производных базисных функций
        {
            double basisFuncVal = basisFuncsAndTheirDerivs[rowBasisFunc][colBasisFunc];
            coefficients[row][col] = basisFuncVal;

            if (basisFuncVal != 0) ///////////////////////////////////// COL != 4 УБРАТЬ ВОЗМОЖНО ОШИБКА
            {
                if (prevBasisFuncVal < 0 && basisFuncVal < 0 && col != basisFuncsAndTheirDerivs[0].size()) // Если предыдущий был отрицательным и следующий тоже отрицательный и колонка не равна началу
                {
                    basisFuncVal *= -1;
                }
                else if (prevBasisFuncVal > 0 && basisFuncVal > 0)
                {
                    basisFuncVal *= -1;
                }
                else if (prevBasisFuncVal == 0 && basisFuncVal > 0)
                {
                    basisFuncVal *= -1;
                }
            }

            prevBasisFuncVal = basisFuncVal;
            coefficients[row][reverseCol] = basisFuncVal;

            ++colBasisFunc;
            --reverseCol;
        }

        ++rowBasisFunc;
    }

    bool fixStartPoint = false;  // Фиксация первой граничной точки
    bool fixEndPoint = false;    // Фиксация последней граничной точки
    bool fixFirstDivStartPoint = false; // Фиксация первой производной первой граничной точки
    bool fixFirstDivEndPoint = false;   // Фиксация первой производной последней граничной точки

    if (fixStartPoint) // Фиксация первой граничной точки
    {
        for (size_t i = NUMBER_EPSILONS; i != NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size(); ++i)
        {
            coefficients[0][i] = 0;
        }
    }

    if (fixFirstDivStartPoint) // Фиксация первой производной первой граничной точки
    {
        for (size_t i = NUMBER_EPSILONS; i != NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size(); ++i)
        {
            coefficients[1][i] = 0;
        }
    }

    if (fixFirstDivEndPoint) // Фиксация первой производной последней граничной точки
    {
        for (size_t col = NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size(); col != coefficients[0].size(); ++col)
        {
            coefficients[NUMBER_EPSILONS - 2][col] = 0;
        }
    }

    if (fixEndPoint) // Фиксация последней граничной точки
    {
        for (size_t col = NUMBER_EPSILONS + basisFuncsAndTheirDerivs[0].size(); col != coefficients[0].size(); ++col)
        {
            coefficients[NUMBER_EPSILONS - 1][col] = 0;
        }
    }

   std::vector<std::vector<Point3D>> controlPointsBezierCurves (NUMBER_BEZIER_CURVES, std::vector<Point3D>());

   for (size_t i = 0; i != NUMBER_BEZIER_CURVES; ++i)
   {
        controlPointsBezierCurves[i] = bezierCurves[i].getControlPoints();
   }

   std::vector<Point3D> freeMembers(MATRIX_SIZE);
   size_t indexFreeMembers = NUMBER_EPSILONS;

   parameter = 0;
   span = CalcCurve::findSpanForParameter(parameter, bezierCurves[0].getNodalVector(), bezierCurves[0].getDegree());
   std::vector<std::vector<double>> basisFuncsAndTheirDerivsRev = CalcCurve::calcBasisFuncsAndTheirDerivs(bezierCurves[0].getNodalVector(), parameter, span, bezierCurves[0].getDegree());

   for (size_t row = 0; row != controlPointsBezierCurves.size() - 1; ++row)
   {
        size_t rowBasisFunc = 0;

        for (size_t col = 0; col != basisFuncsAndTheirDerivs[0].size(); ++col)
        {
            for (size_t i = 0; i != controlPointsBezierCurves[0].size(); ++i)
            {
                freeMembers[indexFreeMembers] += controlPointsBezierCurves[row][i] * -basisFuncsAndTheirDerivs[rowBasisFunc][i]; // Текущая кривая
                freeMembers[indexFreeMembers] += controlPointsBezierCurves[row + 1][i] * basisFuncsAndTheirDerivsRev[rowBasisFunc][i]; // След. кривая
            }

            ++rowBasisFunc;
            ++indexFreeMembers;
        }
   }

   //freeMembers[9] = controlPointsBezierCurves[0][0] * -N_001 + controlPointsBezierCurves[0][1] * -N_011 + controlPointsBezierCurves[0][2] * -N_021 + controlPointsBezierCurves[1][0] * N_000 + controlPointsBezierCurves[1][1] * N_010 + controlPointsBezierCurves[1][2] * N_020;
   //freeMembers[10] = controlPointsBezierCurves[0][0] * -N_101 + controlPointsBezierCurves[0][1] * -N_111 + controlPointsBezierCurves[0][2] * -N_121 + controlPointsBezierCurves[1][0] * N_100 + controlPointsBezierCurves[1][1] * N_110 + controlPointsBezierCurves[1][2] * N_120;
   //freeMembers[11] = controlPointsBezierCurves[0][0] * -N_201 + controlPointsBezierCurves[0][1] * -N_211 + controlPointsBezierCurves[0][2] * -N_221 + controlPointsBezierCurves[1][0] * N_200 + controlPointsBezierCurves[1][1] * N_210 + controlPointsBezierCurves[1][2] * N_220;

   //freeMembers[12] = controlPointsBezierCurves[1][0] * -N_001 + controlPointsBezierCurves[1][1] * -N_011 + controlPointsBezierCurves[1][2] * -N_021 + controlPointsBezierCurves[2][0] * N_000 + controlPointsBezierCurves[2][1] * N_010 + controlPointsBezierCurves[2][2] * N_020;
   //freeMembers[13] = controlPointsBezierCurves[1][0] * -N_101 + controlPointsBezierCurves[1][1] * -N_111 + controlPointsBezierCurves[1][2] * -N_121 + controlPointsBezierCurves[2][0] * N_100 + controlPointsBezierCurves[2][1] * N_110 + controlPointsBezierCurves[2][2] * N_120;
   //freeMembers[14] = controlPointsBezierCurves[1][0] * -N_201 + controlPointsBezierCurves[1][1] * -N_211 + controlPointsBezierCurves[1][2] * -N_221 + controlPointsBezierCurves[2][0] * N_200 + controlPointsBezierCurves[2][1] * N_210 + controlPointsBezierCurves[2][2] * N_220;

   auto operation = IMatrixOperations::GetMatrixOperationsClass(OperationClass::eigen); // Создаём указатель на интерфейс операций СЛАУ

   if (operation == nullptr)
   {
        qDebug() << "Error! attachBSplines: oper == nullptr";;
        return {};
   }

   std::vector<Point3D> solution = operation->solveEquation(coefficients, freeMembers); // Решаем СЛАУ
   int rankMatrixA = operation->getMatrixRank(coefficients);
   double detMatrixA = operation->getMatrixDet(coefficients);

   qDebug() << "Coefficients matrix info:" <<
           "\nRank = " << rankMatrixA <<
            "\nDet = " << detMatrixA;

   int tempCounter = 0;

   // Регулируем котрольные точки Безье кривых для сопряжения
   for (size_t i = 0; i != NUMBER_BEZIER_CURVES; ++i)
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

   for (size_t i = 0; i != NUMBER_BEZIER_CURVES; ++i) // Создаём новые кривые Безье и добавляем в вектор, чтобы функция возвратила его
   {
        Curve tempCurve(controlPointsBezierCurves[i], WEIGHTS, DEGREE, CURVE_NUM_POINTS);
        newBezierCurves.push_back(tempCurve);
   }

   return newBezierCurves;
}
