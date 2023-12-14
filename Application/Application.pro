QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ApproximationAlgorithms/AlgBasedCurveConjugation.cpp \
    ApproximationAlgorithms/AlgBasedIntegralNorm.cpp \
    ApproximationAlgorithms/IApproxAlgs.cpp \
    ApproximationAlgorithms/Metrics.cpp \
    ApproximationAlgorithms/NativeAlg.cpp \
    ApproximationAlgorithms/ResourceApproxAlgs.cpp \
    Curve3D/CalcCurve.cpp \
    Curve3D/Curve.cpp \
    Curve3D/Point3d.cpp \
    Main.cpp \
    MainWindow.cpp \
    Plotting/Graph2D.cpp \
    Plotting/qcustomplot.cpp \
    Plotting/qcustomplot.cpp \
    Utils/EigenMatrixOperations.cpp \
    Utils/FindDistanceBetweenCurves.cpp \
    Utils/IMatrixOperations.cpp \
    Utils/MathUtils.cpp \
    Utils/MergeCurves.cpp \
    Utils/UsefulUtils.cpp

HEADERS += \
    ApproximationAlgorithms/AlgBasedCurveConjugation.h \
    ApproximationAlgorithms/AlgBasedIntegralNorm.h \
    ApproximationAlgorithms/IApproxAlgs.h \
    ApproximationAlgorithms/Metrics.h \
    ApproximationAlgorithms/NativeAlg.h \
    ApproximationAlgorithms/ResourceApproxAlgs.h \
    Curve3D/CalcCurve.h \
    Curve3D/Curve.h \
    Curve3D/CurvePoint.h \
    Curve3D/Point3d.h \
    MainWindow.h \
    Plotting/Graph2D.h \
    Plotting/qcustomplot.h \
    Plotting/qcustomplot.h \
    Utils/EigenMatrixOperations.h \
    Utils/FindDistanceBetweenCurves.h \
    Utils/IMatrixOperations.h \
    Utils/MathUtils.h \
    Utils/MergeCurves.h \
    Utils/UsefulUtils.h

FORMS += \
    MainWindow.ui

INCLUDEPATH += C:\Main\Projects\NURBS_2D\Dependencies\include\eigen-3.4.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES +=
