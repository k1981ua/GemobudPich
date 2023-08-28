#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QHash>

#include "qcustomplot.h"
#include "modbusreader.h"
#include "analoginputchannel.h"
#include "dialogcalibr.h"
#include "dialogconfig.h"


#define X_RANGETEST 1800  //seconds стартовая ширина полотна графика по х для режима тестов 180 minutes
//#define X_RANGEVIEW 3600   //стартовая ширина полотна графика по х для режима просмотра
#define X_TICKSTEP 200  //20  //деления на шкале интервал 20 минут

//#define Y_MOVEMENT_RANGE_MAX 50  //стартовая ширина полотна графика по y  units mm
//#define Y_MOVEMENT_RANGE_MIN -50

#define Y_TEMPERATURE_RANGE_MAX 1300  //стартовая ширина полотна графика по y  units mm
#define Y_TEMPERATURE_RANGE_MIN 0

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


enum RunningMode
{
    ModeView=0,
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

    RunningMode runningMode=ModeView;
    CommandButton cmdButton=NoCmd;
    QString infoText;

    QDateTime startTestDT;
    QString startTestDT_str;
    QDateTime startViewDT;
    QString startViewDT_str;


    QVector<OperatorPoint> levelUpPoints_1;
    QVector<OperatorPoint> levelDownPoints_1;
    QVector<OperatorPoint> levelUpPoints_2;
    QVector<OperatorPoint> levelDownPoints_2;

    QPalette orig_palette;

    QCustomPlot *wGraphic_1;



    QCPGraph *graphicTemperature_1;
    QCPGraph *graphicTemperature_2;
    QCPGraph *graphicTemperature_3;
    QCPGraph *graphicTemperature_4;


    DialogConfig dialogConfig;


    QString iniFile;
    QString comPort;

    QTimer timer1000ms;

    AnalogInputChannel temperature_1;
    AnalogInputChannel temperature_2;
    AnalogInputChannel temperature_3;
    AnalogInputChannel temperature_4;

    QHash<QString, AnalogInputChannel *> hashAnalogInputChannels;

    void LoadIniFile(QString iniFileName);
    void SaveIniFile();

    //reports
    void setCurrentBlockAlignment(QTextCursor &cursor,
                                  Qt::Alignment alignment);
    QTextCharFormat charFormat(int pointSize,
                               bool bold);

    void createReport(const QString &fileName,
                                const QString &startDT,
                                const QString &timeDT,
                                const QImage &plot_movement_1,
                                const QImage &plot_movement_2,
                                const double X_1,
                                const double X_2,
                                const QVector<OperatorPoint> levelup_points_1,
                                const QVector<OperatorPoint> leveldown_points_1,
                                const QVector<OperatorPoint> levelup_points_2,
                                const QVector<OperatorPoint> leveldown_points_2
                                );

    double getLinearApproximatedValue(QVector<OperatorPoint> points, double x);

private slots:

    void Timer1000ms();
    double calcAverage(QVector<double> vec);
    double calcMaxDeviate(double average, QVector<double> vec);
    double calcRegression(QVector<double> vec);

    void SliderSetVoltage(int value);
    void DoubleSpinBoxSetVoltage(double value);
    void VoltageSetted();


    void ButtonExit();
    void ButtonReset();
    void ButtonStartStop();
    void ButtonReports();
    void ViewDialogConfig();

    //void CheckBoxTemperature1Changed(bool newState);
    //void CheckBoxTemperature2Changed(bool newState);
    //void CheckBoxTemperature3Changed(bool newState);
    //void CheckBoxTemperature4Changed(bool newState);


    void ValueChanged(QString str);

    void slotRangeChanged (const QCPRange &newRange);
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
