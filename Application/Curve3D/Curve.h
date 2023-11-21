#pragma once

#include "CurvePoint.h"
#include <vector>

class Curve
{
public:
    Curve(const std::vector<Point3D> &controlPoints, const std::vector<double> &weights, int degree, int curveNumPoints) noexcept;

    void calcCurve(); // Рассчитывает точки кривой

    void setNodalVector(const std::vector<double> &nodalVector); // Устанавливает новый узловой вектор

    std::vector<CurvePoint> getCurvePoints() const noexcept;    // Возвращает точки кривой
    std::vector<Point3D> getControlPoints() const noexcept;     // Возвращает контрольные точки
    std::vector<double> getNodalVector() const noexcept;        // Возвращает узловой вектор
    std::vector<double> getWeights() const noexcept;            // Возвращает весовые коэффициенты точек определяющего многоугольника
    int getDegree() const noexcept;                             // Возвращает степень кривой

    void addMultipleKnot(); // Добавляет кратные узлы в кривую

    // Возвращает индекс узлового промежутка кривой для заданной точки узлового вектора
    int findSpanCurve(double realPoint, const std::vector<double> &nodalVector, int degreeCurve, int numVertices);

    void fillEvenlyNodalVector(); // Равномерно заполняет узловой вектор

private:
    std::vector<CurvePoint> _curvePoints;   // Точки кривой
    std::vector<Point3D> _controlPoints;    // Точки определяющего многоугольника
    std::vector<double> _nodalVector;       // Узловой вектор
    std::vector<double> _weights;           // Весовые коэффициенты точек определяющего многоугольника
    int _realRangeStart, _realRangeEnd;     // Начало/конец реального диапазона узл. вектора
    int _numRealRangeKnots;     // Кол-во узлов (длина) реальной части узлового вектора
    int _numRealRangePoints;    // Кол-во точек (разбиений) в реальной части узлового вектора
    int _numVertices;   // Кол-во вершин определяющего многоугольника
    int _numKnots;      // Кол-во узлов в узл. векторе
    int _degree;        // Степень кривой

    void _checkNodalVector(); // Проверяет узловой вектор на соответствие правил

    // Переопределяет контрольные точки после добавления кратных узлов
    void redefineControlPointsNodalVectorCurve(int newDegreeCurve, const std::vector<double>& oldNodalVector, const std::vector<Point3D>& oldControlPoints, std::vector<double> newNodes,
                                               std::vector<double>& newNodalVector, std::vector<Point3D>& newControlPoints);

    // Переопределяет кривую после добавления кратных узлов
    void redefineCurve(const std::vector<Point3D>& newControlPoints, const std::vector<double>& newNodalVector, int newDegreeCurve = 0);
};
