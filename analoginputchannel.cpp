#include "analoginputchannel.h"
#include <QString>
#include <QDebug>
#include <QFile>

//===========================================================================
AnalogInputChannel::AnalogInputChannel(QObject *parent) : QObject(parent)
{
    configFileName="";
    chName="";
    valueRaw=0;
    valueEU=0;
    valueStatus=ValueStatus::ValueNotReading;
    valueDT=QDateTime::currentDateTime();

    Reset();

    //statistic reads
    numReads=0;
    numReadsPerSec=0;
    prevDTsec=-1;
    prevNumReads=0;

    connect(&timerReadTimeout,SIGNAL(timeout()),this,SLOT(TimerReadTimeout()));
    timerReadTimeout.start(msecTimerReadTimeoutInterval);
}
//===========================================================================
void AnalogInputChannel::Reset()
{
    avgPointsVector.clear();
    valueEU_min=qQNaN();
    valueEU_max=qQNaN();

}
//===========================================================================
void AnalogInputChannel::SetConfigFileName(const QString &fileName)
{
    configFileName=fileName;
}
//===========================================================================
QString AnalogInputChannel::GetConfigFileName()
{
    return configFileName;
}
//===========================================================================
void AnalogInputChannel::LoadChannelConfigFile()
{
    //*.ch file format example:
    //
    // EU=Па
    // avgPoints=10
    // zeroOffset=0.545
    // timeAvgInterval=10
    // -4.0000=0.00
    // -3.880457=1000.0

    if (configFileName!="")
    {
        QFile configFile(configFileName);
        configFile.open(QFile::ReadOnly);
        QTextStream txtStream(&configFile);

        txtStream.setCodec("UTF-8");


        //0- строчкой калибровки должны быть channel name ChName=pressure_1
        QString strChName=txtStream.readLine();
        QString strChNamePrefix = "ChName=";
        if (strChName.left(strChNamePrefix.length()).contains(strChNamePrefix,Qt::CaseInsensitive)) SetChName(strChName.remove(strChNamePrefix,Qt::CaseInsensitive));
        else SetChName("noName");

        //Первой строчкой калибровки должны быть инженерные еденицы в формате EU=Па
        QString strEU=txtStream.readLine();
        QString strEUPrefix = "EU=";
        if (strEU.left(strEUPrefix.length()).contains(strEUPrefix,Qt::CaseInsensitive)) SetEU(strEU.remove(strEUPrefix,Qt::CaseInsensitive));
        else SetEU("noEU");

        //second line is number of average points
        QString strAvgPoints=txtStream.readLine();
        QString strAvgPointsPrefix = "avgPoints=";
        if (strAvgPoints.left(strAvgPointsPrefix.length()).contains(strAvgPointsPrefix,Qt::CaseInsensitive)) SetNumberAveragePoints(strAvgPoints.remove(strAvgPointsPrefix,Qt::CaseInsensitive).toUInt());
        else SetNumberAveragePoints(0);

        //third line is offset
        QString strZeroOffset=txtStream.readLine();
        QString strZeroOffsetPrefix = "zeroOffset=";
        if (strZeroOffset.left(strZeroOffsetPrefix.length()).contains(strZeroOffsetPrefix,Qt::CaseInsensitive)) SetZeroOffset(strZeroOffset.remove(strZeroOffsetPrefix,Qt::CaseInsensitive).toDouble());
        else SetZeroOffset(0);

        //4 line is interval for time averaging
        QString strTimeAvgInterval=txtStream.readLine();
        QString strTimeAvgIntervalPrefix = "timeAvgInterval=";
        if (strTimeAvgInterval.left(strTimeAvgIntervalPrefix.length()).contains(strTimeAvgIntervalPrefix,Qt::CaseInsensitive)) SetTimeAvgInterval(strTimeAvgInterval.remove(strTimeAvgIntervalPrefix,Qt::CaseInsensitive).toUInt());
        else SetTimeAvgInterval(0);

        //next lines is calibrating points
        QVector<CalPoint> loadedCalPoints;

        while(!txtStream.atEnd())
        {
            QRegExp regExp("^(-?\\d+\\.?\\d*)=(-?\\d+\\.?\\d*)$");
            QString str=txtStream.readLine();
            if (regExp.indexIn(str)!=-1)
            {
                double key=regExp.cap(1).toDouble();
                double val=regExp.cap(2).toDouble();
                loadedCalPoints.push_back(CalPoint(key,val));
                //ui->listRegCalibrate->addItem(str);

            }
            else
            {
                //ui->listRegCalibrate->addItem(str + "  ----  error");
            }
        }

        configFile.close();

        //qSort( loadedCalPoints.begin(), loadedCalPoints.end());  -- deprecated. Use std::sort
        std::sort( loadedCalPoints.begin(), loadedCalPoints.end());
        SetVectorCalPoints(loadedCalPoints);
    }
    else
    {
        SetChName("noName");
        SetEU("noEU");
        SetNumberAveragePoints(0);
        SetZeroOffset(0);
        QVector<CalPoint> emptyCalPoints;
        SetVectorCalPoints(emptyCalPoints);
    }

}
//===========================================================================
void AnalogInputChannel::SaveChannelConfigFile()
{
    //*.ch file format example:
    //
    // EU=Па
    // avgPoints=10
    // zeroOffset=0.545
    // timeAvgInterval=10
    // -4.0000=0.00
    // -3.880457=1000.0

    if (configFileName!="")
    {
        QFile configFile(configFileName);
        configFile.open(QFile::WriteOnly | QFile::Text);

        QTextStream out(&configFile);

        out.setCodec("UTF-8");

        out << "ChName=" << GetChName() << "\n";
        out << "EU=" << GetEU() << "\n";
        out << "avgPoints=" << GetNumberAveragePoints() << "\n";
        out << "zeroOffset=" << QString::number(GetZeroOffset(),'f',6) << "\n";
        out << "timeAvgInterval=" << GetTimeAvgInterval() << "\n";

        foreach (CalPoint cp, calPointsVector)
        {
            out << QString::number(cp.key,'f',6) << "=" << QString::number(cp.val,'f',2) << "\n";
        }
        configFile.close();

        LoadChannelConfigFile();
    }
}
//===========================================================================
void AnalogInputChannel::SetChName(QString name)
{
    chName=name;
}
//===========================================================================
QString AnalogInputChannel::GetChName()
{
    return chName;
}
//===========================================================================
double AnalogInputChannel::Average(double newValueRaw)
{
    if (numAvgPoints<=1)
    {
        return newValueRaw;
    }

    avgPointsVector.push_back(newValueRaw);
    if (avgPointsVector.size() > numAvgPoints) avgPointsVector.pop_front();

    //delete from vector all NaNs
    avgPointsVector.removeAll(qQNaN());


    double summ=0;
    for(int i=0;i<avgPointsVector.size();++i)
    {
        /*if (!qIsNaN(avgPointsVector[i])) {*/  summ+=avgPointsVector[i];   /*}*/
    }

    if (avgPointsVector.size()>0)
    {
        return summ / avgPointsVector.size();
    }
    else
    {
        return newValueRaw;
    }
}
//===========================================================================
bool AnalogInputChannel::Calibrate(double x, double &y)
{
    if (calPointsVector.size()<2)
    {
        y=x;
        return false;
    }

    if (qIsNaN(x))
    {
        y=qQNaN();
        return false;
    }

    double x1,x2,y1,y2;

    if (x <= calPointsVector[0].key)
    {
        x1=calPointsVector[0].key;
        x2=calPointsVector[1].key;
        y1=calPointsVector[0].val;
        y2=calPointsVector[1].val;
        //   x-x1
        //y=------- * (y2-y1) + y1
        //   x2-x1
        y=(x-x1)*(y2-y1)/(x2-x1) + y1;
        return true;
    }

    if (x > calPointsVector[calPointsVector.size()-1].key)
    {
        x1=calPointsVector[calPointsVector.size()-2].key;
        x2=calPointsVector[calPointsVector.size()-1].key;
        y1=calPointsVector[calPointsVector.size()-2].val;
        y2=calPointsVector[calPointsVector.size()-1].val;
        //   x-x1
        //y=------- * (y2-y1) + y1
        //   x2-x1
        y=(x-x1)*(y2-y1)/(x2-x1) + y1;
        return true;
    }

    for (int i=0;i<calPointsVector.size()-1;++i)
    {
        if (x>calPointsVector[i].key && x<=calPointsVector[i+1].key)
        {
            x1=calPointsVector[i].key;
            x2=calPointsVector[i+1].key;
            y1=calPointsVector[i].val;
            y2=calPointsVector[i+1].val;
            //   x-x1
            //y=------- * (y2-y1) + y1
            //   x2-x1
            y=(x-x1)*(y2-y1)/(x2-x1) + y1;
            return true;
        }

    }

    return false;
}
//===========================================================================
double AnalogInputChannel::ApplyOffset(double calibratedValueEU)
{
    return calibratedValueEU-zero_offset;
}
//===========================================================================
uint AnalogInputChannel::GetNumberAveragePoints()
{
    return numAvgPoints;
}
//===========================================================================
void AnalogInputChannel::SetNumberAveragePoints(uint numAvgPnts)
{
    numAvgPoints=numAvgPnts;
}
//===========================================================================
void AnalogInputChannel::SetVectorCalPoints(QVector<CalPoint> calPnts)
{
    calPointsVector=calPnts;
    std::sort(calPointsVector.begin(),calPointsVector.end());
}
//===========================================================================
QVector<CalPoint> AnalogInputChannel::GetVectorCalPoints()
{
    return calPointsVector;
}
//===========================================================================
double AnalogInputChannel::GetZeroOffset()
{
    return zero_offset;
}
//===========================================================================
void AnalogInputChannel::SetZeroOffset()
{
    zero_offset=valueEU+zero_offset;
}
//===========================================================================
void AnalogInputChannel::SetZeroOffset(double offset)
{
    zero_offset=offset;
}
//===========================================================================
double AnalogInputChannel::GetValueRaw()
{
    return valueRaw;
}
//===========================================================================
double AnalogInputChannel::GetValue()
{
    return valueEU;
}
//===========================================================================
ValueStatus AnalogInputChannel::GetValueStatus()
{
    return valueStatus;
}
//===========================================================================
QDateTime AnalogInputChannel::GetValueDT()
{
    return valueDT;
}
//===========================================================================
void AnalogInputChannel::SetEU(const QString &eU)
{
    engUnits=eU;
}
//===========================================================================
QString AnalogInputChannel::GetEU()
{
    return engUnits;
}
//===========================================================================
QString AnalogInputChannel::GetValueString(uint precision)
{
    if (GetValueStatus()==ValueStatus::ValueOk)
    {
        return QString::number(GetValue(),'f',precision)+" "+GetEU();
    }
    else
    {
        return QString("---");
    }
}
//===========================================================================
QString AnalogInputChannel::GetValueString_noEU(uint precision)
{
    if (GetValueStatus()==ValueStatus::ValueOk)
    {
        return QString::number(GetValue(),'f',precision);
    }
    else
    {
        return QString("---");
    }
}
//===========================================================================
void AnalogInputChannel::SetTimeAvgInterval(uint newTimeAvgInterval)
{
    timeAvgIntervalSec=newTimeAvgInterval;
}
//===========================================================================
uint AnalogInputChannel::GetTimeAvgInterval()
{
    return timeAvgIntervalSec;
}
//===========================================================================
double AnalogInputChannel::GetValueTimeAvg()
{
    return valueEUTimeAvg;
}
//===========================================================================
double AnalogInputChannel::GetValueMin()
{
    return valueEU_min;
}
//===========================================================================
double AnalogInputChannel::GetValueMax()
{
    return valueEU_max;
}
//===========================================================================
void AnalogInputChannel::RawValueReaded(double newValueRaw, ValueStatus newValueStatus)
{

    valueRaw=Average(newValueRaw);
    double calibratedValueEU;
    Calibrate(valueRaw,calibratedValueEU);
    valueEU=ApplyOffset(calibratedValueEU);
    valueStatus=newValueStatus;
    valueDT=QDateTime::currentDateTime();

    if (qIsNaN(valueEU_min) || valueEU_min > valueEU) valueEU_min=valueEU;
    if (qIsNaN(valueEU_max) || valueEU_max < valueEU) valueEU_max=valueEU;

    //time averaging
    QDateTime currDT=QDateTime::currentDateTime();
    TimeAvgPoint timeAvgPoint(currDT, valueEU);
    pointsVectorTimeAvg.push_back(timeAvgPoint);
    foreach(TimeAvgPoint point, pointsVectorTimeAvg)
    {
        if (point.dt.addSecs(timeAvgIntervalSec) < currDT) pointsVectorTimeAvg.removeOne(point);
    }

    double summ=0;
    foreach(TimeAvgPoint point, pointsVectorTimeAvg)
    {
        summ+=point.val;
    }

    valueEUTimeAvg=summ / pointsVectorTimeAvg.size();

    /*
    for(int i=0;i<pointsVectorTimeAvg.size();++i)
    {
        if (pointsVectorTimeAvg[i].dt.addSecs(timeAvgIntervalSec) < currDT)
            pointsVectorTimeAvg.removeAt(i);
    }
    */
    //auto new_end=std::remove_if(pointsVectorTimeAvg.begin(),pointsVectorTimeAvg.end(),[=](const TimeAvgPoint& point){return currDT > point.dt.addSecs(timeAvgIntervalSec); });
    //pointsVectorTimeAvg.erase(new_end, pointsVectorTimeAvg.end());

/*
    qDebug() << timeAvgIntervalSec << " " << configFileName << "---------\n";
    foreach(TimeAvgPoint point, pointsVectorTimeAvg)
    {
    qDebug() << timeAvgIntervalSec << "  " << point.dt << "    " << point.val;
    }
*/

    //statistic read
    numReads++;
    currDTsec=QDateTime::currentDateTime().time().second();

    if (prevDTsec != currDTsec)
    {
        numReadsPerSec=numReads - prevNumReads;
        prevNumReads=numReads;
        prevDTsec=currDTsec;
    }



    emit valueReaded(valueEU, valueStatus);
}
//===========================================================================
void AnalogInputChannel::TimerReadTimeout()
{
    if ( valueStatus==ValueStatus::ValueOk && valueDT.msecsTo(QDateTime::currentDateTime()) > msecTimeoutRead )
    {
        valueStatus=ValueStatus::ValueErrorRead;
        emit valueReaded(valueEU, valueStatus);
    }
}
//===========================================================================
/*
void AnalogInputChannel::RawValueReaded(double newValueRaw)
{
    RawValueReaded(newValueRaw,ValueStatus::ValueOk);
}
*/
//===========================================================================
