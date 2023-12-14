#pragma once

#include <QDebug>

struct Point3D
{
    double x, y, z;

    Point3D(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) {}

    Point3D operator+(const Point3D& other) const;

    Point3D operator-(const Point3D& other) const;
    Point3D operator-(double value) const;

    Point3D operator*(double scalar) const;

    friend Point3D operator*(double scalar, Point3D p)
    {
        return Point3D(p.x * scalar, p.y * scalar, p.z * scalar);
    }

    Point3D operator / (double scalar) const;

    Point3D &operator+=(const Point3D& other);

    Point3D &operator-=(const Point3D& other);
};
