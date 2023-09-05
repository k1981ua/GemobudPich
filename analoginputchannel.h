#ifndef ANALOGINPUTCHANNEL_H
#define ANALOGINPUTCHANNEL_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include <QDateTime>
#include "modbusreader.h"

class QString;
class QFile;

struct CalPoint
{
    CalPoint(){}
    CalPoint(double akey, double aval): key(akey),val(aval) {}
    bool operator <(const CalPoint &other) const {return (key < other.key);}
    double key;
    double val;
};

struct TimeAvgPoint
{
    TimeAvgPoint(){}
    TimeAvgPoint(QDateTime adt, double aval): dt(adt),val(aval) {}
    bool operator==(const TimeAvgPoint& other){return (this->dt==other.dt && this->val==other.val);}
    QDateTime dt;
    double val;
};

/*
enum ValueStatus
{
    ValueOk=0,
    ValueErrorRead,
    ValueNotReading
};

Q_DECLARE_METATYPE(ValueStatus)
*/

class AnalogInputChannel : public QObject
{
    Q_OBJECT
public:
    explicit AnalogInputChannel(QObject *parent = 0);

private:

    QString configFileName;

    QString chName;
    QString engUnits;
    //сначала усредняем сірое значение в мВ
    //потом калибруем к инженернім еденицам
    //потом применяем офсет в индженерніх еденицах

    //averaging
    int numAvgPoints;
    QVector<double> avgPointsVector;
    double Average(double newValueRaw);

    //calibrating
    QVector<CalPoint> calPointsVector;
    bool Calibrate(double x, double &y);

    //offset
    double zero_offset;
    double ApplyOffset(double calibratedValueEU);

    //value raw
    double valueRaw;
    //value calibrated and averaged in engineering units
    double valueEU;
    ValueStatus valueStatus;
    QDateTime valueDT;

    //усреднение итогового значения по времени
    uint timeAvgIntervalSec;
    double valueEUTimeAvg;
    QVector<TimeAvgPoint> pointsVectorTimeAvg;


    //статистика мин/макс по каналу
    double valueEU_max;
    double valueEU_min;



    //Таймер опрелеляет таймаут чтения
    QTimer timerReadTimeout;
    constexpr static uint msecTimerReadTimeoutInterval=500;  //таймер проверяет таймаут каждіе 0.5 сек.
    constexpr static int msecTimeoutRead=5000;  //таймаут чтения - 5 секунд


public:
    void SetConfigFileName(const QString &fileName);
    QString GetConfigFileName();
    void LoadChannelConfigFile();
    void SaveChannelConfigFile();

    void SetChName(QString name);
    QString GetChName();

    uint GetNumberAveragePoints();
    void SetNumberAveragePoints(uint numAvgPnts);

    void SetVectorCalPoints(QVector<CalPoint> calPnts);
    QVector<CalPoint> GetVectorCalPoints();

    double GetZeroOffset();
    void SetZeroOffset();
    void SetZeroOffset(double offset);

    double GetValueRaw();
    double GetValue();
    ValueStatus GetValueStatus();
    QDateTime GetValueDT();
    void SetEU(const QString &eU);
    QString GetEU();
    QString GetValueString(uint precision);  //with EU
    QString GetValueString_noEU(uint precision);  //without EU

    void SetTimeAvgInterval(uint newTimeAvgInterval);
    uint GetTimeAvgInterval();
    double GetValueTimeAvg();

    double GetValueMin();
    double GetValueMax();

    void Reset();

    //statistic
    uint numReads;
    uint numReadsPerSec;
    int currDTsec;
    int prevDTsec;
    int prevNumReads;

signals:
    void valueReaded(double newValueEU, ValueStatus newStatus);
public slots:
    void RawValueReaded(double newValueRaw, ValueStatus newValueStatus);
    //void RawValueReaded(double newValueRaw);

    //Таймер опрелеляет таймаут чтения
    void TimerReadTimeout();

};

#endif // ANALOGINPUTCHANNEL_H
