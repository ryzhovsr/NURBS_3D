#include "imatrixoperations.h"
#include "eigenmatrixoperations.h"

IMatrixOperationsPtr IMatrixOperations::GetMatrixOperationsClass(OperationClass className)
{
    switch (className)
    {
    case eigen:
        return std::make_shared<EigenMatrixOperations>();
    default:
        return nullptr;
    }
}
