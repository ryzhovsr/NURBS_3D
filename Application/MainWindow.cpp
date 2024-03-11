#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Plotting/Graph2D.h"
#include "Curve3d/Point3d.h"
#include "Curve3d/CalcCurve.h"
#include "Utils/MergeCurves.h"
#include "Utils/MathUtils.h"
#include "ApproximationAlgorithms/Metrics.h"
#include "Utils/UsefulUtils.h"

// В консоль пишет координаты X и Y у курсора при дабл клике
void MainWindow::doubleClickCanvas(QMouseEvent *event)
{
    qDebug() << "\nx =" << ui->canvas->xAxis->pixelToCoord(event->pos().x())
             << "\ny =" << ui->canvas->yAxis->pixelToCoord(event->pos().y());
}

/*
// Тестовый пример соединения (Эталон)
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->canvas, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickCanvas(QMouseEvent*)));

    const std::vector<Point3D> CONTROL_POINTS
    {
        {1, 1.2, 0},
        {1.5, 1.39, 0},
        {3, 1.5, 0},
        {4.5, 1.39, 0},
        {5, 1.2, 0}
    };

    const std::vector<double> WEIGHTS(CONTROL_POINTS.size(), 1);   // Весовые коэффициенты контрольных точек
    const int CURVE_NUM_POINTS = 61;   // Кол-во точек, из которых будет состоять кривая
    const int DEGREE = 2;   // Степень кривой

    Curve originalCurve(CONTROL_POINTS, WEIGHTS, DEGREE, CURVE_NUM_POINTS);

    Graph2D canvas(ui->canvas);
    //canvas.drawCurve(bSpline, "Оригинальная кривая", QColor(0, 1, 230));
    //canvas.drawDefiningPolygon(bSpline.getControlPoints(), "Определяющий многоуг. ориг. кривой");

    // 1 Начало Вставляем кратные узлы в узл. вектор
    originalCurve.addMultipleKnot();
    //canvas.drawCurve(originalCurve, "Кривая с кратными узлами", QColor(220, 150, 30));
    //canvas.drawDefiningPolygon(bSpline.getControlPoints(), "Определяющий многоуг. новой. кривой");
    // 1. Конец

    std::vector<Point3D> controlPointsBezier_1
    {
        {1, 1.2, 0},
        {1.5, 1.39, 0},
        {2.25, 1.4449999999999998, 0}
    };

    std::vector<Point3D> controlPointsBezier_2
    {
        {2.25, 1.4449999999999998, 0},
        {3, 1.5, 0},
        {3.75, 1.4449999999999998, 0},
    };

    std::vector<Point3D> controlPointsBezier_3
    {
        {3.75, 1.4449999999999998, 0},
        {4.5, 1.39, 0},
        {5, 1.2, 0},
    };

    // Нарущаем непрерывность в точках соединения Безье Кривых
    controlPointsBezier_1[1].y += 1 * 0.09;
    controlPointsBezier_2[1].y += - 0.0 * 0.0;
    controlPointsBezier_3[1].y += 1 * 0.09;

    const std::vector<double> weightsBezierCurves(controlPointsBezier_1.size(), 1);   // Весовые коэффициенты контрольных точек

    Curve bezier_1(controlPointsBezier_1, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);
    Curve bezier_2(controlPointsBezier_2, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);
    Curve bezier_3(controlPointsBezier_3, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);

    canvas.drawCurve(bezier_1, "Безье 1", QColor(20, 255, 30));
    canvas.drawCurve(bezier_2, "Безье 2", QColor(20, 200, 30));
    canvas.drawCurve(bezier_3, "Безье 3", QColor(20, 100, 30));
    //canvas.drawDefiningPolygon(bezier_1.getControlPoints(), "", QColor(20, 150, 30), Qt::DashLine);
    //canvas.drawDefiningPolygon(bezier_2.getControlPoints(), "", QColor(20, 200, 30), Qt::DashLine);
    //canvas.drawDefiningPolygon(bezier_3.getControlPoints(), "", QColor(20, 250, 30), Qt::DashLine);
    // 2. Конец

    // 3. Соединяем B-сплайны
    std::vector<Curve> bezierCurves {bezier_1, bezier_2, bezier_3};

    MergeCurves merge;
    bezierCurves = merge.attachAllBezierCurves(originalCurve);

    for (const auto& bezierCurve: bezierCurves)
    {
        //canvas.drawCurve(bezierCurve, "", QColor(20, 0, 230));
        //canvas.drawDefiningPolygon(bezierCurve.getControlPoints(), "", QColor(20, 150, 30));
    }

    Curve merdgedCurve = UsefulUtils::bezierCurvesToNURBSCurve(bezierCurves, DEGREE, CURVE_NUM_POINTS);

    //merdgedCurve.setNodalVector({0, 0, 0, 0.3333333333333333, 0.3333333333333333, 0.6666666666666666, 0.6666666666666666, 1, 1, 1});
    canvas.drawCurve(merdgedCurve, "", QColor(200, 0, 0));

    UsefulUtils::checkCurveBreakPoint(merdgedCurve, 0.3333333333333333);
    UsefulUtils::checkCurveBreakPoint(merdgedCurve, 0.6666666666666666);
    UsefulUtils::checkAllCurveBreaks(merdgedCurve);
}
*/


// Тестовый пример соединения

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->canvas, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickCanvas(QMouseEvent*)));

    const std::vector<Point3D> CONTROL_POINTS
    {
        {10, 12, 0},
        {13.5, 12.5, 0},
        {14.25, 13.199999999999998, 0},
        {17.125, 13.733333333333333, 0},
        {20, 14.266666666666667, 0},
        {25, 14.633333333333334, 0},
        {27.83333333333333, 14.766666666666666, 0},
        {30.66666666666667, 14.9, 0},
        {31.333333333333337, 14.8, 0},
        {34.91666666666667, 14.55, 0},
        {38.5, 14.3, 0},
        {45, 13.9, 0},
        {50, 12, 0},
    };

    const std::vector<double> NODAL_VECTOR
    {
        0, 0, 0, 0,
        0.2625, 0.2625, 0.2625,
        0.55, 0.55, 0.55,
        0.6678, 0.6678, 0.6678,
        1, 1, 1, 1
    };

    const std::vector<double> WEIGHTS(CONTROL_POINTS.size(), 1);   // Весовые коэффициенты контрольных точек
    const int CURVE_NUM_POINTS = 501;   // Кол-во точек, из которых будет состоять кривая
    const int DEGREE = 3;   // Степень кривой

    Curve originalCurve(CONTROL_POINTS, WEIGHTS, DEGREE, CURVE_NUM_POINTS);

    Graph2D canvas(ui->canvas);
    //canvas.drawCurve(bSpline, "Оригинальная кривая", QColor(0, 1, 230));
    //canvas.drawDefiningPolygon(bSpline.getControlPoints(), "Определяющий многоуг. ориг. кривой");

    // 1 Начало Вставляем кратные узлы в узл. вектор
    originalCurve.setNodalVector(NODAL_VECTOR);
    //originalCurve.setNodalVector();
    //originalCurve.addMultipleKnot();
    //canvas.drawCurve(originalCurve, "Кривая с кратными узлами", QColor(220, 150, 30));
    //canvas.drawDefiningPolygon(originalCurve.getControlPoints(), "Определяющий многоуг. новой. кривой");
    // 1. Конец


    std::vector<Point3D> controlPointsBezier_1
    {
        {10, 12, 0},
        {13.5, 12.5, 0},
        {14.25, 13.199999999999998, 0},
        {17.125, 13.733333333333333, 0}
    };

    std::vector<Point3D> controlPointsBezier_2
    {
        {17.125, 13.733333333333333, 0},
        {20, 14.266666666666667, 0},
        {25, 14.633333333333334, 0},
        {27.83333333333333, 14.766666666666666, 0}
    };

    std::vector<Point3D> controlPointsBezier_3
    {
        {27.83333333333333, 14.766666666666666, 0},
        {30.66666666666667, 14.9, 0},
        {31.333333333333337, 14.8, 0},
        {34.91666666666667, 14.55, 0},
    };

    std::vector<Point3D> controlPointsBezier_4
    {
        {34.91666666666667, 14.55, 0},
        {38.5, 14.3, 0},
        {45, 13.9, 0},
        {50, 12, 0},
    };

    // Нарущаем непрерывность в точках соединения Безье Кривых
    controlPointsBezier_1[1].y += 1 * 0.9;
    controlPointsBezier_2[1].y += -1 * 1.3;
    controlPointsBezier_3[1].y += 1 * 0.9;
    controlPointsBezier_4[1].y += 1 * 0.9;

    const std::vector<double> weightsBezierCurves(controlPointsBezier_1.size(), 1);   // Весовые коэффициенты контрольных точек

    Curve bezier_1(controlPointsBezier_1, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);
    Curve bezier_2(controlPointsBezier_2, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);
    Curve bezier_3(controlPointsBezier_3, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);
    Curve bezier_4(controlPointsBezier_4, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);

    canvas.drawCurve(bezier_1, "Безье 1", QColor(20, 255, 30));
    canvas.drawCurve(bezier_2, "Безье 2", QColor(20, 200, 30));
    canvas.drawCurve(bezier_3, "Безье 3", QColor(20, 100, 30));
    canvas.drawCurve(bezier_4, "Безье 4", QColor(20, 50, 30));


    UsefulUtils::outNURBSPoints(bezier_1, "bezier_1.txt");
    UsefulUtils::outNURBSPoints(bezier_2, "bezier_2.txt");
    UsefulUtils::outNURBSPoints(bezier_3, "bezier_3.txt");
    UsefulUtils::outNURBSPoints(bezier_4, "bezier_4.txt");

    //canvas.drawDefiningPolygon(bezier_1.getControlPoints(), "", QColor(20, 150, 30), Qt::DashLine);
    //canvas.drawDefiningPolygon(bezier_2.getControlPoints(), "", QColor(20, 200, 30), Qt::DashLine);
    //canvas.drawDefiningPolygon(bezier_3.getControlPoints(), "", QColor(20, 250, 30), Qt::DashLine);
    // 2. Конец

    // 3. Соединяем B-сплайны

    std::vector<Curve> bezierCurves {bezier_1, bezier_2, bezier_3, bezier_4};
    MergeCurves merge;
    bezierCurves = merge.attachAllBezierCurves(originalCurve);

    for (const auto& bezierCurve: bezierCurves)
    {
        canvas.drawCurve(bezierCurve, "", QColor(20, 0, 230));
        canvas.drawDefiningPolygon(bezierCurve.getControlPoints(), "", QColor(20, 150, 30));
    }

    Curve merdgedCurve = UsefulUtils::bezierCurvesToNURBSCurve(bezierCurves, DEGREE, CURVE_NUM_POINTS);

    UsefulUtils::outNURBSPoints(merdgedCurve, "mergedCurve.txt");


    qDebug() << "До:\n"
             << "Кривизна:" << Metrics::calcCurveCurvature(originalCurve) << '\n'
             << "Метрика Хаусдорфа:" << Metrics::calcHausdorffMetric(originalCurve, merdgedCurve) << '\n';

    qDebug() << "После:\n"
             << "Кривизна:" << Metrics::calcCurveCurvature(merdgedCurve)
             << '\n';

    qDebug() << "Квадратичная разность:" << Metrics::calcQuadraticDifference(originalCurve, merdgedCurve);

    /*
    Curve merdgedCurve = UsefulUtils::bezierCurvesToNURBSCurve(bezierCurves, DEGREE, CURVE_NUM_POINTS);

    merdgedCurve.setNodalVector(NODAL_VECTOR);
    canvas.drawCurve(merdgedCurve, "", QColor(200, 0, 0));

    UsefulUtils::checkCurveBreakPoint(merdgedCurve, 0.2625);
    UsefulUtils::checkCurveBreakPoint(merdgedCurve, 0.55);
    UsefulUtils::checkCurveBreakPoint(merdgedCurve, 0.6678);
    UsefulUtils::checkAllCurveBreaks(merdgedCurve);
*/
}

/*
// Соединение кривых Безье с помощью метода множителей Лагранжа
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->canvas, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickCanvas(QMouseEvent*)));

    const std::vector<Point3D> CONTROL_POINTS
    {
        { 0.09369765818, 0.09242290592, 0.07375858978 },
        { 0.09325591137, 0.09253203852, 0.07298330686 },     // 1 кривая Безье
        { 0.0927895828, 0.09263416043, 0.07226065343 },
        { 0.09230654483, 0.09272927165, 0.07158988912 },
        { 0.09181369864, 0.09281737218, 0.07096937714 },
        { 0.09131697426, 0.09289846203, 0.07039658432 }, // Общая точка
        { 0.0907597628, 0.09298942634, 0.06975404141 },
        { 0.09018028722, 0.0930744852, 0.06915155827 },      // 2 кривая Безье
        { 0.08958286867, 0.0931536386, 0.06858921393 },
        { 0.08897148971, 0.09322688656, 0.06806657877 },
        { 0.08834979438, 0.09329422906, 0.06758271455 }, // Общая точка
        { 0.08704879498, 0.09343515428, 0.06657014935 },
        { 0.08564834227, 0.09355545262, 0.06568518012 },     // 3 кривая Безье
        { 0.0841651054, 0.09365512407, 0.06494085682 },
        { 0.08261835363, 0.09373416863, 0.06434690239 },
        { 0.08102850602, 0.09379258629, 0.06390897215 }, // Общая точка
        { 0.08001096525, 0.09382997501, 0.06362868624 },
        { 0.0789403992, 0.09386030103, 0.06340257259 },      // 4 кривая Безье
        { 0.07781970507, 0.09388356437, 0.06324099033 },
        { 0.07665405568, 0.09389976501, 0.06315493243 },
        { 0.07545167069, 0.09390890296, 0.06315493243 }, // Общая точка
        { 0.07455102692, 0.09391574772, 0.06315493243 },
        { 0.07355750039, 0.09391885817, 0.06320323075 },     // 5 кривая Безье
        { 0.07247455408, 0.09391823431, 0.06331965017 },
        { 0.07130985856, 0.09391387613, 0.06352815844 },
        { 0.07008001319, 0.09390578364, 0.06385357051 }, // Общая точка
        { 0.06974013889, 0.09390354724, 0.06394349986 },
        { 0.0693097577, 0.09390041031, 0.06406499171 },      // 6 кривая Безье
        { 0.0687911516, 0.09389637287, 0.06422569034 },
        { 0.0681881908, 0.09389143489, 0.06443741861 },
        { 0.0675088848, 0.0938855964, 0.06471496966 },   // Общая точка
        { 0.06685128587, 0.09387994447, 0.06498365164 },
        { 0.0662819555, 0.09387509155, 0.06524871988 },      // 7 кривая Безье
        { 0.06579711064, 0.09387103764, 0.0654965197 },
        { 0.06539211967, 0.09386778274, 0.06571724343 },
        { 0.06506354041, 0.09386532684, 0.06590426808 }, // Общая точка
        { 0.06359745194, 0.09385436886, 0.06673875376 },
        { 0.06223129762, 0.09384845179, 0.06771126685 },
        { 0.06098112135, 0.09384757564, 0.06880409254 },     // 8 кривая Безье
        { 0.05985807762, 0.09385174041, 0.06999723285 },
        { 0.05886866626, 0.09386094609, 0.07127067986 }, // Общая точка
        { 0.05860227546, 0.09386342464, 0.07161354491 },
        { 0.05822450747, 0.09386767817, 0.07211811656 },     // 9 кривая Безье
        { 0.05775163795, 0.09387370666, 0.07279606636 },
        { 0.05721538328, 0.09388151013, 0.07367058672 },
        { 0.05667044536, 0.09389108857, 0.07475995637 }, // Общая точка
        { 0.05620443796, 0.09389927964, 0.07569153832 },
        { 0.05584496304, 0.0939058288, 0.07655454164 },      // 10 кривая Безье
        { 0.05557013431, 0.09391073607, 0.07733212234 },
        { 0.05536000025, 0.09391400144, 0.0780155792 },
        { 0.0551997702, 0.09391562491, 0.07859970729 },  // Общая точка
        { 0.05495909629, 0.09391806345, 0.07947709821 },
        { 0.05477683797, 0.09391787081, 0.08032787517 },     // 11 кривая Безье
        { 0.05464521563, 0.093915047, 0.08114637917 },
        { 0.05455671332, 0.09390959202, 0.0819286119 },
        { 0.0545040787, 0.09390150586, 0.08267223573 },  // Общая точка
        { 0.05444326251, 0.09389216279, 0.083531449 },
        { 0.05442787669, 0.09387902347, 0.08437449146 },     // 12 кривая Безье
        { 0.05445412982, 0.0938620879, 0.08519809635 },
        { 0.05451817113, 0.09384135608, 0.08599973972 },
        { 0.05461609049, 0.09381682802, 0.08677764046 }, // Общая точка
        { 0.05482384345, 0.09376478746, 0.08842809243 },
        { 0.05519413143, 0.09369254863, 0.09005094326 },     // 13 кривая Безье
        { 0.05572146414, 0.09360011155, 0.09162495441 },
        { 0.0563961919, 0.0934874762, 0.09313058622 },
        { 0.0572056832, 0.09335464259, 0.09455127508 },
    };

    const std::vector<double> NODAL_VECTOR
    {
        0, 0, 0, 0, 0, 0,
        0.0625, 0.0625, 0.0625, 0.0625, 0.0625,
        0.125, 0.125, 0.125, 0.125, 0.125,
        0.25, 0.25, 0.25, 0.25, 0.25,
        0.335603714, 0.335603714, 0.335603714, 0.335603714, 0.335603714,
        0.417801857, 0.417801857, 0.417801857, 0.417801857, 0.417801857,
        0.4589009285, 0.4589009285, 0.4589009285, 0.4589009285, 0.4589009285,
        0.5, 0.5, 0.5, 0.5, 0.5,
        0.625, 0.625, 0.625, 0.625, 0.625,
        0.6875, 0.6875, 0.6875, 0.6875, 0.6875,
        0.75, 0.75, 0.75, 0.75, 0.75,
        0.8125, 0.8125, 0.8125, 0.8125, 0.8125,
        0.875, 0.875, 0.875, 0.875, 0.875,
        1, 1, 1, 1, 1, 1,
    };

    const std::vector<double> WEIGHTS(CONTROL_POINTS.size(), 1);   // Весовые коэффициенты контрольных точек
    const int CURVE_NUM_POINTS = 501;   // Кол-во точек, из которых будет состоять кривая
    const int DEGREE = 5;   // Степень кривой

    Curve originalCurve(CONTROL_POINTS, WEIGHTS, DEGREE, CURVE_NUM_POINTS);
    originalCurve.setNodalVector(NODAL_VECTOR);

    //checkCurveBreakPoint(originalCurve, 0.0625);
    //checkCurveBreakPoint(originalCurve, 0.125);
    //checkCurveBreakPoint(originalCurve, 0.25);
    //checkCurveBreakPoint(originalCurve, 0.335603714);
    //checkCurveBreakPoint(originalCurve, 0.417801857);

    Graph2D canvas(ui->canvas);
    canvas.drawCurve(originalCurve, "Оригинальная кривая", QColor(20, 150, 30));
    //canvas.drawDefiningPolygon(originalCurve.getControlPoints(), "Определяющий многоуг. ориг. кривой");

    // Рисуем точки начал и концов кривых Безье в оригинальной кривой
    canvas.drawPoint(Point3D(0.09369765818, 0.09242290592, 0.07375858978), 7, QColor(12, 50, 250));     // Начало оригинальной кривой
    canvas.drawPoint(Point3D(0.09131697426, 0.09289846203, 0.07039658432), 7, QColor(12, 50, 250));     // Между 1 и 2 кривой
    canvas.drawPoint(Point3D(0.08834979438, 0.09329422906, 0.06758271455), 7, QColor(12, 50, 250));     // Между 2 и 3 кривой
    canvas.drawPoint(Point3D(0.08102850602, 0.09379258629, 0.06390897215), 7, QColor(12, 50, 250));     // Между 3 и 4 кривой
    canvas.drawPoint(Point3D(0.07545167069, 0.09390890296, 0.06315493243), 7, QColor(12, 50, 250));     // Между 4 и 5 кривой
    canvas.drawPoint(Point3D(0.07008001319, 0.09390578364, 0.06385357051), 7, QColor(12, 50, 250));     // Между 5 и 6 кривой
    canvas.drawPoint(Point3D(0.0675088848, 0.0938855964, 0.06471496966), 7, QColor(12, 50, 250));       // Между 6 и 7 кривой
    canvas.drawPoint(Point3D( 0.06506354041, 0.09386532684, 0.06590426808), 7, QColor(12, 50, 250));    // Между 7 и 8 кривой
    canvas.drawPoint(Point3D(0.05886866626, 0.09386094609, 0.07127067986), 7, QColor(12, 50, 250));     // Между 8 и 9 кривой
    canvas.drawPoint(Point3D(0.05667044536, 0.09389108857, 0.07475995637), 7, QColor(12, 50, 250));     // Между 9 и 10 кривой
    canvas.drawPoint(Point3D(0.0551997702, 0.09391562491, 0.07859970729), 7, QColor(12, 50, 250));      // Между 10 и 11 кривой
    canvas.drawPoint(Point3D(0.0545040787, 0.09390150586, 0.08267223573), 7, QColor(12, 50, 250));      // Между 11 и 12 кривой
    canvas.drawPoint(Point3D(0.05461609049, 0.09381682802, 0.08677764046), 7, QColor(12, 50, 250));     // Между 12 и 13 кривой
    canvas.drawPoint(Point3D(0.0572056832, 0.09335464259, 0.09455127508),7, QColor(12, 50, 250));       // Конец оригинальной кривой

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_1
    {
        { 0.09369765818, 0.09242290592, 0.07375858978 },
        { 0.09325591137, 0.09253203852, 0.07298330686 },
        { 0.0927895828, 0.09263416043, 0.07226065343},      // 1
        { 0.09230654483, 0.09272927165, 0.07158988912},
        { 0.09181369864, 0.09281737218, 0.07096937714},
        { 0.09131697426, 0.09289846203, 0.07039658432},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_2
    {
        { 0.09131697426, 0.09289846203, 0.07039658432},
        { 0.0907597628, 0.09298942634, 0.06975404141},
        { 0.09018028722, 0.0930744852, 0.06915155827},      // 2
        { 0.08958286867, 0.0931536386, 0.06858921393},
        { 0.08897148971, 0.09322688656, 0.06806657877},
        { 0.08834979438, 0.09329422906, 0.06758271455},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_3
    {
        { 0.08834979438, 0.09329422906, 0.06758271455},
        { 0.08704879498, 0.09343515428, 0.06657014935},
        { 0.08564834227, 0.09355545262, 0.06568518012},     //3
        { 0.0841651054, 0.09365512407, 0.06494085682},
        { 0.08261835363, 0.09373416863, 0.06434690239},
        { 0.08102850602, 0.09379258629, 0.06390897215},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_4
    {
        { 0.08102850602, 0.09379258629, 0.06390897215},
        { 0.08001096525, 0.09382997501, 0.06362868624},
        { 0.0789403992, 0.09386030103, 0.06340257259},      //4
        { 0.07781970507, 0.09388356437, 0.06324099033},
        { 0.07665405568, 0.09389976501, 0.06315493243},
        { 0.07545167069, 0.09390890296, 0.06315493243},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_5
    {
        { 0.07545167069, 0.09390890296, 0.06315493243},
        { 0.07455102692, 0.09391574772, 0.06315493243},
        { 0.07355750039, 0.09391885817, 0.06320323075},     //5
        { 0.07247455408, 0.09391823431, 0.06331965017},
        { 0.07130985856, 0.09391387613, 0.06352815844},
        { 0.07008001319, 0.09390578364, 0.06385357051},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_6
    {
        { 0.07008001319, 0.09390578364, 0.06385357051},
        { 0.06974013889, 0.09390354724, 0.06394349986},
        { 0.0693097577, 0.09390041031, 0.06406499171},      //6
        { 0.0687911516, 0.09389637287, 0.06422569034},
        { 0.0681881908, 0.09389143489, 0.06443741861},
        { 0.0675088848, 0.0938855964, 0.06471496966},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_7
    {
        { 0.0675088848, 0.0938855964, 0.06471496966},
        { 0.06685128587, 0.09387994447, 0.06498365164},
        { 0.0662819555, 0.09387509155, 0.06524871988},
        { 0.06579711064, 0.09387103764, 0.0654965197},      //7
        { 0.06539211967, 0.09386778274, 0.06571724343},
        { 0.06506354041, 0.09386532684, 0.06590426808},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_8
    {
        { 0.06506354041, 0.09386532684, 0.06590426808},
        { 0.06359745194, 0.09385436886, 0.06673875376},
        { 0.06223129762, 0.09384845179, 0.06771126685},
        { 0.06098112135, 0.09384757564, 0.06880409254},     //8
        { 0.05985807762, 0.09385174041, 0.06999723285},
        { 0.05886866626, 0.09386094609, 0.07127067986},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_9
    {
        { 0.05886866626, 0.09386094609, 0.07127067986},
        { 0.05860227546, 0.09386342464, 0.07161354491},
        { 0.05822450747, 0.09386767817, 0.07211811656},     //9
        { 0.05775163795, 0.09387370666, 0.07279606636},
        { 0.05721538328, 0.09388151013, 0.07367058672},
        { 0.05667044536, 0.09389108857, 0.07475995637},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_10
    {
        { 0.05667044536, 0.09389108857, 0.07475995637},
        { 0.05620443796, 0.09389927964, 0.07569153832},
        { 0.05584496304, 0.0939058288, 0.07655454164},     //10
        { 0.05557013431, 0.09391073607, 0.07733212234},
        { 0.05536000025, 0.09391400144, 0.0780155792},
        { 0.0551997702, 0.09391562491, 0.07859970729},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_11
    {
        { 0.0551997702, 0.09391562491, 0.07859970729},
        { 0.05495909629, 0.09391806345, 0.07947709821},
        { 0.05477683797, 0.09391787081, 0.08032787517},     //11
        { 0.05464521563, 0.093915047, 0.08114637917},
        { 0.05455671332, 0.09390959202, 0.0819286119},
        { 0.0545040787, 0.09390150586, 0.08267223573},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_12
    {
        { 0.0545040787, 0.09390150586, 0.08267223573},
        { 0.05444326251, 0.09389216279, 0.083531449},
        { 0.05442787669, 0.09387902347, 0.08437449146},
        { 0.05445412982, 0.0938620879, 0.08519809635},      //12
        { 0.05451817113, 0.09384135608, 0.08599973972},
        { 0.05461609049, 0.09381682802, 0.08677764046},
    };

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_13
    {
        { 0.05461609049, 0.09381682802, 0.08677764046},
        { 0.05482384345, 0.09376478746, 0.08842809243},
        { 0.05519413143, 0.09369254863, 0.09005094326},     //13
        { 0.05572146414, 0.09360011155, 0.09162495441},
        { 0.0563961919, 0.0934874762, 0.09313058622},
        { 0.0572056832, 0.09335464259, 0.09455127508},
    };

    Curve bezier_1(CONTROL_POINTS_BEZIER_1, std::vector<double> (CONTROL_POINTS_BEZIER_1.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_2(CONTROL_POINTS_BEZIER_2, std::vector<double> (CONTROL_POINTS_BEZIER_2.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_3(CONTROL_POINTS_BEZIER_3, std::vector<double> (CONTROL_POINTS_BEZIER_3.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_4(CONTROL_POINTS_BEZIER_4, std::vector<double> (CONTROL_POINTS_BEZIER_4.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_5(CONTROL_POINTS_BEZIER_5, std::vector<double> (CONTROL_POINTS_BEZIER_5.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_6(CONTROL_POINTS_BEZIER_6, std::vector<double> (CONTROL_POINTS_BEZIER_6.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_7(CONTROL_POINTS_BEZIER_7, std::vector<double> (CONTROL_POINTS_BEZIER_7.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_8(CONTROL_POINTS_BEZIER_8, std::vector<double> (CONTROL_POINTS_BEZIER_8.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_9(CONTROL_POINTS_BEZIER_9, std::vector<double> (CONTROL_POINTS_BEZIER_9.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_10(CONTROL_POINTS_BEZIER_10, std::vector<double> (CONTROL_POINTS_BEZIER_10.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_11(CONTROL_POINTS_BEZIER_11, std::vector<double> (CONTROL_POINTS_BEZIER_11.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_12(CONTROL_POINTS_BEZIER_12, std::vector<double> (CONTROL_POINTS_BEZIER_12.size(), 1), DEGREE, CURVE_NUM_POINTS);
    Curve bezier_13(CONTROL_POINTS_BEZIER_13, std::vector<double> (CONTROL_POINTS_BEZIER_13.size(), 1), DEGREE, CURVE_NUM_POINTS);

    //qDebug() << "До сопряжения:";
    //checkContinuityTwoCurves(bezier_11, bezier_12);

    std::vector<Curve> bezierCurves = {bezier_1, bezier_2, bezier_3, bezier_4, bezier_5, bezier_6, bezier_7, bezier_8, bezier_9, bezier_10, bezier_11, bezier_12, bezier_13};

    //for (const auto& bezierCurve: bezierCurves) // Отрисовываем все кривые Безье до сопряжения
    //{
    //   canvas.drawCurve(bezierCurve, "Кривая Безье отдельно", QColor(20, 30, 150));
    //}

    // Выполнение сопряжение исходной кривой NURBS
    MergeCurves merge;
    bezierCurves = merge.attachAllBezierCurves(originalCurve);

    //for (const auto& bezierCurve: bezierCurves) // Отрисовываем все кривые Безье после сопряжения
    //{
        //canvas.drawCurve(bezierCurve, "", QColor(200, 0, 0));
        //canvas.drawDefiningPolygon(bezierCurve.getControlPoints(), "", QColor(20, 150, 30));
    //}

    Curve merdgedCurve = UsefulUtils::bezierCurvesToNURBSCurve(bezierCurves, DEGREE, CURVE_NUM_POINTS);

    UsefulUtils::outNURBSPoints(originalCurve);
    UsefulUtils::outNURBSPoints(merdgedCurve);

    qDebug() << "До:\n"
             << "Кривизна:" << Metrics::calcCurveCurvature(originalCurve) << '\n'
             << "Метрика Хаусдорфа:" << Metrics::calcHausdorffMetric(originalCurve, merdgedCurve) << '\n';

    qDebug() << "После:\n"
             << "Кривизна:" << Metrics::calcCurveCurvature(merdgedCurve)
             << '\n';

    qDebug() << "Квадратичная разность:" << Metrics::calcQuadraticDifference(originalCurve, merdgedCurve);

    //merdgedCurve.setNodalVector(NODAL_VECTOR);
    canvas.drawCurve(merdgedCurve, "", QColor(200, 0, 0));
    //checkAllCurveBreaks(merdgedCurve);

    /*
    for (size_t i = 0; i != 50; i += 5)
    {
        checkCurveBreakPoint(merdgedCurve, merdgedCurve.getNodalVector()[6 + i]);
    }
    */

    /*
    checkCurveBreakPoint(merdgedCurve, 0.0625);
    checkCurveBreakPoint(merdgedCurve, 0.125);
    checkCurveBreakPoint(merdgedCurve, 0.25);
    checkCurveBreakPoint(merdgedCurve, 0.335603714);
    checkCurveBreakPoint(merdgedCurve, 0.417801857);
    checkCurveBreakPoint(merdgedCurve, 0.4589009285);
    checkCurveBreakPoint(merdgedCurve, 0.5);
    checkCurveBreakPoint(merdgedCurve, 0.625);
    checkCurveBreakPoint(merdgedCurve, 0.6875);
    checkCurveBreakPoint(merdgedCurve, 0.75);
    checkCurveBreakPoint(merdgedCurve, 0.8125);
    checkCurveBreakPoint(merdgedCurve, 0.875);
    */

    //qDebug() << "После сопряжения:";
    //checkContinuityTwoCurves(bezierCurves[0], bezierCurves[1]);
    //checkContinuityTwoCurves(bezierCurves[1], bezierCurves[2]);
    //checkContinuityTwoCurves(bezierCurves[2], bezierCurves[3]);
//}

MainWindow::~MainWindow() { delete ui; }
