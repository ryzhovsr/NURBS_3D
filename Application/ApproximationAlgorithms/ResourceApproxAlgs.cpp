#include "ResourceApproxAlgs.h"

ResourceApproxAlgs::ResourceApproxAlgs(IApproxAlgs *approxAlgorithm) : _approxAlgorithm { approxAlgorithm }
{

}

ResourceApproxAlgs::~ResourceApproxAlgs()
{
    delete _approxAlgorithm;
}

void ResourceApproxAlgs::setStrategy(IApproxAlgs *approxAlgorithm)
{
    if (_approxAlgorithm != nullptr)
        delete _approxAlgorithm;
    _approxAlgorithm = approxAlgorithm;
}

Curve ResourceApproxAlgs::approximateCurve(const Curve &curve, int degreeApprox)
{
    return _approxAlgorithm->approximateCurve(curve, degreeApprox);
}
