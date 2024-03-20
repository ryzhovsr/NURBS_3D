#include "MergeCurves.h"
#include "Utils/MathUtils.h"
#include "Utils/IMatrixOperations.h"
#include "Curve3D/CalcCurve.h"
#include "usefulutils.h"

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

// Пространсто со вспомогательным функциями сопряжения кривой для attachAllBezierCurves
namespace Impl
{
    // Заполняет элементы матрицы коээфициентов над её главной диагональю
    void fillUpperTriangularMatrix(std::vector<std::vector<double>>& coefficientMatrix, std::vector<std::vector<double>>& basisFuncs, size_t numberEpsilons, size_t numberBreakPoints)
    {
        // Количество базисных функций
        const size_t NUMBER_BASIS_FUNCS = basisFuncs.size();

        // Каждый breakPoint - одна итерация заполнения базисных функций в coefficientMatrix
        for (size_t breakPointsCounter = 0; breakPointsCounter != numberBreakPoints; ++breakPointsCounter)
        {
            // Реверс строка для противоположной сторны треугольника
            size_t reverseRow = NUMBER_BASIS_FUNCS * 2 - 1 + NUMBER_BASIS_FUNCS * breakPointsCounter;
            size_t colBasisFunc = 0;

            // Итерируемся по общему числу базисных функций
            for (size_t row = 0 + NUMBER_BASIS_FUNCS * breakPointsCounter; row != NUMBER_BASIS_FUNCS + NUMBER_BASIS_FUNCS * breakPointsCounter; ++row)
            {
                // Строка базисных функций
                size_t rowBasisFunc = 0;
                // Предыдущее значение базисной функции (для правильного заполнения коэффициентов с нужным знаком "+" или "-")
                double prevBasisFuncVal = basisFuncs[rowBasisFunc][colBasisFunc];

                for (size_t col = numberEpsilons + NUMBER_BASIS_FUNCS * breakPointsCounter; col != numberEpsilons + NUMBER_BASIS_FUNCS + NUMBER_BASIS_FUNCS * breakPointsCounter; ++col)
                {
                    double nextBasisFuncVal = basisFuncs[rowBasisFunc][colBasisFunc];
                    coefficientMatrix[row][col] = nextBasisFuncVal;

                    // Регулируем знак у противоположной части базисных функций
                    if (prevBasisFuncVal < 0 && nextBasisFuncVal < 0)
                    {
                        nextBasisFuncVal *= -1;
                    }
                    else if (prevBasisFuncVal >= 0 && nextBasisFuncVal > 0)
                    {
                        nextBasisFuncVal *= -1;
                    }

                    coefficientMatrix[reverseRow][col] = nextBasisFuncVal;
                    prevBasisFuncVal = nextBasisFuncVal;
                    ++rowBasisFunc;
                }

                --reverseRow;
                ++colBasisFunc;
            }
        }
    }

    // Заполняет элементы матрицы коээфициентов под её главной диагональю
    void fillLowerTriangularMatrix(std::vector<std::vector<double>>& coefficientMatrix, std::vector<std::vector<double>>& basisFuncs, size_t numberEpsilons, size_t numberBreakPoints)
    {
        // Количество базисных функций
        const size_t NUMBER_BASIS_FUNCS = basisFuncs.size();

        // Каждый breakPoint - одна итерация заполнения базисных функций в coefficientMatrix
        for (size_t breakPointCounter = 0; breakPointCounter != numberBreakPoints; ++breakPointCounter)
        {
            size_t rowBasisFunc = 0;

            // Итерируемся по общему числу базисных функций
            for (size_t row = numberEpsilons + NUMBER_BASIS_FUNCS * breakPointCounter; row != numberEpsilons + NUMBER_BASIS_FUNCS + NUMBER_BASIS_FUNCS * breakPointCounter; ++row)
            {
                // Реверс столбец для противоположной части
                size_t reverseCol = NUMBER_BASIS_FUNCS * 2 - 1 + NUMBER_BASIS_FUNCS * breakPointCounter;
                size_t colBasisFunc = 0;
                // Предыдущее значение базисной функции (для правильного заполнения коэффициентов с нужным знаком "+" или "-")
                double prevBasisFuncVal = basisFuncs[rowBasisFunc][colBasisFunc];

                for (size_t col = 0 + NUMBER_BASIS_FUNCS * breakPointCounter; col != NUMBER_BASIS_FUNCS + NUMBER_BASIS_FUNCS * breakPointCounter; ++col)
                {
                    double nextBasisFuncVal = basisFuncs[rowBasisFunc][colBasisFunc];
                    coefficientMatrix[row][col] = nextBasisFuncVal;

                    // Регулируем знак у противоположной части базисных функций
                    if (prevBasisFuncVal < 0 && nextBasisFuncVal < 0 && col != NUMBER_BASIS_FUNCS * breakPointCounter)
                    {
                        nextBasisFuncVal *= -1;
                    }
                    else if (prevBasisFuncVal >= 0 && nextBasisFuncVal > 0)
                    {
                        nextBasisFuncVal *= -1;
                    }

                    prevBasisFuncVal = nextBasisFuncVal;
                    coefficientMatrix[row][reverseCol] = nextBasisFuncVal;
                    ++colBasisFunc;
                    --reverseCol;
                }

                ++rowBasisFunc;
            }
        }
    }

    // Заполнение матрицу коэффициентов
    static void fillCoefficientsMatrix(std::vector<std::vector<double>>& coefficientMatrix, std::vector<std::vector<double>>& basisFuncs, size_t numberEpsilons, size_t numberBreakPoints)
    {
        // Заполняем двойками главную диагональ
        for (size_t i = 0; i != numberEpsilons; ++i)
        {
            coefficientMatrix[i][i] = 2;
        }

        // Заполняем матрицу коэффциентов базисными функциями
        fillUpperTriangularMatrix(coefficientMatrix, basisFuncs, numberEpsilons, numberBreakPoints);
        fillLowerTriangularMatrix(coefficientMatrix, basisFuncs, numberEpsilons, numberBreakPoints);
    }

    // Фиксирует первую и последнюю точки кривых и их первые произнодеые (зануление определенных столбцов и строк у матрицы коэффициентов). По умолчанию фиксируются все 4 точки
    static void fixPointAtCurve(std::vector<std::vector<double>>& coefficientMatrix, size_t numberEpsilons, size_t numberBasisFuncs,
                                bool fixStartPoint = true, bool fixEndPoint = true, bool fixFirstDivStartPoint = true, bool fixFirstDivEndPoint = true)
    {
        const size_t MATRIX_SIZE = coefficientMatrix.size();

        // Фиксация первой граничной точки кривой
        if(fixStartPoint)
        {
            for (size_t i = numberEpsilons; i != numberEpsilons + numberBasisFuncs; ++i)
            {
                coefficientMatrix[0][i] = 0;
            }
        }

        // Фиксация последней граничной точки кривой
        if (fixEndPoint)
        {
            for (size_t col = numberEpsilons + numberBasisFuncs; col != MATRIX_SIZE; ++col)
            {
                coefficientMatrix[numberEpsilons - 1][col] = 0;
            }
        }

        // Фиксация первой производной первой граничной точки кривой
        if (fixFirstDivStartPoint)
        {
            for (size_t i = numberEpsilons; i != numberEpsilons + numberBasisFuncs; ++i)
            {
                coefficientMatrix[1][i] = 0;
            }
        }

        // Фиксация первой производной последней граничной точки кривой
        if (fixFirstDivEndPoint)
        {
            for (size_t col = numberEpsilons + numberBasisFuncs; col != MATRIX_SIZE; ++col)
            {
                coefficientMatrix[numberEpsilons - 2][col] = 0;
            }
        }
    }

    void fillFreeMemberMatrix(std::vector<Point3D>& freeMembersMatrix, const std::vector<std::vector<Point3D>>& controlPointsBezierCurves, const std::vector<std::vector<double>>& basisFunc, const std::vector<std::vector<double>>& reverseBasisFunc, size_t numberEpsilons)
    {
        size_t indexFreeMembers = numberEpsilons;

        for (size_t row = 0; row != controlPointsBezierCurves.size() - 1; ++row)
        {
            size_t rowBasisFunc = 0;

            for (size_t col = 0; col != basisFunc[0].size(); ++col)
            {
                for (size_t i = 0; i != controlPointsBezierCurves[0].size(); ++i)
                {
                    // Текущая кривая
                    freeMembersMatrix[indexFreeMembers] += controlPointsBezierCurves[row][i] * -basisFunc[rowBasisFunc][i];
                    // След. кривая
                    freeMembersMatrix[indexFreeMembers] += controlPointsBezierCurves[row + 1][i] * reverseBasisFunc[rowBasisFunc][i];
                }

                ++rowBasisFunc;
                ++indexFreeMembers;
            }
        }
    }

    // Возвращает точки сдвига для соблюдения полного сопряжения кривой
    static std::vector<Point3D> calcShiftPoints(const std::vector<std::vector<double>>& coefficientMatrix, const std::vector<Point3D>& freeMembersMatrix)
    {
        // Создаём указатель на интерфейс операций СЛАУ
        auto operation = IMatrixOperations::GetMatrixOperationsClass(OperationClass::eigen);

        if (operation == nullptr)
        {
            throw "Error! conjugateCurve: operation = nullptr";;
        }

        // Вычисляем определитель матрицы коэффициентов
        double coefficientMatrixDet = operation->getMatrixDet(coefficientMatrix);

        if (coefficientMatrixDet == 0)
        {
            throw "Error! coefficientMatrixDet = 0";
        }

        // Решаем СЛАУ и возвращаем ответ
        return operation->solveEquation(coefficientMatrix, freeMembersMatrix);
    }

    // Возвращает точки сдвига для соблюдения полного сопряжения кривой
    static std::vector<Point3D> calculateShiftPoints(const std::vector<std::vector<double>>& coefficientMatrix,
                                                     const std::vector<Point3D>& freeMembersMatrix)
    {
        // Создаём указатель на интерфейс операций СЛАУ
        auto operation = IMatrixOperations::GetMatrixOperationsClass(OperationClass::eigen);

        if (operation == nullptr)
        {
            throw "Error! conjugateCurve: operation = nullptr";;
        }

        // Вычисляем определитель матрицы коэффициентов
        double coefficientMatrixDet = operation->getMatrixDet(coefficientMatrix);

        if (coefficientMatrixDet == 0)
        {
            throw "Error! coefficientMatrixDet = 0";
        }

        // Решаем СЛАУ и возвращаем ответ
        return operation->solveEquation(coefficientMatrix, freeMembersMatrix);
    }

    std::vector<Curve> correctionPoints(std::vector<std::vector<Point3D>>& controlPointsBezierCurves, std::vector<Point3D>& shiftPoints, size_t numberBezierCurves, int degree)
    {
        int tempCounter = 0;
        std::vector<Curve> newBezierCurves;


        for (size_t i = 0; i != numberBezierCurves; ++i)
        {
            for (size_t j = 0; j != controlPointsBezierCurves[i].size(); ++j) // Регулируем котрольные точки Безье кривых для сопряжения
            {
                controlPointsBezierCurves[i][j] += shiftPoints[tempCounter++];
            }

            // Создаём новую кривую Безье и добавляем в вектор, чтобы функция возвратила его
            const int CURVE_NUM_POINTS = 1001;   // Кол-во точек, из которых будет состоять кривая
            Curve bezierCurve(controlPointsBezierCurves[i], std::vector<double> (controlPointsBezierCurves[0].size(), 1), degree, CURVE_NUM_POINTS);
            newBezierCurves.push_back(bezierCurve);
        }

        return newBezierCurves;
    }
}


std::vector<Curve> MergeCurves::attachAllBezierCurves(const Curve& curve)
{
    // Разбиваем составную кривую на кривые Безье
    std::vector<Curve> bezierCurves = UsefulUtils::splittingСurveIntoBezierCurves(curve);

    // Рассчитываем базисные функции у кривой в конечной параметрической точке (базисные функции у всех кривых одинаковые)
    double curveParameter = 1;
    int span = CalcCurve::findSpanForParameter(curveParameter, bezierCurves[0].getNodalVector(), bezierCurves[0].getDegree());
    std::vector<std::vector<double>> basisFuncs = CalcCurve::calcBasisFuncsAndTheirDerivs(bezierCurves[0].getNodalVector(), curveParameter, span, bezierCurves[0].getDegree());

    // Количество базисных функций
    const size_t NUMBER_BASIS_FUNCS = static_cast<size_t>(curve.getDegree()) + 1;
    // Количество кривых Безье
    const size_t NUMBER_BEZIER_CURVES = bezierCurves.size();
    // Количество потенциальных точек разрыва между кривыми
    const size_t NUMBER_BREAK_POINTS = NUMBER_BEZIER_CURVES - 1;
    // Количество эпсилон, которые будут регулировать контрольные точки
    const size_t NUMBER_EPSILONS = bezierCurves.size() * bezierCurves[0].getControlPoints().size();
    // Размер матрицы коэффициентов
    const size_t MATRIX_SIZE = NUMBER_BASIS_FUNCS * (NUMBER_BEZIER_CURVES + NUMBER_BEZIER_CURVES - 1);

    // Матрица коэффициентов
    std::vector<std::vector<double>> coefficientMatrix(MATRIX_SIZE, std::vector<double>(MATRIX_SIZE));

    // Заполняем матрицу коэффициентов
    Impl::fillCoefficientsMatrix(coefficientMatrix, basisFuncs, NUMBER_EPSILONS, NUMBER_BREAK_POINTS);

    // Фиксируем первую и последнюю точки и их первые производные
    Impl::fixPointAtCurve(coefficientMatrix, NUMBER_EPSILONS, NUMBER_BASIS_FUNCS, true, true, false, false);

    // Контрольные точки кривых Безье
    std::vector<std::vector<Point3D>> controlPointsBezierCurves(NUMBER_BEZIER_CURVES);

    for (size_t i = 0; i != NUMBER_BEZIER_CURVES; ++i)
    {
        controlPointsBezierCurves[i] = bezierCurves[i].getControlPoints();
    }

    // Матрица свободных членов
    std::vector<Point3D> freeMembersMatrix(MATRIX_SIZE);

    // Вычисляем реверсивные базисные функции и их производные в параметре 0
    double parameter = 0;
    span = CalcCurve::findSpanForParameter(parameter, bezierCurves[0].getNodalVector(), bezierCurves[0].getDegree());
    std::vector<std::vector<double>> reverseBasisFuncs = CalcCurve::calcBasisFuncsAndTheirDerivs(bezierCurves[0].getNodalVector(), parameter, span, bezierCurves[0].getDegree());

    // Заполняем матрицу свободных членов
    Impl::fillFreeMemberMatrix(freeMembersMatrix, controlPointsBezierCurves, basisFuncs, reverseBasisFuncs, NUMBER_EPSILONS);

    // Вычисляем точки смещения для новых контрольных точек
    std::vector<Point3D> shiftPoints = Impl::calculateShiftPoints(coefficientMatrix, freeMembersMatrix);

     // Делаем сдивг исходных контрольных точек для сопряжения
    std::vector<Curve> newBezierCurves = Impl::correctionPoints(controlPointsBezierCurves, shiftPoints, NUMBER_BEZIER_CURVES, bezierCurves[0].getDegree());

    return newBezierCurves;
}

