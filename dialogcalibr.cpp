#include "dialogcalibr.h"
#include "ui_dialogcalibr.h"
#include <QMessageBox>
#include <QDebug>

//=============================================================================================
DialogCalibr::DialogCalibr(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCalibr)
{
    ui->setupUi(this);


    connect(ui->buttonAddValue,SIGNAL(clicked()),this,SLOT(ButtonAddValue()));
    connect(ui->buttonDeleteValue,SIGNAL(clicked()),this,SLOT(ButtonDeleteValue()));
    connect(ui->tableWidgetCalPoints->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SLOT(TableWidgetCalPointsItemChanged()));


    connect(ui->buttonSetCurrentToOffset,SIGNAL(clicked()),this,SLOT(ButtonSetCurrentToOffset()));
    connect(ui->doubleSpinBox_zeroOffset,SIGNAL(valueChanged(double)),this,SLOT(ZeroOffsetSpinBoxChanged(double)));
    connect(ui->spinBox_numAvgPoints,SIGNAL(valueChanged(int)),this,SLOT(NumAvgPointsSpinBoxChanged(int)));
    connect(ui->spinBox_timeAvgInterval,SIGNAL(valueChanged(int)),this,SLOT(TimeAvgIntervalSpinBoxChanged(int)));
    connect(ui->lineEdit_EU,SIGNAL(textChanged(QString)),this,SLOT(EULineEditChanged(QString)));
    connect(ui->lineEditChName,SIGNAL(textChanged(QString)),this,SLOT(ChNameLineEditChanged(QString)));
    //connect(ui->comboBox_EU,SIGNAL(currentTextChanged(QString)),this,SLOT(ComboBoxEUChanged()));

    connect(ui->buttonSaveCalibr,SIGNAL(clicked()),this,SLOT(ButtonSaveCalibr()));
    connect(ui->buttonExit,SIGNAL(clicked()),this,SLOT(ButtonCancelExit()));




}
//==============================================================================================
DialogCalibr::~DialogCalibr()
{
    delete ui;
}
//=============================================================================================
void DialogCalibr::SetAnalogInputChannel(AnalogInputChannel *aich)
{
    aichannel=aich;
    connect(aichannel,SIGNAL(valueReaded(double,ValueStatus)),this,SLOT(ValueReaded(double,ValueStatus)));

    connect(this,SIGNAL(finished(int)),this,SLOT(DisconnectSlots()));

    ApplyChannelValuesToDialog();

}
//==============================================================================================
void DialogCalibr::DisconnectSlots()
{
    disconnect(aichannel,SIGNAL(valueReaded(double,ValueStatus)),this,SLOT(ValueReaded(double,ValueStatus)));
    disconnect(this,SIGNAL(finished(int)),this,SLOT(DisconnectSlots()));
}
//==============================================================================================
void DialogCalibr::ApplyChannelValuesToDialog()
{
    this->setWindowTitle(aichannel->GetConfigFileName());
    ui->lineEdit_Value->setText("--- "+aichannel->GetEU());

    ui->lineEditChName->setText(aichannel->GetChName());
    ui->lineEdit_EU->setText(aichannel->GetEU());
    ui->spinBox_numAvgPoints->setValue(aichannel->GetNumberAveragePoints());
    ui->spinBox_timeAvgInterval->setValue(aichannel->GetTimeAvgInterval());
    ui->doubleSpinBox_zeroOffset->setValue(aichannel->GetZeroOffset());

    ui->label_EU->setText(aichannel->GetEU());
    ui->label_EU_new->setText(aichannel->GetEU());


    //Calibration Points
    ui->tableWidgetCalPoints->setRowCount(aichannel->GetVectorCalPoints().size());

    QStringList headers;
    headers << "raw" << aichannel->GetEU();

    ui->tableWidgetCalPoints->setHorizontalHeaderLabels(headers);

    uint rowCount=0;
    foreach(CalPoint cp, aichannel->GetVectorCalPoints())
    {

        ui->tableWidgetCalPoints->setItem(rowCount,0,new QTableWidgetItem(QString::number(cp.key,'f',6)));
        ui->tableWidgetCalPoints->setItem(rowCount,1,new QTableWidgetItem(QString::number(cp.val,'f',2)));
        rowCount++;
    }
}
//==============================================================================================
void DialogCalibr::ValueReaded(double newValue, ValueStatus newValueStatus)
{
    ui->lineEdit_Value->setText(aichannel->GetValueString(4));
    ui->label_statistic->setText(QString("Statistic: read:"+QString::number(aichannel->numReads)+  //" err:"+QString::number(mbReader.numErrorReads) +
                                                                          " rps:"+QString::number(aichannel->numReadsPerSec)));
    ui->lineEdit_CalibrValueRaw->setText(QString::number(aichannel->GetValueRaw(),'f',6));
    ui->lineEdit_CalibrValueEU->setText(QString::number(aichannel->GetValue(),'f',6));


}
//==============================================================================================
void DialogCalibr::ButtonAddValue()
{

    uint rowCount=ui->tableWidgetCalPoints->rowCount();

    QVector<CalPoint> tableCalPoints;
    for(uint i=0;i<rowCount;i++)
    {
        CalPoint cp;
        cp.key=ui->tableWidgetCalPoints->item(i,0)->text().toFloat();
        cp.val=ui->tableWidgetCalPoints->item(i,1)->text().toFloat();
        tableCalPoints.push_back(cp);
    }

    //new point
    CalPoint cp_new;
    cp_new.key=ui->lineEdit_CalibrValueRaw->text().toFloat();
    cp_new.val=ui->lineEdit_CalibrVaueEU_new->text().toFloat();

    tableCalPoints.push_back(cp_new);

    //SetCalibr(engUnits,tableCalPoints, avgPoints,zeroOffset);
    aichannel->SetVectorCalPoints(tableCalPoints);
    ApplyChannelValuesToDialog();
}
//==============================================================================================
void DialogCalibr::ButtonDeleteValue()
{
    if (ui->tableWidgetCalPoints->currentRow()>-1)
    {
        ui->tableWidgetCalPoints->removeRow(ui->tableWidgetCalPoints->currentRow());
    }

    uint rowCount=ui->tableWidgetCalPoints->rowCount();

    QVector<CalPoint> tableCalPoints;
    for(uint i=0;i<rowCount;i++)
    {
        CalPoint cp;
        cp.key=ui->tableWidgetCalPoints->item(i,0)->text().toFloat();
        cp.val=ui->tableWidgetCalPoints->item(i,1)->text().toFloat();
        tableCalPoints.push_back(cp);
    }

    //SetCalibr(engUnits,tableCalPoints, avgPoints, zeroOffset);
    aichannel->SetVectorCalPoints(tableCalPoints);
    ApplyChannelValuesToDialog();
}
//==============================================================================================
void DialogCalibr::TableWidgetCalPointsItemChanged()
{
    //QMessageBox::information(nullptr,"sdsa","fas");

    uint rowCount=ui->tableWidgetCalPoints->rowCount();

    QVector<CalPoint> tableCalPoints;
    for(uint i=0;i<rowCount;i++)
    {
        CalPoint cp;
        cp.key=ui->tableWidgetCalPoints->item(i,0)->text().toFloat();
        cp.val=ui->tableWidgetCalPoints->item(i,1)->text().toFloat();
        tableCalPoints.push_back(cp);
    }

    //SetCalibr(engUnits,tableCalPoints, avgPoints, zeroOffset);
    aichannel->SetVectorCalPoints(tableCalPoints);
    ApplyChannelValuesToDialog();

}
//==============================================================================================
void DialogCalibr::EULineEditChanged(QString newEU)
{
    aichannel->SetEU(newEU);
    ui->label_EU->setText(aichannel->GetEU());
    ui->label_EU_new->setText(aichannel->GetEU());
}
//==============================================================================================
void DialogCalibr::ChNameLineEditChanged(QString newChName)
{
    aichannel->SetChName(newChName);
}
//==============================================================================================
void DialogCalibr::ButtonSetCurrentToOffset()
{
    aichannel->SetZeroOffset();
    ui->doubleSpinBox_zeroOffset->setValue(aichannel->GetZeroOffset());
}
//==============================================================================================
void DialogCalibr::ZeroOffsetSpinBoxChanged(double newZeroOffset)
{
    aichannel->SetZeroOffset(newZeroOffset);
}
//==============================================================================================
void DialogCalibr::NumAvgPointsSpinBoxChanged(int newNumAvgPoints)
{
    aichannel->SetNumberAveragePoints(newNumAvgPoints);
}
//==============================================================================================
void DialogCalibr::TimeAvgIntervalSpinBoxChanged(int newTimeAvgInterval)
{
    aichannel->SetTimeAvgInterval(newTimeAvgInterval);
}
//==============================================================================================
void DialogCalibr::ButtonSaveCalibr()
{
    aichannel->SaveChannelConfigFile();
}
//==============================================================================================
void DialogCalibr::ButtonCancelExit()
{
    aichannel->LoadChannelConfigFile();
    close();
}
//==============================================================================================
//==============================================================================================
