#include "dialogconfig.h"
#include "ui_dialogconfig.h"
#include "mainwindow.h"
//=================================================================
DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{

    ui->setupUi(this);

    this->setWindowTitle("Конфігурація і управління");

    connect(ui->buttonCalibr,SIGNAL(clicked()),this,SLOT(ButtonCalibr()));

    connect(ui->buttonSaveConfig,SIGNAL(clicked()),this,SLOT(accept()));
    connect(ui->buttonCancelConfig,SIGNAL(clicked()),this,SLOT(reject()));

    pmbReader=nullptr;

    foreach(QSerialPortInfo info,QSerialPortInfo::availablePorts())
    {
#ifdef LINUX
        ui->comboBoxComPort->addItem("/dev/"+info.portName());
#else
        ui->comboBoxComPort->addItem(info.portName());
#endif
    }

}
//=================================================================
DialogConfig::~DialogConfig()
{
    delete ui;
}
//=============================================================================================
void DialogConfig::SetModbusReader(ModbusReader *pmbRdr)
{
    pmbReader=pmbRdr;
}
//==================================================================
void DialogConfig::SetAnalogInputChannels(QHash<QString, AnalogInputChannel *> hashaichannels)
{
    hashAnalogInputChannels=hashaichannels;
    ui->comboBoxCalibrChannels->clear();
    QStringList chlist;

    foreach(QString aichname, hashAnalogInputChannels.keys())
    {
        chlist.append(aichname);
    }

    std::sort(chlist.begin(),chlist.end());

    foreach(QString aichname, chlist)
    {
        ui->comboBoxCalibrChannels->addItem(aichname);
    }

}
//==================================================================
void DialogConfig::SetComPort(QString port)
{
    ui->comboBoxComPort->setCurrentText(port);
}
//==================================================================
QString DialogConfig::GetComPort()
{
    return ui->comboBoxComPort->currentText();
}
//==================================================================

//==================================================================
void DialogConfig::ButtonCalibr()
{
     if (hashAnalogInputChannels.contains(ui->comboBoxCalibrChannels->currentText()))
     {
         dialogCalibr.SetAnalogInputChannel(hashAnalogInputChannels[ui->comboBoxCalibrChannels->currentText()]);
         dialogCalibr.setModal(true);
         dialogCalibr.exec();
     }
}
//===================================================================


//=======================================================================================
