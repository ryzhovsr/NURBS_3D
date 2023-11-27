#include "Curve.h"
#include <QDebug>
#include "CalcCurve.h"

Curve::Curve(const std::vector<Point3D> &controlPoints, const std::vector<double> &weights, int degree, int curveNumPoints) noexcept
    : _controlPoints {controlPoints}, _weights { weights }, _numRealRangePoints { curveNumPoints }, _degree { degree }
{
    _curvePoints.resize(_numRealRangePoints); // Устанавливаем количество точек, из которых будет состоять кривая

    _numVertices = static_cast<int>(_controlPoints.size()); // Кол-во вершин определяющего многоугольника
    _numRealRangeKnots = _numVertices - _degree + 1;        // Кол-во узлов реального диапазона узл. вектора
    _numKnots = _numVertices + _degree + 1;                 // Общее кол-во узлов узл. вектора

    // Начало и конец реального диапазона узл. вектора
    _realRangeStart = _degree;
    _realRangeEnd = _numKnots - _degree - 1;

    double step = 1.0 / (_numRealRangeKnots - 1); // Шаг в реальном диапазоне узл. вектора
    _nodalVector.resize(_numKnots);

    for (int i = _realRangeStart + 1; i < _realRangeEnd; ++i)    // Заполняем реальный диапазон узл. вектора
        _nodalVector[i] = _nodalVector[i - 1] + step;

    for (int i = _realRangeEnd; i < _numKnots; ++i)    // Заполняем оставшуюся часть узл. вектора единицами
        _nodalVector[i] = 1;

    _checkNodalVector();
    calcCurve();
}

void Curve::setNodalVector(const std::vector<double> &nodalVector)
{
    _numKnots = static_cast<int>(nodalVector.size());
    _nodalVector = nodalVector;
    _checkNodalVector();
    calcCurve();
}

std::vector<CurvePoint> Curve::getCurvePoints() const noexcept
{
    return _curvePoints;
}

std::vector<Point3D> Curve::getControlPoints() const noexcept
{
    return _controlPoints;
}

std::vector<double> Curve::getNodalVector() const noexcept
{
    return _nodalVector;
}

std::vector<double> Curve::getWeights() const noexcept
{
    return _weights;
}

int Curve::getDegree() const noexcept
{
    return _degree;
}

void Curve::_checkNodalVector()
{
    for (int i = 0; i < _numKnots - 1; ++i) // Проверка узлового вектора на убывание
    {
        if (_nodalVector[i] > _nodalVector[i + 1])
            qDebug() << "Error! _checkNodalVector: узловой вектор убывает _nodalVector[i] > _nodalVector[i + 1]!";
    }

    if (_numKnots != (_numVertices + _degree + 1))
        qDebug() << "Error! _checkNodalVector: nodalVector.size()) != (numVertices + degreeCurve + 1)!";
}

void Curve::calcCurve()
{
    for (int i = 0; i < _numRealRangePoints; ++i) // Итерируемся по каждой точке кривой
    {
        // Находим точку реальной части узл. вектора (параметр кривой ∈ [0, 1])
        double parameter = static_cast<double>(i) / (_numRealRangePoints - 1) * (_nodalVector[_realRangeEnd] - _nodalVector[_realRangeStart]) + _nodalVector[_realRangeStart];
        CalcCurve::calcCurvePointAndDerivs(*this, _curvePoints[i], parameter); // Рассчитываем точку кривой и её первую и вторую производную
    }
}

void Curve::addMultipleKnot()
{
    //std::vector<double> newNodes = {0.3333333333333333, 0.6666666666666666};
    //std::vector<double> newNodes = {0.3333333333333333, 0.3333333333333333, 0.6666666666666666,  0.6666666666666666};
    std::vector<double> newNodes = {0.25, 0.5, 0.75};
    std::vector<Point3D> newControlPoints(_controlPoints.size() + newNodes.size());
    std::vector<double> newNodalVector(_nodalVector.size() + newNodes.size());

    redefineControlPointsNodalVectorCurve(_degree, _nodalVector, _controlPoints, newNodes, newNodalVector, newControlPoints);
    redefineCurve(newControlPoints, newNodalVector, _degree);
}

void Curve::redefineCurve(const std::vector<Point3D>& newControlPoints, const std::vector<double>& newNodalVector, int newDegreeCurve)
{
    _weights = std::vector<double> (newControlPoints.size(), 1);
    _controlPoints = newControlPoints;
    _degree = newDegreeCurve;

    _numVertices = static_cast<int>(_controlPoints.size());
    _numKnots = _numVertices + _degree + 1;
    _numRealRangeKnots = _numVertices - _degree + 1;

    // Рассчитываем начало и конец реального диапазона
    _realRangeStart = _degree;
    _realRangeEnd = _numKnots - _degree - 1;

    if (newNodalVector.empty()) // Если новый узловой вектор пустой, то заполним его равномерно
        this->fillEvenlyNodalVector();
    else
        _nodalVector = newNodalVector;

    this->calcCurve();
}


void Curve::fillEvenlyNodalVector()
{
    _nodalVector.resize(_numKnots);

    double step = 1 / static_cast<double>(_numRealRangeKnots - 1); // Шаг в реальном диапазоне

    for (int i = _realRangeStart + 1; i < _realRangeEnd; ++i)    // Заполняем реальный диапазон
        _nodalVector[i] = _nodalVector[i - 1] + step;

    for (size_t i = _realRangeEnd; i < _nodalVector.size(); ++i) // Заполняем последние параметры единицами
        _nodalVector[i] = 1;
}

int Curve::findSpanCurve(double realPoint, const std::vector<double>& nodalVector, int degreeCurve, int numVertices)
{
    int numKnots = static_cast<int>(nodalVector.size());

    for (size_t i = 0; i < nodalVector.size() - 1; ++i)
    {
        if (nodalVector[i] > nodalVector[i + 1])
        {
            qDebug() << "Error! findSpanCurve: узловой вектор убывает nodalVector[i] > nodalVector[i + 1]";
            return {};
        }
    }

    if (static_cast<int>(nodalVector.size()) != (numVertices + degreeCurve + 1))
    {
        qDebug() << "Error! findSpanCurve: nodalVector.size()) != (numVertices + degreeCurve + 1)";
        return {};
    }

    if (realPoint < nodalVector[degreeCurve] || realPoint > nodalVector[numKnots - degreeCurve - 1])
    {
        qDebug() << "Error! findSpanCurve: nodalVector вышел за реальный диапазон";
        return {};
    }

    if (realPoint == nodalVector[numKnots - degreeCurve - 1]) // Если дошли до конца реального диапазона
        return numKnots - degreeCurve - 2;

    int low = degreeCurve, high = numKnots - degreeCurve - 1, middle = (low + high) / 2;

    while ((realPoint < nodalVector[middle]) || (realPoint >= nodalVector[middle + 1]))
    {
        if (realPoint < nodalVector[middle])
            high = middle;
        else
            low = middle;

        middle = (low + high) / 2;
    }

    return middle;
}


void Curve::redefineControlPointsNodalVectorCurve(int newDegreeCurve, const std::vector<double>& oldNodalVector, const std::vector<Point3D>& oldControlPoints, std::vector<double> newNodes,
                                                      std::vector<double>& newNodalVector, std::vector<Point3D>& newControlPoints)
{
    int numOldControlPoints = static_cast<int>(oldControlPoints.size());    // Коли-во контрольных точек до вставки
    int maxIndexNewNodes = static_cast<int>(newNodes.size()) - 1;

    double a = findSpanCurve(newNodes[0], oldNodalVector, newDegreeCurve, static_cast<int>(oldControlPoints.size()));
    double b = findSpanCurve(newNodes[maxIndexNewNodes], oldNodalVector, newDegreeCurve, static_cast<int>(oldControlPoints.size())) + 1;

    for (int i = 0; i < a - newDegreeCurve + 1; ++i)
        newControlPoints[i] = oldControlPoints[i];

    for (int i = b - 1; i < numOldControlPoints; ++i)
        newControlPoints[i + maxIndexNewNodes + 1] = oldControlPoints[i];

    for (int i = 0; i < a + 1; ++i)
        newNodalVector[i] = oldNodalVector[i];

    for (int i = b + newDegreeCurve; i < numOldControlPoints + newDegreeCurve + 1; ++i)
        newNodalVector[i + maxIndexNewNodes + 1] = oldNodalVector[i];

    int ind = b + newDegreeCurve - 1;
    int k = b + newDegreeCurve + maxIndexNewNodes;

    for (int i = maxIndexNewNodes; i > -1; --i)
    {
        while ((newNodes[i] <= oldNodalVector[ind]) && (ind > a))
        {
            newControlPoints[k - newDegreeCurve - 1] = oldControlPoints[ind - newDegreeCurve - 1];
            newNodalVector[k] = oldNodalVector[ind];
            --k, --ind;
        }

        newControlPoints[k - newDegreeCurve - 1] = newControlPoints[k - newDegreeCurve];

        for (int j = 1; j < newDegreeCurve + 1; ++j)
        {
            int temp = k - newDegreeCurve + j;
            double alpha = newNodalVector[k + j] - newNodes[i];

            if (alpha == 0)
                newControlPoints[temp - 1] = newControlPoints[temp];
            else
            {
                alpha /= (newNodalVector[k + j] - oldNodalVector[ind - newDegreeCurve + j]);
                newControlPoints[temp - 1].x = alpha * newControlPoints[temp - 1].x + (1 - alpha) * newControlPoints[temp].x;
                newControlPoints[temp - 1].y = alpha * newControlPoints[temp - 1].y + (1 - alpha) * newControlPoints[temp].y;
            }
        }

        newNodalVector[k] = newNodes[i];
        --k;
    }
}
