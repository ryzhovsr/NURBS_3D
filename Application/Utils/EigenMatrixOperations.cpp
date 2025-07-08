#include "eigenmatrixoperations.h"
#include "Curve3d/Point3d.h"
#include <../Dependencies/include/eigen/Eigen/Dense>

using namespace Eigen;

// Переводит двумерный вектор в матрицу класса Eigen
inline MatrixXd vector2DToMatrix(const IMatrixOperations::vector2D &vec2D)
{
    const size_t ROWS = vec2D.size(), COLS = vec2D[0].size();
    MatrixXd matrix = MatrixXd::Constant(ROWS, COLS, 0);

    for (size_t i = 0; i != ROWS; ++i)
    {
        for (size_t j = 0; j != COLS; ++j)
        {
            matrix(i, j) = vec2D[i][j];
        }
    }

    return matrix;
}

// Переводит матрицу класса Eigen в двумерный вектор
inline IMatrixOperations::vector2D matrixToVector2D(const MatrixXd &matrix)
{
    const auto ROWS = matrix.rows(), COLS = matrix.cols();
    IMatrixOperations::vector2D vec2D(ROWS, std::vector<double>(COLS));

    for (int i = 0; i != ROWS; ++i)
    {
        for (int j = 0; j != COLS; ++j)
        {
            vec2D[i][j] = matrix(i, j);
        }
    }

    return vec2D;
}

// Переводит вектор трехмерных координат в матрицу класса Eigen
inline MatrixXd vector3DToMatrix(const IMatrixOperations::vector3D &vec3D)
{
    // COLS = 3, потому что 3 координаты (x, y, z)
    const size_t ROWS = vec3D.size(), COLS = 3;
    MatrixXd matrix = MatrixXd::Constant(ROWS, COLS, 0);

    for (size_t i = 0; i != ROWS; ++i)
    {
        matrix(i, 0) = vec3D[i].x;
        matrix(i, 1) = vec3D[i].y;
        matrix(i, 2) = vec3D[i].z;
    }

    return matrix;
}

// Переводит матрицу класса Eigen в вектор трехмерных координат
inline IMatrixOperations::vector3D matrixToVector3D(const MatrixXd &matrix)
{
    const auto ROWS = matrix.rows();
    IMatrixOperations::vector3D vec3D(ROWS);

    for (int i = 0; i != ROWS; ++i)
    {
        vec3D[i].x = matrix(i, 0);
        vec3D[i].y = matrix(i, 1);
        vec3D[i].z = matrix(i, 2);
    }

    return vec3D;
}

// Решает СЛАУ
IMatrixOperations::vector2D EigenMatrixOperations::solveEquation(const vector2D &coefficients, const vector2D &freeMembers)
{
    // Переводим двумерные векторы в матрицу класса Eigen
    MatrixXd coefficientMatrix = vector2DToMatrix(coefficients);
    MatrixXd freeTermMatrix = vector2DToMatrix(freeMembers);

    // Решаем СЛАУ
    Eigen::MatrixXd decisionMatrix = Eigen::MatrixXd::Constant(freeMembers.size(), freeMembers[0].size(), 0);
    decisionMatrix = coefficientMatrix.lu().solve(freeTermMatrix);

    // Обратная конвертация
    return matrixToVector2D(decisionMatrix);
}

// Решает СЛАУ
IMatrixOperations::vector3D EigenMatrixOperations::solveEquation(const vector2D &coefficients, const vector3D &freeMembers)
{
    // Переводим двумерные векторы в матрицу класса Eigen
    MatrixXd coefficientMatrix = vector2DToMatrix(coefficients);
    MatrixXd freeTermMatrix = vector3DToMatrix(freeMembers);

    // Решаем СЛАУ
    Eigen::MatrixXd decisionMatrix = Eigen::MatrixXd::Constant(freeMembers.size(), freeMembers.size(), 0);
    decisionMatrix = coefficientMatrix.lu().solve(freeTermMatrix);

    // Обратная конвертация
    return matrixToVector3D(decisionMatrix);
}

// Возвращяет определитель матрицы
double EigenMatrixOperations::getMatrixDet(const vector2D &vec2D)
{
    MatrixXd matrix = vector2DToMatrix(vec2D);
    return matrix.determinant();
}

// Возвращает ранг матрицы
int EigenMatrixOperations::getMatrixRank(const vector2D &matrix)
{
    MatrixXd m = vector2DToMatrix(matrix);
    // Используем LU-разложение
    FullPivLU<MatrixXd> lu_decomp(m);
    return static_cast<int>(lu_decomp.rank());
}
