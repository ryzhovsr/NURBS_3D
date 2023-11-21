#pragma once

#include <vector>
#include "Curve.h"

class CalcCurve
{
public:
    // Вычисляет точку кривой и её производные
    static void calcCurvePointAndDerivs(const Curve &curve, CurvePoint &curvePoint, double parameter);

    // Находит номер узлового промежутка (спан) для точки реального диапазона узл. вектора - параметра
    static int findSpanForParameter(double parameter, const std::vector<double> &nodalVector, int degree);

    // Вычисляет базисные функции и их производные
    static std::vector<std::vector<double>> calcBasisFuncsAndTheirDerivs(const std::vector<double>& nodalVector, double parameter, int span, int degree);

    // Вычисляет базисные функции
    static void calcBasisFuncs(std::vector<std::vector<double>> &basisFuncsAndTheirDerivs, std::vector<std::vector<double>> &tempStorage, const std::vector<double>& nodalVector, double parameter, int span, int degree);

    // Вычисляет производные базисных функций
    static void calcDerivsBasisFuncs(std::vector<std::vector<double>> &basisFuncsAndTheirDerivs, std::vector<std::vector<double>> &tempStorage, int degree);

    static void calcDerivCurve(CurvePoint &curvePoint, int span, const std::vector<Point3D> &controlPoints, const std::vector<double>& weights, const std::vector<std::vector<double>>& basisFuncsAndTheirDerivs, int degree);
};
