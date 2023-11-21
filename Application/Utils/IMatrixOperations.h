#pragma once

#include "Curve3d/Point3d.h"
#include <vector>
#include <memory>

class IMatrixOperations;
using IMatrixOperationsPtr = std::shared_ptr<IMatrixOperations>;

enum OperationClass // Содержит названия библиотек для работы с матрицами
{
    eigen
};

class IMatrixOperations
{
public:
    using vector2D = std::vector<std::vector<double>>;
    using vector3D = std::vector<Point3D>;

    // Решает СЛАУ
    virtual vector2D solveEquation(const vector2D& coefficients, const vector2D& freeMembers) = 0;
    virtual vector3D solveEquation(const vector2D& coefficients, const vector3D& freeMembers) = 0;

    // Возвращяет ранг матрицы
    virtual int getMatrixRank(const vector2D& matrix) = 0;

    // Возвращяет определитель матрицы
    virtual double getMatrixDet(const vector2D &vec2D) = 0;

    static IMatrixOperationsPtr GetMatrixOperationsClass (OperationClass className);
};
