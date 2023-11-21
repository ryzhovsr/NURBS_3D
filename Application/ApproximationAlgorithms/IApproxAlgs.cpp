#include "IApproxAlgs.h"
#include <QDebug>

void IApproxAlgs::checkDegreeApprox(const Curve &curve, int degreeApprox) const
{
    if (curve.getDegree() < degreeApprox)
        qDebug() << "Error! checkDegreeApprox: Степень исходной кривой больше степени аппроксимирующей кривой!";
}
