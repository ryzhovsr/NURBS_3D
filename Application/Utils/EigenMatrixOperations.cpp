#include "eigenmatrixoperations.h"
#include "Curve3d/Point3d.h"
#include <Eigen/Dense>

using namespace Eigen;

// Переводит двумерный вектор в матрицу класса Eigen
inline MatrixXd vector2DToMatrix(const IMatrixOperations::vector2D &vec2D)
{
    size_t rows = vec2D.size(), cols = vec2D[0].size();
    MatrixXd matrix = MatrixXd::Constant(rows, cols, 0);

    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            matrix(i, j) = vec2D[i][j];
        }
    }

    return matrix;
}

// Переводит матрицу класса Eigen в двумерный вектор
inline IMatrixOperations::vector2D matrixToVector2D(const MatrixXd &matrix)
{
    auto rows = matrix.rows(), cols = matrix.cols();
    IMatrixOperations::vector2D vec2D(rows, std::vector<double>(cols));

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            vec2D[i][j] = matrix(i, j);
        }
    }

    return vec2D;
}

// Переводит вектор трехмерных координат в матрицу класса Eigen
inline MatrixXd vector3DToMatrix(const IMatrixOperations::vector3D &vec3D)
{
    size_t rows = vec3D.size(), cols = 3; // cols = 3, потому что всего 3 координаты (x, y, z)
    MatrixXd matrix = MatrixXd::Constant(rows, cols, 0);

    for (size_t i = 0; i < rows; ++i)
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
    auto rows = matrix.rows();
    IMatrixOperations::vector3D vec3D(rows);

    for (int i = 0; i < rows; ++i)
    {
        vec3D[i].x = matrix(i, 0);
        vec3D[i].y = matrix(i, 1);
        vec3D[i].z = matrix(i, 2);
    }

    return vec3D;
}

IMatrixOperations::vector2D EigenMatrixOperations::solveEquation(const vector2D &coefficients, const vector2D &freeMembers)
{
    // Переводим двумерные векторы в матрицу класса Eigen
    MatrixXd coefficientMatrix = vector2DToMatrix(coefficients);
    MatrixXd freeTermMatrix = vector2DToMatrix(freeMembers);

    // Решаем СЛАУ
    Eigen::MatrixXd decisionMatrix = Eigen::MatrixXd::Constant(freeMembers.size(), freeMembers[0].size(), 0);
    decisionMatrix = coefficientMatrix.lu().solve(freeTermMatrix);

    // Revert convertion
    vector2D decisionVector2D = matrixToVector2D(decisionMatrix);

    return decisionVector2D;
}

IMatrixOperations::vector3D EigenMatrixOperations::solveEquation(const vector2D &coefficients, const vector3D &freeMembers)
{
    // Переводим двумерные векторы в матрицу класса Eigen
    MatrixXd coefficientMatrix = vector2DToMatrix(coefficients);
    MatrixXd freeTermMatrix = vector3DToMatrix(freeMembers);

    // Решаем СЛАУ
    Eigen::MatrixXd decisionMatrix = Eigen::MatrixXd::Constant(freeMembers.size(), freeMembers.size(), 0);
    decisionMatrix = coefficientMatrix.lu().solve(freeTermMatrix);

    // Revert convertion
    vector3D decisionVector3D = matrixToVector3D(decisionMatrix);

    return decisionVector3D;
}

double EigenMatrixOperations::getMatrixDet(const vector2D &vec2D)
{
    MatrixXd matrix = vector2DToMatrix(vec2D);
    return matrix.determinant();
}

int EigenMatrixOperations::getMatrixRank(const vector2D &matrix)
{
    MatrixXd m = vector2DToMatrix(matrix);
    FullPivLU<MatrixXd> lu_decomp(m); // Используем LU-разложение
    return static_cast<int>(lu_decomp.rank());
}
