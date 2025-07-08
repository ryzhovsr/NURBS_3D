#include "Point3d.h"

Point3D Point3D::cross(const Point3D &iVector) const
{
    return Point3D(y * iVector.z - z * iVector.y, z * iVector.x - x * iVector.z, x * iVector.y - y * iVector.x);
}

double Point3D::dot(const Point3D& iVector) const
{
    return x * iVector.x + y * iVector.y + z * iVector.z;
}

double Point3D::magnitude() const
{
    return std::sqrt(x * x + y * y + z * z);
}

Point3D Point3D::operator+(const Point3D &other) const {
    return Point3D(x + other.x, y + other.y, z + other.z);
}

Point3D Point3D::operator-(const Point3D &other) const {
    return Point3D(x - other.x, y - other.y, z - other.z);
}

Point3D Point3D::operator-(double value) const {
    return Point3D(x - value, y - value, z - value);
}

Point3D Point3D::operator*(double scalar) const {
    return Point3D(x * scalar, y * scalar, z * scalar);
}

Point3D Point3D::operator / (double scalar) const {
    if (scalar != 0.0)
        return Point3D(x / scalar, y / scalar, z / scalar);
    else
    {
        qDebug() << "Error: Division by zero";
        return *this;
    }
}

Point3D &Point3D::operator+=(const Point3D& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Point3D &Point3D::operator-=(const Point3D &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}
