#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Plotting/Graph2D.h"
#include "Curve3d/Point3d.h"
#include "Curve3d/CalcCurve.h"
#include "Utils/MergeCurves.h"
#include <Utils/MathUtils.h>

void MainWindow::doubleClickCanvas(QMouseEvent *event)
{
    qDebug() << "\nx =" << ui->canvas->xAxis->pixelToCoord(event->pos().x())
             << "\ny =" << ui->canvas->yAxis->pixelToCoord(event->pos().y());
}

// Переводит вектор кривых в одну кривую
Curve bezierCurvesToCurve(const std::vector<Curve> &bezierCurves, int degree, int curveNumPoints)
{
    std::vector<Point3D> newControlPoints;
    bool firstCheck = false; // Для того, чтобы не было первых дублей точек

    for (size_t curveCount = 0; curveCount != bezierCurves.size(); ++curveCount)
    {
        std::vector<Point3D> tempControlPoints = bezierCurves[curveCount].getControlPoints();

        for (size_t i = 0; i != tempControlPoints.size(); ++i)
        {
            if (firstCheck && i == 0)
            {
                continue;
            }

            if (curveCount == 0)
            {
                firstCheck = true;
            }

            newControlPoints.push_back(tempControlPoints[i]);
        }
    }

    return Curve(newControlPoints, std::vector<double> (newControlPoints.size(), 1), degree, curveNumPoints);
}

// Проверка непрерывности всех точек в одной в кривой
void checkAllCurveBreaks(const Curve& curve)
{
    std::vector<CurvePoint> curvePoint = curve.getCurvePoints();

    for (size_t i = 0; i < curvePoint.size(); ++i)
    {
        if (i == 0 || i == curvePoint.size() - 1)
            continue;

        CurvePoint leftPoint, rightPoint;

        double leftParameter = curvePoint[i].parameter - 0.0000000001;
        double rightParameter = curvePoint[i].parameter + 0.0000000001;

        CalcCurve::calcCurvePointAndDerivs(curve, leftPoint, leftParameter);
        CalcCurve::calcCurvePointAndDerivs(curve, rightPoint, rightParameter);

        for (int j = 0; j < curve.getCurvePoints()[0].derivs.size(); ++j)
        {
            double leftLength = sqrt(leftPoint.derivs[j].x * leftPoint.derivs[j].x + leftPoint.derivs[j].y * leftPoint.derivs[j].y + leftPoint.derivs[j].z * leftPoint.derivs[j].z);
            double rightLength = sqrt(rightPoint.derivs[j].x * rightPoint.derivs[j].x + rightPoint.derivs[j].y * rightPoint.derivs[j].y + rightPoint.derivs[j].z * rightPoint.derivs[j].z);
            double diff = abs(leftLength - rightLength);


            if (diff > 0.01)
            {
                qDebug() << "-----------------Разрыв!!------------------" << j << "-й производной, в точке " << curvePoint[i].parameter << " и diff = " << diff;
            }
        }
    }
}

// Проверяет конкретную точки в кривой на непрерывность
void checkCurveBreakPoint(Curve& curve, double parametr)
{
    CurvePoint leftPoint, rightPoint;
    double leftParameter = parametr - 1e-15;
    double rightParameter =  parametr + 1e-15;

    CalcCurve::calcCurvePointAndDerivs(curve, leftPoint, leftParameter);
    CalcCurve::calcCurvePointAndDerivs(curve, rightPoint, rightParameter);

    for (size_t i = 0; i != leftPoint.derivs.size(); ++i)
    {
        double leftLength = MathUtils::calcRadiusVectorLength(leftPoint.derivs[i]);
        double rightLength = MathUtils::calcRadiusVectorLength(rightPoint.derivs[i]);
        double diff = abs(leftLength - rightLength);

        qDebug() << "Левая кривая в " << parametr << " производная " << i << ":\t" << leftPoint.derivs[i].x << "\t" << leftPoint.derivs[i].y << "\t" << leftPoint.derivs[i].z;
        qDebug() << "Права кривая в " << parametr << " производная " << i << ":\t" << rightPoint.derivs[i].x << "\t" << rightPoint.derivs[i].y << "\t" << rightPoint.derivs[i].z;
        qDebug() << "Difference: " << diff;;

        if (diff > 0.01)
        {
            qDebug() << "-----------------Разрыв!!------------------" << i << "-й производной, в точке " << parametr << " и diff = " << diff;
        }


        qDebug() << "";
    }
}

// Проверка непрерывности двух Безье кривых
void checkContinuityTwoCurves(const Curve& firstBezierCurve, const Curve& secondBezierCurve)
{
    CurvePoint leftPoint, rightPoint;

    double leftParameter = 1 - 0.000001;
    double rightParameter =  0 + 0.000001;

    CalcCurve::calcCurvePointAndDerivs(firstBezierCurve, leftPoint, leftParameter);
    CalcCurve::calcCurvePointAndDerivs(secondBezierCurve, rightPoint, rightParameter);

    bool check = true;

    for (size_t i = 0; i < firstBezierCurve.getCurvePoints()[0].derivs.size(); ++i)
    {
        double leftLength = sqrt(leftPoint.derivs[i].x * leftPoint.derivs[i].x + leftPoint.derivs[i].y * leftPoint.derivs[i].y + leftPoint.derivs[i].z * leftPoint.derivs[i].z);
        double rightLength = sqrt(rightPoint.derivs[i].x * rightPoint.derivs[i].x + rightPoint.derivs[i].y * rightPoint.derivs[i].y + rightPoint.derivs[i].z * rightPoint.derivs[i].z);
        double diff = abs(leftLength - rightLength);

        qDebug() << "Левая кривая: производная " << i << ":\t" << leftPoint.derivs[i].x << "\t" << leftPoint.derivs[i].y << "\t" << leftPoint.derivs[i].z;
        qDebug() << "Права кривая: производная " << i << ":\t" << rightPoint.derivs[i].x << "\t" << rightPoint.derivs[i].y << "\t" << rightPoint.derivs[i].z;
        qDebug() << "Difference: " << diff << '\n';

        if (diff > 0.01)
        {
            qDebug() << "Разрыв " << i << "-й производной, в точке " << &leftPoint.derivs[i] << " и diff = " << diff;
            check = false;
        }
    }

    if (check)
        qDebug() << "Выполняется непрерывность между кривыми!\n";
}

// Пример мерджа через создание новой кривой Безье
/*
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->canvas, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickCanvas(QMouseEvent*)));

    const std::vector<Point3D> CONTROL_POINTS
    {
        { 0.09369765818, 0.09242290592, 0.07375858978 },
        { 0.09325591137, 0.09253203852, 0.07298330686 },
        { 0.0927895828, 0.09263416043, 0.07226065343},      //1
        { 0.09230654483, 0.09272927165, 0.07158988912},
        { 0.09181369864, 0.09281737218, 0.07096937714},
        { 0.09131697426, 0.09289846203, 0.07039658432}, ////
        { 0.0907597628, 0.09298942634, 0.06975404141},
        { 0.09018028722, 0.0930744852, 0.06915155827},      //2
        { 0.08958286867, 0.0931536386, 0.06858921393},
        { 0.08897148971, 0.09322688656, 0.06806657877},
        { 0.08834979438, 0.09329422906, 0.06758271455}, ////
        { 0.08704879498, 0.09343515428, 0.06657014935},
        { 0.08564834227, 0.09355545262, 0.06568518012},     //3
        { 0.0841651054, 0.09365512407, 0.06494085682},
        { 0.08261835363, 0.09373416863, 0.06434690239},
        { 0.08102850602, 0.09379258629, 0.06390897215}, ////
        { 0.08001096525, 0.09382997501, 0.06362868624},
        { 0.0789403992, 0.09386030103, 0.06340257259},      //4
        { 0.07781970507, 0.09388356437, 0.06324099033},
        { 0.07665405568, 0.09389976501, 0.06315493243},
        { 0.07545167069, 0.09390890296, 0.06315493243}, ////
        { 0.07455102692, 0.09391574772, 0.06315493243},
        { 0.07355750039, 0.09391885817, 0.06320323075},     //5
        { 0.07247455408, 0.09391823431, 0.06331965017},
        { 0.07130985856, 0.09391387613, 0.06352815844},
        { 0.07008001319, 0.09390578364, 0.06385357051}, ////
        { 0.06974013889, 0.09390354724, 0.06394349986},
        { 0.0693097577, 0.09390041031, 0.06406499171},      //6
        { 0.0687911516, 0.09389637287, 0.06422569034},
        { 0.0681881908, 0.09389143489, 0.06443741861},
        { 0.0675088848, 0.0938855964, 0.06471496966}, ////
        { 0.06685128587, 0.09387994447, 0.06498365164},
        { 0.0662819555, 0.09387509155, 0.06524871988},
        { 0.06579711064, 0.09387103764, 0.0654965197},      //7
        { 0.06539211967, 0.09386778274, 0.06571724343},
        { 0.06506354041, 0.09386532684, 0.06590426808}, ////
        { 0.06359745194, 0.09385436886, 0.06673875376},
        { 0.06223129762, 0.09384845179, 0.06771126685},
        { 0.06098112135, 0.09384757564, 0.06880409254},     //8
        { 0.05985807762, 0.09385174041, 0.06999723285},
        { 0.05886866626, 0.09386094609, 0.07127067986}, ////
        { 0.05860227546, 0.09386342464, 0.07161354491},
        { 0.05822450747, 0.09386767817, 0.07211811656},     //9
        { 0.05775163795, 0.09387370666, 0.07279606636},
        { 0.05721538328, 0.09388151013, 0.07367058672},
        { 0.05667044536, 0.09389108857, 0.07475995637}, ////
        { 0.05620443796, 0.09389927964, 0.07569153832},
        { 0.05584496304, 0.0939058288, 0.07655454164},     //10
        { 0.05557013431, 0.09391073607, 0.07733212234},
        { 0.05536000025, 0.09391400144, 0.0780155792},
        { 0.0551997702, 0.09391562491, 0.07859970729}, ////
        { 0.05495909629, 0.09391806345, 0.07947709821},
        { 0.05477683797, 0.09391787081, 0.08032787517},     //11
        { 0.05464521563, 0.093915047, 0.08114637917},
        { 0.05455671332, 0.09390959202, 0.0819286119},
        { 0.0545040787, 0.09390150586, 0.08267223573}, ////
        { 0.05444326251, 0.09389216279, 0.083531449},
        { 0.05442787669, 0.09387902347, 0.08437449146},
        { 0.05445412982, 0.0938620879, 0.08519809635},      //12
        { 0.05451817113, 0.09384135608, 0.08599973972},
        { 0.05461609049, 0.09381682802, 0.08677764046},  ////
        { 0.05482384345, 0.09376478746, 0.08842809243},
        { 0.05519413143, 0.09369254863, 0.09005094326},     //13
        { 0.05572146414, 0.09360011155, 0.09162495441},
        { 0.0563961919, 0.0934874762, 0.09313058622},
        { 0.0572056832, 0.09335464259, 0.09455127508},
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
    const int CURVE_NUM_POINTS = 1001;   // Кол-во точек, из которых будет состоять кривая
    const int DEGREE = 5;   // Степень кривой

    Curve originalCurve(CONTROL_POINTS, WEIGHTS, DEGREE, CURVE_NUM_POINTS);
    originalCurve.setNodalVector(NODAL_VECTOR);

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

    qDebug() << "Основная кривая в 0.0625";
    checkCurveBreakPoint(originalCurve, 0.0625);

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_1   // Контрольные точки определяющего многоугольника
    {
        { 0.09369765818, 0.09242290592, 0.07375858978 },
        { 0.09325591137, 0.09253203852, 0.07298330686 },
        { 0.0927895828, 0.09263416043, 0.07226065343},
        { 0.09230654483, 0.09272927165, 0.07158988912},
        { 0.09181369864, 0.09281737218, 0.07096937714},
        { 0.09131697426, 0.09289846203, 0.07039658432}
    };

    Curve bezier_1(CONTROL_POINTS_BEZIER_1, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    std::vector<Point3D> CONTROL_POINTS_BEZIER_2   // Контрольные точки определяющего многоугольника
    {
        { 0.09131697426, 0.09289846203, 0.07039658432},
        { 0.0907597628, 0.09298942634, 0.06975404141},
        { 0.09018028722, 0.0930744852, 0.06915155827},
        { 0.08958286867, 0.0931536386, 0.06858921393},
        { 0.08897148971, 0.09322688656, 0.06806657877},
        { 0.08834979438, 0.09329422906, 0.06758271455}
    };

    Curve bezier_2(CONTROL_POINTS_BEZIER_2, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    //canvas.drawCurve(bezier_1, "Кривая Безье 1", QColor(20, 30, 150));
    //canvas.drawCurve(bezier_2, "Кривая Безье 2", QColor(20, 30, 150));

    /////////////НАЧАЛО - ПРОВЕРКА РАЗРЫВОВ В КРИВЫХ БЕЗЬЕ////////////////////
    /*
    qDebug() << "Проверка двух безье кривых из сплайна (1 и 2 часть)";
    CurvePoint leftPoint, rightPoint;

    double leftParameter = 1 - 0.000001;
    double rightParameter = 0 + 0.000001;

    CalcCurve::calcCurvePointAndDerivs(bezier_1, leftPoint, leftParameter);
    CalcCurve::calcCurvePointAndDerivs(bezier_2, rightPoint, rightParameter);

    for (int i = 0; i != leftPoint.derivs.size(); ++i)
    {
        double leftLength = sqrt(leftPoint.derivs[i].x * leftPoint.derivs[i].x + leftPoint.derivs[i].y * leftPoint.derivs[i].y + leftPoint.derivs[i].z * leftPoint.derivs[i].z);
        double rightLength = sqrt(rightPoint.derivs[i].x * rightPoint.derivs[i].x + rightPoint.derivs[i].y * rightPoint.derivs[i].y + rightPoint.derivs[i].z * rightPoint.derivs[i].z);
        double diff = abs(leftLength - rightLength);

        QString str = "";
        if (i == 0)
            str = "кривой";
        else if (i == 1)
            str = "1-й производной";
        else if (i == 2)
            str = "2-й производной";
        else if (i == 3)
            str = "3-й производной";
        else if (i == 4)
            str = "4-й производной";
        else if (i == 5)
            str = "5-й производной";

        qDebug() << "В точке" << " 1 и 0 двух кривых в " << str << "diff =" << diff;
    }

    qDebug() << '\n';
    */
    /////////////КОНЕЦ - ПРОВЕРКА РАЗРЫВОВ В КРИВЫХ БЕЗЬЕ////////////////////
    /* Расскоментить
    MergeCurves connectCurves;
    Curve newCurve_1_2 = connectCurves.attachCurves(bezier_1, bezier_2, true); // Делаем одну кривую из второй и третьей кривой
    //newCurve.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Сплайн после сопряжения 1 и 2";
    checkCurveBreakPoint(newCurve_1_2, 0.5);;
    */
    //canvas.drawCurve(newCurve_1_2, "Кривая после мерджа 1 и 2", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_1_2.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);

    /*
    std::vector<Point3D> new_CONTROL_POINTS_1   // Контрольные точки определяющего многоугольника
    {
        {0.09369765818, 0.09242290592, 0.07375858978},
        {0.09324784814450444, 0.0925334792474762, 0.0729727954654417},
        {0.0927924326883078, 0.09263330030450219, 0.07226704517736109},
        {0.09232421244071656, 0.09272644887105173, 0.07161023507242507},
        {0.09182594385130015, 0.0928159198645782, 0.07097938542717185},
        {0.09129389973418604, 0.09290225106773256, 0.07036959953593024}
    };

    Curve new_bezier_1(new_CONTROL_POINTS_1, {1, 1, 1, 1, 1, 1}, DEGREE, 501);

    std::vector<Point3D> new_CONTROL_POINTS_2   // Контрольные точки определяющего многоугольника
    {
        {0.09129389973418604, 0.09290225106773256, 0.07036959953593024},
        {0.09076185561707195, 0.09298858227088692, 0.06975981364468863},
        {0.09019603597226018, 0.09307177368366919, 0.06917109150745863},
        {0.08959271361387826, 0.09315236308873037, 0.06859853641856914},
        {0.08896167772061184, 0.09322843113391915, 0.06805533469013969},
        {0.08834979438, 0.09329422906, 0.06758271455}
    };

    Curve new_bezier_2(new_CONTROL_POINTS_2, {1, 1, 1, 1, 1, 1}, DEGREE, 501);
    */
    //////////////ПРОВЕРКА РАЗРЫВОВ В КРИВЫХ БЕЗЬЕ////////////////////
    /*
    qDebug() << "Две кривые безье после сопряжения";

    leftParameter = 1 - 0.000001;
    rightParameter = 0 + 0.000001;

    CalcCurve::calcCurvePointAndDerivs(new_bezier_1, leftPoint, leftParameter);
    CalcCurve::calcCurvePointAndDerivs(new_bezier_2, rightPoint, rightParameter);

    for (int i = 0; i != leftPoint.derivs.size(); ++i)
    {
        double leftLength = sqrt(leftPoint.derivs[i].x * leftPoint.derivs[i].x + leftPoint.derivs[i].y * leftPoint.derivs[i].y + leftPoint.derivs[i].z * leftPoint.derivs[i].z);
        double rightLength = sqrt(rightPoint.derivs[i].x * rightPoint.derivs[i].x + rightPoint.derivs[i].y * rightPoint.derivs[i].y + rightPoint.derivs[i].z * rightPoint.derivs[i].z);
        double diff = abs(leftLength - rightLength);

        QString str = "";
        if (i == 0)
            str = "кривой";
        else if (i == 1)
            str = "1-й производной";
        else if (i == 2)
            str = "2-й производной";
        else if (i == 3)
            str = "3-й производной";
        else if (i == 4)
            str = "4-й производной";
        else if (i == 5)
            str = "5-й производной";

        qDebug() << "В точке" << " 1 и 0 двух кривых в " << str << "diff =" << diff;
    }

    qDebug() << '\n';
    */
    ////////////////////////////////////////////////////


    ///////////////////////////////////////////--------------------------------------------------------------------------------------------------------- 1 и 2 до
    /* Расскоментить
    qDebug() << "Основная кривая в 0.125";
    checkCurveBreakPoint(originalCurve, 0.125);

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_3   // Контрольные точки определяющего многоугольника
    {
        { 0.08834979438, 0.09329422906, 0.06758271455}, ////
        { 0.08704879498, 0.09343515428, 0.06657014935},
        { 0.08564834227, 0.09355545262, 0.06568518012},     //3
        { 0.0841651054, 0.09365512407, 0.06494085682},
        { 0.08261835363, 0.09373416863, 0.06434690239},
        { 0.08102850602, 0.09379258629, 0.06390897215}, ////
    };

    Curve bezier_3(CONTROL_POINTS_BEZIER_3, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    std::vector<Point3D> CONTROL_POINTS_BEZIER_4   // Контрольные точки определяющего многоугольника
    {
        { 0.08102850602, 0.09379258629, 0.06390897215}, ////
        { 0.08001096525, 0.09382997501, 0.06362868624},
        { 0.0789403992, 0.09386030103, 0.06340257259},      //4
        { 0.07781970507, 0.09388356437, 0.06324099033},
        { 0.07665405568, 0.09389976501, 0.06315493243},
        { 0.07545167069, 0.09390890296, 0.06315493243}, ////
    };

    Curve bezier_4(CONTROL_POINTS_BEZIER_4, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    //canvas.drawCurve(bezier_3, "Кривая Безье 3", QColor(20, 30, 150));
    //canvas.drawCurve(bezier_4, "Кривая Безье 4", QColor(20, 30, 150));

    /////////////НАЧАЛО - ПРОВЕРКА РАЗРЫВОВ В КРИВЫХ БЕЗЬЕ////////////////////
    /*
    qDebug() << "Проверка двух безье кривых из сплайна (3 и 4 часть)";
    CurvePoint leftPoint, rightPoint;

    double leftParameter = 1 - 0.000001;
    double rightParameter = 0 + 0.000001;

    CalcCurve::calcCurvePointAndDerivs(bezier_3, leftPoint, leftParameter);
    CalcCurve::calcCurvePointAndDerivs(bezier_4, rightPoint, rightParameter);

    for (int i = 0; i != leftPoint.derivs.size(); ++i)
    {
        double leftLength = sqrt(leftPoint.derivs[i].x * leftPoint.derivs[i].x + leftPoint.derivs[i].y * leftPoint.derivs[i].y + leftPoint.derivs[i].z * leftPoint.derivs[i].z);
        double rightLength = sqrt(rightPoint.derivs[i].x * rightPoint.derivs[i].x + rightPoint.derivs[i].y * rightPoint.derivs[i].y + rightPoint.derivs[i].z * rightPoint.derivs[i].z);
        double diff = abs(leftLength - rightLength);

        QString str = "";
        if (i == 0)
            str = "кривой";
        else if (i == 1)
            str = "1-й производной";
        else if (i == 2)
            str = "2-й производной";
        else if (i == 3)
            str = "3-й производной";
        else if (i == 4)
            str = "4-й производной";
        else if (i == 5)
            str = "5-й производной";

        qDebug() << "В точке" << " 1 и 0 двух кривых в " << str << "diff =" << diff;
    }

    qDebug() << '\n';
    */
    /////////////КОНЕЦ - ПРОВЕРКА РАЗРЫВОВ В КРИВЫХ БЕЗЬЕ////////////////////
    /* Раскоментить
    Curve newCurve_3_4 = connectCurves.attachCurves(bezier_3, bezier_4, true); // Делаем одну кривую из второй и третьей кривой
    //newCurve.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Безье после сопряжения 3 и 4";
    checkCurveBreakPoint(newCurve_3_4, 0.5);;

    //canvas.drawCurve(newCurve_3_4, "Кривая после мерджа 3 и 4", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_3_4.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);

    //////////////ПРОВЕРКА РАЗРЫВОВ В КРИВЫХ БЕЗЬЕ////////////////////
    /*
    qDebug() << "Две кривые безье после сопряжения";

    leftParameter = 1 - 0.000001;
    rightParameter = 0 + 0.000001;

    CalcCurve::calcCurvePointAndDerivs(new_bezier_1, leftPoint, leftParameter);
    CalcCurve::calcCurvePointAndDerivs(new_bezier_2, rightPoint, rightParameter);

    for (int i = 0; i != leftPoint.derivs.size(); ++i)
    {
        double leftLength = sqrt(leftPoint.derivs[i].x * leftPoint.derivs[i].x + leftPoint.derivs[i].y * leftPoint.derivs[i].y + leftPoint.derivs[i].z * leftPoint.derivs[i].z);
        double rightLength = sqrt(rightPoint.derivs[i].x * rightPoint.derivs[i].x + rightPoint.derivs[i].y * rightPoint.derivs[i].y + rightPoint.derivs[i].z * rightPoint.derivs[i].z);
        double diff = abs(leftLength - rightLength);

        QString str = "";
        if (i == 0)
            str = "кривой";
        else if (i == 1)
            str = "1-й производной";
        else if (i == 2)
            str = "2-й производной";
        else if (i == 3)
            str = "3-й производной";
        else if (i == 4)
            str = "4-й производной";
        else if (i == 5)
            str = "5-й производной";

        qDebug() << "В точке" << " 1 и 0 двух кривых в " << str << "diff =" << diff;
    }

    qDebug() << '\n';
    */
    ////////////////////////////////////////////////////


    ///////////////////////////////////////////--------------------------------------------------------------------------------------------------------- 3 и 4 до

    /* это сплайн по 1-2 и 3-4 кривой безье
    const std::vector<Point3D> CONTROL_POINTS_1_2_3_4  // Контрольные точки определяющего многоугольника
    {
        {0.09369765818, 0.09242290592, 0.07375858978},
        {0.09279803810900888, 0.09264405257495238, 0.07218700115088342},
        {0.0918759963552134, 0.09282219014810399, 0.07093558862767751},
        {0.09087393795306564, 0.09298995687924476, 0.0697555214637178},
        {0.08957356106122386, 0.09316263320783867, 0.06852795483027957},
        {0.08834979437999924, 0.09329422905999922, 0.06758271454999945},
        {0.08592642665159513, 0.09356844742249572, 0.06561374390571872},
        {0.08212657185674636, 0.09380997872948475, 0.0637868325281847},
        {0.07951227150097458, 0.09385316691753855, 0.06343771387046643},
        {0.07803313024817236, 0.09388554396169006, 0.06319163669149067},
        {0.0754516706899996, 0.09390890296000078, 0.06315493243000014},
    };

    Curve curve_1_2_3_4(CONTROL_POINTS_1_2_3_4, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    checkCurveBreakPoint(curve_1_2_3_4, 0.5);
    checkAllCurveBreaks(curve_1_2_3_4);
    canvas.drawCurve(curve_1_2_3_4, "Кривая после мерджа 1 2 3 4", QColor(16, 25, 150));
    canvas.drawDefiningPolygon(curve_1_2_3_4.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);
    */
    /* Раскоментить
    Curve newCurve_1_2_3_4 = connectCurves.attachCurves(newCurve_1_2, newCurve_3_4, true); // Делаем одну кривую из второй и третьей кривой
    //newCurve.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Безье после сопряжения 1 и 2 и 3 и 4";
    checkCurveBreakPoint(newCurve_1_2_3_4, 0.5);

    //canvas.drawCurve(newCurve_1_2_3_4, "Кривая после мерджа 1 и 2 и 3 и 4", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_1_2_3_4.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);

    ///////////////////////////////////////////--------------------------------------------------------------------------------------------------------- 1 и 2 до

    qDebug() << "Основная кривая в 0.25";
    checkCurveBreakPoint(originalCurve, 0.25);

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_5   // Контрольные точки определяющего многоугольника
    {
        { 0.07545167069, 0.09390890296, 0.06315493243}, ////
        { 0.07455102692, 0.09391574772, 0.06315493243},
        { 0.07355750039, 0.09391885817, 0.06320323075},     //5
        { 0.07247455408, 0.09391823431, 0.06331965017},
        { 0.07130985856, 0.09391387613, 0.06352815844},
        { 0.07008001319, 0.09390578364, 0.06385357051}, ////
    };

    Curve bezier_5(CONTROL_POINTS_BEZIER_5, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    std::vector<Point3D> CONTROL_POINTS_BEZIER_6   // Контрольные точки определяющего многоугольника
    {
        { 0.07008001319, 0.09390578364, 0.06385357051}, ////
        { 0.06974013889, 0.09390354724, 0.06394349986},
        { 0.0693097577, 0.09390041031, 0.06406499171},      //6
        { 0.0687911516, 0.09389637287, 0.06422569034},
        { 0.0681881908, 0.09389143489, 0.06443741861},
        { 0.0675088848, 0.0938855964, 0.06471496966}, ////
    };

    Curve bezier_6(CONTROL_POINTS_BEZIER_6, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    //canvas.drawCurve(bezier_3, "Кривая Безье 3", QColor(20, 30, 150));
    //canvas.drawCurve(bezier_4, "Кривая Безье 4", QColor(20, 30, 150));

    Curve newCurve_5_6 = connectCurves.attachCurves(bezier_5, bezier_6, true); // Делаем одну кривую из второй и третьей кривой
    //newCurve.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Безье после сопряжения 5 и 6";
    checkCurveBreakPoint(newCurve_3_4, 0.5);;

    //canvas.drawCurve(newCurve_5_6, "Кривая после мерджа 5 и 6", QColor(16, 25, 150));

    Curve newCurve_1_2_3_4_5_6 = connectCurves.attachCurves(newCurve_1_2_3_4, newCurve_5_6, true); // Делаем одну кривую из второй и третьей кривой
    //newCurve.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Безье после сопряжения 1-2-3-4-5-6";
    checkCurveBreakPoint(newCurve_1_2_3_4_5_6, 0.5);

    canvas.drawCurve(newCurve_1_2_3_4_5_6, "Кривая после мерджа 1-2-3-4-5-6", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_1_2_3_4_5_6.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);
}
*/

/*
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->canvas, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickCanvas(QMouseEvent*)));

    const std::vector<Point3D> CONTROL_POINTS
    {
        { 0.09369765818, 0.09242290592, 0.07375858978 },
        { 0.09325591137, 0.09253203852, 0.07298330686 },
        { 0.0927895828, 0.09263416043, 0.07226065343},      //1
        { 0.09230654483, 0.09272927165, 0.07158988912},
        { 0.09181369864, 0.09281737218, 0.07096937714},
        { 0.09131697426, 0.09289846203, 0.07039658432}, ////
        { 0.0907597628, 0.09298942634, 0.06975404141},
        { 0.09018028722, 0.0930744852, 0.06915155827},      //2
        { 0.08958286867, 0.0931536386, 0.06858921393},
        { 0.08897148971, 0.09322688656, 0.06806657877},
        { 0.08834979438, 0.09329422906, 0.06758271455}, ////
        { 0.08704879498, 0.09343515428, 0.06657014935},
        { 0.08564834227, 0.09355545262, 0.06568518012},     //3
        { 0.0841651054, 0.09365512407, 0.06494085682},
        { 0.08261835363, 0.09373416863, 0.06434690239},
        { 0.08102850602, 0.09379258629, 0.06390897215}, ////
        { 0.08001096525, 0.09382997501, 0.06362868624},
        { 0.0789403992, 0.09386030103, 0.06340257259},      //4
        { 0.07781970507, 0.09388356437, 0.06324099033},
        { 0.07665405568, 0.09389976501, 0.06315493243},
        { 0.07545167069, 0.09390890296, 0.06315493243}, ////
        { 0.07455102692, 0.09391574772, 0.06315493243},
        { 0.07355750039, 0.09391885817, 0.06320323075},     //5
        { 0.07247455408, 0.09391823431, 0.06331965017},
        { 0.07130985856, 0.09391387613, 0.06352815844},
        { 0.07008001319, 0.09390578364, 0.06385357051}, ////
        { 0.06974013889, 0.09390354724, 0.06394349986},
        { 0.0693097577, 0.09390041031, 0.06406499171},      //6
        { 0.0687911516, 0.09389637287, 0.06422569034},
        { 0.0681881908, 0.09389143489, 0.06443741861},
        { 0.0675088848, 0.0938855964, 0.06471496966}, ////
        { 0.06685128587, 0.09387994447, 0.06498365164},
        { 0.0662819555, 0.09387509155, 0.06524871988},
        { 0.06579711064, 0.09387103764, 0.0654965197},      //7
        { 0.06539211967, 0.09386778274, 0.06571724343},
        { 0.06506354041, 0.09386532684, 0.06590426808}, ////
        { 0.06359745194, 0.09385436886, 0.06673875376},
        { 0.06223129762, 0.09384845179, 0.06771126685},
        { 0.06098112135, 0.09384757564, 0.06880409254},     //8
        { 0.05985807762, 0.09385174041, 0.06999723285},
        { 0.05886866626, 0.09386094609, 0.07127067986}, ////
        { 0.05860227546, 0.09386342464, 0.07161354491},
        { 0.05822450747, 0.09386767817, 0.07211811656},     //9
        { 0.05775163795, 0.09387370666, 0.07279606636},
        { 0.05721538328, 0.09388151013, 0.07367058672},
        { 0.05667044536, 0.09389108857, 0.07475995637}, ////
        { 0.05620443796, 0.09389927964, 0.07569153832},
        { 0.05584496304, 0.0939058288, 0.07655454164},     //10
        { 0.05557013431, 0.09391073607, 0.07733212234},
        { 0.05536000025, 0.09391400144, 0.0780155792},
        { 0.0551997702, 0.09391562491, 0.07859970729}, ////
        { 0.05495909629, 0.09391806345, 0.07947709821},
        { 0.05477683797, 0.09391787081, 0.08032787517},     //11
        { 0.05464521563, 0.093915047, 0.08114637917},
        { 0.05455671332, 0.09390959202, 0.0819286119},
        { 0.0545040787, 0.09390150586, 0.08267223573}, ////
        { 0.05444326251, 0.09389216279, 0.083531449},
        { 0.05442787669, 0.09387902347, 0.08437449146},
        { 0.05445412982, 0.0938620879, 0.08519809635},      //12
        { 0.05451817113, 0.09384135608, 0.08599973972},
        { 0.05461609049, 0.09381682802, 0.08677764046},  ////
        { 0.05482384345, 0.09376478746, 0.08842809243},
        { 0.05519413143, 0.09369254863, 0.09005094326},     //13
        { 0.05572146414, 0.09360011155, 0.09162495441},
        { 0.0563961919, 0.0934874762, 0.09313058622},
        { 0.0572056832, 0.09335464259, 0.09455127508},
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
    const int CURVE_NUM_POINTS = 1001;   // Кол-во точек, из которых будет состоять кривая
    const int DEGREE = 5;   // Степень кривой

    Curve originalCurve(CONTROL_POINTS, WEIGHTS, DEGREE, CURVE_NUM_POINTS);
    originalCurve.setNodalVector(NODAL_VECTOR);

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


    const std::vector<Point3D> CONTROL_POINTS_BEZIER_1   // Контрольные точки определяющего многоугольника
    {
        { 0.09369765818, 0.09242290592, 0.07375858978 },
        { 0.09325591137, 0.09253203852, 0.07298330686 },
        { 0.0927895828, 0.09263416043, 0.07226065343},
        { 0.09230654483, 0.09272927165, 0.07158988912},
        { 0.09181369864, 0.09281737218, 0.07096937714},
        { 0.09131697426, 0.09289846203, 0.07039658432}
    };

    Curve bezier_1(CONTROL_POINTS_BEZIER_1, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    std::vector<Point3D> CONTROL_POINTS_BEZIER_2   // Контрольные точки определяющего многоугольника
    {
        { 0.09131697426, 0.09289846203, 0.07039658432},
        { 0.0907597628, 0.09298942634, 0.06975404141},
        { 0.09018028722, 0.0930744852, 0.06915155827},
        { 0.08958286867, 0.0931536386, 0.06858921393},
        { 0.08897148971, 0.09322688656, 0.06806657877},
        { 0.08834979438, 0.09329422906, 0.06758271455}
    };

    Curve bezier_2(CONTROL_POINTS_BEZIER_2, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    //canvas.drawCurve(bezier_1, "Кривая Безье 1", QColor(20, 30, 150));
    //canvas.drawCurve(bezier_2, "Кривая Безье 2", QColor(20, 30, 150));

    MergeCurves connectCurves;
    Curve newCurve_1_2 = connectCurves.attachCurves(bezier_1, bezier_2, true); // Делаем одну кривую из второй и третьей кривой
    newCurve_1_2.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Сплайн после сопряжения 1 и 2";
    checkCurveBreakPoint(newCurve_1_2, 0.5);;

    //canvas.drawCurve(newCurve_1_2, "Кривая после сопряжения 1 и 2", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_1_2.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);



    const std::vector<Point3D> CONTROL_POINTS_BEZIER_3   // Контрольные точки определяющего многоугольника
    {
        { 0.08834979438, 0.09329422906, 0.06758271455}, ////
        { 0.08704879498, 0.09343515428, 0.06657014935},
        { 0.08564834227, 0.09355545262, 0.06568518012},     //3
        { 0.0841651054, 0.09365512407, 0.06494085682},
        { 0.08261835363, 0.09373416863, 0.06434690239},
        { 0.08102850602, 0.09379258629, 0.06390897215}, ////
    };

    Curve bezier_3(CONTROL_POINTS_BEZIER_3, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    std::vector<Point3D> CONTROL_POINTS_BEZIER_4   // Контрольные точки определяющего многоугольника
    {
        { 0.08102850602, 0.09379258629, 0.06390897215}, ////
        { 0.08001096525, 0.09382997501, 0.06362868624},
        { 0.0789403992, 0.09386030103, 0.06340257259},      //4
        { 0.07781970507, 0.09388356437, 0.06324099033},
        { 0.07665405568, 0.09389976501, 0.06315493243},
        { 0.07545167069, 0.09390890296, 0.06315493243}, ////
    };

    Curve bezier_4(CONTROL_POINTS_BEZIER_4, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    //canvas.drawCurve(bezier_3, "Кривая Безье 3", QColor(20, 30, 150));
    //canvas.drawCurve(bezier_4, "Кривая Безье 4", QColor(20, 30, 150));

    Curve newCurve_3_4 = connectCurves.attachCurves(bezier_3, bezier_4, true); // Делаем одну кривую из второй и третьей кривой
    newCurve_3_4.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Сплайн после сопряжения 3 и 4";
    checkCurveBreakPoint(newCurve_3_4, 0.5);;

    //canvas.drawCurve(newCurve_3_4, "Кривая после сопряжения 3 и 4", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_3_4.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);


    Curve newCurve_1_2_3_4 = connectCurves.attachCurves(newCurve_1_2, newCurve_3_4, true); // Делаем одну кривую из второй и третьей кривой
    newCurve_1_2_3_4.setNodalVector({0, 0, 0, 0, 0, 0, 0.25, 0.25, 0.25, 0.25, 0.25,
                                     0.5, 0.5, 0.5, 0.5, 0.5, 0.75, 0.75, 0.75, 0.75, 0.75,
                                     1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Сплайн после сопряжения 1, 2, 3, 4";
    checkCurveBreakPoint(newCurve_1_2_3_4, 0.5);
    //checkAllCurveBreaks(newCurve_1_2_3_4);

    //canvas.drawCurve(newCurve_1_2_3_4, "Сплайн после сопряжения 1, 2, 3, 4", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_1_2_3_4.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);

    const std::vector<Point3D> CONTROL_POINTS_BEZIER_5   // Контрольные точки определяющего многоугольника
    {
        { 0.07545167069, 0.09390890296, 0.06315493243}, ////
        { 0.07455102692, 0.09391574772, 0.06315493243},
        { 0.07355750039, 0.09391885817, 0.06320323075},     //5
        { 0.07247455408, 0.09391823431, 0.06331965017},
        { 0.07130985856, 0.09391387613, 0.06352815844},
        { 0.07008001319, 0.09390578364, 0.06385357051}, ////
    };

    Curve bezier_5(CONTROL_POINTS_BEZIER_5, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    std::vector<Point3D> CONTROL_POINTS_BEZIER_6   // Контрольные точки определяющего многоугольника
    {
        { 0.07008001319, 0.09390578364, 0.06385357051}, ////
        { 0.06974013889, 0.09390354724, 0.06394349986},
        { 0.0693097577, 0.09390041031, 0.06406499171},      //6
        { 0.0687911516, 0.09389637287, 0.06422569034},
        { 0.0681881908, 0.09389143489, 0.06443741861},
        { 0.0675088848, 0.0938855964, 0.06471496966}, ////
    };

    Curve bezier_6(CONTROL_POINTS_BEZIER_6, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    //canvas.drawCurve(bezier_3, "Кривая Безье 3", QColor(20, 30, 150));
    //canvas.drawCurve(bezier_4, "Кривая Безье 4", QColor(20, 30, 150));

    Curve newCurve_5_6 = connectCurves.attachCurves(bezier_5, bezier_6, true); // Делаем одну кривую из второй и третьей кривой
    newCurve_5_6.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Сплайн после сопряжения 5 и 6";
    checkCurveBreakPoint(newCurve_5_6, 0.5);;

    //canvas.drawCurve(newCurve_3_4, "Кривая после сопряжения 3 и 4", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_3_4.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);





    const std::vector<Point3D> CONTROL_POINTS_BEZIER_7   // Контрольные точки определяющего многоугольника
    {
        { 0.0675088848, 0.0938855964, 0.06471496966}, ////
        { 0.06685128587, 0.09387994447, 0.06498365164},
        { 0.0662819555, 0.09387509155, 0.06524871988},
        { 0.06579711064, 0.09387103764, 0.0654965197},      //7
        { 0.06539211967, 0.09386778274, 0.06571724343},
        { 0.06506354041, 0.09386532684, 0.06590426808}, ////
    };

    Curve bezier_7(CONTROL_POINTS_BEZIER_7, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    std::vector<Point3D> CONTROL_POINTS_BEZIER_8   // Контрольные точки определяющего многоугольника
    {
        { 0.06506354041, 0.09386532684, 0.06590426808}, ////
        { 0.06359745194, 0.09385436886, 0.06673875376},
        { 0.06223129762, 0.09384845179, 0.06771126685},
        { 0.06098112135, 0.09384757564, 0.06880409254},     //8
        { 0.05985807762, 0.09385174041, 0.06999723285},
        { 0.05886866626, 0.09386094609, 0.07127067986}, ////
    };

    Curve bezier_8(CONTROL_POINTS_BEZIER_8, {1, 1, 1, 1, 1, 1}, DEGREE, 1001);

    //canvas.drawCurve(bezier_3, "Кривая Безье 3", QColor(20, 30, 150));
    //canvas.drawCurve(bezier_4, "Кривая Безье 4", QColor(20, 30, 150));

    Curve newCurve_7_8 = connectCurves.attachCurves(bezier_7, bezier_8, true); // Делаем одну кривую из второй и третьей кривой
    newCurve_7_8.setNodalVector({0, 0, 0, 0, 0, 0, 0.5, 0.5, 0.5, 0.5, 0.5, 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Сплайн после сопряжения 7 и 8";
    checkCurveBreakPoint(newCurve_7_8, 0.5);;

    //canvas.drawCurve(newCurve_3_4, "Кривая после сопряжения 3 и 4", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_3_4.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);




    Curve newCurve_5_6_7_8 = connectCurves.attachCurves(newCurve_5_6, newCurve_7_8, true); // Делаем одну кривую из второй и третьей кривой
    newCurve_5_6_7_8.setNodalVector({0, 0, 0, 0, 0, 0, 0.25, 0.25, 0.25, 0.25, 0.25,
                                     0.5, 0.5, 0.5, 0.5, 0.5, 0.75, 0.75, 0.75, 0.75, 0.75,
                                     1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Сплайн после сопряжения 5_6_7_8";

    //canvas.drawCurve(newCurve_5_6_7_8, "Сплайн после сопряжения 5_6_7_8", QColor(16, 25, 150));
    //canvas.drawDefiningPolygon(newCurve_5_6_7_8.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);



    Curve newCurve_1_8 = connectCurves.attachCurves(newCurve_1_2_3_4, newCurve_5_6_7_8, true); // Делаем одну кривую из второй и третьей кривой
    newCurve_1_8.setNodalVector({0, 0, 0, 0, 0, 0,
                                 0.125, 0.125, 0.125, 0.125, 0.125,
                                 0.25, 0.25, 0.25, 0.25, 0.25,
                                 0.375, 0.375, 0.375, 0.375, 0.375,
                                 0.5, 0.5, 0.5, 0.5, 0.5,
                                 0.625,  0.625,  0.625,  0.625,  0.625,
                                 0.75, 0.75, 0.75, 0.75, 0.75,
                                 0.875, 0.875, 0.875, 0.875, 0.875,
                                 1, 1, 1, 1, 1, 1}); // Для кривой безье не работает
    qDebug() << "Сплайн после сопряжения 1-8";
    checkAllCurveBreaks(newCurve_1_8);

    canvas.drawCurve(newCurve_1_8, "Сплайн после сопряжения 1-8", QColor(16, 25, 150));
    canvas.drawDefiningPolygon(newCurve_1_8.getControlPoints(), "", QColor(0,0,0), Qt::PenStyle::SolidLine);
}
*/

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
    bezierCurves = merge.attachAllBezierCurves(bezierCurves);

    for (const auto& bezierCurve: bezierCurves)
    {
        //canvas.drawCurve(bezierCurve, "", QColor(20, 0, 230));
        //canvas.drawDefiningPolygon(bezierCurve.getControlPoints(), "", QColor(20, 150, 30));
    }

    Curve merdgedCurve = bezierCurvesToCurve(bezierCurves, DEGREE, CURVE_NUM_POINTS);

    merdgedCurve.setNodalVector({0, 0, 0, 0.3333333333333333, 0.3333333333333333, 0.6666666666666666, 0.6666666666666666, 1, 1, 1});
    canvas.drawCurve(merdgedCurve, "", QColor(200, 0, 0));

    checkCurveBreakPoint(merdgedCurve, 0.3333333333333333);
    checkCurveBreakPoint(merdgedCurve, 0.6666666666666666);
    checkAllCurveBreaks(merdgedCurve);
}
*/

/*
// Тестовый пример соединения
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->canvas, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(doubleClickCanvas(QMouseEvent*)));

    const std::vector<Point3D> CONTROL_POINTS
    {
        {1, 1.2, 0},
        {1.35, 1.25, 0},
        {1.425, 1.3199999999999998, 0},
        {1.7125, 1.3733333333333333, 0},
        {2, 1.4266666666666667, 0},
        {2.5, 1.4633333333333334, 0},
        {2.783333333333333, 1.4766666666666666, 0},
        {3.066666666666667, 1.49, 0},
        {3.1333333333333337, 1.48, 0},
        {3.491666666666667, 1.455, 0},
        {3.85, 1.43, 0},
        {4.5, 1.39, 0},
        {5, 1.2, 0},
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
    const int CURVE_NUM_POINTS = 61;   // Кол-во точек, из которых будет состоять кривая
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
        {1, 1.2, 0},
        {1.35, 1.25, 0},
        {1.425, 1.3199999999999998, 0},
        {1.7125, 1.3733333333333333, 0}
    };

    std::vector<Point3D> controlPointsBezier_2
    {
        {1.7125, 1.3733333333333333, 0},
        {2, 1.4266666666666667, 0},
        {2.5, 1.4633333333333334, 0},
        {2.783333333333333, 1.4766666666666666, 0}
    };

    std::vector<Point3D> controlPointsBezier_3
    {
        {2.783333333333333, 1.4766666666666666, 0},
        {3.066666666666667, 1.49, 0},
        {3.1333333333333337, 1.48, 0},
        {3.491666666666667, 1.455, 0},
    };

    std::vector<Point3D> controlPointsBezier_4
    {
        {3.491666666666667, 1.455, 0},
        {3.85, 1.43, 0},
        {4.5, 1.39, 0},
        {5, 1.2, 0},
    };

    // Нарущаем непрерывность в точках соединения Безье Кривых
    controlPointsBezier_1[1].y += 1 * 0.09;
    controlPointsBezier_2[1].y += - 0.0 * 0.0;
    controlPointsBezier_3[1].y += 1 * 0.09;
    controlPointsBezier_4[1].y += 1 * 0.09;

    const std::vector<double> weightsBezierCurves(controlPointsBezier_1.size(), 1);   // Весовые коэффициенты контрольных точек

    Curve bezier_1(controlPointsBezier_1, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);
    Curve bezier_2(controlPointsBezier_2, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);
    Curve bezier_3(controlPointsBezier_3, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);
    Curve bezier_4(controlPointsBezier_4, weightsBezierCurves, DEGREE, CURVE_NUM_POINTS);

    canvas.drawCurve(bezier_1, "Безье 1", QColor(20, 255, 30));
    canvas.drawCurve(bezier_2, "Безье 2", QColor(20, 200, 30));
    canvas.drawCurve(bezier_3, "Безье 3", QColor(20, 100, 30));
    canvas.drawCurve(bezier_4, "Безье 4", QColor(20, 50, 30));
    //canvas.drawDefiningPolygon(bezier_1.getControlPoints(), "", QColor(20, 150, 30), Qt::DashLine);
    //canvas.drawDefiningPolygon(bezier_2.getControlPoints(), "", QColor(20, 200, 30), Qt::DashLine);
    //canvas.drawDefiningPolygon(bezier_3.getControlPoints(), "", QColor(20, 250, 30), Qt::DashLine);
    // 2. Конец

    // 3. Соединяем B-сплайны

    std::vector<Curve> bezierCurves {bezier_1, bezier_2, bezier_3, bezier_4};

    MergeCurves merge;
    bezierCurves = merge.attachAllBezierCurves(bezierCurves);

    for (const auto& bezierCurve: bezierCurves)
    {
        //canvas.drawCurve(bezierCurve, "", QColor(20, 0, 230));
        //canvas.drawDefiningPolygon(bezierCurve.getControlPoints(), "", QColor(20, 150, 30));
    }

    Curve merdgedCurve = bezierCurvesToCurve(bezierCurves, DEGREE, CURVE_NUM_POINTS);

    merdgedCurve.setNodalVector(NODAL_VECTOR);
    canvas.drawCurve(merdgedCurve, "", QColor(200, 0, 0));

    checkCurveBreakPoint(merdgedCurve, 0.2625);
    checkCurveBreakPoint(merdgedCurve, 0.55);
    checkCurveBreakPoint(merdgedCurve, 0.6678);
    checkAllCurveBreaks(merdgedCurve);
}
*/


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
    const int CURVE_NUM_POINTS = 2001;   // Кол-во точек, из которых будет состоять кривая
    const int DEGREE = 5;   // Степень кривой

    Curve originalCurve(CONTROL_POINTS, WEIGHTS, DEGREE, CURVE_NUM_POINTS);
    originalCurve.setNodalVector(NODAL_VECTOR);

    checkCurveBreakPoint(originalCurve, 0.0625);
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

    MergeCurves merge;
    bezierCurves = merge.attachAllBezierCurves(bezierCurves);

    //for (const auto& bezierCurve: bezierCurves) // Отрисовываем все кривые Безье после сопряжения
    //{
        //canvas.drawCurve(bezierCurve, "", QColor(200, 0, 0));
        //canvas.drawDefiningPolygon(bezierCurve.getControlPoints(), "", QColor(20, 150, 30));
    //}

    Curve merdgedCurve = bezierCurvesToCurve(bezierCurves, DEGREE, CURVE_NUM_POINTS);

    //merdgedCurve.setNodalVector(NODAL_VECTOR);
    canvas.drawCurve(merdgedCurve, "", QColor(200, 0, 0));
    checkAllCurveBreaks(merdgedCurve);


    for (size_t i = 0; i != 50; i += 5)
    {
        //checkCurveBreakPoint(merdgedCurve, merdgedCurve.getNodalVector()[6 + i]);
    }

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

}


MainWindow::~MainWindow() { delete ui; }
