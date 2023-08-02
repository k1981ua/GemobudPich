#ifndef DIALOGCALIBR_H
#define DIALOGCALIBR_H

#include "analoginputchannel.h"
#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class DialogCalibr;
}

class DialogCalibr : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCalibr(QWidget *parent = 0);
    ~DialogCalibr();
    void SetAnalogInputChannel(AnalogInputChannel *aich);
    void ApplyChannelValuesToDialog();

private:
    Ui::DialogCalibr *ui;
    AnalogInputChannel *aichannel;

private slots:
    void ValueReaded(double newValue, ValueStatus newValueStatus);

    void ButtonAddValue();
    void ButtonDeleteValue();
    void TableWidgetCalPointsItemChanged();

    void ButtonSetCurrentToOffset();
    void ZeroOffsetSpinBoxChanged(double newZeroOffset);
    void NumAvgPointsSpinBoxChanged(int newNumAvgPoints);
    void TimeAvgIntervalSpinBoxChanged(int newTimeAvgInterval);
    void EULineEditChanged(QString newEU);
    void ChNameLineEditChanged(QString newChName);
    void ButtonSaveCalibr();
    void ButtonCancelExit();

    void DisconnectSlots();

signals:
    void newCalibrAccepted(QString eU, QVector<CalPoint> calPnts, double offset);

};

#endif // DIALOGCALIBR_H
