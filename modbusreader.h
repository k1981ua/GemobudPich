#ifndef MODBUSREADER_H
#define MODBUSREADER_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include "autostopthread.h"
#include "src_libmodbus/modbus.h"

//симуляция без физического реле
//#define NO_DO_RELE


//#define COM_PORT "COM1"
//#define BAUD_RATE 115200
#define BAUD_RATE 9600
//baudrate 9600 =30-31 requests per second

#define PARITY 'N'    //'N' even paruty for new registrator
#define BYTESIZE 8
#define STOPBITS 1



                                        //INPUT OR HOLDING REGISTERS
                                        //0x00-dec.point  по первому каналу
                                        //0x01,0x02-intValue  по первому каналу
#define TRM251_1_ADDRESS 8              //0x03 - статус измерения входа 1 (код исключительной ситуации) 0 == OK по первому каналу
#define TRM251_START_REGISTER 0x00      //0x04,0x05 = float температура по первому каналу
#define TRM251_NUM_REGISTERS 6  //18         //0x06 - 0x0B - то же по второму каналу
                                        //0x0C (dec 12) - выходная мощность 0..1000
                                        //0x0D (dec 13) - мгновенное значение уставки
                                        //0x0E (dec 14) - состояние устройства сигнализации
                                        //0x0F (dec 15) - номер текущей программы технолога

#define TRM251_START_REGISTER_2 0x10      //0x10 (dec 16) - номер текущего шага программы технолога
#define TRM251_NUM_REGISTERS_2 1          //0x11 (dec 17) - режим работы прибора

#define TRM251_2_ADDRESS 16


//адрес для чтения значения В с преобразователя, float inverse
//адрес ОВЕНА будет 5
//#define OWEN_AI_ADDRESS 1
//#define OWEN_AI_START_REGISTER 0x0120  //ch1- 0x0120,0x0121-float; 0x0122-time label    ch2-0x0123,0x0124-float; 0x0125-time label ..... ch8-0x0135,0x0136-float;0x0137-time label
//#define OWEN_AI_NUM_REGISTERS 24       //ch1- 40289, 40290 -float; 40291 -time label    ch2-40292 ,40293 -float; 40294 -time label ..... ch8-0x0135,0x0136-float;0x0137-time label
                                        //3registers * 8channels

//ОВЕН 2-канальній   или 8-канальній МВ-110-2А - два датчика температурі, Pt100
#define OWEN_2AI_ADDRESS 24
#define OWEN_2AI_START_REGISTER 0x0  //ch1- 0x00-dec.point,0x01-intValue, 0x02-Status, 0x03-measTime, 0x04-0x05-floatVal и т.д. остальніе каналі
#define OWEN_2AI_NUM_REGISTERS 12       //6registers * 2channels = 12 регистров


//#define TRM200_1_ADDRESS 4
//#define TRM200_START_REGISTER 0x1009  //HOLDING REGISTERS 0x1009, 0x100A - float value 1;  0x100B?0x100C - float value 2
//#define TRM200_NUM_REGISTERS 4        //44106, 44107 - float value 1;   44108, 44109 - float value 1;

//7018

#define ICP_7018_ADDRESS 1
#define ICP_7018_START_REGISTER 0   //40001, 40002, 40003 ... - все по 1reg, int
#define ICP_7018_NUM_REGISTERS 8

//7024

#define ICP_7024_ADDRESS 2
#define ICP_7024_VOLTAGE_REGISTER 0   //40001, 40002, 40003 ... - все по 1reg, int
//#define ICP_7024_NUM_REGISTERS 8

//=============================================================================


enum ValueStatus
{
    ValueOk=0,
    ValueErrorRead,
    ValueNotReading
};

Q_DECLARE_METATYPE(ValueStatus)

enum VoltageSetCommand
{
    VOLTSET_NONE_CMD=0,
    VOLTSET_SET_CMD
};

//=============================================================================

class ModbusReader : public AutoStopThread
{
    Q_OBJECT

public:
    ModbusReader()
    {
        ctx = NULL;
        sec_to = 1;
        usec_to = 0;
        numReads=0;
        numErrorReads=0;



        connect(this,SIGNAL(finished()),this,SLOT(Release()));
    }

    virtual ~ModbusReader() {}

    bool Init(QString serialport="", int registratoraddress=1);
    bool Connect();



    void StartPoll();
    void StopPoll();
    void SetComPort(QString serPort);

    void VoltageSet(float newVoltage);

private:
    QString serialPort;

    virtual void run();



    modbus_t *ctx;
    uint32_t sec_to;
    uint32_t usec_to;

    uint16_t regTrm251_1[200];
    uint16_t regTrm251_2[200];
    //uint16_t regOWEN_AI[200];
    uint16_t regOWEN_2AI[200];
    uint16_t regICP_7018[200];

    uint numReads;
    uint numErrorReads;


    VoltageSetCommand voltageSetCommand;
    float voltageToSet=0.0;


private slots:
    void Release();

signals:
/*
    void readTrm251_1(float newValue,float newStep, ValueStatus newValueStatus);
    void readTrm251_2(float newValue,float newStep, ValueStatus newValueStatus);
    void readOwen2ai(float newValue1,float newValue2, ValueStatus newValueStatus);
*/
    void readTrm251_1(double newValue, ValueStatus newValueStatus);
    void readTrm251_2(double newValue, ValueStatus newValueStatus);
    //void readOWEN_AI_ch1(double newValue, ValueStatus newValueStatus);
    //void readOWEN_AI_ch2(double newValue, ValueStatus newValueStatus);

    void readOWEN_2AI_ch1(double newValue, ValueStatus newValueStatus);
    void readOWEN_2AI_ch2(double newValue, ValueStatus newValueStatus);

    //void readOWEN_AI_ch3(double newValue, ValueStatus newValueStatus);
    //void readOWEN_AI_ch4(double newValue, ValueStatus newValueStatus);
    //void readOWEN_AI_ch5(double newValue, ValueStatus newValueStatus);
    //void readOWEN_AI_ch6(double newValue, ValueStatus newValueStatus);
    //void readOWEN_AI_ch7(double newValue, ValueStatus newValueStatus);
    //void readOWEN_AI_ch8(double newValue, ValueStatus newValueStatus);

    void readICP_7018_ch1(double newValue, ValueStatus newValueStatus);
    void readICP_7018_ch2(double newValue, ValueStatus newValueStatus);
    void readICP_7018_ch3(double newValue, ValueStatus newValueStatus);
    void readICP_7018_ch4(double newValue, ValueStatus newValueStatus);
    void readICP_7018_ch5(double newValue, ValueStatus newValueStatus);
    void readICP_7018_ch6(double newValue, ValueStatus newValueStatus);


    void voltageSettedOK();
    void voltageSettedError();
};

#endif // MODBUSREADER_H
