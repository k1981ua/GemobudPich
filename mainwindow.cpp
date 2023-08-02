#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fmdlg.h"
#include "modbusreader.h"

#include <QPainter>
#include <QTextEdit>

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
    appDir.mkdir("png");

    //connect(ui->lineEditOperatorPoint_1,SIGNAL(textEdited(QString)),this,SLOT(ValueChanged(QString)));
    //connect(ui->lineEditOperatorPoint_2,SIGNAL(textEdited(QString)),this,SLOT(ValueChanged(QString)));

    double operatorInputMin=Y_MOVEMENT_RANGE_MIN;
    double operatorInputMax=Y_MOVEMENT_RANGE_MAX;

    temperature_1.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_1.ch");
    temperature_2.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_2.ch");
    temperature_3.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_3.ch");
    temperature_4.SetConfigFileName(qApp->applicationDirPath()+"/"+"temperature_4.ch");

    temperature_1.LoadChannelConfigFile();
    temperature_2.LoadChannelConfigFile();
    temperature_3.LoadChannelConfigFile();
    temperature_4.LoadChannelConfigFile();

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
    connect(&mbReader, &ModbusReader::readICP_7018_ch3,&temperature_3, &AnalogInputChannel::RawValueReaded);
    connect(&mbReader, &ModbusReader::readICP_7018_ch4,&temperature_4, &AnalogInputChannel::RawValueReaded);

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

    connect(ui->buttonExit,     SIGNAL(clicked()),this,SLOT(ButtonExit()));
    connect(ui->buttonConfig,   SIGNAL(clicked()),this,SLOT(ViewDialogConfig()));
    connect(ui->buttonReset,    SIGNAL(clicked()),this,SLOT(ButtonReset()));
    connect(ui->buttonStartStop,SIGNAL(clicked()),this,SLOT(ButtonStartStop()));
    connect(ui->buttonReports,  SIGNAL(clicked()),this,SLOT(ButtonReports()));


    connect(&timer200ms,SIGNAL(timeout()),this,SLOT(Timer200ms()));

    //connect(&timer3000ms,SIGNAL(timeout()),this,SLOT(Timer3000ms()));

    orig_palette=ui->lineEditValueTemperature_1->palette();


    //ГРАФИК 1
    // Инициализируем объект полотна для графика ...
    wGraphic_1 = new QCustomPlot();
    wGraphic_1->setBackground(QColor(221,221,221));//Qt::lightGray);
    wGraphic_1->axisRect()->setBackground(QColor(255,251,240));  //оригинал - слоновая кость - 255,251,240

    ui->verticalLayoutGraphic_1->addWidget(wGraphic_1);
    wGraphic_1->xAxis->setLabel("Час, хв.");
    wGraphic_1->xAxis->setRange(0,X_RANGETEST);
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
     penTemperature_1.setWidth(1);
     graphicTemperature_1->setPen(penTemperature_1); // Устанавливаем цвет графика
     graphicTemperature_1->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_1->setLineStyle(QCPGraph::lsLine);


     // Инициализируем график Temperature 2 и привязываем его к Осям
     graphicTemperature_2 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicTemperature_2);  // Устанавливаем график на полотно
     QPen penTemperature_2=graphicTemperature_2->pen();
     penTemperature_2.setColor(Qt::blue);
     penTemperature_2.setWidth(1);
     graphicTemperature_2->setPen(penTemperature_2); // Устанавливаем цвет графика
     graphicTemperature_2->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_2->setLineStyle(QCPGraph::lsLine);


     // Инициализируем график Temperature 3 и привязываем его к Осям
     graphicTemperature_3 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicTemperature_3);  // Устанавливаем график на полотно
     QPen penTemperature_3=graphicTemperature_3->pen();
     penTemperature_3.setColor(Qt::darkBlue);
     penTemperature_3.setWidth(1);
     graphicTemperature_3->setPen(penTemperature_3); // Устанавливаем цвет графика
     graphicTemperature_3->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_3->setLineStyle(QCPGraph::lsLine);


     // Инициализируем график Temperature 3 и привязываем его к Осям
     graphicTemperature_4 = new QCPGraph(wGraphic_1->xAxis, wGraphic_1->yAxis);
     wGraphic_1->addPlottable(graphicTemperature_4);  // Устанавливаем график на полотно
     QPen penTemperature_4=graphicTemperature_4->pen();
     penTemperature_4.setColor(Qt::magenta);
     penTemperature_4.setWidth(1);
     graphicTemperature_4->setPen(penTemperature_4); // Устанавливаем цвет графика
     graphicTemperature_4->setAntialiased(false);         // Отключаем сглаживание, по умолчанию включено
     graphicTemperature_4->setLineStyle(QCPGraph::lsLine);

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



      ButtonReset();
      ui->labelTemperature_1->setText(temperature_1.GetChName());
      ui->labelTemperature_2->setText(temperature_2.GetChName());
      ui->labelTemperature_3->setText(temperature_3.GetChName());
      ui->labelTemperature_4->setText(temperature_4.GetChName());


      wGraphic_1->yAxis->setLabel("Температура , гр.С");
 //     wGraphic_1->yAxis->setLabel(temperature_1.GetChName()+", "+temperature_1.GetEU());
 //     wGraphic_2->yAxis->setLabel(movement_2.GetChName()+", "+movement_2.GetEU());

      QFont font=wGraphic_1->xAxis->labelFont();
      font.setPointSize(12);
      wGraphic_1->xAxis->setLabelFont(font);
      wGraphic_1->yAxis->setLabelFont(font);
      wGraphic_1->yAxis2->setLabelFont(font);


      timer200ms.start(3000);

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

}
//=======================================================================================
void MainWindow::SaveIniFile()
{
    QSettings settings(iniFile,QSettings::IniFormat);
    settings.beginGroup("main");

    settings.setValue("comPort",comPort);

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
void MainWindow::Timer200ms()
{

    timer200ms.setInterval(2000);

    ui->labelTemperature_1->setText(temperature_1.GetChName());
    ui->labelTemperature_2->setText(temperature_2.GetChName());
    ui->labelTemperature_3->setText(temperature_3.GetChName());
    ui->labelTemperature_4->setText(temperature_4.GetChName());


    //wGraphic_1->yAxis->setLabel(movement_1.GetChName()+", "+movement_1.GetEU());
    //wGraphic_2->yAxis->setLabel(movement_2.GetChName()+", "+movement_2.GetEU());

    ui->lineEditValueTemperature_1->setText(temperature_1.GetValueString(2));
    ui->lineEditValueTemperature_2->setText(temperature_2.GetValueString(2));
    ui->lineEditValueTemperature_3->setText(temperature_3.GetValueString(2));
    ui->lineEditValueTemperature_4->setText(temperature_4.GetValueString(2));


    if (cmdButton==StartCmd)
    {
         ui->buttonStartStop->setIcon(QIcon(":/icons/player_stop.png"));
         ui->buttonStartStop->setText("Стоп");
         runningMode=ModeTest;
         //controlPoints.clear();
         infoText="ВИПРОБУВАННЯ";
         startTestDT=QDateTime::currentDateTime();
         startTestDT_str=startTestDT.toString("yyyy.MM.dd_hh.mm.ss");
         infoText+="\nПочаток:\n"+startTestDT.toString("hh:mm dd.MM.yy");

         //set "0" on start of test
         //movement_1.SetZeroOffset();
         //movement_2.SetZeroOffset();
         //infoText+="\nВідкр. реле...";

         //infoText+=QString("\nЧАС: 00:00:00");
         //ui->listWidgetInfo->clear();
         //ui->listWidgetInfo->addItem(QTime::currentTime().toString()+ QString(" Started Test Mode"));
         //ui->listWidgetInfo->addItem(QTime::currentTime().toString()+ QString(" Wait temperature:"));
         wGraphic_1->xAxis->setRange(0,X_RANGETEST);//xInterval);
         wGraphic_1->yAxis->setRange(Y_MOVEMENT_RANGE_MIN,Y_MOVEMENT_RANGE_MAX);
         //wGraphic->yAxis2->setRange(0,Y_TEMPERATURE_RANGE);
         //graphicMovement_1->clearData();
         //graphicMovement_2->clearData();
         graphicTemperature_1->clearData();
         graphicTemperature_2->clearData();
         graphicTemperature_3->clearData();
         graphicTemperature_4->clearData();




         startTestDT=QDateTime::currentDateTime();
         //startTestTemperature=value_grC_offseted;
         //sound_start->play();
         //QProcess::startDetached(QString("aplay ")+QApplication::applicationDirPath() + "/start.wav");
         cmdButton=NoCmd;
         ui->buttonReset->setEnabled(false);
         ui->buttonConfig->setEnabled(false);
         ui->buttonExit->setEnabled(false);


         //return;

    } //if (cmdButton==StartCmd)



    if (runningMode==ModeView)
    {
        //добавляем данные на график
        /*
        double seconds_from_start;
        seconds_from_start=startModeViewDT.msecsTo(QDateTime::currentDateTime())/1000.0;

        graphicPpm_pwm1->addData(seconds_from_start,co2_ch1.GetValue());
        graphicPpm_pwm2->addData(seconds_from_start,co2_ch2.GetValue());
        graphicPpm_pwm3->addData(seconds_from_start,co2_ch3.GetValue());
        graphicPressure_1->addData(seconds_from_start,pressure_1.GetValue());
        graphicPressure_2->addData(seconds_from_start,pressure_2.GetValue());
        graphicAirFlow_1->addData(seconds_from_start,airFlow_1.GetValue());
        graphicAirFlow_2->addData(seconds_from_start,airFlow_2.GetValue());
        graphicHeatFlow->addData(seconds_from_start,heatFlow.GetValue());
        graphicForce->addData(seconds_from_start,force.GetValue());

        //new in GemobudMaskTest3
        graphicTemperature_1->addData(seconds_from_start,temperature_1.GetValue());
        graphicTemperature_2->addData(seconds_from_start,temperature_2.GetValue());
        graphicPpm_O2->addData(seconds_from_start,o2_ch1.GetValue());
        */

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

        wGraphic->replot();           // Отрисовываем график

        */



    } //if (runningMode==ModeView)



    if (runningMode==ModeTest)
    {
        //добавляем данные на график
        double seconds_from_start;

        seconds_from_start=startTestDT.msecsTo(QDateTime::currentDateTime())/1000.0;
        double minutes_from_start=seconds_from_start/60.0;

        //graphicMovement_1->addData(minutes_from_start,movement_1.GetValue());
        //graphicMovement_2->addData(minutes_from_start,movement_2.GetValue());

        graphicTemperature_1->addData(minutes_from_start,temperature_1.GetValue());
        graphicTemperature_2->addData(minutes_from_start,temperature_2.GetValue());
        graphicTemperature_3->addData(minutes_from_start,temperature_3.GetValue());
        graphicTemperature_4->addData(minutes_from_start,temperature_4.GetValue());


        //автоизменение шкалы по Х
        if (minutes_from_start >= wGraphic_1->xAxis->range().upper)
        {
            double newMaxRange=wGraphic_1->xAxis->range().upper + 10;
            wGraphic_1->xAxis->setRange(0,newMaxRange);

        }



        wGraphic_1->replot();           // Отрисовываем график


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

        ui->labelInfo->setText(infoText+QString("\nЧАС: ") + testRunningStr);




        //если достигнуто условие окончания
        //для режима Test (Испытание) - нажатие кнопки СТОП
        //для режима Test (Испытание) - резкое падение давления на 20% за 1сек.


        if (cmdButton==StopCmd)
        {

            ui->buttonStartStop->setIcon(QIcon(":/icons/player_play.png"));
            ui->buttonStartStop->setText("Старт");
            runningMode=ModeTestStopped;
            //sound_stop->play();
            ui->labelInfo->setText(ui->labelInfo->text()+"\nТЕСТ ЗАВЕРШЕНО\n");
            //QProcess::startDetached(QString("aplay ")+QApplication::applicationDirPath() + "/stop.wav");






            //get 1st graph

            wGraphic_1->replot();
            wGraphic_1->savePng(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_1.png",640,480);
            QImage img_1(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_1.png");

            //get 2nd graph
            //wGraphic_2->replot();
            //wGraphic_2->savePng(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_2.png",640,480);
            //QImage img_2(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_2.png");


            //img_2 = img_2.convertToFormat(QImage::Format_Grayscale16 ,Qt::AutoColor | Qt::ThresholdDither);
            //img_2.save(qApp->applicationDirPath()+"/png/"+startTestDT_str+"_2_ttt.png");



            createReport(qApp->applicationDirPath()+"/reports/"+startTestDT_str, startTestDT.toString("dd.MM.yyyy hh:mm:ss") ,
                         testRunningStr, img_1, img_1, temperature_1.GetValue(), temperature_2.GetValue(), levelUpPoints_1, levelDownPoints_1, levelUpPoints_2, levelDownPoints_2);

            ui->buttonReset->setEnabled(true);
            ui->buttonConfig->setEnabled(true);
            ui->buttonExit->setEnabled(true);


        }





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



    wGraphic_1->xAxis->setRange(0,X_RANGETEST);//xInterval);
    wGraphic_1->yAxis->setRange(Y_MOVEMENT_RANGE_MIN,Y_MOVEMENT_RANGE_MAX);
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
    if (runningMode==ModeView || runningMode==ModeTestStopped)
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
    FMDlg dlg(tr("Reports"), this);



    //dlg.setLocalPath("reports");
    //dlg.setFilter(QStringList() << "*.odf");
    //dlg.enablePreviewButton();

    dlg.launch();

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
                              const QImage &plot_movement_2,
                              const double X_1,
                              const double X_2,
                              const QVector<OperatorPoint> levelup_points_1,
                              const QVector<OperatorPoint> leveldown_points_1,
                              const QVector<OperatorPoint> levelup_points_2,
                              const QVector<OperatorPoint> leveldown_points_2)
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



  //TEST 2

  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignHCenter);
  cursor.insertText("Тест 2", charFormat(14, true));//12
  cursor.movePosition(QTextCursor::End);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n


  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();
  setCurrentBlockAlignment(cursor, Qt::AlignLeft);
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n

  cursor.insertText("Заміри верхнього рівня пластичного шару:", charFormat(12, true));//12
  cursor.movePosition(QTextCursor::End);
  //cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n
  //table
  cursor.movePosition(QTextCursor::End);
  cursor.insertTable(levelup_points_2.size()+1,4);

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


  foreach(OperatorPoint pnt, levelup_points_2)
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
  cursor.insertTable(leveldown_points_2.size()+1,4);

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


  foreach(OperatorPoint pnt, leveldown_points_2)
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

  cursor.insertText(QString("X (значення усадки по закінченню) = ") + QString::number(X_2,'f',1) + " мм", charFormat(12, true));


  QVector<OperatorPoint> all_points_2 = levelup_points_2 + leveldown_points_2;

  double max_difference_2=0.0;
  foreach (OperatorPoint point, all_points_2)
  {
      double difference=getLinearApproximatedValue(levelup_points_2, point.time_min) -
                        getLinearApproximatedValue(leveldown_points_2, point.time_min);

      if (max_difference_2 < difference) max_difference_2 = difference;
  }

  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n
  cursor.insertText(QString("Y (максимальна різниця верхніого і нижнього рівнів пл. шару) = ") + QString::number(max_difference_2,'f',1) + " мм", charFormat(12, true));
  cursor.insertText(QObject::tr("\n"), charFormat(12, true));//casey - line \n


  //graphic
  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();

  setCurrentBlockAlignment(cursor, Qt::AlignCenter);


  //adding image to document

  //cursor.insertImage(plot_movement_1);

  document->addResource(QTextDocument::ImageResource, QUrl("plot_movement_2.png"), plot_movement_2);
  QTextImageFormat imageFormat_2;
  imageFormat_2.setQuality(100);
  imageFormat_2.setName("plot_movement_2.png");
  cursor.insertImage(imageFormat_2);
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

//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================