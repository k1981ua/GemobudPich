#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QHash>
#include <QMap>

#include "qcustomplot.h"
#include "modbusreader.h"
#include "analoginputchannel.h"
#include "dialogcalibr.h"
#include "dialogconfig.h"
#include "dialogtableresult.h"


#define X_RANGEPRETEST_MIN 0  //seconds стартовая ширина полотна графика по х для режима перед тестов 0 minutes
#define X_RANGEPRETEST_MAX 1800  //seconds стартовая ширина полотна графика по х для режима перед тестов 30 minutes

#define X_RANGETEST_MIN -600  //seconds стартовая ширина полотна графика по х для режима тестов -10 minutes
#define X_RANGETEST_MAX 1800  //seconds стартовая ширина полотна графика по х для режима тестов 30 minutes

//#define X_RANGEVIEW 3600   //стартовая ширина полотна графика по х для режима просмотра
#define X_TICKSTEP 200  //20  //деления на шкале интервал 5 минут

//#define Y_MOVEMENT_RANGE_MAX 50  //стартовая ширина полотна графика по y  units mm
//#define Y_MOVEMENT_RANGE_MIN -50

#define Y_TEMPERATURE_RANGE_MAX 1300  //стартовая ширина полотна графика по y  units mm
#define Y_TEMPERATURE_RANGE_MIN 0

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


enum RunningMode
{
    ModePreTest=0,
    ModeTest,
    ModeTestStopped
};

enum CommandButton
{
    NoCmd=0,
    StartCmd,
    StopCmd
};

struct OperatorPoint
{
    double time_min;
    double operatorEnteredPoint;
    double movement;
    double temperature;
};

//=====================================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    RunningMode runningMode=ModePreTest;
    CommandButton cmdButton=NoCmd;
    QString infoText;

    int maxTemperature=900;
    bool interfaceStabConditions=true;
    bool interfaceTavg=true;
    bool interfaceT_Tavg=true;
    bool interfaceTreg=true;
    bool interfaceAutoMode=false;

    QDateTime startPreTestDT;
    QString startPreTestDT_str;
    QDateTime startTestDT;
    QString startTestDT_str;

    QString csvFileName;



//    QVector<OperatorPoint> levelUpPoints_1;
//    QVector<OperatorPoint> levelDownPoints_1;
//    QVector<OperatorPoint> levelUpPoints_2;
//    QVector<OperatorPoint> levelDownPoints_2;

    QPalette orig_palette;

    QCustomPlot *wGraphic_1;

    QCPGraph *graphicTemperature_1;
    QCPGraph *graphicTemperature_2;
    QCPGraph *graphicTemperature_3;
    QCPGraph *graphicTemperature_4;
    QCPGraph *graphicRegress_1;
    QCPGraph *graphicRegress_2;
    bool temp1_PreTestStabilized=false;
    bool temp2_PreTestStabilized=false;
    double temp1_PreTestTavg_stabilized=0.0;
    double temp2_PreTestTavg_stabilized=0.0;
    QVector<QCPData> temp_1_data;
    QVector<QCPData> temp_2_data;

    //ModeTest regression
    QCPGraph *graphicTestRegress_1[7]; //всего надо 7 графиков, на 7 интервалов:
    QCPGraph *graphicTestRegress_2[7];  // 20-30мин.; 25-35; 30-40; 35-45; 40-50;45-55;50-60;


    //для ожидания первого интервала и потом добавляем пять минут и ждем следующий, последний будет на 60 минуте;
    double seconds_end_first_interval=1800; // 30 minutes
    double secons_add_to_switch_next_interval=300;   //5 minutes
    double secons_last_interval=3600;   //60 minutes
    int num_interval=0;   //num_interval=0..6 , всего 7 интервалов
    bool temp1_TestStabilized=false;
    bool temp2_TestStabilized=false;

    QString temp1TestStabilizationInfo,temp2TestStabilizationInfo;


    QString testStopReason;

//calibration page graphics
    bool isCalibrationPageEnabled=false;

    QCustomPlot *wGraphic_56;
    QCPGraph *graphicTemperature_5;
    QCPGraph *graphicTemperature_6;
    //bool temp5_isStabilized=false;
    //bool temp6_isStabilized=false;


    QCustomPlot *wGraphic_Curve;
    QCPCurve *graphicCurve;
    QCPCurve *graphicCurveMin;
    QCPCurve *graphicCurveMax;

    QMap<int,int> curveMax, curveMin;

    struct CurvePoint
    {
        int h;
        double val;
        bool operator==(const CurvePoint &other) const
        {
            if (this == &other) {
                return true;
            }
            return (h==other.h && val==other.val);
        }
    };

    QMap<int, QList<CurvePoint> > curveData; // данные для отрисовки всех точек "восьмеркой", int-колонка,
                                             // QList<CurvePoint> - точки в колонке

    void SetTablePoint(QLineEdit *lineEdit);
    void SetCurvePoint(int row, int h, QLineEdit *lineEdit);

    DialogTableResult dialogTableResult;

    bool eventFilter(QObject *obj, QEvent *event);


    DialogConfig dialogConfig;


    QString iniFile;
    QString comPort;

    QTimer timer1000ms;

    AnalogInputChannel temperature_1;
    AnalogInputChannel temperature_2;
    AnalogInputChannel temperature_3;
    AnalogInputChannel temperature_4;
    AnalogInputChannel temperature_5;
    AnalogInputChannel temperature_6;

    QHash<QString, AnalogInputChannel *> hashAnalogInputChannels;

    void LoadIniFile(QString iniFileName);
    void SaveIniFileComPort();
    void SaveIniFileOperator();

    //reports
    void setCurrentBlockAlignment(QTextCursor &cursor,
                                  Qt::Alignment alignment);
    QTextCharFormat charFormat(int pointSize,
                               bool bold);

    void createReport(const QString &fileName,
                                const QString &startDT,
                                const QString &timeDT,
                                const QImage &plot_movement_1,
                                const double X_1,
                                const double X_2
                                );





    double getLinearApproximatedValue(QVector<OperatorPoint> points, double x);

private slots:

    void Timer1000ms();
    void AddCsvMessageColumns();
    void AddCsvMessage(QString message="",bool addTParams=false, double T1avg=0.0, double T1_T1avg=0.0, double T1reg=0.0,
                                                                 double T2avg=0.0, double T2_T2avg=0.0, double T2reg=0.0);

    double calcAverage(QVector<double> vec);

    bool calcAvgMinMaxRegress(const QVector<QCPData> &data, double &avg, double &min, double &max, double &regress, double &regress_koeff_a, double &regress_koeff_b);

    void SliderSetVoltage(int value);
    void DoubleSpinBoxSetVoltage(double value);
    void VoltageSettedOK();
    void VoltageSettedError();
    void SetLastPowerToIniFile(double power);

    void ButtonExit();
    void ButtonReset();
    void ButtonStartStop();
    void ButtonReports();
    void ButtonTrendZoomOnOff(bool toggled);
    void ViewDialogConfig();
    void ButtonPageCalibr(bool toggled);

    void ButtonPowerOn(bool toggled);

    //void CheckBoxTemperature1Changed(bool newState);
    //void CheckBoxTemperature2Changed(bool newState);
    //void CheckBoxTemperature3Changed(bool newState);
    //void CheckBoxTemperature4Changed(bool newState);

    void CreateTestReport();
    void CreateTableReport();

    void ValueChanged(QString str);

    void slotRangeChanged (const QCPRange &newRange);
    void closeEvent(QCloseEvent *event);

};
//======================================================================================
class MyValidator : public QDoubleValidator
{
    public:
    MyValidator(double bottom, double top, int decimals, QObject * parent) :
    QDoubleValidator(bottom, top, decimals, parent)
    {
    }

    QValidator::State validate(QString &s, int &i) const
    {
        if (s.isEmpty() || s == "-") {
            return QValidator::Intermediate;
        }

        //QLocale locale;

        QChar decimalPoint = '.';//locale.decimalPoint();

        int charsAfterPoint = s.length() - s.indexOf(decimalPoint) -1;

        if (charsAfterPoint > decimals() && s.indexOf(decimalPoint) != -1) {
            return QValidator::Invalid;
        }

        bool ok;
        double d = s.toDouble(&ok);//locale.toDouble(s, &ok);

        if (ok && d >= bottom() && d <= top()) {
            return QValidator::Acceptable;
        } else {
            return QValidator::Invalid;
        }
    }
};

#endif // MAINWINDOW_H
