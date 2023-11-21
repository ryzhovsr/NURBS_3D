#include "NativeAlg.h"
#include "Utils/FindDistanceBetweenCurves.h"

Curve NativeAlg::approximateCurve(const Curve &curve, int degreeApprox) const
{
    checkDegreeApprox(curve, degreeApprox);
    return Curve(curve.getControlPoints(), curve.getWeights(), degreeApprox, static_cast<int>(curve.getCurvePoints().size()));
}

Curve NativeAlg::approximateCurveForEpsilon(const Curve &curve, double epsilon) const
{
    int oldDegree = curve.getDegree();
    int degreeNew = 1; // Начинаем с 1-ой степени кривой

    while (oldDegree > degreeNew)
    {
        Curve curveNew(curve.getControlPoints(), curve.getWeights(), degreeNew, static_cast<int>(curve.getCurvePoints().size()));
        double lenBetweenCurves = FindDistanceBetweenCurves::findMaxLenBetweenCurves(curve, curveNew); // Расстояние между исходной и аппроксимирующей кривой

        if (epsilon > lenBetweenCurves) // Если длина между кривыми входит в заданную точность
            return curveNew;

        ++degreeNew;
    }

    return curve;
}
