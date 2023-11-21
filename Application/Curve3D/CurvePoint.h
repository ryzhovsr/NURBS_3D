#pragma once

#include <vector>
#include "Point3d.h"

struct CurvePoint
{
    std::vector<Point3D> derivs;
    double x, y, z;
    double parameter;     // Параметр реальной части узл. вектора
    int span;   // Номер интервала, к которому относится parameter
};
