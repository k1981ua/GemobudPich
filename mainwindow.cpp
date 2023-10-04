#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fmdlg.h"
#include "modbusreader.h"

#include <QPainter>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QTextTable>


ModbusReader mbReader;



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qRegisterMetaType<ValueStatus>();

    ui->buttonReset->setVisible(false);

    QDir appDir(qApp->applicationDirPath());
    appDir.mkdir("reports");
    appDir.mkdir("calibration_reports");
    appDir.mkdir("png");
    appDir.mkdir("csv");

    //connect(ui->lineEditOperatorPoint_1,SIGNAL(textEdited(QString)),this,SLOT(ValueChanged(QString)));
    //connect(ui->lineEditOperatorPoint_2,SIGNAL(textEdited(QString)),this,SLOT(ValueChanged(QString)));

    //double operatorInputMin=Y_MOVEMENT_RANGE_MIN;
    //double operatorInputMax=Y_MOVEMENT_RANGE_MAX;

    temperature_1.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_1.ch");
    temperature_2.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_2.ch");
    temperature_3.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_3.ch");
    temperature_4.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_4.ch");
    temperature_5.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_5.ch");
    temperature_6.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_6.ch");


    temperature_1.LoadChannelConfigFile();
    temperature_2.LoadChannelConfigFile();
    temperature_3.LoadChannelConfigFile();
    temperature_4.LoadChannelConfigFile();
    temperature_5.LoadChannelConfigFile();
    temperature_6.LoadChannelConfigFile();

    if (qApp->arguments().count()>1)
    {
        iniFile=qApp->arguments().last();
    }
    else
    {
       iniFile="main.ini";
    }

    LoadIniFile(qApp->applicationDirPath()+"/"+iniFile);
/*
 *  old syntaxis
    connect(&mbReader, SIGNAL(readTrm251_1(double,ValueStatus)),&temperature_1, SLOT(RawValueReaded(double,ValueStatus)));
    connect(&mbReader, SIGNAL(readTrm251_2(double,ValueStatus)),&temperature_2, SLOT(RawValueReaded(double,ValueStatus)));
    connect(&mbReader, SIGNAL(readOWEN_2AI_ch1(double,ValueStatus)),&movement_1, SLOT(RawValueReaded(double,ValueStatus)));
    connect(&mbReader, SIGNAL(readOWEN_2AI_ch2(double,ValueStatus)),&movement_2, SLOT(RawValueReaded(double,ValueStatus)));
*/
    connect(&mbReader, &ModbusReader::readICP_7018_ch1, &temperature_1, &AnalogInputChannel::RawValueReaded);
    connect(&mbReader, &ModbusReader::readICP_7018_ch2, &temperature_2, &AnalogInputChannel::RawValueReaded);
    connect(&mbReader, &ModbusReader::readICP_7018_ch3, &temperature_3, &AnalogInputChannel::RawValueReaded);
    connect(&mbReader, &ModbusReader::readICP_7018_ch4, &temperature_4, &AnalogInputChannel::RawValueReaded);
    connect(&mbReader, &ModbusReader::readICP_7018_ch5, &temperature_5, &AnalogInputChannel::RawValueReaded);
    connect(&mbReader, &ModbusReader::readICP_7018_ch6, &temperature_6, &AnalogInputChannel::RawValueReaded);
    connect(&mbReader, &ModbusReader::voltageSettedOK, this, &MainWindow::VoltageSettedOK);
    connect(&mbReader, &ModbusReader::voltageSettedError, this, &MainWindow::VoltageSettedError);


    qDebug() << "port:" << comPort;

    //if (QFile(comPort).exists())
    {
        qDebug() << "port exist";
        mbReader.Init(comPort);
        mbReader.StartPoll();
    }
    dialogConfig.SetModbusReader(&mbReader);

    hashAnalogInputChannels[QString("temperature_1") + "(" + temperature_1.GetChName()+")"]=&temperature_1;
    hashAnalogInputChannels[QString("temperature_2") + "(" + temperature_2.GetChName()+")"]=&temperature_2;
    hashAnalogInputChannels[QString("temperature_3") + "(" + temperature_3.GetChName()+")"]=&temperature_3;
    hashAnalogInputChannels[QString("temperature_4") + "(" + temperature_4.GetChName()+")"]=&temperature_4;
    hashAnalogInputChannels[QString("temperature_5") + "(" + temperature_5.GetChName()+")"]=&temperature_5;
    hashAnalogInputChannels[QString("temperature_6") + "(" + temperature_6.GetChName()+")"]=&temperature_6;


    connect(ui->buttonExit,     SIGNAL(clicked()),this,SLOT(ButtonExit()));
    connect(ui->buttonConfig,   SIGNAL(clicked()),this,SLOT(ViewDialogConfig()));
    connect(ui->buttonReset,    SIGNAL(clicked()),this,SLOT(ButtonReset()));
    connect(ui->buttonStartStop,SIGNAL(clicked()),this,SLOT(ButtonStartStop()));
    connect(ui->buttonReports,  SIGNAL(clicked()),this,SLOT(ButtonReports()));
    connect(ui->buttonTrendZoom,  SIGNAL(toggled(bool)),this,SLOT(ButtonTrendZoomOnOff(bool)));

    connect(&dialogConfig, SIGNAL(buttonPageCalibr(bool)),this,SLOT(ButtonPageCalibr(bool)));

    connect(ui->buttonPowerOn,  SIGNAL(toggled(bool)),this,SLOT(ButtonPowerOn(bool)));

    connect(&timer1000ms,&QTimer::timeout,this,&MainWindow::Timer1000ms);

    //connect(&timer3000ms,SIGNAL(timeout()),this,SLOT(Timer3000ms()));

    connect(ui->buttonTableResultSave,QPushButton::clicked,[&](){dialogTableResult.setModal(true); dialogTableResult.show();});
    connect(&dialogTableResult,SIGNAL(createTableReport()),this,SLOT(CreateTableReport()));

    orig_palette=ui->lineEditValueTemperature_1->palette();


    //ГРАФИК 1
    // Инициализируем объект полотна для графика ...
    wGraphic_1 = new QCustomPlot();
    wGraphic_1->setBackground(QColor(221,221,221));//Qt::lightGray);
    wGraphic_1->axisRect()->setBackground(QColor(255,251,240));  //оригинал - слоновая кость - 255,251,240

    ui->verticalLayoutGraphic_1->addWidget(wGraphic_1);
    wGraphic_1->xAxis->setLabel("Час, сек.");
    wGraphic_1->xAxis->setRange(X_RANGEPRETEST_MIN,X_RANGEPRETEST_MAX);
    wGraphic_1->xAxis->setTickStep(X_TICKSTEP);
    wGraphic_1->xAxis->setAutoTickStep(false);

     //wGraphic->setba

 /*
     //шкалы справа, mm
     wGraphic_1->yAxis2->setLabel("Переміщення, мм");//+calEU);
     wGraphic_1->yAxis2->setRange(Y_MOVEMENT_RANGE_MIN,Y_MOVEMENT_RANGE_MAX);
     wGraphic_1->yAxis2->setAutoTickStep(false);
     wGraphic_1->yAxis2->setTickStep(10.0);
     //wGraphic->yAxis2->setTickLabelColor(Qt::transparent);
     wGraphic_1->yAxis2->setVisible(true);
*/
     //шкалы слева, гр.С
     wGraphic_1->yAxis->setLabel("Температура, °C");//+calEU);
     wGraphic_1->yAxis->setRange(Y_TEMPERATURE_RANGE_MIN,Y_TEMPERATURE_RANGE_MAX);
     wGraphic_1->yAxis->setAutoTickStep(false);
     wGraphic_1->yAxis->setTickStep(100.0);
     //wGraphic->yAxis->setTickLabelColor(Qt::transparent);
     wGraphic_1->yAxis->setVisible(true);

     /*
     // Инициализируем график Movement 1 и привязываем его к Осям
     graphicMovement_1 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicMovement_1);  // Устанавливаем график на полотно
     QPen penMovement_1=graphicMovement_1->pen();
     penMovement_1.setColor(Qt::black);
     penMovement_1.setWidth(2);
     graphicMovement_1->setPen(penMovement_1); // Устанавливаем цвет графика
     graphicMovement_1->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicMovement_1->setLineStyle(QCPGraph::lsLine);
     */


     // Инициализируем график Temperature 1 и привязываем его к Осям
     graphicTemperature_1 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicTemperature_1);  // Устанавливаем график на полотно
     QPen penTemperature_1=graphicTemperature_1->pen();
     penTemperature_1.setColor(Qt::red);
     penTemperature_1.setWidth(2);
     graphicTemperature_1->setPen(penTemperature_1); // Устанавливаем цвет графика
     graphicTemperature_1->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_1->setLineStyle(QCPGraph::lsLine);


     // Инициализируем график Regress 1 и привязываем его к Осям
     graphicRegress_1 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicRegress_1);  // Устанавливаем график на полотно
     QPen penRegress_1=graphicRegress_1->pen();
     penRegress_1.setColor(Qt::gray);
     penRegress_1.setWidth(2);
     graphicRegress_1->setPen(penRegress_1); // Устанавливаем цвет графика
     graphicRegress_1->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicRegress_1->setLineStyle(QCPGraph::lsLine);



     // Инициализируем график Temperature 2 и привязываем его к Осям
     graphicTemperature_2 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicTemperature_2);  // Устанавливаем график на полотно
     QPen penTemperature_2=graphicTemperature_2->pen();
     penTemperature_2.setColor(Qt::blue);
     penTemperature_2.setWidth(2);
     graphicTemperature_2->setPen(penTemperature_2); // Устанавливаем цвет графика
     graphicTemperature_2->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_2->setLineStyle(QCPGraph::lsLine);

     // Инициализируем график Regress 1 и привязываем его к Осям
     graphicRegress_2 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicRegress_2);  // Устанавливаем график на полотно
     QPen penRegress_2=graphicRegress_2->pen();
     penRegress_2.setColor(Qt::gray);
     penRegress_2.setWidth(2);
     graphicRegress_2->setPen(penRegress_2); // Устанавливаем цвет графика
     graphicRegress_2->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicRegress_2->setLineStyle(QCPGraph::lsLine);

     // Инициализируем график Temperature 3 и привязываем его к Осям
     graphicTemperature_3 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicTemperature_3);  // Устанавливаем график на полотно
     QPen penTemperature_3=graphicTemperature_3->pen();
     penTemperature_3.setColor(Qt::darkGreen);
     penTemperature_3.setWidth(2);
     graphicTemperature_3->setPen(penTemperature_3); // Устанавливаем цвет графика
     graphicTemperature_3->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_3->setLineStyle(QCPGraph::lsLine);


     // Инициализируем график Temperature 3 и привязываем его к Осям
     graphicTemperature_4 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicTemperature_4);  // Устанавливаем график на полотно
     QPen penTemperature_4=graphicTemperature_4->pen();
     penTemperature_4.setColor(Qt::magenta);
     penTemperature_4.setWidth(2);
     graphicTemperature_4->setPen(penTemperature_4); // Устанавливаем цвет графика
     graphicTemperature_4->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_4->setLineStyle(QCPGraph::lsLine);



     //графики регрессии для ModeTest, 7 штук максимум, массив ,стр.25 ДСТУ ISO 1182:2022

     // Инициализируем график TestRegress_1[i] и _2 и привязываем его к Осям
     for(int i=0;i<7;++i)
     {
         // graphicTestRegress_1
         graphicTestRegress_1[i] = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
         wGraphic_1->addPlottable(graphicTestRegress_1[i]);  // Устанавливаем график на полотно
         QPen penTestRegress_1=graphicTestRegress_1[i]->pen();
         penTestRegress_1.setColor(Qt::gray);
         penTestRegress_1.setWidth(2);
         graphicTestRegress_1[i]->setPen(penTestRegress_1); // Устанавливаем цвет графика
         graphicTestRegress_1[i]->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
         graphicTestRegress_1[i]->setLineStyle(QCPGraph::lsLine);

         // graphicTestRegress_2
         graphicTestRegress_2[i] = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
         wGraphic_1->addPlottable(graphicTestRegress_2[i]);  // Устанавливаем график на полотно
         QPen penTestRegress_2=graphicTestRegress_2[i]->pen();
         penTestRegress_2.setColor(Qt::gray);
         penTestRegress_2.setWidth(2);
         graphicTestRegress_2[i]->setPen(penTestRegress_2); // Устанавливаем цвет графика
         graphicTestRegress_2[i]->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
         graphicTestRegress_2[i]->setLineStyle(QCPGraph::lsLine);
     }

     /* Подключаем сигнал от Оси X об изменении видимого диапазона координат
      * к СЛОТу для переустановки формата времени оси.
      * */
     //connect(wGraphic_1->xAxis, SIGNAL(rangeChanged(QCPRange)),
     //        this, SLOT(slotRangeChanged(QCPRange)));

     wGraphic_1->setInteraction(QCP::iRangeZoom,false);   // Выключаем взаимодействие удаления/приближения
     wGraphic_1->setInteraction(QCP::iRangeDrag, false);  // Выключаем взаимодействие перетаскивания графика
     //wGraphic_1->axisRect()->setRangeDrag(Qt::Horizontal);   // Включаем перетаскивание только по горизонтальной оси
     //wGraphic_1->axisRect()->setRangeZoom(Qt::Horizontal);   // Включаем удаление/приближение только по горизонтальной оси
     wGraphic_1->replot();










   //ГРАФИК 56
    // Инициализируем объект полотна для графика ...
    wGraphic_56 = new QCustomPlot();
    wGraphic_56->setBackground(QColor(221,221,221));//Qt::lightGray);
    wGraphic_56->axisRect()->setBackground(QColor(255,251,240));  //оригинал - слоновая кость - 255,251,240

    ui->verticalLayoutGraphic_56->addWidget(wGraphic_56);
    wGraphic_56->xAxis->setLabel("Час, сек.");
    wGraphic_56->xAxis->setRange(X_RANGEPRETEST_MIN,X_RANGEPRETEST_MAX);
    wGraphic_56->xAxis->setTickStep(X_TICKSTEP);
    wGraphic_56->xAxis->setAutoTickStep(false);


     //шкалы слева, гр.С
     wGraphic_56->yAxis->setLabel("Температура, °C");//+calEU);
     wGraphic_56->yAxis->setRange(Y_TEMPERATURE_RANGE_MIN,Y_TEMPERATURE_RANGE_MAX);
     wGraphic_56->yAxis->setAutoTickStep(false);
     wGraphic_56->yAxis->setTickStep(100.0);
     //wGraphic->yAxis->setTickLabelColor(Qt::transparent);
     wGraphic_56->yAxis->setVisible(true);


     // Инициализируем график Temperature 1 и привязываем его к Осям
     graphicTemperature_5 = new QCPGraph(wGraphic_56->xAxis, wGraphic_56->yAxis);
     wGraphic_56->addPlottable(graphicTemperature_5);  // Устанавливаем график на полотно
     QPen penTemperature_5=graphicTemperature_5->pen();
     penTemperature_5.setColor(Qt::red);
     penTemperature_5.setWidth(2);
     graphicTemperature_5->setPen(penTemperature_5); // Устанавливаем цвет графика
     graphicTemperature_5->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_5->setLineStyle(QCPGraph::lsLine);


     // Инициализируем график Temperature 2 и привязываем его к Осям
     graphicTemperature_6 = new QCPGraph(wGraphic_56->xAxis, wGraphic_56->yAxis);
     wGraphic_56->addPlottable(graphicTemperature_6);  // Устанавливаем график на полотно
     QPen penTemperature_6=graphicTemperature_6->pen();
     penTemperature_6.setColor(Qt::blue);
     penTemperature_6.setWidth(2);
     graphicTemperature_6->setPen(penTemperature_6); // Устанавливаем цвет графика
     graphicTemperature_6->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_6->setLineStyle(QCPGraph::lsLine);


     wGraphic_56->setInteraction(QCP::iRangeZoom,false);   // Выключаем взаимодействие удаления/приближения
     wGraphic_56->setInteraction(QCP::iRangeDrag, false);  // Выключаем взаимодействие перетаскивания графика
     //wGraphic_1->axisRect()->setRangeDrag(Qt::Horizontal);   // Включаем перетаскивание только по горизонтальной оси
     //wGraphic_1->axisRect()->setRangeZoom(Qt::Horizontal);   // Включаем удаление/приближение только по горизонтальной оси
     wGraphic_56->replot();










   //ГРАФИК Curve
    // Инициализируем объект полотна для графика ...
    wGraphic_Curve = new QCustomPlot();
    wGraphic_Curve->setBackground(QColor(221,221,221));//Qt::lightGray);
    wGraphic_Curve->axisRect()->setBackground(QColor(255,251,240));  //оригинал - слоновая кость - 255,251,240

    ui->verticalLayoutGraphic_Curve->addWidget(wGraphic_Curve);
    wGraphic_Curve->xAxis->setLabel("Температура, °C");
    wGraphic_Curve->xAxis->setRange(550, 775);
    wGraphic_Curve->xAxis->setTickStep(25);
    wGraphic_Curve->xAxis->setAutoTickStep(false);


     //шкалы слева, гр.С
     wGraphic_Curve->yAxis->setLabel("Висота печі, мм");//+calEU);
     wGraphic_Curve->yAxis->setRange(0,150);
     wGraphic_Curve->yAxis->setAutoTickStep(false);
     wGraphic_Curve->yAxis->setTickStep(10.0);
     //wGraphic->yAxis->setTickLabelColor(Qt::transparent);
     wGraphic_Curve->yAxis->setVisible(true);


     // Инициализируем график CurveMin и привязываем его к Осям
     graphicCurveMin = new QCPCurve(wGraphic_Curve->xAxis, wGraphic_Curve->yAxis);
     wGraphic_Curve->addPlottable(graphicCurveMin);  // Устанавливаем график на полотно
     QPen penCurveMin=graphicCurveMin->pen();
     penCurveMin.setColor(Qt::green);
     penCurveMin.setWidth(2);
     graphicCurveMin->setPen(penCurveMin); // Устанавливаем цвет графика
     graphicCurveMin->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     //graphicCurveMin->setLineStyle(QCPGraph::lsLine);




     // Инициализируем график CurveMax и привязываем его к Осям
     graphicCurveMax = new QCPCurve(wGraphic_Curve->xAxis, wGraphic_Curve->yAxis);
     wGraphic_Curve->addPlottable(graphicCurveMax);  // Устанавливаем график на полотно
     QPen penCurveMax=graphicCurveMax->pen();
     penCurveMax.setColor(Qt::red);
     penCurveMax.setWidth(2);
     graphicCurveMax->setPen(penCurveMax); // Устанавливаем цвет графика
     graphicCurveMax->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     //graphicCurveMax->setLineStyle(QCPGraph::lsLine);


     // Инициализируем график CurveMin и привязываем его к Осям
     graphicCurve = new QCPCurve(wGraphic_Curve->xAxis, wGraphic_Curve->yAxis);
     wGraphic_Curve->addPlottable(graphicCurve);  // Устанавливаем график на полотно
     QPen penCurve=graphicCurve->pen();
     penCurve.setColor(Qt::blue);
     penCurve.setWidth(2);
     graphicCurve->setPen(penCurve); // Устанавливаем цвет графика
     graphicCurve->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     //graphicCurve->setLineStyle(QCPGraph::lsLine);

     wGraphic_Curve->setInteraction(QCP::iRangeZoom,false);   // Выключаем взаимодействие удаления/приближения
     wGraphic_Curve->setInteraction(QCP::iRangeDrag, false);  // Выключаем взаимодействие перетаскивания графика
     //wGraphic_1->axisRect()->setRangeDrag(Qt::Horizontal);   // Включаем перетаскивание только по горизонтальной оси
     //wGraphic_1->axisRect()->setRangeZoom(Qt::Horizontal);   // Включаем удаление/приближение только по горизонтальной оси

     /*
     //заполним графики по функции стр.14 ДСТУ 1182:2022 п.7.3.2.2
     for(int h=0;h<=150;++h)
     {
        double Tmin=541.653+(5.901*h)-(0.067*h*h)+(3.375*0.0001*h*h*h)-(8.553*0.0000001*h*h*h*h);
        double Tmax=614.167+(5.347*h)-(0.08138*h*h)+(5.826*0.0001*h*h*h)-(1.772*0.000001*h*h*h*h);
        graphicCurveMin->addData(Tmin, (double)h );
        graphicCurveMax->addData(Tmax, (double)h );
     }
    */

     //заполним графики по таблице стр.15 ДСТУ 1182:2022 п.7.3.2.2

     curveMin={
         {5,   570},
         {15,  616},
         {25,  652},
         {35,  679},
         {45,  699},
         {55,  712},
         {65,  720},
         {75,  723},
         {85,  722},
         {95,  717},
         {105, 709},
         {115, 698},
         {125, 683},
         {135, 664},
         {145, 639},

     };

     foreach (int key, curveMin.keys())
     {
        graphicCurveMin->addData(curveMin.value(key), key );
     }

/*
        graphicCurveMin->addData(570, 5 );
        graphicCurveMin->addData(616, 15 );
        graphicCurveMin->addData(652, 25 );
        graphicCurveMin->addData(679, 35 );
        graphicCurveMin->addData(699, 45 );
        graphicCurveMin->addData(712, 55 );
        graphicCurveMin->addData(720, 65 );
        graphicCurveMin->addData(723, 75 );
        graphicCurveMin->addData(722, 85 );
        graphicCurveMin->addData(717, 95 );
        graphicCurveMin->addData(709, 105 );
        graphicCurveMin->addData(698, 115 );
        graphicCurveMin->addData(683, 125 );
        graphicCurveMin->addData(664, 135 );
        graphicCurveMin->addData(639, 145 );
*/

     curveMax={
                 {5,   639},
                 {15,  678},
                 {25,  705},
                 {35,  724},
                 {45,  736},
                 {55,  743},
                 {65,  746},
                 {75,  747},
                 {85,  746},
                 {95,  743},
                 {105, 737},
                 {115, 729},
                 {125, 716},
                 {135, 698},
                 {145, 671},

                 };

     foreach (int key, curveMax.keys())
     {
        graphicCurveMax->addData(curveMax.value(key), key );
     }

/*
        graphicCurveMax->addData(639, 5 );
        graphicCurveMax->addData(678, 15 );
        graphicCurveMax->addData(705, 25 );
        graphicCurveMax->addData(724, 35 );
        graphicCurveMax->addData(736, 45 );
        graphicCurveMax->addData(743, 55 );
        graphicCurveMax->addData(746, 65 );
        graphicCurveMax->addData(747, 75 );
        graphicCurveMax->addData(746, 85 );
        graphicCurveMax->addData(743, 95 );
        graphicCurveMax->addData(737, 105 );
        graphicCurveMax->addData(729, 115 );
        graphicCurveMax->addData(716, 125 );
        graphicCurveMax->addData(698, 135 );
        graphicCurveMax->addData(671, 145 );
*/

     wGraphic_Curve->replot();










     ui->lineEditValueTemperature_1->setStyleSheet("QLineEdit{border: 3px solid red; border-radius:10px;}");
     ui->lineEditValueTemperature_2->setStyleSheet("QLineEdit{border: 3px solid blue; border-radius:10px;}");
     ui->lineEditValueTemperature_3->setStyleSheet("QLineEdit{border: 3px solid darkgreen; border-radius:10px;}");
     ui->lineEditValueTemperature_4->setStyleSheet("QLineEdit{border: 3px solid magenta; border-radius:10px;}");
     ui->lineEditValueTemperature_5->setStyleSheet("QLineEdit{border: 3px solid red; border-radius:10px;}");
     ui->lineEditValueTemperature_6->setStyleSheet("QLineEdit{border: 3px solid blue; border-radius:10px;}");

      ButtonReset();
      ui->labelTemperature_1->setText(temperature_1.GetChName());
      ui->labelTemperature_2->setText(temperature_2.GetChName());
      ui->labelTemperature_3->setText(temperature_3.GetChName());
      ui->labelTemperature_4->setText(temperature_4.GetChName());
      ui->labelTemperature_5->setText(temperature_5.GetChName());
      ui->labelTemperature_6->setText(temperature_6.GetChName());

 //     wGraphic_1->yAxis->setLabel("Температура , гр.С");
 //     wGraphic_1->yAxis->setLabel(temperature_1.GetChName()+", "+temperature_1.GetEU());
 //     wGraphic_2->yAxis->setLabel(movement_2.GetChName()+", "+movement_2.GetEU());

      QFont font=wGraphic_1->xAxis->labelFont();
      font.setPointSize(12);
      wGraphic_1->xAxis->setLabelFont(font);
      wGraphic_1->yAxis->setLabelFont(font);
      wGraphic_1->yAxis2->setLabelFont(font);


      graphicTemperature_1->setVisible(true); graphicRegress_1->setVisible(true);   ui->checkBoxTemperature1->setChecked(true);
      graphicTemperature_2->setVisible(true); graphicRegress_2->setVisible(true);   ui->checkBoxTemperature2->setChecked(true);
      graphicTemperature_3->setVisible(false);   ui->checkBoxTemperature3->setChecked(false);
      graphicTemperature_4->setVisible(false);   ui->checkBoxTemperature4->setChecked(false);
      graphicTemperature_5->setVisible(true);    ui->checkBoxTemperature5->setChecked(true);
      graphicTemperature_6->setVisible(false);   ui->checkBoxTemperature6->setChecked(false);



      connect(ui->checkBoxTemperature1,QCheckBox::toggled,this,[&](bool checked){graphicTemperature_1->setVisible(checked);graphicRegress_1->setVisible(checked);wGraphic_1->replot();});
      connect(ui->checkBoxTemperature2,QCheckBox::toggled,this,[&](bool checked){graphicTemperature_2->setVisible(checked);graphicRegress_2->setVisible(checked);wGraphic_1->replot();});
      connect(ui->checkBoxTemperature3,QCheckBox::toggled,this,[&](bool checked){graphicTemperature_3->setVisible(checked);wGraphic_1->replot();});
      connect(ui->checkBoxTemperature4,QCheckBox::toggled,this,[&](bool checked){graphicTemperature_4->setVisible(checked);wGraphic_1->replot();});
      connect(ui->checkBoxTemperature5,QCheckBox::toggled,this,[&](bool checked){graphicTemperature_5->setVisible(checked);wGraphic_56->replot();});
      connect(ui->checkBoxTemperature6,QCheckBox::toggled,this,[&](bool checked){graphicTemperature_6->setVisible(checked);wGraphic_56->replot();});
     // connect(ui->checkBoxTemperature1,SIGNAL(toggled(bool)),this,SLOT(CheckBoxTemperature1Changed(bool)));

     // connect(ui->sliderPowerSet,QSlider::valueChanged,this,MainWindow::SliderSetVoltage);   
      connect(ui->doubleSpinBoxPowerSet, SIGNAL(valueChanged(double)),this,SLOT(DoubleSpinBoxSetVoltage(double)));

     // connect(ui->doubleSpinBoxPowerSet,QDoubleSpinBox::valueChanged, this, MainWindow::DoubleSpinBoxSetVoltage);

      timer1000ms.start(3000);  //дадим время для первого опроса датчиков, далее будет установлено 1000 мс.

      startPreTestDT=QDateTime::currentDateTime();

      infoText="СТАРТ ПРОГРАМИ";
      infoText+="   " + startPreTestDT.toString("hh:mm:ss dd.MM.yy");


      //startViewDT_str=startViewDT.toString("yyyy.MM.dd_hh.mm.ss");
      infoText+="\nОЧІКУЄМ СТАБІЛІЗАЦІЇ...";
      infoText+="\nУмови: Tavg=750±5°C, |T-Tavg|≤10°C, Treg≤2°C, 10 хв.";

      //ui->groupBoxPowerSet->setStyleSheet("QGroupBox{border: 10px solid grey; border-radius:10px;}");
      //ui->groupBoxPowerSet->setFixedSize(20,20);
      ui->labelCirclePowerSet->setStyleSheet("QLabel{border: 2px solid grey; border-radius:10px; background-color:grey;}");
      ui->labelCirclePowerSet->setFixedSize(20,20);

      //Calibration Mode

      ui->groupBoxCal_1->setStyleSheet("QLabel{font-size: 16px;} QLineEdit:focus{ border: 3px solid #40bd06; border-radius:3px;} QLineEdit{font-size: 16px;} ");
    /*
      ui->lineEditT1a->setStyleSheet("QLineEdit:focus{ border: 2px solid green; border-radius:3px;}");
      ui->lineEditT1b->setStyleSheet("QLineEdit:focus{ border: 2px solid green; border-radius:3px;}");
      ui->lineEditT1c->setStyleSheet("QLineEdit:focus{ border: 2px solid green; border-radius:3px;}");
*/
      connect(ui->lineEditT1a,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT2a->setFocus();});
      connect(ui->lineEditT1b,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT2b->setFocus();});
      connect(ui->lineEditT1c,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT2c->setFocus();});
      connect(ui->lineEditT2a,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT3a->setFocus();});
      connect(ui->lineEditT2b,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT3b->setFocus();});
      connect(ui->lineEditT2c,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT3c->setFocus();});
      connect(ui->lineEditT3a,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT1b->setFocus();});
      connect(ui->lineEditT3b,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT1c->setFocus();});
      connect(ui->lineEditT3c,QLineEdit::returnPressed,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); ui->lineEditT3c->clearFocus();});

/*
      connect(ui->lineEditT1a,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1b,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1c,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2a,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2b,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2c,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT3a,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT3b,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT3c,QLineEdit::editingFinished,this,[&](){SetTablePoint(qobject_cast<QLineEdit*>(sender())); });
*/

      foreach(QObject *obj, ui->groupBoxCal_1->children())
      {
        QLineEdit *le=qobject_cast<QLineEdit *>(obj);
        if (le)
        {
            le->installEventFilter(this);
            //le->setValidator(new MyValidator(0.0,1000.0, 2, le));
        }
      }



      ui->groupBoxCal_2->setStyleSheet("QLabel{font-size: 16px;} QLineEdit:focus{ border: 3px solid #40bd06; border-radius:3px;} QLineEdit{font-size: 16px;} ");


      connect(ui->lineEditT1_75,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(1,75, qobject_cast<QLineEdit*>(sender()));ui->lineEditT1_65->setFocus();});
      connect(ui->lineEditT1_65,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(1,65, qobject_cast<QLineEdit*>(sender()));ui->lineEditT1_55->setFocus();});
      connect(ui->lineEditT1_55,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(1,55, qobject_cast<QLineEdit*>(sender()));ui->lineEditT1_45->setFocus();});
      connect(ui->lineEditT1_45,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(1,45, qobject_cast<QLineEdit*>(sender()));ui->lineEditT1_35->setFocus();});
      connect(ui->lineEditT1_35,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(1,35, qobject_cast<QLineEdit*>(sender()));ui->lineEditT1_25->setFocus();});
      connect(ui->lineEditT1_25,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(1,25, qobject_cast<QLineEdit*>(sender()));ui->lineEditT1_15->setFocus();});
      connect(ui->lineEditT1_15,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(1,15, qobject_cast<QLineEdit*>(sender()));ui->lineEditT1_5->setFocus();});
      connect(ui->lineEditT1_5,   QLineEdit::returnPressed,this,[&](){SetCurvePoint(1, 5, qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_5->setFocus();});

      connect(ui->lineEditT2_5,   QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 5, qobject_cast<QLineEdit*>(sender())); ui->lineEditT2_15->setFocus();});
      connect(ui->lineEditT2_15,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 15,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_25->setFocus();});
      connect(ui->lineEditT2_25,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 25,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_35->setFocus();});
      connect(ui->lineEditT2_35,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 35,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_45->setFocus();});
      connect(ui->lineEditT2_45,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 45,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_55->setFocus();});
      connect(ui->lineEditT2_55,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 55,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_65->setFocus();});
      connect(ui->lineEditT2_65,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 65,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_75->setFocus();});
      connect(ui->lineEditT2_75,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 75,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_85->setFocus();});
      connect(ui->lineEditT2_85,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 85,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_95->setFocus();});
      connect(ui->lineEditT2_95,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(2, 95,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_105->setFocus();});
      connect(ui->lineEditT2_105, QLineEdit::returnPressed,this,[&](){SetCurvePoint(2,105,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_115->setFocus();});
      connect(ui->lineEditT2_115, QLineEdit::returnPressed,this,[&](){SetCurvePoint(2,115,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_125->setFocus();});
      connect(ui->lineEditT2_125, QLineEdit::returnPressed,this,[&](){SetCurvePoint(2,125,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_135->setFocus();});
      connect(ui->lineEditT2_135, QLineEdit::returnPressed,this,[&](){SetCurvePoint(2,135,qobject_cast<QLineEdit*>(sender()));ui->lineEditT2_145->setFocus();});
      connect(ui->lineEditT2_145, QLineEdit::returnPressed,this,[&](){SetCurvePoint(2,145,qobject_cast<QLineEdit*>(sender()));ui->lineEditT3_145->setFocus();});

      connect(ui->lineEditT3_145, QLineEdit::returnPressed,this,[&](){SetCurvePoint(3,145,qobject_cast<QLineEdit*>(sender()));ui->lineEditT3_135->setFocus();});
      connect(ui->lineEditT3_135, QLineEdit::returnPressed,this,[&](){SetCurvePoint(3,135,qobject_cast<QLineEdit*>(sender()));ui->lineEditT3_125->setFocus();});
      connect(ui->lineEditT3_125, QLineEdit::returnPressed,this,[&](){SetCurvePoint(3,125,qobject_cast<QLineEdit*>(sender()));ui->lineEditT3_115->setFocus();});
      connect(ui->lineEditT3_115, QLineEdit::returnPressed,this,[&](){SetCurvePoint(3,115,qobject_cast<QLineEdit*>(sender()));ui->lineEditT3_105->setFocus();});
      connect(ui->lineEditT3_105, QLineEdit::returnPressed,this,[&](){SetCurvePoint(3,105,qobject_cast<QLineEdit*>(sender()));ui->lineEditT3_95->setFocus();});
      connect(ui->lineEditT3_95,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(3,95, qobject_cast<QLineEdit*>(sender())); ui->lineEditT3_85->setFocus();});
      connect(ui->lineEditT3_85,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(3,85, qobject_cast<QLineEdit*>(sender())); ui->lineEditT3_75->setFocus();});
      connect(ui->lineEditT3_75,  QLineEdit::returnPressed,this,[&](){SetCurvePoint(3,75, qobject_cast<QLineEdit*>(sender())); ui->lineEditT3_75->clearFocus();});


/*
      connect(ui->lineEditT1_75, QLineEdit::editingFinished,this,[&](){SetCurvePoint(1,75,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1_65, QLineEdit::editingFinished,this,[&](){SetCurvePoint(1,65,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1_55, QLineEdit::editingFinished,this,[&](){SetCurvePoint(1,55,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1_45, QLineEdit::editingFinished,this,[&](){SetCurvePoint(1,45,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1_35, QLineEdit::editingFinished,this,[&](){SetCurvePoint(1,35,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1_25, QLineEdit::editingFinished,this,[&](){SetCurvePoint(1,25,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1_15, QLineEdit::editingFinished,this,[&](){SetCurvePoint(1,15,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT1_5,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(1, 5,qobject_cast<QLineEdit*>(sender())); });

      connect(ui->lineEditT2_5,   QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 5,qobject_cast<QLineEdit*>(sender()));  });
      connect(ui->lineEditT2_15,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 15,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_25,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 25,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_35,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 35,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_45,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 45,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_55,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 55,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_65,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 65,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_75,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 75,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_85,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 85,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_95,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(2, 95,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_105, QLineEdit::editingFinished,this,[&](){SetCurvePoint(2,105,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_115, QLineEdit::editingFinished,this,[&](){SetCurvePoint(2,115,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_125, QLineEdit::editingFinished,this,[&](){SetCurvePoint(2,125,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_135, QLineEdit::editingFinished,this,[&](){SetCurvePoint(2,135,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT2_145, QLineEdit::editingFinished,this,[&](){SetCurvePoint(2,145,qobject_cast<QLineEdit*>(sender())); });

      connect(ui->lineEditT3_145, QLineEdit::editingFinished,this,[&](){SetCurvePoint(3,145,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT3_135, QLineEdit::editingFinished,this,[&](){SetCurvePoint(3,135,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT3_125, QLineEdit::editingFinished,this,[&](){SetCurvePoint(3,125,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT3_115, QLineEdit::editingFinished,this,[&](){SetCurvePoint(3,115,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT3_105, QLineEdit::editingFinished,this,[&](){SetCurvePoint(3,105,qobject_cast<QLineEdit*>(sender())); });
      connect(ui->lineEditT3_95,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(3,95,qobject_cast<QLineEdit*>(sender()));  });
      connect(ui->lineEditT3_85,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(3,85,qobject_cast<QLineEdit*>(sender()));  });
      connect(ui->lineEditT3_75,  QLineEdit::editingFinished,this,[&](){SetCurvePoint(3,75,qobject_cast<QLineEdit*>(sender()));  });
*/


      foreach(QObject *obj, ui->groupBoxCal_2->children())
      {
        QLineEdit *le=qobject_cast<QLineEdit *>(obj);
        if (le)
        {
            le->installEventFilter(this);
            //le->setValidator(new MyValidator(0,1000.0, 2, le));
        }
      }

      //ui->labelTemperature_5_Stabilization->setText(temperature_5.GetChName() + " - очікуєм стабілізації...");
      //ui->labelTemperature_6_Stabilization->setText(temperature_6.GetChName() + " - очікуєм стабілізації...");
      //ui->labelTemperature_5_Stabilization->setStyleSheet("QLabel{color: red; font-size:16px;}");
      //ui->labelTemperature_6_Stabilization->setStyleSheet("QLabel{color: red; font-size:16px;}");

      //ui->labelTemperature_56_StabilizationCond->setText("Умови стабілізації:Tavg=750±5°C, |T-Tavg|≤10°C, Treg≤2°C");


//      ui->stackedWidget->setCurrentIndex(0);

    csvFileName=qApp->applicationDirPath()+"/csv/"+startPreTestDT.toString("yyyy.MM.dd_hh.mm.ss")+".csv";

    ui->buttonPowerOn->setStyleSheet("QPushButton{font-size: 12pt;} QPushButton:checked{font-size: 12pt;border: 3px solid red; border-radius:3px;}");

}
//======================================================================
//=====================================================================
void MainWindow::AddCsvMessage(QString message)
{

    QString strDateTime;
    QDateTime dt;
    dt=QDateTime::currentDateTime();
    strDateTime.sprintf("%.2u.%.2u.%.4u %.2u:%.2u:%.2u.%.3u",
                        dt.date().day(),dt.date().month(),dt.date().year(),
                        dt.time().hour(),dt.time().minute(),dt.time().second(),dt.time().msec());

    //Logging to file

    QFile csvfile(csvFileName);

    if (csvfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QString record;
        if (message=="")
        {
            record.sprintf("%s;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\n",strDateTime.toStdString().c_str(),
                                                                temperature_1.GetValue(),
                                                                temperature_2.GetValue(),
                                                                temperature_3.GetValue(),
                                                                temperature_4.GetValue(),
                                                                temperature_5.GetValue(),
                                                                temperature_6.GetValue());
        }
        else
        {
            record.sprintf("%s;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%s\n",strDateTime.toStdString().c_str(),
                                                                   temperature_1.GetValue(),
                                                                   temperature_2.GetValue(),
                                                                   temperature_3.GetValue(),
                                                                   temperature_4.GetValue(),
                                                                   temperature_5.GetValue(),
                                                                   temperature_6.GetValue(),
                                                                   message.toStdString().c_str());
        }
        csvfile.write( record.toStdString().c_str());
        csvfile.close();
    }


}
//======================================================================
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
      QLineEdit *lineEdit=qobject_cast<QLineEdit *>(obj);

        if (event->type() == QEvent::MouseButtonDblClick)
        {
            lineEdit->setReadOnly(false);
            return true;
        }
        else
        {
            return QObject::eventFilter(obj, event);
            //return false;
        }

}
//=======================================================================================
void MainWindow::SetTablePoint(QLineEdit *lineEdit)
{


        if (lineEdit->isReadOnly())
        {
            lineEdit->setText(temperature_5.GetValueString_noEU(2));
        }
        else
        {
            if (!lineEdit->text().isEmpty())
            {
                double value=lineEdit->text().replace(",",".").toDouble();  // по пути заменим запятую на точку дя удобства ввода
                lineEdit->setText(QString::number(value,'f',2));
            }
            else
            {
                lineEdit->setText(temperature_5.GetValueString_noEU(2));
            }
            lineEdit->setReadOnly(true);
        }


        double T1a=ui->lineEditT1a->text().toDouble();
        double T1b=ui->lineEditT1b->text().toDouble();
        double T1c=ui->lineEditT1c->text().toDouble();
        double T2a=ui->lineEditT2a->text().toDouble();
        double T2b=ui->lineEditT2b->text().toDouble();
        double T2c=ui->lineEditT2c->text().toDouble();
        double T3a=ui->lineEditT3a->text().toDouble();
        double T3b=ui->lineEditT3b->text().toDouble();
        double T3c=ui->lineEditT3c->text().toDouble();

        double Tavg=(T1a+T1b+T1c+T2a+T2b+T2c+T3a+T3b+T3c)/9.0;
        double Tavg_axis1=(T1a+T1b+T1c)/3.0;
        double Tavg_axis2=(T2a+T2b+T2c)/3.0;
        double Tavg_axis3=(T3a+T3b+T3c)/3.0;
        double Tdev_axis1=100.0*fabs(Tavg-Tavg_axis1)/Tavg;
        double Tdev_axis2=100.0*fabs(Tavg-Tavg_axis2)/Tavg;
        double Tdev_axis3=100.0*fabs(Tavg-Tavg_axis3)/Tavg;
        double Tavg_dev_axis=(Tdev_axis1+Tdev_axis2+Tdev_axis3)/3.0;  //formula 8
        double Tavg_levela=(T1a+T2a+T3a)/3.0;
        double Tavg_levelb=(T1b+T2b+T3b)/3.0;
        double Tavg_levelc=(T1c+T2c+T3c)/3.0;
        double Tdev_levela=100.0*fabs((Tavg-Tavg_levela)/Tavg);
        double Tdev_levelb=100.0*fabs((Tavg-Tavg_levelb)/Tavg);
        double Tdev_levelc=100.0*fabs((Tavg-Tavg_levelc)/Tavg);
        double Tavg_dev_level=(Tdev_levela+Tdev_levelb+Tdev_levelc)/3.0;  //formula 15

        ui->labelCalibration1Result->setText("T<sub>avg,dev,axis</sub>="+QString::number(Tavg_dev_axis,'f',2)+"   T<sub>avg,dev,level</sub>="+QString::number(Tavg_dev_level,'f',2));


        QString tableRes;

        tableRes+="<br><center>Розрахунок згідно ДСТУ EN ISO 1182:2022 п.7.3.1</center><br><br><br>";

        tableRes+="T<sub>avg</sub> = (T<sub>1a</sub>+T<sub>1b</sub>+T<sub>1c</sub>+T<sub>2a</sub>+T<sub>2b</sub>+T<sub>2c</sub>+T<sub>3a</sub>+T<sub>3b</sub>+T<sub>3c</sub>) / 9 = "+QString::number(Tavg,'f',2) + "<br>";
        tableRes+="T<sub>avg,axis1</sub> = (T<sub>1a</sub>+T<sub>1b</sub>+T<sub>1c</sub>) / 3 = "+QString::number(Tavg_axis1,'f',2) + "<br>";
        tableRes+="T<sub>avg,axis2</sub> = (T<sub>2a</sub>+T<sub>2b</sub>+T<sub>2c</sub>) / 3 = "+QString::number(Tavg_axis2,'f',2) + "<br>";
        tableRes+="T<sub>avg,axis3</sub> = (T<sub>3a</sub>+T<sub>3b</sub>+T<sub>3c</sub>) / 3 = "+QString::number(Tavg_axis3,'f',2) + "<br>";
        tableRes+="T<sub>dev,axis1</sub> = 100 * |T<sub>avg</sub>-T<sub>avg,axis1</sub>| / T<sub>avg</sub> = "+QString::number(Tdev_axis1,'f',2) + "<br>";
        tableRes+="T<sub>dev,axis2</sub> = 100 * |T<sub>avg</sub>-T<sub>avg,axis2</sub>| / T<sub>avg</sub> = "+QString::number(Tdev_axis2,'f',2) + "<br>";
        tableRes+="T<sub>dev,axis3</sub> = 100 * |T<sub>avg</sub>-T<sub>avg,axis3</sub>| / T<sub>avg</sub> = "+QString::number(Tdev_axis3,'f',2) + "<br>";
        tableRes+="T<sub>avg,dev,axis</sub> = (T<sub>dev,axis1</sub>+T<sub>dev,axis2</sub>+T<sub>dev,axis3</sub>) / 3 = "+QString::number(Tavg_dev_axis,'f',2) + "     (має бути меншим за 0.5 %) " + "<br>";

        tableRes+="T<sub>avg,levela</sub> = (T<sub>1a</sub>+T<sub>2a</sub>+T<sub>3a</sub>) / 3 = "+QString::number(Tavg_levela,'f',2) + "<br>";
        tableRes+="T<sub>avg,levelb</sub> = (T<sub>1b</sub>+T<sub>2b</sub>+T<sub>3b</sub>) / 3 = "+QString::number(Tavg_levelb,'f',2) + "<br>";
        tableRes+="T<sub>avg,levelc</sub> = (T<sub>1c</sub>+T<sub>2c</sub>+T<sub>3c</sub>) / 3 = "+QString::number(Tavg_levelc,'f',2) + "<br>";
        tableRes+="T<sub>dev,levela</sub> = 100 * |(T<sub>avg</sub>-T<sub>avg,levela</sub>)/T<sub>avg</sub>| = "+QString::number(Tdev_levela,'f',2) + "<br>";
        tableRes+="T<sub>dev,levelb</sub> = 100 * |(T<sub>avg</sub>-T<sub>avg,levelb</sub>)/T<sub>avg</sub>| = "+QString::number(Tdev_levelb,'f',2) + "<br>";
        tableRes+="T<sub>dev,levelc</sub> = 100 * |(T<sub>avg</sub>-T<sub>avg,levelc</sub>)/T<sub>avg</sub>| = "+QString::number(Tdev_levelc,'f',2) + "<br>";
        tableRes+="T<sub>avg,dev,level</sub> = (T<sub>dev,levela</sub>+T<sub>dev,levelb</sub>+T<sub>dev,levelc</sub>) / 3 = "+QString::number(Tavg_dev_level,'f',2) + "     (має бути меншим за 1.5 %) " + "<br>";


        dialogTableResult.SetLabelText(tableRes);


}
//=======================================================================================
void MainWindow::SetCurvePoint(int row, int h, QLineEdit *lineEdit) //row==1,2,3  ,  h=5 15 25 35 ...  145
{

    double value=0.0;

    if (lineEdit->isReadOnly())
    {
        lineEdit->setText(temperature_6.GetValueString_noEU(2));
    }
    else
    {
        if (!lineEdit->text().isEmpty())
        {
            value=lineEdit->text().replace(",",".").toDouble();
            lineEdit->setText(QString::number(value,'f',2));
        }
        else
        {
            lineEdit->setText(temperature_6.GetValueString_noEU(2));
        }
        lineEdit->setReadOnly(true);
    }

    value=lineEdit->text().toDouble();


    QList<CurvePoint> &rowList=curveData[row];

    CurvePoint cp;
    cp.h=h;
    cp.val=value;


    if (cp.val<curveMin[h] || cp.val>curveMax[h])
    {
        //lineEdit->setText(lineEdit->text()+"-----");
        //QPalette palette=orig_lineEditPalette;
        //palette.setColor(QPalette::Base, QColor(250,155,155));
        //lineEdit->setPalette(palette);
        lineEdit->setStyleSheet("QLabel{font-size: 16px;} QLineEdit:focus{ border: 3px solid #40bd06; border-radius:3px; color:red} QLineEdit{font-size: 16px;color:red} ");
    }
    else
    {
        lineEdit->setStyleSheet("QLabel{font-size: 16px;} QLineEdit:focus{ border: 3px solid #40bd06; border-radius:3px;} QLineEdit{font-size: 16px;} ");
    }



    foreach(CurvePoint temp_cp, rowList)
    {
        if(temp_cp.h==cp.h) rowList.removeOne(temp_cp);
    }

    rowList.append(cp);

    if (row==1 || row==3)
    {
        std::sort(rowList.begin(), rowList.end(), [&](CurvePoint first, CurvePoint second) -> bool {return first.h>second.h;});
    }
    else //row==2, движение вверх, сортировка в обратом порядке
    {
        std::sort(rowList.begin(), rowList.end(), [&](CurvePoint first, CurvePoint second) -> bool {return first.h<second.h;});
    }


    graphicCurve->clearData();
    //graphicCurve->addData(temperature_5.GetValue(),h);

    foreach(QList<CurvePoint> rowlst, curveData)
    {
        foreach(CurvePoint crvpnt, rowlst)
        {
            graphicCurve->addData(crvpnt.val,crvpnt.h);
        }

    }


    wGraphic_Curve->replot();


}



//=======================================================================================
MainWindow::~MainWindow()
{
    delete ui;
}
//=======================================================================================
void MainWindow::LoadIniFile(QString iniFileName)
{
    iniFile=iniFileName;
    QSettings settings(iniFileName,QSettings::IniFormat);
    settings.beginGroup("main");

#ifdef LINUX
    comPort=settings.value("comPort","/dev/ttyUSB0").toString();
#else
    comPort=settings.value("comPort","COM1").toString();
#endif

    double power=settings.value("lastPower").toDouble();
    ui->doubleSpinBoxPowerSet->setValue(power);

}
//=======================================================================================
void MainWindow::SaveIniFile()
{
    QSettings settings(iniFile,QSettings::IniFormat);
    settings.beginGroup("main");

    settings.setValue("comPort",comPort);

}
//=======================================================================================
void MainWindow::SetLastPowerToIniFile(double power)
{
    QSettings settings(iniFile,QSettings::IniFormat);
    settings.beginGroup("main");

    settings.setValue("lastPower",QString::number(power,'f',2));
}
//=======================================================================================
void MainWindow::ButtonTrendZoomOnOff(bool toggled)
{
    //wGraphic_1->setInteraction(QCP::iRangeDrag, true);
    if (toggled)
    {
        wGraphic_1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // | QCP::iRangeZoom);//QCP::iSelectPlottables | QCP::iSelectAxes);
        wGraphic_1->xAxis->setAutoTickStep(true);
        wGraphic_1->yAxis->setAutoTickStep(true);

        wGraphic_56->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // | QCP::iRangeZoom);//QCP::iSelectPlottables | QCP::iSelectAxes);
        wGraphic_56->xAxis->setAutoTickStep(true);
        wGraphic_56->yAxis->setAutoTickStep(true);

        wGraphic_Curve->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // | QCP::iRangeZoom);//QCP::iSelectPlottables | QCP::iSelectAxes);
        wGraphic_Curve->xAxis->setAutoTickStep(true);
        wGraphic_Curve->yAxis->setAutoTickStep(true);

    }
    else
    {


        wGraphic_1->setInteractions(NULL);

        if (runningMode==ModePreTest)
        {
            wGraphic_1->xAxis->setRange(X_RANGEPRETEST_MIN,X_RANGEPRETEST_MAX);//xInterval);
        }
        else
        {
            wGraphic_1->xAxis->setRange(X_RANGETEST_MIN,X_RANGETEST_MAX);//xInterval);
        }

        wGraphic_1->xAxis->setTickStep(X_TICKSTEP);
        wGraphic_1->xAxis->setAutoTickStep(false);
        wGraphic_1->yAxis->setRange(Y_TEMPERATURE_RANGE_MIN,Y_TEMPERATURE_RANGE_MAX);
        wGraphic_1->yAxis->setAutoTickStep(false);
        wGraphic_1->yAxis->setTickStep(100.0);


        //автоизменение шкалы по Х
        if ((graphicTemperature_1->data()->size()>0) && (graphicTemperature_1->data()->last().key >= wGraphic_1->xAxis->range().upper))
        {
            double newMaxRange=wGraphic_1->xAxis->range().upper + ( (int)((graphicTemperature_1->data()->last().key - wGraphic_1->xAxis->range().upper)/100.0) + 1)*100;
            wGraphic_1->xAxis->setRange(0,newMaxRange);

        }


        wGraphic_1->replot();


        wGraphic_56->setInteractions(NULL);
        wGraphic_56->xAxis->setRange(X_RANGEPRETEST_MIN,X_RANGEPRETEST_MIN);//xInterval);
        wGraphic_56->xAxis->setTickStep(X_TICKSTEP);
        wGraphic_56->xAxis->setAutoTickStep(false);
        wGraphic_56->yAxis->setRange(Y_TEMPERATURE_RANGE_MIN,Y_TEMPERATURE_RANGE_MAX);
        wGraphic_56->yAxis->setAutoTickStep(false);
        wGraphic_56->yAxis->setTickStep(100.0);

        //автоизменение шкалы по Х
        if ((graphicTemperature_5->data()->size()>0) && (graphicTemperature_5->data()->last().key >= wGraphic_56->xAxis->range().upper))
        {
            double newMaxRange=wGraphic_56->xAxis->range().upper + ( (int)((graphicTemperature_5->data()->last().key - wGraphic_56->xAxis->range().upper)/100.0) + 1)*100;
            wGraphic_56->xAxis->setRange(0,newMaxRange);

        }


        wGraphic_56->replot();

        wGraphic_Curve->setInteractions(NULL);
        wGraphic_Curve->xAxis->setRange(550,775);//xInterval);
        wGraphic_Curve->xAxis->setTickStep(25);
        wGraphic_Curve->xAxis->setAutoTickStep(false);
        wGraphic_Curve->yAxis->setRange(0,150);
        wGraphic_Curve->yAxis->setAutoTickStep(false);
        wGraphic_Curve->yAxis->setTickStep(10.0);


        wGraphic_Curve->replot();

    }

}
//=======================================================================================
void MainWindow::ButtonPageCalibr(bool toggled)
{
    ui->stackedWidget->setCurrentIndex(toggled?1:0);
    isCalibrationPageEnabled=toggled;
}
//=======================================================================================
void MainWindow::ViewDialogConfig()
{
    //QMessageBox::information(NULL,"ButonDialogConfig","pressed");
    dialogConfig.SetAnalogInputChannels(hashAnalogInputChannels);
    dialogConfig.SetComPort(comPort);

    dialogConfig.setModal(true);

    if (dialogConfig.exec()==QDialog::Accepted)
    {
        if (comPort!=dialogConfig.GetComPort())
        {
            comPort=dialogConfig.GetComPort();
            mbReader.StopPoll();
            //if (!mbReader.TestConnection(comPort))
            //{
            //    QMessageBox::critical(this,"Error","Немає підключення");
                //return;
            //}
            mbReader.SetComPort(comPort);
            mbReader.StartPoll();

        }

        SaveIniFile();

    }
}
//=======================================================================================
void MainWindow::slotRangeChanged(const QCPRange &newRange)
{
/*
    float range_x=newRange.upper - newRange.lower;
    //if (range_x > stopInterval)
    //{
        //wGraphic->xAxis->setRange(0,stopInterval);
    //}
    //чтоб умещалось 10 точек
    if (range_x>100) wGraphic_1->xAxis->setAutoTickStep(true);
    else wGraphic_1->xAxis->setAutoTickStep(false);

    if (range_x<=100 && range_x>10) wGraphic_1->xAxis->setTickStep(5.0);
    if (range_x<=10 && range_x>5) wGraphic_1->xAxis->setTickStep(1.0);
    if (range_x<=5 && range_x>0) wGraphic_1->xAxis->setTickStep(0.5);

    //wGraphic->yAxis->rescale(true);   //void QCPAxis::rescale ( bool  onlyVisiblePlottables = false)
    //wGraphic->replot();
*/
}
//=======================================================================================
double MainWindow::calcAverage(QVector<double> vec)
{
    if (vec.empty()) {
        return 0.0;
    }
    return std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();

    //QVector::
}
//=======================================================================================
//=======================================================================================
//=======================================================================================
void MainWindow::SliderSetVoltage(int value)
{
    double value_proc=(double)value/100.0;
    double value_volt=(double)value/1000.0;
    //ui->labelPowerSet->setText(QString("Потужність: ")+QString::number(value_proc,'f',1)+" %" + " ...");

    //ui->doubleSpinBoxPowerSet->setValue(value_proc);


    qDebug() << "SliderSetVoltage set=" << value_proc << value_volt;
    mbReader.VoltageSet(value_volt);
}
//=======================================================================================
void MainWindow::DoubleSpinBoxSetVoltage(double value)
{

    double value_proc=value;
    double value_volt=value/10.0;

    if (ui->buttonPowerOn->isChecked())
    {
        ui->sliderPowerSet->setValue(value_volt*1000);
        qDebug() << "DoubleSpinBoxSetVoltage=" << value_proc << "% " << value_volt<<"V";
        mbReader.VoltageSet(value_volt);
        //ui->groupBoxPowerSet->setStyleSheet("QGroupBox{border: 3px solid grey; border-radius:3px;}");
        ui->labelCirclePowerSet->setStyleSheet("QLabel{border: 2px solid grey; border-radius:10px; background-color:grey;}");
        SetLastPowerToIniFile(value);
    }


}
//=======================================================================================
void MainWindow::ButtonPowerOn(bool toggled)
{


    if (toggled)
    {
        double value_proc=ui->doubleSpinBoxPowerSet->value();
        double value_volt=value_proc/10.0;

        ui->sliderPowerSet->setValue(value_volt*1000);
        qDebug() << "DoubleSpinBoxSetVoltage=" << value_proc << "% " << value_volt<<"V";
        mbReader.VoltageSet(value_volt);
        //ui->groupBoxPowerSet->setStyleSheet("QGroupBox{border: 3px solid grey; border-radius:3px;}");
        ui->labelCirclePowerSet->setStyleSheet("QLabel{border: 2px solid grey; border-radius:10px; background-color:grey;}");

    }
    else
    {
        double value_proc=0.0;
        double value_volt=value_proc/10.0;

        ui->sliderPowerSet->setValue(value_volt*1000);
        qDebug() << "DoubleSpinBoxSetVoltage=" << value_proc << "% " << value_volt<<"V";
        mbReader.VoltageSet(value_volt);
        //ui->groupBoxPowerSet->setStyleSheet("QGroupBox{border: 3px solid grey; border-radius:3px;}");
        ui->labelCirclePowerSet->setStyleSheet("QLabel{border: 2px solid grey; border-radius:10px; background-color:grey;}");
    }


}
//=======================================================================================
void MainWindow::VoltageSettedOK()
{
    //ui->groupBoxPowerSet->setStyleSheet("QGroupBox{border: 3px solid darkgreen; border-radius:3px;}");
    ui->labelCirclePowerSet->setStyleSheet("QLabel{border: 2px solid darkgreen; border-radius:10px; background-color:darkgreen;}");
}
//=======================================================================================
void MainWindow::VoltageSettedError()
{
    //ui->groupBoxPowerSet->setStyleSheet("QGroupBox{border: 3px solid red; border-radius:3px;}");
    ui->labelCirclePowerSet->setStyleSheet("QLabel{border: 2px solid red; border-radius:10px; background-color:red;}");
}
//=======================================================================================
bool MainWindow::calcAvgMinMaxRegress(QList<QCPData> &data, double &avg, double &min, double &max, double &regress, double &regress_koeff_a, double &regress_koeff_b)
{
    double accuT1=0.0;
    double avgT1=0.0;
    double minT1=data.first().value;
    double maxT1=data.first().value;

    //for regression
    double S1=data.size();
    double S2=0.0;   //sum(t_i)
    double S3=0.0;
    double S4=0.0;
    double S5=0.0;

    if (data.size()==0) return false;

    foreach(QCPData cpdata, data)
    {
        //

        accuT1+=cpdata.value;

        if (minT1>cpdata.value) minT1=cpdata.value;
        if (maxT1<cpdata.value) maxT1=cpdata.value;

        S2+=cpdata.key;
        S3+=cpdata.value;
        S4+=cpdata.key * cpdata.key;
        S5+=cpdata.value * cpdata.key;

    }
    avgT1=accuT1 / data.size();


    if ((S1*S4 - S2*S2) == 0.0) return false;


    //в присланном Геннадием документе для рассчета линейной регрессии перепутаны а и в коэффициенты,
    //смотреть рассчет коэффициентов в http://mathprofi.ru/linejnyj_koefficient_korrelyacii.html
    //T- temperature, t- time
    //T(t)=a*t+b
    double b = (S3*S4 - S5*S2) / (S1*S4 - S2*S2);
    double a = (S1*S5 - S2*S3) / (S1*S4 - S2*S2);



    //regression = (a*t_last+b) - (a*t_first + b) ==  a*t_last - a*t_first
    double regressT1= (a*data.last().key + b) - (a*data.first().key + b);

    avg=avgT1;
    min=minT1;
    max=maxT1;
    regress=regressT1;
    regress_koeff_a=a;
    regress_koeff_b=b;
    return true;

}


//=======================================================================================
void MainWindow::Timer1000ms()
{

    timer1000ms.setInterval(1000);

    ui->labelTemperature_1->setText(temperature_1.GetChName());
    ui->labelTemperature_2->setText(temperature_2.GetChName());
    ui->labelTemperature_3->setText(temperature_3.GetChName());
    ui->labelTemperature_4->setText(temperature_4.GetChName());
    ui->labelTemperature_5->setText(temperature_5.GetChName());
    ui->labelTemperature_6->setText(temperature_6.GetChName());

    //wGraphic_1->yAxis->setLabel(movement_1.GetChName()+", "+movement_1.GetEU());
    //wGraphic_2->yAxis->setLabel(movement_2.GetChName()+", "+movement_2.GetEU());

    ui->lineEditValueTemperature_1->setText(temperature_1.GetValueString(2));
    ui->lineEditValueTemperature_2->setText(temperature_2.GetValueString(2));
    ui->lineEditValueTemperature_3->setText(temperature_3.GetValueString(2));
    ui->lineEditValueTemperature_4->setText(temperature_4.GetValueString(2));
    ui->lineEditValueTemperature_5->setText(temperature_5.GetValueString(2));
    ui->lineEditValueTemperature_6->setText(temperature_6.GetValueString(2));

    if (cmdButton==StartCmd)
    {
         ui->buttonStartStop->setIcon(QIcon(":/icons/player_stop.png"));
         ui->buttonStartStop->setText("Стоп");
         runningMode=ModeTest;
         //controlPoints.clear();
         infoText="СТАРТ ВИПРОБУВАННЯ   ";
         startTestDT=QDateTime::currentDateTime();
         startTestDT_str=startTestDT.toString("yyyy.MM.dd_hh.mm.ss");
         infoText+=" "+startTestDT.toString("hh:mm:ss dd.MM.yy");

         AddCsvMessage("start test");

         //infoText+=QString("\nЧАС: 00:00:00");
         //ui->listWidgetInfo->clear();
         //ui->listWidgetInfo->addItem(QTime::currentTime().toString()+ QString(" Started Test Mode"));
         //ui->listWidgetInfo->addItem(QTime::currentTime().toString()+ QString(" Wait temperature:"));
         wGraphic_1->xAxis->setRange(X_RANGETEST_MIN,X_RANGETEST_MAX);//xInterval);
         wGraphic_1->yAxis->setRange(Y_TEMPERATURE_RANGE_MIN,Y_TEMPERATURE_RANGE_MAX);
         //wGraphic->yAxis2->setRange(0,Y_TEMPERATURE_RANGE);
         //graphicMovement_1->clearData();
         //graphicMovement_2->clearData();
         //graphicTemperature_1->clearData();
         //graphicTemperature_2->clearData();
         //graphicTemperature_3->clearData();
         //graphicTemperature_4->clearData();
         //graphicTemperature_5->clearData();
         //graphicTemperature_6->clearData();


         //перешкалировать температуры 1,2,3,4 по Х, чтобы получить шкалу на графике -600...1800,  т.е. от -10мин до +30минут
         //


         double seconds_from_start=startPreTestDT.msecsTo(startTestDT)/1000.0;

         QList<QCPData> temp_1_data;
         foreach(QCPData cpdata, graphicTemperature_1->data()->values())
         {
            cpdata.key=cpdata.key-seconds_from_start;
            temp_1_data.append(cpdata);
         }
         QList<QCPData> temp_2_data;
         foreach(QCPData cpdata, graphicTemperature_2->data()->values())
         {
            cpdata.key=cpdata.key-seconds_from_start;
            temp_2_data.append(cpdata);
         }
         QList<QCPData> temp_3_data;
         foreach(QCPData cpdata, graphicTemperature_3->data()->values())
         {
            cpdata.key=cpdata.key-seconds_from_start;
            temp_3_data.append(cpdata);
         }
         QList<QCPData> temp_4_data;
         foreach(QCPData cpdata, graphicTemperature_4->data()->values())
         {
            cpdata.key=cpdata.key-seconds_from_start;
            temp_4_data.append(cpdata);
         }

         graphicTemperature_1->clearData();
         foreach(QCPData cpdata, temp_1_data)
         {
            graphicTemperature_1->addData(cpdata);
         }

         graphicTemperature_2->clearData();
         foreach(QCPData cpdata, temp_2_data)
         {
            graphicTemperature_2->addData(cpdata);
         }

         graphicTemperature_3->clearData();
         foreach(QCPData cpdata, temp_3_data)
         {
            graphicTemperature_3->addData(cpdata);
         }

         graphicTemperature_4->clearData();
         foreach(QCPData cpdata, temp_4_data)
         {
            graphicTemperature_4->addData(cpdata);
         }

         //то же графики регрессов
         QList<QCPData> regr_1_data;
         foreach(QCPData cpdata, graphicRegress_1->data()->values())
         {
            cpdata.key=cpdata.key-seconds_from_start;
            regr_1_data.append(cpdata);
         }
         QList<QCPData> regr_2_data;
         foreach(QCPData cpdata, graphicRegress_2->data()->values())
         {
            cpdata.key=cpdata.key-seconds_from_start;
            regr_2_data.append(cpdata);
         }

         graphicRegress_1->clearData();
         foreach(QCPData cpdata, regr_1_data)
         {
            graphicRegress_1->addData(cpdata);
         }

         graphicRegress_2->clearData();
         foreach(QCPData cpdata, regr_2_data)
         {
            graphicRegress_2->addData(cpdata);
         }


         //startTestDT=QDateTime::currentDateTime();
         //startTestTemperature=value_grC_offseted;
         //sound_start->play();
         //QProcess::startDetached(QString("aplay ")+QApplication::applicationDirPath() + "/start.wav");
         cmdButton=NoCmd;
         ui->buttonReset->setEnabled(false);
         ui->buttonConfig->setEnabled(false);
         ui->buttonExit->setEnabled(false);


         //return;

    } //if (cmdButton==StartCmd)



    if (runningMode==ModePreTest)
    {
         //добавляем данные на график
         double seconds_from_start;

         seconds_from_start=startPreTestDT.msecsTo(QDateTime::currentDateTime())/1000.0;
         //double minutes_from_start=seconds_from_start/60.0;

         //graphicMovement_1->addData(minutes_from_start,movement_1.GetValue());
         //graphicMovement_2->addData(minutes_from_start,movement_2.GetValue());

         graphicTemperature_1->addData(seconds_from_start,temperature_1.GetValue());
         graphicTemperature_2->addData(seconds_from_start,temperature_2.GetValue());
         graphicTemperature_3->addData(seconds_from_start,temperature_3.GetValue());
         graphicTemperature_4->addData(seconds_from_start,temperature_4.GetValue());
         graphicTemperature_5->addData(seconds_from_start,temperature_5.GetValue());
         graphicTemperature_6->addData(seconds_from_start,temperature_6.GetValue());

         AddCsvMessage();

         if (!ui->buttonTrendZoom->isChecked())
         {
             //автоизменение шкалы по Х
             if (seconds_from_start >= wGraphic_1->xAxis->range().upper)
             {
                double newMaxRange=wGraphic_1->xAxis->range().upper + 100;
                wGraphic_1->xAxis->setRange(0,newMaxRange);

             }
         }




        /*
        //автоизменение шкалы вверх - увеличиваем так, чтоб график не біл на верхней границе
        if (ppm_pwm1*1.03>Y_PRESSURE_RANGE && ppm_pwm1*1.03 > wGraphic->yAxis->range().upper)        {
            wGraphic->yAxis->setRange(wGraphic->yAxis->range().lower, ppm_pwm1*1.03);
            //wGraphic->yAxis->rescale();      // Масштабируем график по данным
        }
        //автоизменение шкалы вниз
        if (ppm_pwm1 < wGraphic->yAxis->range().lower)
        {
            wGraphic->yAxis->setRange(ppm_pwm1,wGraphic->yAxis->range().upper);
            //wGraphic->yAxis->rescale();      // Масштабируем график по данным
        }

        //автоизменение шкалы вверх - увеличиваем так, чтоб график не біл на верхней границе
        if (ppm_pwm2*1.03>Y_TEMPERATURE_RANGE && ppm_pwm2*1.03 > wGraphic->yAxis2->range().upper)
        {
            wGraphic->yAxis2->setRange(wGraphic->yAxis2->range().lower, ppm_pwm2*1.03);
            //wGraphic->yAxis->rescale();      // Масштабируем график по данным
        }
        //автоизменение шкалы вниз
        if (ppm_pwm2 < wGraphic->yAxis2->range().lower)
        {
            wGraphic->yAxis2->setRange(ppm_pwm2,wGraphic->yAxis2->range().upper);
            //wGraphic->yAxis->rescale();      // Масштабируем график по данным
        }
        */

        /*
        //автоизменение шкалы по Х


        if (seconds_from_start>X_RANGEVIEW)
        {
            wGraphic->xAxis->setRange(seconds_from_start-X_RANGEVIEW,seconds_from_start);
            graphicPpm_pwm1->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicPpm_pwm2->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicPpm_pwm3->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicPressure_1->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicPressure_2->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicAirFlow_1->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicAirFlow_2->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicHeatFlow->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicForce->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);

            //new in GemobudMaskTest3
            graphicTemperature_1->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicTemperature_2->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
            graphicPpm_O2->removeDataBefore(seconds_from_start-X_RANGEVIEW-1);
        }
        */


         int viewRunningSecs=startPreTestDT.secsTo(QDateTime::currentDateTime());
         QString viewRunningStr="";
         if (viewRunningSecs / 3600 < 10) viewRunningStr+="0";
         viewRunningStr+=QString::number(viewRunningSecs / 3600) + ":";

         viewRunningSecs=viewRunningSecs % 3600;
         if (viewRunningSecs / 60 < 10) viewRunningStr+="0";
         viewRunningStr+=QString::number(viewRunningSecs / 60) + ":";

         viewRunningSecs=viewRunningSecs % 60;
         if (viewRunningSecs < 10) viewRunningStr+="0";
         viewRunningStr+=QString::number(viewRunningSecs);



         //calc
         //double aveT=calcAverage(graphicTemperature_1->data()->values().toVector());


         //данные за последние 10 минут
         QList<QCPData> temp_1_data;
         foreach(QCPData cpdata, graphicTemperature_1->data()->values())
         {
            if (cpdata.key >= seconds_from_start - 600) temp_1_data.append(cpdata);
         }
         QList<QCPData> temp_2_data;
         foreach(QCPData cpdata, graphicTemperature_2->data()->values())
         {
            if (cpdata.key >= seconds_from_start - 600) temp_2_data.append(cpdata);
         }


         if (temp_1_data.size()==0 || temp_2_data.size()==0)
         {
            ui->labelInfo->setText(infoText+QString("\nЧАС: ") + viewRunningStr+"\n n/d");
            return;
         }

         //double accuT1=0.0;
         double avgT1=0.0;
         double minT1=0.0;//temp_1_data.first().value;
         double maxT1=0.0;//temp_1_data.first().value;
         double regressT1=0.0;
         double regressT1_koeff_a=0.0;
         double regressT1_koeff_b=0.0;


         double avgT2=0.0;
         double minT2=0.0;
         double maxT2=0.0;
         double regressT2=0.0;
         double regressT2_koeff_a=0.0;
         double regressT2_koeff_b=0.0;

         calcAvgMinMaxRegress(temp_1_data,avgT1,minT1,maxT1,regressT1,regressT1_koeff_a,regressT1_koeff_b);
         calcAvgMinMaxRegress(temp_2_data,avgT2,minT2,maxT2,regressT2,regressT2_koeff_a,regressT2_koeff_b);


         // Tavg=(750±5)°C  |T-Tavg|≤10°C  Treg≤2°C  на протязі 10 хв.

         QString temp1StabilizationInfo,temp2StabilizationInfo;

         if (temp1_PreTestStabilized || ((seconds_from_start-600>=0) && (fabs(avgT1-750.0)<=5.0) && ((std::max(fabs(maxT1-avgT1),fabs(minT1-avgT1)))<=10.0) && (fabs(regressT1)<=2.0)))
         {
            temp1_PreTestStabilized=true;
            temp1StabilizationInfo=temperature_1.GetChName() +  " - стабілізації досягнуто.";
            AddCsvMessage("T1-stabilized");
         }
         else
         {
            temp1StabilizationInfo=temperature_1.GetChName()+": Tavg="+QString::number(avgT1,'f',2)+"  |T-Tavg|="+QString::number(std::max(fabs(maxT1-avgT1),fabs(minT1-avgT1)),'f',2)+"  Treg="+QString::number(regressT1,'f',3);
         }

         if (temp2_PreTestStabilized || ((seconds_from_start-600>=0) && (fabs(avgT2-750.0)<=5.0) && ((std::max(fabs(maxT2-avgT2),fabs(minT2-avgT2)))<=10.0) && (fabs(regressT2)<=2.0)))
         {
            temp2_PreTestStabilized=true;
            temp2StabilizationInfo=temperature_2.GetChName() +  " - стабілізації досягнуто.";
            AddCsvMessage("T2-stabilized");
         }
         else
         {
            temp2StabilizationInfo=temperature_2.GetChName()+": Tavg="+QString::number(avgT2,'f',2)+"  |T-Tavg|="+QString::number(std::max(fabs(maxT2-avgT2),fabs(minT2-avgT2)),'f',2)+"  Treg="+QString::number(regressT2,'f',3);
         }


         ui->labelInfo->setText(infoText+QString("\nЧАС: ") + viewRunningStr+"\n"+temp1StabilizationInfo+
                                                                             "\n"+temp2StabilizationInfo);



         graphicRegress_1->clearData();
         graphicRegress_1->addData(temp_1_data.first().key, regressT1_koeff_a*temp_1_data.first().key + regressT1_koeff_b);
         graphicRegress_1->addData(temp_1_data.last().key,  regressT1_koeff_a*temp_1_data.last().key  + regressT1_koeff_b);

         graphicRegress_2->clearData();
         graphicRegress_2->addData(temp_2_data.first().key, regressT2_koeff_a*temp_2_data.first().key + regressT2_koeff_b);
         graphicRegress_2->addData(temp_2_data.last().key,  regressT2_koeff_a*temp_2_data.last().key  + regressT2_koeff_b);

        wGraphic_1->replot();           // Отрисовываем график





   //Graphic_56

        if (!ui->buttonTrendZoom->isChecked())
        {
            //автоизменение шкалы по Х
            if (seconds_from_start >= wGraphic_56->xAxis->range().upper)
            {
                double newMaxRange=wGraphic_56->xAxis->range().upper + 100;
                wGraphic_56->xAxis->setRange(0,newMaxRange);

            }
        }

        /*
        //данные за последние 10 минут
        QList<QCPData> temp_5_data;
        foreach(QCPData cpdata, graphicTemperature_5->data()->values())
        {
            if (cpdata.key >= seconds_from_start - 600) temp_5_data.append(cpdata);
        }
        QList<QCPData> temp_6_data;
        foreach(QCPData cpdata, graphicTemperature_6->data()->values())
        {
            if (cpdata.key >= seconds_from_start - 600) temp_6_data.append(cpdata);
        }


        if (temp_5_data.size()==0 || temp_5_data.size()==0)
        {
            ui->labelInfo->setText(infoText+QString("\nЧАС: ") + viewRunningStr+"\n n/d");
            return;
        }

        double avgT5=0.0;
        double minT5=0.0;//temp_1_data.first().value;
        double maxT5=0.0;//temp_1_data.first().value;
        double regressT5=0.0;
        double regressT5_koeff_a=0.0;
        double regressT5_koeff_b=0.0;


        double avgT6=0.0;
        double minT6=0.0;
        double maxT6=0.0;
        double regressT6=0.0;
        double regressT6_koeff_a=0.0;
        double regressT6_koeff_b=0.0;

        calcAvgMinMaxRegress(temp_5_data,avgT5,minT5,maxT5,regressT5,regressT5_koeff_a,regressT5_koeff_b);
        calcAvgMinMaxRegress(temp_6_data,avgT6,minT6,maxT6,regressT6,regressT6_koeff_a,regressT6_koeff_b);


        // Умови стабілізації:  Tavg=750±5°C   |T-Tavg|≤10°C   Treg≤2°C  на протязі 10 хвилин.

        if (temp5_isStabilized || ((seconds_from_start-600>=0) && (fabs(avgT5-750.0)<=5.0) && ((std::max(fabs(maxT5-avgT5),fabs(minT5-avgT5)))<=10.0) && (fabs(regressT5)<=2.0)))
        {
            temp5_isStabilized=true;
            ui->labelTemperature_5_Stabilization->setText(temperature_5.GetChName() +  " - стабілізації досягнуто.");
            ui->labelTemperature_5_Stabilization->setStyleSheet("QLabel{color: green; font-size:16px;}");
        }
        else
        {
            ui->labelTemperature_5_Stabilization->setText(QString(temperature_5.GetChName() + " - очікуєм стабілізації...")+ " Tavg="+QString::number(avgT5,'f',2)+
                                                                  "  |T-Tavg|="+QString::number(std::max(fabs(maxT5-avgT5),fabs(minT5-avgT5)),'f',2)+"  Treg="+QString::number(regressT5,'f',2));
            ui->labelTemperature_5_Stabilization->setStyleSheet("QLabel{color: red; font-size:16px;}");
        }

        if (temp6_isStabilized || ((seconds_from_start-600>=0) && (fabs(avgT6-750.0)<=5.0) && ((std::max(fabs(maxT6-avgT6),fabs(minT6-avgT6)))<=10.0) && (fabs(regressT6)<=2.0)))
        {
            temp6_isStabilized=true;
            ui->labelTemperature_6_Stabilization->setText(temperature_6.GetChName() + " - стабілізації досягнуто.");
            ui->labelTemperature_6_Stabilization->setStyleSheet("QLabel{color: green; font-size:16px;}");
        }
        else
        {
            ui->labelTemperature_6_Stabilization->setText(QString(temperature_6.GetChName() + " - очікуєм стабілізації...")+ " Tavg="+QString::number(avgT6,'f',2)+
                                                                  "  |T-Tavg|="+QString::number(std::max(fabs(maxT6-avgT6),fabs(minT6-avgT6)),'f',2)+"  Treg="+QString::number(regressT6,'f',2));
            ui->labelTemperature_6_Stabilization->setStyleSheet("QLabel{color: red; font-size:16px;}");
        }

        */









        wGraphic_56->replot();





    } //if (runningMode==ModePreTest)



    if (runningMode==ModeTest)
    {
        //добавляем данные на график
        double seconds_from_start;

        seconds_from_start=startTestDT.msecsTo(QDateTime::currentDateTime())/1000.0;
        //double minutes_from_start=seconds_from_start/60.0;

        //graphicMovement_1->addData(minutes_from_start,movement_1.GetValue());
        //graphicMovement_2->addData(minutes_from_start,movement_2.GetValue());

        graphicTemperature_1->addData(seconds_from_start,temperature_1.GetValue());
        graphicTemperature_2->addData(seconds_from_start,temperature_2.GetValue());
        graphicTemperature_3->addData(seconds_from_start,temperature_3.GetValue());
        graphicTemperature_4->addData(seconds_from_start,temperature_4.GetValue());

        AddCsvMessage();

        if (!ui->buttonTrendZoom->isChecked())
        {
            //автоизменение шкалы по Х
            if (seconds_from_start >= wGraphic_1->xAxis->range().upper)
            {
                double newMaxRange=wGraphic_1->xAxis->range().upper + 100;
                wGraphic_1->xAxis->setRange(0,newMaxRange);

            }
        }






        int testRunningSecs=startTestDT.secsTo(QDateTime::currentDateTime());
        QString testRunningStr="";
        if (testRunningSecs / 3600 < 10) testRunningStr+="0";
        testRunningStr+=QString::number(testRunningSecs / 3600) + ":";

        testRunningSecs=testRunningSecs % 3600;
        if (testRunningSecs / 60 < 10) testRunningStr+="0";
        testRunningStr+=QString::number(testRunningSecs / 60) + ":";

        testRunningSecs=testRunningSecs % 60;
        if (testRunningSecs < 10) testRunningStr+="0";
        testRunningStr+=QString::number(testRunningSecs);





        //для ожидания первого интервала и потом добавляем пять минут и ждем следующий, последний будет на 60 минуте;
        //double seconds_end_current_interval=1800; // 30 minutes
        //double secons_add_to_switch_next_interval=300;   //5 minutes
        //double secons_last_interval=3600;   //60 minutes


        if (seconds_from_start>=seconds_end_first_interval+num_interval*secons_add_to_switch_next_interval)
        {

            //взять данные за последние 10 минут, рассчитать среднее, регрессию, построить графики регрессий
            //данные за последние 10 минут
            QList<QCPData> temp_1_data;
            foreach(QCPData cpdata, graphicTemperature_1->data()->values())
            {
                if (cpdata.key >= seconds_from_start - 600) temp_1_data.append(cpdata);
            }
            QList<QCPData> temp_2_data;
            foreach(QCPData cpdata, graphicTemperature_2->data()->values())
            {
                if (cpdata.key >= seconds_from_start - 600) temp_2_data.append(cpdata);
            }


            if (temp_1_data.size()==0 || temp_2_data.size()==0)
            {
                return;
            }

            //double accuT1=0.0;
            double avgT1=0.0;
            double minT1=0.0;//temp_1_data.first().value;
            double maxT1=0.0;//temp_1_data.first().value;
            double regressT1=0.0;
            double regressT1_koeff_a=0.0;
            double regressT1_koeff_b=0.0;


            double avgT2=0.0;
            double minT2=0.0;
            double maxT2=0.0;
            double regressT2=0.0;
            double regressT2_koeff_a=0.0;
            double regressT2_koeff_b=0.0;

            calcAvgMinMaxRegress(temp_1_data,avgT1,minT1,maxT1,regressT1,regressT1_koeff_a,regressT1_koeff_b);
            calcAvgMinMaxRegress(temp_2_data,avgT2,minT2,maxT2,regressT2,regressT2_koeff_a,regressT2_koeff_b);


            // Tavg=(750±5)°C  |T-Tavg|≤10°C  Treg≤2°C  на протязі 10 хв.

            //QString temp1StabilizationInfo,temp2StabilizationInfo;

            if (/*temp1_TestStabilized || */((seconds_from_start-600>=0) && (fabs(regressT1)<=2.0)))
            {
                temp1_TestStabilized=true;
                temp1TestStabilizationInfo=temperature_1.GetChName() +  " - стабілізації досягнуто.";
            }
            else
            {
                temp1_TestStabilized=false;
                temp1TestStabilizationInfo=temperature_1.GetChName()+":  Treg="+QString::number(regressT1,'f',3);
            }

            if (/*temp2_TestStabilized || */((seconds_from_start-600>=0) && (fabs(regressT2)<=2.0)))
            {
                temp2_TestStabilized=true;
                temp2TestStabilizationInfo=temperature_2.GetChName() +  " - стабілізації досягнуто.";
            }
            else
            {
                temp2_TestStabilized=false;
                temp2TestStabilizationInfo=temperature_2.GetChName()+":  Treg="+QString::number(regressT2,'f',3);
            }


            //ui->labelInfo->setText(infoText+QString("\nЧАС: ") + testRunningStr+"\n");

            //ui->labelInfo->setText(ui->labelInfo->text()+QString("ОЧІКУЄМ ")+QString::number(30+num_interval*5)+" хвилину...");






            //graphicTestRegress_1->clearData();
            graphicTestRegress_1[num_interval]->addData(temp_1_data.first().key, regressT1_koeff_a*temp_1_data.first().key + regressT1_koeff_b);
            graphicTestRegress_1[num_interval]->addData(temp_1_data.last().key,  regressT1_koeff_a*temp_1_data.last().key  + regressT1_koeff_b);

            //graphicRegress_2->clearData();
            graphicTestRegress_2[num_interval]->addData(temp_2_data.first().key, regressT2_koeff_a*temp_2_data.first().key + regressT2_koeff_b);
            graphicTestRegress_2[num_interval]->addData(temp_2_data.last().key,  regressT2_koeff_a*temp_2_data.last().key  + regressT2_koeff_b);



            //проверяем на последний интервал
            if (num_interval>=7)
            {
                runningMode=ModeTestStopped;//STOP
                testStopReason=QString("ЗАВЕРШЕНО... Стабілізації не досягнуто за 60 хв.");
                AddCsvMessage("stop test - stabilization did not occur within 60 minutes");
            }


            if(temp1_TestStabilized &&  temp2_TestStabilized)
            {
                runningMode=ModeTestStopped; //STOP
                testStopReason=QString("ЗАВЕРШЕНО... Стабілізація досягнута на ")+QString::number(30+num_interval*5)+" хв.";
                AddCsvMessage("stop test - stabilization occurred at "+ QString::number(30+num_interval*5) +" minutes");
            }


            //переключаем интервал
            num_interval++;
            //seconds_end_current_interval=seconds_end_current_interval+secons_add_to_switch_next_interval;



        }

        ui->labelInfo->setText(infoText+QString("\nЧАС: ") + testRunningStr+"\n"+
                                        QString("ОЧІКУЄМ ")+QString::number(30+num_interval*5)+" хвилину..."+"\n"+
                                        temp1TestStabilizationInfo+"\n"+
                                        temp2TestStabilizationInfo+"\n"+
                                        testStopReason);


        if (runningMode==ModeTestStopped)
        {
            CreateTestReport();
        }



        //если достигнуто условие окончания
        //для режима Test (Испытание) - нажатие кнопки СТОП
        //для режима Test (Испытание) - резкое падение давления на 20% за 1сек.


        if (cmdButton==StopCmd)
        {

            ui->buttonStartStop->setIcon(QIcon(":/icons/player_play.png"));
            ui->buttonStartStop->setText("Старт");
            runningMode=ModeTestStopped;
            //sound_stop->play();

            //QProcess::startDetached(QString("aplay ")+QApplication::applicationDirPath() + "/stop.wav");
            testStopReason=QString("ЗАВЕРШЕНО... Зупинено оператором");
            ui->labelInfo->setText(infoText+QString("\nЧАС: ") + testRunningStr+"\n"+
                                        QString("ОЧІКУЄМ ")+QString::number(30+num_interval*5)+" хвилину..."+"\n"+
                                        temp1TestStabilizationInfo+"\n"+
                                   temp2TestStabilizationInfo+"\n"+
                                   testStopReason);


            AddCsvMessage("stop test -  stopped by the operator");


            //get 1st graph

            //wGraphic_1->replot();
            //wGraphic_1->savePng(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_1.png",640,480);
            //QImage img_1(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_1.png");

            //get 2nd graph
            //wGraphic_2->replot();
            //wGraphic_2->savePng(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_2.png",640,480);
            //QImage img_2(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_2.png");


            //img_2 = img_2.convertToFormat(QImage::Format_Grayscale16 ,Qt::AutoColor | Qt::ThresholdDither);
            //img_2.save(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_2_ttt.png");


            CreateTestReport();
            //createReport(qApp->applicationDirPath()+"/reports/"+startTestDT_str, startTestDT.toString("dd.MM.yyyy hh:mm:ss") ,
            //             testRunningStr, img_1, temperature_1.GetValue(), temperature_2.GetValue());

            ui->buttonReset->setEnabled(true);
            ui->buttonConfig->setEnabled(true);
            ui->buttonExit->setEnabled(true);


        }


        wGraphic_1->replot();           // Отрисовываем график

    }   //     if(runningMode==ModeTest)


















}

//=======================================================================================


//=======================================================================================
void MainWindow::ButtonReset()
{


    //graphicMovement_1->clearData();
    //graphicMovement_2->clearData();
    graphicTemperature_1->clearData();
    graphicTemperature_2->clearData();
    graphicTemperature_3->clearData();
    graphicTemperature_4->clearData();




    wGraphic_1->xAxis->setRange(X_RANGEPRETEST_MIN,X_RANGEPRETEST_MAX);//xInterval);
    wGraphic_1->yAxis->setRange(Y_TEMPERATURE_RANGE_MIN,Y_TEMPERATURE_RANGE_MAX);
    //wGraphic->yAxis2->setRange(0, Y_CO2_RANGE_PROCENTS);

    startTestDT=QDateTime::currentDateTime();
    //ui->labelInfo->setText("ОЧІКУЄМ СТАРТ");
    //mbReader.SetDOState(DO_Command::DO_ON_CMD);



    wGraphic_1->replot();
}
//=======================================================================================
void MainWindow::ButtonStartStop()
{
    //команда на старт
    if (runningMode==ModePreTest || runningMode==ModeTestStopped)
    {
        cmdButton=StartCmd;
    }

    //команда на стоп
    if (runningMode==ModeTest)
    {
        if (QMessageBox::question(NULL,"Припинення тесту","Зупинити тест та сформувати звіт?",QMessageBox::Yes | QMessageBox::No)==QMessageBox::Yes)
        {
            cmdButton=StopCmd;
        }
    }
}
//=======================================================================================
void MainWindow::ButtonExit()
{
    //QProcess::startDetached("sudo shutdown -h now");
    close();

}
//=======================================================================================
void MainWindow::ButtonReports()
{

    if (!isCalibrationPageEnabled)
    {

        FMDlg dlg(tr("Reports"), qApp->applicationDirPath()+"/reports/", this);

        //dlg.setLocalPath("reports");
        //dlg.setFilter(QStringList() << "*.odf");
        //dlg.enablePreviewButton();

        dlg.launch();
    }
    else
    {
        FMDlg dlg(tr("Calibration Reports"), qApp->applicationDirPath()+"/calibration_reports/", this);
        dlg.launch();
    }

}
//=======================================================================================
void MainWindow::ValueChanged(QString str)
{
/*
    //  QLineEdit *snd=(QLineEdit *)sender();
    float operatorPointMin=0;
    float operatorPointMax=50;

    QPalette palette_green = ui->lineEditOperatorPoint_1->palette();
    QPalette palette_red = ui->lineEditOperatorPoint_1->palette();

    palette_green.setColor( QPalette::Text, Qt::black );
    palette_red.setColor( QPalette::Text, Qt::red );
 //   p.setColor(QPalette::Base, Qt::red);
 //   p.setColor(QPalette::Background, Qt::red);
  //    snd->setPalette( palette );


      if (ui->lineEditOperatorPoint_1->text().length()==0 ||
         ( ui->lineEditOperatorPoint_1->text().toFloat()>=operatorPointMin && ui->lineEditOperatorPoint_1->text().toFloat()<=operatorPointMax))
      {
        ui->lineEditOperatorPoint_1->setPalette(palette_green);
      }
      else
      {
        ui->lineEditOperatorPoint_1->setPalette(palette_red);
      }
*/
 // QMessageBox::information(this,"Test","ddasfsd",QMessageBox::Ok);

}
//=======================================================================================
//================================================================================================
//
//Reports
//
//================================================================================================
void MainWindow::setCurrentBlockAlignment(QTextCursor &cursor,
                              Qt::Alignment alignment) {
  QTextBlockFormat tmp = cursor.blockFormat();
  tmp.setAlignment(alignment);
  cursor.setBlockFormat(tmp);
}
//================================================================================================
QTextCharFormat MainWindow::charFormat(int pointSize,
                           bool bold) {
  QTextCharFormat tmp;
  QFont font;
  font.setFamily("Times");
  tmp.setFont(font);
  tmp.setFontPointSize(pointSize);
  tmp.setFontWeight(bold ? QFont::Bold : QFont::Normal);
  return tmp;
}
//================================================================================================

void MainWindow::createReport(const QString &fileName,
                              const QString &startDT,
                              const QString &timeDT,
                              const QImage &plot_movement_1,
                              const double X_1,
                              const double X_2)
{

  QTextDocument *document = new QTextDocument();




  //QFont font;
  //font.setFamily("Helvetica");//("Swiss");//("Times New Roman");//("Times");

  QTextCursor cursor(document);

//  QString date = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

//  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
//  cursor.insertText(date + '\n', charFormat(11, false));//12

//  cursor.insertBlock();
//  setCurrentBlockAlignment(cursor, Qt::AlignCenter);
//  cursor.insertText(company, charFormat(11, false));//12

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignCenter);
  cursor.insertText("Звіт по випробуванню", charFormat(16, true));

  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  cursor.insertText("Початок випробування: " + startDT , charFormat(14, true));//12
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertText("Час випробування: " + timeDT , charFormat(14, true));//12
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n


  //TEST 1

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Тест 1", charFormat(14, true));//12
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

//  cursor.insertText("Умова закінчення: " + stopCondition , charFormat(14, true));//12
//  cursor.movePosition(QTextCursor::End);
//  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n
\
  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  cursor.insertText("Заміри верхнього рівня пластичного шару:", charFormat(12, true));//12
  cursor.movePosition(QTextCursor::End);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n


/*
  //table
  cursor.movePosition(QTextCursor::End);
  cursor.insertTable(levelup_points_1.size()+1,4);

  //первая строка - 4 ячейки
  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Час, хв.", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Замір опер., мм", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Усадка, мм", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Температура, С", charFormat(12, true));


  foreach(OperatorPoint pnt, levelup_points_1)
  {

      cursor.movePosition(QTextCursor::NextCell);
      setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
      cursor.insertText(QString::number(pnt.time_min,'f',0));

      cursor.movePosition(QTextCursor::NextCell);
      setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
      cursor.insertText(QString::number(pnt.operatorEnteredPoint,'f',1));

      cursor.movePosition(QTextCursor::NextCell);
      setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
      cursor.insertText(QString::number(pnt.movement,'f',1));

      cursor.movePosition(QTextCursor::NextCell);
      setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
      cursor.insertText(QString::number(pnt.temperature,'f',1));
  }


  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertText("Заміри нижнього рівня пластичного шару:", charFormat(12, true));//12
  cursor.movePosition(QTextCursor::End);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  //table
  cursor.movePosition(QTextCursor::End);
  cursor.insertTable(leveldown_points_1.size()+1,4);
  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);

  //первая строка - 4 ячейки
  cursor.insertText("Час, хв.", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Замір опер., мм", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Усадка, мм", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Температура, С", charFormat(12, true));


  foreach(OperatorPoint pnt, leveldown_points_1)
  {

      cursor.movePosition(QTextCursor::NextCell);
      setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
      cursor.insertText(QString::number(pnt.time_min,'f',0));

      cursor.movePosition(QTextCursor::NextCell);
      setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
      cursor.insertText(QString::number(pnt.operatorEnteredPoint,'f',1));

      cursor.movePosition(QTextCursor::NextCell);
      setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
      cursor.insertText(QString::number(pnt.movement,'f',1));

      cursor.movePosition(QTextCursor::NextCell);
      setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
      cursor.insertText(QString::number(pnt.temperature,'f',1));
  }

  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertText(QString("X (значення усадки по закінченню) = ") + QString::number(X_1,'f',1) + " мм", charFormat(12, true));


  QVector<OperatorPoint> all_points_1 = levelup_points_1 + leveldown_points_1;

  double max_difference_1=0.0;
  foreach (OperatorPoint point, all_points_1)
  {
      double difference=getLinearApproximatedValue(levelup_points_1, point.time_min) -
                        getLinearApproximatedValue(leveldown_points_1, point.time_min);

      if (max_difference_1 < difference) max_difference_1 = difference;
  }

  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n
  cursor.insertText(QString("Y (максимальна різниця верхніого і нижнього рівнів пл. шару) = ") + QString::number(max_difference_1,'f',1) + " мм", charFormat(12, true));
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n
*/
  //graphic
  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();

  setCurrentBlockAlignment(cursor, Qt::AlignCenter);


  //adding image to document

  //cursor.insertImage(plot_movement_1);

  document->addResource(QTextDocument::ImageResource, QUrl("plot_movement_1.png"), plot_movement_1);
  QTextImageFormat imageFormat_1;
  imageFormat_1.setQuality(100);
  imageFormat_1.setName("plot_movement_1.png");
  cursor.insertImage(imageFormat_1);
  cursor.movePosition(QTextCursor::End);





  cursor.insertText(QObject::tr("\n\n"), charFormat(12, true));//casey - line \n
  cursor.movePosition(QTextCursor::End);

  //TESTS END





  cursor.insertText(QObject::tr("\n\n"), charFormat(12, true));//casey - line \n
  cursor.movePosition(QTextCursor::End);

  qDebug() << fileName;
  QTextDocumentWriter writer(fileName+".odt");
  if (!writer.write(document)) qDebug() << "errro write";

  qDebug() << writer.supportedDocumentFormats();

  //QDesktopServices::openUrl(QUrl(fileName+".odt"));

  //QTextEdit  *te = new QTextEdit();
  //te->setDocument(document);
  //te->showMaximized();

  delete document;

}

//=================================================================================================
double MainWindow::getLinearApproximatedValue(QVector<OperatorPoint> points, double x)
{

    if (points.size()==0)
    {
        return 0.0;
    }

    if (points.size()==1)
    {
        return points[0].operatorEnteredPoint;
    }

    double y, x1,x2,y1,y2;

    //key==time_min
    //val==operatorEnteredPoint

    if (x <= points[0].time_min)
    {
        x1=points[0].time_min;
        x2=points[1].time_min;
        y1=points[0].operatorEnteredPoint;
        y2=points[1].operatorEnteredPoint;
        //   x-x1
        //y=------- * (y2-y1) + y1
        //   x2-x1
        y=(x-x1)*(y2-y1)/(x2-x1) + y1;
        return y;
    }

    if (x > points[points.size()-1].time_min)
    {
        x1=points[points.size()-2].time_min;
        x2=points[points.size()-1].time_min;
        y1=points[points.size()-2].operatorEnteredPoint;
        y2=points[points.size()-1].operatorEnteredPoint;
        //   x-x1
        //y=------- * (y2-y1) + y1
        //   x2-x1
        y=(x-x1)*(y2-y1)/(x2-x1) + y1;
        return y;
    }

    for (int i=0;i<points.size()-1;++i)
    {
        if (x>points[i].time_min && x<=points[i+1].time_min)
        {
            x1=points[i].time_min;
            x2=points[i+1].time_min;
            y1=points[i].operatorEnteredPoint;
            y2=points[i+1].operatorEnteredPoint;
            //   x-x1
            //y=------- * (y2-y1) + y1
            //   x2-x1
            y=(x-x1)*(y2-y1)/(x2-x1) + y1;
            return y;
        }

    }

    return 0.0;
}
//=======================================================================================
void MainWindow::CreateTestReport()
{

    QDateTime dtReport=QDateTime::currentDateTime();
    QString fileName=qApp->applicationDirPath()+"/reports/"+dtReport.toString("yyyy.MM.dd_hh.mm.ss")+".odt";


  QTextDocument *document = new QTextDocument();




  //QFont font;
  //font.setFamily("Helvetica");//("Swiss");//("Times New Roman");//("Times");

  QTextCursor cursor(document);

//  QString date = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

//  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
//  cursor.insertText(date + '\n', charFormat(11, false));//12

//  cursor.insertBlock();
//  setCurrentBlockAlignment(cursor, Qt::AlignCenter);
//  cursor.insertText(company, charFormat(11, false));//12

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignCenter);
  cursor.insertText("Звіт згідно ДСТУ EN ISO 1182:2022", charFormat(16, true));

  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  cursor.insertText("Звіт сформовано: " + dtReport.toString("yyyy.MM.dd hh:mm:ss") , charFormat(14, true));//12
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n


  cursor.insertText("Умова закінчення: " + testStopReason.replace("ЗАВЕРШЕНО...","") , charFormat(14, true));//12  //просто уберем из строки префикс "ЗАВЕРШЕНО..."
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

/*
  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  cursor.insertText("Заміри температури стінок пічі:", charFormat(12, true));//12
*/


  cursor.movePosition(QTextCursor::End);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  //get 1st graph

  wGraphic_1->replot();
  wGraphic_1->savePng(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_1.png",640,480);
  QImage img_1(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_1.png");


  //graphic
  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();

  setCurrentBlockAlignment(cursor, Qt::AlignCenter);


  //adding image to document

  //cursor.insertImage(plot_movement_1);

  document->addResource(QTextDocument::ImageResource, QUrl("plot_img_1.png"), img_1);
  QTextImageFormat imageFormat_1;
  imageFormat_1.setQuality(100);
  imageFormat_1.setName("plot_img_1.png");
  cursor.insertImage(imageFormat_1);
  cursor.movePosition(QTextCursor::End);






  cursor.insertText(QObject::tr("\n\n"), charFormat(12, true));//casey - line \n
  cursor.movePosition(QTextCursor::End);

  qDebug() << fileName;
  QTextDocumentWriter writer(fileName);
  if (!writer.write(document)) qDebug() << "errro write";

  qDebug() << writer.supportedDocumentFormats();

  //QDesktopServices::openUrl(QUrl(fileName+".odt"));

  //QTextEdit  *te = new QTextEdit();
  //te->setDocument(document);
  //te->showMaximized();

  delete document;





    QMessageBox::information(this,"Збереження в файл",QString("Результати збережені в файл\n")+fileName);
}

//=======================================================================================
void MainWindow::CreateTableReport()
{

    QDateTime dtReport=QDateTime::currentDateTime();
    QString fileName=qApp->applicationDirPath()+"/calibration_reports/"+dtReport.toString("yyyy.MM.dd_hh.mm.ss")+".odt";


  QTextDocument *document = new QTextDocument();




  //QFont font;
  //font.setFamily("Helvetica");//("Swiss");//("Times New Roman");//("Times");

  QTextCursor cursor(document);

//  QString date = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

//  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
//  cursor.insertText(date + '\n', charFormat(11, false));//12

//  cursor.insertBlock();
//  setCurrentBlockAlignment(cursor, Qt::AlignCenter);
//  cursor.insertText(company, charFormat(11, false));//12

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignCenter);
  cursor.insertText("Звіт по калібруванню печі згідно ДСТУ EN ISO 1182:2022", charFormat(16, true));

  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  cursor.insertText("Звіт сформовано: " + dtReport.toString("yyyy.MM.dd hh:mm:ss") , charFormat(14, true));//12
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n


//  cursor.insertText("Умова закінчення: " + stopCondition , charFormat(14, true));//12
//  cursor.movePosition(QTextCursor::End);
//  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  cursor.insertText("Заміри температури стінок пічі:", charFormat(12, true));//12
  cursor.movePosition(QTextCursor::End);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  //table
  cursor.movePosition(QTextCursor::End);



  //QTextTable *table=
  cursor.insertTable(4,4);

  //table->mergeCells(0,0,2,1);
  //table->mergeCells(2,1,1,3);


  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText("Вертикальна вісь", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  //setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  //cursor.insertText("Рівень", charFormat(12, true));
  //cursor.movePosition(QTextCursor::NextCell);



  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText("Рівень а на висоті 30 мм.", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText("Рівень b на висоті 0 мм.", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText("Рівень c на висоті -30 мм.", charFormat(12, true));
  cursor.movePosition(QTextCursor::NextCell);

  //1(під кутом 0°)
  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText("1(під кутом 0°)");
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(11,'f',2));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(12,'f',2));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(13,'f',2));
  cursor.movePosition(QTextCursor::NextCell);

  //2(під кутом 120°)
  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText("2(під кутом 120°)");
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(21,'f',2));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(22,'f',2));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(23,'f',2));
  cursor.movePosition(QTextCursor::NextCell);

  //3(під кутом 240°)
  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText("3(під кутом 240°)");
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(31,'f',2));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(32,'f',2));
  cursor.movePosition(QTextCursor::NextCell);

  setCurrentBlockAlignment(cursor, Qt::AlignHCenter | Qt::AlignVCenter);
  cursor.insertText(QString::number(33,'f',2));
  //cursor.movePosition(QTextCursor::NextCell);







/*

  QString table;
  table+="<table>";
  table+="<tr>";
  table+="<td align=\"center\" valign=\"middle\" rowspan=\"2\"> Час </td>";
  table+="<td align=\"center\" valign=\"middle\" colspan=\"3\"> Сумарні значення</td>";
  table+="</tr>";



  table+="<tr>";
  table+=" <td align=\"center\" valign=\"middle\"><center>&nbsp</center></font></td>";
  table+=" <td align=\"center\" valign=\"middle\"><center>aaaaa</center></font></td>";
  table+=" <td align=\"center\" valign=\"middle\"><center>bbbbbb</center></font></td>";
  table+=" <td align=\"center\" valign=\"middle\"><center>ccccc</center></font></td>";
  table+="</tr>";

  table+="<tr>";
  table+=" <td align=\"center\" valign=\"middle\"><center>ttttttt</center></font></td>";
  table+=" <td align=\"center\" valign=\"middle\"><center>11.11</center></font></td>";
  table+=" <td align=\"center\" valign=\"middle\"><center>21.21</center></font></td>";
  table+=" <td align=\"center\" valign=\"middle\"><center>31.31</center></font></td>";
  table+="</tr>";




  table+="</table>";
  cursor.insertHtml(table);
*/

  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n





  double T1a=ui->lineEditT1a->text().toDouble();
  double T1b=ui->lineEditT1b->text().toDouble();
  double T1c=ui->lineEditT1c->text().toDouble();
  double T2a=ui->lineEditT2a->text().toDouble();
  double T2b=ui->lineEditT2b->text().toDouble();
  double T2c=ui->lineEditT2c->text().toDouble();
  double T3a=ui->lineEditT3a->text().toDouble();
  double T3b=ui->lineEditT3b->text().toDouble();
  double T3c=ui->lineEditT3c->text().toDouble();

  double Tavg=(T1a+T1b+T1c+T2a+T2b+T2c+T3a+T3b+T3c)/9.0;
  double Tavg_axis1=(T1a+T1b+T1c)/3.0;
  double Tavg_axis2=(T2a+T2b+T2c)/3.0;
  double Tavg_axis3=(T3a+T3b+T3c)/3.0;
  double Tdev_axis1=100.0*fabs(Tavg-Tavg_axis1)/Tavg;
  double Tdev_axis2=100.0*fabs(Tavg-Tavg_axis2)/Tavg;
  double Tdev_axis3=100.0*fabs(Tavg-Tavg_axis3)/Tavg;
  double Tavg_dev_axis=(Tdev_axis1+Tdev_axis2+Tdev_axis3)/3.0;  //formula 8
  double Tavg_levela=(T1a+T2a+T3a)/3.0;
  double Tavg_levelb=(T1b+T2b+T3b)/3.0;
  double Tavg_levelc=(T1c+T2c+T3c)/3.0;
  double Tdev_levela=100.0*fabs((Tavg-Tavg_levela)/Tavg);
  double Tdev_levelb=100.0*fabs((Tavg-Tavg_levelb)/Tavg);
  double Tdev_levelc=100.0*fabs((Tavg-Tavg_levelc)/Tavg);
  double Tavg_dev_level=(Tdev_levela+Tdev_levelb+Tdev_levelc)/3.0;  //formula 15



  QString tableRes;
  tableRes+="T<sub>avg</sub> = (T<sub>1a</sub>+T<sub>1b</sub>+T<sub>1c</sub>+T<sub>2a</sub>+T<sub>2b</sub>+T<sub>2c</sub>+T<sub>3a</sub>+T<sub>3b</sub>+T<sub>3c</sub>) / 9 = "+QString::number(Tavg,'f',2) + "<br>";
  tableRes+="T<sub>avg,axis1</sub> = (T<sub>1a</sub>+T<sub>1b</sub>+T<sub>1c</sub>) / 3 = "+QString::number(Tavg_axis1,'f',2) + "<br>";
  tableRes+="T<sub>avg,axis2</sub> = (T<sub>2a</sub>+T<sub>2b</sub>+T<sub>2c</sub>) / 3 = "+QString::number(Tavg_axis2,'f',2) + "<br>";
  tableRes+="T<sub>avg,axis3</sub> = (T<sub>3a</sub>+T<sub>3b</sub>+T<sub>3c</sub>) / 3 = "+QString::number(Tavg_axis3,'f',2) + "<br>";
  tableRes+="T<sub>dev,axis1</sub> = 100 * |T<sub>avg</sub>-T<sub>avg,axis1</sub>| / T<sub>avg</sub> = "+QString::number(Tdev_axis1,'f',2) + "<br>";
  tableRes+="T<sub>dev,axis2</sub> = 100 * |T<sub>avg</sub>-T<sub>avg,axis2</sub>| / T<sub>avg</sub> = "+QString::number(Tdev_axis2,'f',2) + "<br>";
  tableRes+="T<sub>dev,axis3</sub> = 100 * |T<sub>avg</sub>-T<sub>avg,axis3</sub>| / T<sub>avg</sub> = "+QString::number(Tdev_axis3,'f',2) + "<br>";
  tableRes+="T<sub>avg,dev,axis</sub> = (T<sub>dev,axis1</sub>+T<sub>dev,axis2</sub>+T<sub>dev,axis3</sub>) / 3 = "+QString::number(Tavg_dev_axis,'f',2) + "     (має бути меншим за 0.5 %) " + "<br>";

  tableRes+="T<sub>avg,levela</sub> = (T<sub>1a</sub>+T<sub>2a</sub>+T<sub>3a</sub>) / 3 = "+QString::number(Tavg_levela,'f',2) + "<br>";
  tableRes+="T<sub>avg,levelb</sub> = (T<sub>1b</sub>+T<sub>2b</sub>+T<sub>3b</sub>) / 3 = "+QString::number(Tavg_levelb,'f',2) + "<br>";
  tableRes+="T<sub>avg,levelc</sub> = (T<sub>1c</sub>+T<sub>2c</sub>+T<sub>3c</sub>) / 3 = "+QString::number(Tavg_levelc,'f',2) + "<br>";
  tableRes+="T<sub>dev,levela</sub> = 100 * |(T<sub>avg</sub>-T<sub>avg,levela</sub>)/T<sub>avg</sub>| = "+QString::number(Tdev_levela,'f',2) + "<br>";
  tableRes+="T<sub>dev,levelb</sub> = 100 * |(T<sub>avg</sub>-T<sub>avg,levelb</sub>)/T<sub>avg</sub>| = "+QString::number(Tdev_levelb,'f',2) + "<br>";
  tableRes+="T<sub>dev,levelc</sub> = 100 * |(T<sub>avg</sub>-T<sub>avg,levelc</sub>)/T<sub>avg</sub>| = "+QString::number(Tdev_levelc,'f',2) + "<br>";
  tableRes+="T<sub>avg,dev,level</sub> = (T<sub>dev,levela</sub>+T<sub>dev,levelb</sub>+T<sub>dev,levelc</sub>) / 3 = "+QString::number(Tavg_dev_level,'f',2) + "     (має бути меншим за 1.5 %) " + "<br>";


  //cursor.insertText(tableRes, charFormat(12, true));//12
  cursor.insertHtml(tableRes);
  cursor.movePosition(QTextCursor::End);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n
  /*




  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertText("Заміри нижнього рівня пластичного шару:", charFormat(12, true));//12
  cursor.movePosition(QTextCursor::End);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n



  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertText(QString("X (значення усадки по закінченню) = ") + QString::number(X_1,'f',1) + " мм", charFormat(12, true));


  QVector<OperatorPoint> all_points_1 = levelup_points_1 + leveldown_points_1;

  double max_difference_1=0.0;
  foreach (OperatorPoint point, all_points_1)
  {
      double difference=getLinearApproximatedValue(levelup_points_1, point.time_min) -
                        getLinearApproximatedValue(leveldown_points_1, point.time_min);

      if (max_difference_1 < difference) max_difference_1 = difference;
  }

  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n
  cursor.insertText(QString("Y (максимальна різниця верхніого і нижнього рівнів пл. шару) = ") + QString::number(max_difference_1,'f',1) + " мм", charFormat(12, true));
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  //graphic
  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();

  setCurrentBlockAlignment(cursor, Qt::AlignCenter);


  //adding image to document

  //cursor.insertImage(plot_movement_1);

  document->addResource(QTextDocument::ImageResource, QUrl("plot_movement_1.png"), plot_movement_1);
  QTextImageFormat imageFormat_1;
  imageFormat_1.setQuality(100);
  imageFormat_1.setName("plot_movement_1.png");
  cursor.insertImage(imageFormat_1);
  cursor.movePosition(QTextCursor::End);





  cursor.insertText(QObject::tr("\n\n"), charFormat(12, true));//casey - line \n
  cursor.movePosition(QTextCursor::End);


*/



  cursor.insertText(QObject::tr("\n\n"), charFormat(12, true));//casey - line \n
  cursor.movePosition(QTextCursor::End);

  qDebug() << fileName;
  QTextDocumentWriter writer(fileName);
  if (!writer.write(document)) qDebug() << "errro write";

  qDebug() << writer.supportedDocumentFormats();

  //QDesktopServices::openUrl(QUrl(fileName+".odt"));

  //QTextEdit  *te = new QTextEdit();
  //te->setDocument(document);
  //te->showMaximized();

  delete document;





    QMessageBox::information(&dialogTableResult,"Збереження в файл",QString("Результати збережені в файл\n")+fileName);
}
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
