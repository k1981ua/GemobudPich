#include "modbusreader.h"
#include <QProcess>
#include <QMessageBox>

//=======================================================================
bool ModbusReader::Init(QString serialport, int registratoraddress)
{
    if (serialport!="") serialPort=serialport;
    stopping=false;
    return true;
}
//======================================================================
void ModbusReader::SetComPort(QString serPort)
{
    if (serPort!="") serialPort=serPort;
}
//=======================================================================

//=======================================================================
void ModbusReader::StartPoll()
{
    stopping=false;
    qDebug() << "starting modbus poll";
    start();
}
//=======================================================================
void ModbusReader::StopPoll()
{
    stopping=true;
    wait();
}
//=======================================================================

//=======================================================================
bool ModbusReader::Connect()
{

    qDebug() << "connecting to " << serialPort << " ...";
#ifdef __linux__
    QProcess::execute(QString("sudo chmod 777")+" /dev/"+serialPort);
    ctx = modbus_new_rtu((QString("/dev/")+serialPort).toStdString().c_str(), BAUD_RATE, PARITY, BYTESIZE, STOPBITS);
#else
    ctx = modbus_new_rtu(serialPort.toStdString().c_str(), BAUD_RATE, PARITY, BYTESIZE, STOPBITS);
#endif
    if (ctx == NULL)
    {
        //fprintf(stderr, "Unable to allocate libmodbus context\n");
        qDebug() << "Unable to allocate libmodbus context comPort=" << serialPort ;
        return false;
    }
    //modbus_set_debug(ctx, TRUE);
    //modbus_set_error_recovery(ctx, (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL) );
    //modbus_set_error_recovery(ctx, MODBUS_ERROR_RECOVERY_PROTOCOL);

    modbus_set_slave(ctx, TRM251_1_ADDRESS);

    //modbus_get_response_timeout(ctx, &sec_to, &usec_to);
    //modbus_set_response_timeout(ctx, 0, 100000);
    modbus_set_response_timeout(ctx, 0, 500000);  //for new registrator must be no less tnen 300 ms

    if (modbus_connect(ctx) == -1)
    {
        //fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        qDebug() << "Connection failed: " << serialPort;
        modbus_free(ctx);
        ctx=NULL;
        return false;
    }
    modbus_flush(ctx);
    return true;
}
//=======================================================================
//=======================================================================
void ModbusReader::VoltageSet(float newVoltage)
{

    voltageSetCommand = VoltageSetCommand::VOLTSET_SET_CMD;
    voltageToSet=newVoltage;

    //GemobudMaskTest3 - в єтой конфигурации частотников по модбасу  нет
    //QMessageBox::information(NULL,"Помилка конфігурації","В цій конфігурації немає цього частотного перетворювача!");
}
//==========================================================================
void ModbusReader::run()
{

    int errorsTRM251_1=0;
    int errorsTRM251_2=0;
    //int errorsOwen_AI=0;
    int errorsOWEN_2AI=0;
    int errorsICP_7018=0;

    int readTrm251_1_res;
    int readTrm251_2_res;
    //int readOWEN_AI_res;
    int readOWEN_2AI_res;
    int readICP_7018_res;

    msleep(1000);
    if (!Connect()) qDebug() << "modbus connect failed";

    qDebug() << "ModbusReader::run()";

    for(;;)
    {

/*
        static double value=0.0;
        if (value < 19.5f)
        {
            value+=1.0;
        }
        else
        {
            value+=10.0f;
        }

        if (value>401.0f) value=0.0f;

        qDebug() << "modbus run :" << value;
        //if (value>100.0 && value<200.0)
        //{
        //    emit readAKON_AI_ch1(qQNaN(), ValueStatus::ValueOk);
        //}
        //else
        //{
            emit readAKON_AI_ch1(value, ValueStatus::ValueOk);
        //}
        msleep(200);
*/



 //TRM251 -read only temperature

/*
        modbus_set_slave(ctx, TRM251_1_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика

        readTrm251_1_res = modbus_read_registers(ctx, TRM251_START_REGISTER, TRM251_NUM_REGISTERS , regTrm251_1);
        //msleep(500);
        //readTrm251_1_res2 = modbus_read_registers(ctx, TRM251_START_REGISTER_2, TRM251_NUM_REGISTERS_2 , &regTrm251_1[TRM251_START_REGISTER_2]);

        qDebug() << "readTrm251_1_res=" << readTrm251_1_res;

        if (readTrm251_1_res==TRM251_NUM_REGISTERS) // && readTrm251_1_res2==TRM251_NUM_REGISTERS_2)
        {
            numReads++;
            //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

            float temperatureValue;
            //int stepValue;

            temperatureValue=modbus_get_float_dcba(&regTrm251_1[0x04]);
            //stepValue=regTrm251_1[0x10];

            if (!qIsNaN(temperatureValue))
            {
                emit readTrm251_1(temperatureValue, ValueStatus::ValueOk);
            }
            else
            {
                emit readTrm251_1(qQNaN(),  ValueStatus::ValueErrorRead);
            }


            errorsTRM251_1=0;
        }
        else
        {
            //qDebug() << "ERROR READ MODBUS: Address:" << registratorAddress << "StartRegister" << REGISTRATOR_START_REGISTER + 30001 <<
            //            "NumRegisters:" << REGISTRATOR_NUM_REGISTERS << "Port:" << serialPort << "(" << BAUD_RATE << "," <<
            //            BYTESIZE << "," << PARITY << "," << STOPBITS << ")";
            numErrorReads++;
            errorsTRM251_1++;
            //emit valueReaded(0,false,0,false);
            if (errorsTRM251_1>5)
            {
                emit readTrm251_1(qQNaN(), ValueStatus::ValueErrorRead);
                msleep(200);
                Release();
                msleep(200);
                Connect();
                msleep(200);
                errorsTRM251_1=0;
                qDebug() << "COM PORT reconnect";
            }
            //read errorppm
        }

*/



/*
        //TRM251 # 2 -read only temperature
        msleep(200);

               modbus_set_slave(ctx, TRM251_2_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика

               readTrm251_2_res = modbus_read_registers(ctx, TRM251_START_REGISTER, TRM251_NUM_REGISTERS , regTrm251_2);
               //msleep(500);
               //readTrm251_1_res2 = modbus_read_registers(ctx, TRM251_START_REGISTER_2, TRM251_NUM_REGISTERS_2 , &regTrm251_1[TRM251_START_REGISTER_2]);

               //qDebug() << "readTrm251_2_res=" << readTrm251_2_res;

               if (readTrm251_2_res==TRM251_NUM_REGISTERS) // && readTrm251_1_res2==TRM251_NUM_REGISTERS_2)
               {
                   //numReads++;
                   //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

                   float temperatureValue;
                   //int stepValue;

                   temperatureValue=modbus_get_float_dcba(&regTrm251_2[0x04]);
                   //stepValue=regTrm251_1[0x10];

                   //qDebug() << "readTrm251_2_res float=" << temperatureValue;

                   if (!qIsNaN(temperatureValue))
                   {
                       emit readTrm251_2(temperatureValue, ValueStatus::ValueOk);
                   }
                   else
                   {
                       emit readTrm251_2(qQNaN(),  ValueStatus::ValueErrorRead);
                   }


                   errorsTRM251_2=0;
               }
               else
               {
                   //qDebug() << "ERROR READ MODBUS: Address:" << registratorAddress << "StartRegister" << REGISTRATOR_START_REGISTER + 30001 <<
                   //            "NumRegisters:" << REGISTRATOR_NUM_REGISTERS << "Port:" << serialPort << "(" << BAUD_RATE << "," <<
                   //            BYTESIZE << "," << PARITY << "," << STOPBITS << ")";

                   errorsTRM251_2++;
                   //emit valueReaded(0,false,0,false);
                   if (errorsTRM251_2>5)
                   {
                       emit readTrm251_2(qQNaN(), ValueStatus::ValueErrorRead);
                       //msleep(200);
                       //Release();
                       //msleep(200);
                       //Connect();
                       msleep(200);
                       errorsTRM251_2=0;
                       //qDebug() << "COM PORT reconnect";
                   }
                   //read errorppm
               }

*/

/*
        //TRM200

               msleep(1000);
               //modbus_flush(ctx);
               modbus_set_slave(ctx, TRM200_1_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика
               readTrm200_1_res = modbus_read_registers(ctx, TRM200_START_REGISTER, TRM200_NUM_REGISTERS , regTrm200_1);
               //modbus_flush(ctx);


               qDebug() << "readTrm200_1_res=" << readTrm200_1_res;

               if (readTrm200_1_res==TRM200_NUM_REGISTERS)
               {
                   numReads++;
                   //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

                   float temperatureValue1;
                   float temperatureValue2;

                   temperatureValue1=modbus_get_float_dcba(&regTrm200_1[0x00]);
                   temperatureValue2=modbus_get_float_dcba(&regTrm200_1[0x02]);

                   //if (!qIsNaN(temperatureValue1))
                   //{
                       emit readTrm200_1(temperatureValue1, temperatureValue2, ValueStatus::ValueOk);
                   //}
                   //else
                   //{
                   //    emit readTrm200_1(qQNaN(), qQNaN(), ValueStatus::ValueErrorRead);
                   //}

                   //errors=0;
               }
               else
               {
                   emit readTrm200_1(qQNaN(), qQNaN(), ValueStatus::ValueErrorRead);
               }

*/





/*
        //OWEN AI - 8 channels AI
        msleep(200);
        //modbus_flush(ctx);
        modbus_set_slave(ctx, OWEN_AI_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика
        readOWEN_AI_res = modbus_read_registers(ctx, OWEN_AI_START_REGISTER, OWEN_AI_NUM_REGISTERS , regOWEN_AI);
        //modbus_flush(ctx);


        if (readOWEN_AI_res==OWEN_AI_NUM_REGISTERS)
        {
            //numReads++;
            //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

            float readed_owen_channels[8]={0,0,0,0,0,0,0,0};

            for(int i=0;i<8;++i)
            {
                //float inverse
                readed_owen_channels[i]=modbus_get_float_dcba(&regOWEN_AI[i*3]);
            }

            //emit valueReaded(readed_temperature1, readed_temperature1_ok, readed_temperature,readed_temperature_ok);
            if (!qIsNaN(readed_owen_channels[0])) emit readOWEN_AI_ch1(readed_owen_channels[0], ValueStatus::ValueOk);
            if (!qIsNaN(readed_owen_channels[1])) emit readOWEN_AI_ch2(readed_owen_channels[1], ValueStatus::ValueOk);
            //if (!qIsNaN(readed_owen_channels[2])) emit readOWEN_AI_ch3(readed_owen_channels[2], ValueStatus::ValueOk);
            //if (!qIsNaN(readed_owen_channels[3])) emit readOWEN_AI_ch4(readed_owen_channels[3], ValueStatus::ValueOk);
            //if (!qIsNaN(readed_owen_channels[4])) emit readOWEN_AI_ch5(readed_owen_channels[4], ValueStatus::ValueOk);
            //if (!qIsNaN(readed_owen_channels[5])) emit readOWEN_AI_ch6(readed_owen_channels[5], ValueStatus::ValueOk);
            //if (!qIsNaN(readed_owen_channels[6])) emit readOWEN_AI_ch7(readed_owen_channels[6], ValueStatus::ValueOk);
            //if (!qIsNaN(readed_owen_channels[7])) emit readOWEN_AI_ch8(readed_owen_channels[7], ValueStatus::ValueOk);
            errorsOwen_AI=0;
        }
        else
        {
            //numErrorReads++;
            errorsOwen_AI++;
            //emit valueReaded(0,false,0,false);
            if (errorsOwen_AI>5)
            {
                emit readOWEN_AI_ch1(qQNaN(), ValueStatus::ValueErrorRead);
                emit readOWEN_AI_ch2(qQNaN(), ValueStatus::ValueErrorRead);
                //msleep(200);
                //Release();
                //msleep(200);
                //Connect();
                msleep(200);
                errorsOwen_AI=0;
            }
            //read errorppm
        }
*/
/*

        //OVEN MVA8

        msleep(20);

        modbus_set_slave(ctx, OWEN_MVA8_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика
        readOWEN_MVA8_res = modbus_read_registers(ctx, OWEN_MVA8_START_REGISTER, OWEN_MVA8_NUM_REGISTERS , regOWEN_MVA8);
        //modbus_flush(ctx);

        msleep(10);

        if (readOWEN_MVA8_res==OWEN_MVA8_NUM_REGISTERS)
        {
            numReads++;
            //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

            float readed_owen_mva8_channels[8]={0,0,0,0,0,0,0,0};

            for(int i=0;i<3;++i) //читаем только 3 канала
            {
                //float inverse
                readed_owen_mva8_channels[i]=modbus_get_float_dcba(&regOWEN_MVA8[i*6+4]);
            }

            if (!qIsNaN(readed_owen_mva8_channels[0])) emit readOWEN_MVA8_ch1(readed_owen_mva8_channels[0], ValueStatus::ValueOk);
            if (!qIsNaN(readed_owen_mva8_channels[1])) emit readOWEN_MVA8_ch2(readed_owen_mva8_channels[1], ValueStatus::ValueOk);
            if (!qIsNaN(readed_owen_mva8_channels[2])) emit readOWEN_MVA8_ch3(readed_owen_mva8_channels[2], ValueStatus::ValueOk);

            errors=0;
        }

*/

/*
        //AKON AI  -  4AI
        //modbus_flush(ctx);
        modbus_set_slave(ctx, AKON_AI_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика
        readAKON_AI_res = modbus_read_registers(ctx, AKON_AI_START_REGISTER, AKON_AI_NUM_REGISTERS , regAKON_AI);
        //modbus_flush(ctx);


        if (readAKON_AI_res==AKON_AI_NUM_REGISTERS)
        {
            numReads++;
            //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

            float readed_akon_channels[4]={0,0,0,0};

            for(int i=0;i<4;++i)
            {
                //float inverse
                readed_akon_channels[i]=modbus_get_float_dcba(&regAKON_AI[i*2]);
            }

            //emit valueReaded(readed_temperature1, readed_temperature1_ok, readed_temperature,readed_temperature_ok);
            if (!qIsNaN(readed_akon_channels[0])) emit readAKON_AI_ch1(readed_akon_channels[0], ValueStatus::ValueOk);
            if (!qIsNaN(readed_akon_channels[1])) emit readAKON_AI_ch2(readed_akon_channels[1], ValueStatus::ValueOk);
            if (!qIsNaN(readed_akon_channels[2])) emit readAKON_AI_ch3(readed_akon_channels[2], ValueStatus::ValueOk);
            if (!qIsNaN(readed_akon_channels[3])) emit readAKON_AI_ch4(readed_akon_channels[3], ValueStatus::ValueOk);
            errors=0;
        }
        else
        {
            numErrorReads++;
            errors++;
            //emit valueReaded(0,false,0,false);
            if (errors>5)
            {
                msleep(200);
                Release();
                msleep(200);
                Connect();
                msleep(200);
                errors=0;
            }
            //read error
        }
*/



//ICP DAS 7018 Simulate
/*
        emit readICP_7018_ch1(700.0+(qrand()%1000) / 100.0, ValueStatus::ValueOk);
        emit readICP_7018_ch2(750.0+qrand()%5, ValueStatus::ValueOk);
        emit readICP_7018_ch3(600.0+qrand()%50, ValueStatus::ValueOk);
        emit readICP_7018_ch4(650.0+qrand()%100, ValueStatus::ValueOk);
        emit readICP_7018_ch5(707.0+qrand()%100 / 10.0, ValueStatus::ValueOk);
        emit readICP_7018_ch6(757.0+qrand()%10 / 10.0, ValueStatus::ValueOk);
*/
        //ICP DAS 7018

        msleep(20);

        modbus_set_slave(ctx, ICP_7018_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика
        readICP_7018_res = modbus_read_registers(ctx, ICP_7018_START_REGISTER, ICP_7018_NUM_REGISTERS , regICP_7018);
        //modbus_flush(ctx);

        msleep(10);

        if (readICP_7018_res==ICP_7018_NUM_REGISTERS)
        {
            numReads++;
            //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

            float readed_icp_7018_channels[8]={0,0,0,0,0,0,0,0};

            for(int i=0;i<6;++i) //читаем только 4 канала
            {
                readed_icp_7018_channels[i]=regICP_7018[i]/10.0;   //температура, десятичная точка на один разряд
            }

            if (!qIsNaN(readed_icp_7018_channels[0])) emit readICP_7018_ch1(readed_icp_7018_channels[0], ValueStatus::ValueOk);
            if (!qIsNaN(readed_icp_7018_channels[1])) emit readICP_7018_ch2(readed_icp_7018_channels[1], ValueStatus::ValueOk);
            if (!qIsNaN(readed_icp_7018_channels[2])) emit readICP_7018_ch3(readed_icp_7018_channels[2], ValueStatus::ValueOk);
            if (!qIsNaN(readed_icp_7018_channels[3])) emit readICP_7018_ch4(readed_icp_7018_channels[3], ValueStatus::ValueOk);
            if (!qIsNaN(readed_icp_7018_channels[4])) emit readICP_7018_ch5(readed_icp_7018_channels[4], ValueStatus::ValueOk);
            if (!qIsNaN(readed_icp_7018_channels[5])) emit readICP_7018_ch6(readed_icp_7018_channels[5], ValueStatus::ValueOk);
            errorsICP_7018=0;
        }
        else
        {
            //numErrorReads++;
            errorsICP_7018++;
            //emit valueReaded(0,false,0,false);
            if (errorsICP_7018>5)
            {
                emit readICP_7018_ch1(qQNaN(), ValueStatus::ValueErrorRead);
                emit readICP_7018_ch2(qQNaN(), ValueStatus::ValueErrorRead);
                emit readICP_7018_ch3(qQNaN(), ValueStatus::ValueErrorRead);
                emit readICP_7018_ch4(qQNaN(), ValueStatus::ValueErrorRead);
                emit readICP_7018_ch5(qQNaN(), ValueStatus::ValueErrorRead);
                emit readICP_7018_ch6(qQNaN(), ValueStatus::ValueErrorRead);
                //msleep(200);
                //Release();
                //msleep(200);
                //Connect();
                msleep(200);
                errorsICP_7018=0;
            }
            //read errorppm
        }



/*

        //OWEN_2AI   ОВЕН 2-канальній МВ-110-2А

        msleep(200);

        modbus_set_slave(ctx, OWEN_2AI_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика
        readOWEN_2AI_res = modbus_read_registers(ctx, OWEN_2AI_START_REGISTER, OWEN_2AI_NUM_REGISTERS , regOWEN_2AI);
        //modbus_flush(ctx);

        //msleep(10);

        qDebug() << "owen res=" << readOWEN_2AI_res;

        if (readOWEN_2AI_res==OWEN_2AI_NUM_REGISTERS)
        {
            numReads++;
            //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

            float readed_owen_2ai_channels[2]={0,0};

            for(int i=0;i<2;++i) //читаем только 2 канала
            {
                //float inverse
                //readed_owen_2ai_channels[i]=modbus_get_float_dcba(&regOWEN_2AI[i*6+4]);
                readed_owen_2ai_channels[i]=modbus_get_float_dcba(&regOWEN_2AI[i*6+4]);
                //float
                //readed_owen_2ai_channels[i]=modbus_get_float_abcd(&regOWEN_2AI[i*6+4]);
                qDebug() << "owen " << i << " ch (abcd)=" << modbus_get_float_abcd(&regOWEN_2AI[i*6+4]);
                qDebug() << "owen " << i << " ch (badc)=" << modbus_get_float_badc(&regOWEN_2AI[i*6+4]);
                qDebug() << "owen " << i << " ch (cdab)=" << modbus_get_float_cdab(&regOWEN_2AI[i*6+4]);
                qDebug() << "owen " << i << " ch (dcba)=" << modbus_get_float_dcba(&regOWEN_2AI[i*6+4]);
            }

            if (!qIsNaN(readed_owen_2ai_channels[0])) emit readOWEN_2AI_ch1(readed_owen_2ai_channels[0], ValueStatus::ValueOk);
            if (!qIsNaN(readed_owen_2ai_channels[1])) emit readOWEN_2AI_ch2(readed_owen_2ai_channels[1], ValueStatus::ValueOk);

            errorsOWEN_2AI=0;
        }
        else
        {
            //numErrorReads++;
            errorsOWEN_2AI++;
            //emit valueReaded(0,false,0,false);
            if (errorsOWEN_2AI>5)
            {
                emit readOWEN_2AI_ch1(qQNaN(), ValueStatus::ValueErrorRead);
                emit readOWEN_2AI_ch2(qQNaN(), ValueStatus::ValueErrorRead);
                //msleep(200);
                //Release();
                //msleep(200);
                //Connect();
                msleep(200);
                errorsOWEN_2AI=0;
            }
            //read errorppm
        }
*/


/*
        //OWEN_2AI   ОВЕН 2-канальній МВ-110-2А 4-20mA O2 sensor

        msleep(5);

        modbus_set_slave(ctx, OWEN_2AI_4_20_ADDRESS); //переключаем адрес слєйва на чтение с тензодатчика
        readOWEN_2AI_4_20_res = modbus_read_registers(ctx, OWEN_2AI_4_20_START_REGISTER, OWEN_2AI_4_20_NUM_REGISTERS , regOWEN_2AI_4_20);
        //modbus_flush(ctx);

        //msleep(10);

        if (readOWEN_2AI_4_20_res==OWEN_2AI_4_20_NUM_REGISTERS)
        {
            numReads++;
            //printf("Data received is : %d %d \n\n\n",reg[0],reg[1]);

            float readed_owen_2ai_4_20_channels[2]={0,0};

            for(int i=0;i<2;++i) //читаем только 2 канала
            {
                //float inverse
                readed_owen_2ai_4_20_channels[i]=modbus_get_float_dcba(&regOWEN_2AI_4_20[i*6+4]);
            }

            if (!qIsNaN(readed_owen_2ai_4_20_channels[0])) emit readOWEN_2AI_4_20_ch1(readed_owen_2ai_4_20_channels[0], ValueStatus::ValueOk);
            if (!qIsNaN(readed_owen_2ai_4_20_channels[1])) emit readOWEN_2AI_4_20_ch2(readed_owen_2ai_4_20_channels[1], ValueStatus::ValueOk);

            errors=0;
        }
*/




/*
        for(int i=0;i<NUM_FREQ;++i)
        {
            //FREQUENCY START STOP
            if (freq_StartStopCommand_to_set[i]==Freq_StartStopCommand::FREQ_START_CMD || freq_StartStopCommand_to_set[i]==Freq_StartStopCommand::FREQ_STOP_CMD)
            {
                uint16_t value_to_write;
                if (freq_StartStopCommand_to_set[i]==Freq_StartStopCommand::FREQ_START_CMD)
                {
                    value_to_write=2; //Start command
                }
                else
                {
                    value_to_write=1; //Stop command
                }

                modbus_set_slave(ctx, FREQ_ADDRESSES[i]);
                int write_result=modbus_write_register(ctx,FREQ_STARTSTOP_REGISTER,value_to_write);
                //modbus_write_register return 1 if succesfull
                if (write_result==1)
                {
                    freq_StartStopCommand_to_set[i]=Freq_StartStopCommand::FREQ_NONE_CMD;// обязательно сбрасываем команду после успешной записи
                                                              //при неуспешной записи попытка будет повторена
                    emit freqStartStopCmdExecuted();
                }
            }

            //FREQUENCY SET
            if (freq_SetCommand_to_set[i]==Freq_SetCommand::FREQSET_SET_CMD)
            {
                uint16_t value_to_write=static_cast<uint16_t>(freq_to_set[i]*100);

                modbus_set_slave(ctx, FREQ_ADDRESSES[i]);
                int write_result=modbus_write_register(ctx,FREQ_FREQUENCY_REGISTER,value_to_write);
                //modbus_write_register return 1 if succesfull
                if (write_result==1)
                {
                    freq_SetCommand_to_set[i]=Freq_SetCommand::FREQSET_NONE_CMD;// обязательно сбрасываем команду после успешной записи
                                                              //при неуспешной записи попытка будет повторена
                    emit freqSetCmdExecuted();
                }
            }

        }

*/



/*
        //RELE

        //MU110 Command to DO
        for(int t=0;t<100;++t)
        {
            for(int i=0;i<MU110_NUM_RELES;++i)
            {
                if (mu110_DO_Command_to_set[i]==DO_Command::DO_ON_CMD)
                {
                    msleep(5); //Registrator needs small delay between polls, 1 ms is OK too

#ifdef NO_DO_RELE
                    mu110_DO_Command_to_set[i]=DO_Command::DO_NONE_CMD;// обязательно сбрасываем команду после успешной записи
                                                              //при неуспешной записи попытка будет повторена
                    emit doOnCmdExecuted();
#else


                    uint16_t reg[200];
                    reg[0]=MU110_DO_ON;
                    modbus_set_slave(ctx, MU110_DO_ADDRESSES[i]);
                    int write_result=modbus_write_registers(ctx,MU110_DO_REGISTERS[i],MU110_DO_NUM_REGISTERS,reg);
                    if (write_result==MU110_DO_NUM_REGISTERS)
                    {
                        mu110_DO_Command_to_set[i]=DO_Command::DO_NONE_CMD;// обязательно сбрасываем команду после успешной записи
                                                                  //при неуспешной записи попытка будет повторена
                        emit doOnCmdExecuted();
                    }
#endif
                    //msleep(5); //Registrator needs small delay between polls, 1 ms is OK too
                }

                //Command to DO
                if (mu110_DO_Command_to_set[i]==DO_Command::DO_OFF_CMD)
                {
                    msleep(5); //Registrator needs small delay between polls, 1 ms is OK too
#ifdef NO_DO_RELE
                    mu110_DO_Command_to_set[i]=DO_Command::DO_NONE_CMD;// обязательно сбрасываем команду после успешной записи
                                                              //при неуспешной записи попытка будет повторена
                    emit doOffCmdExecuted();
#else
                    uint16_t reg[200];
                    reg[0]=MU110_DO_OFF;
                    modbus_set_slave(ctx, MU110_DO_ADDRESSES[i]);
                    int write_result=modbus_write_registers(ctx,MU110_DO_REGISTERS[i],MU110_DO_NUM_REGISTERS,reg);
                    if (write_result==MU110_DO_NUM_REGISTERS)
                    {
                        mu110_DO_Command_to_set[i]=DO_Command::DO_NONE_CMD;// обязательно сбрасываем команду после успешной записи
                                                                  //при неуспешной записи попытка будет повторена
                        emit doOffCmdExecuted();
                    }
#endif
                    //msleep(5); //Registrator needs small delay between polls, 1 ms is OK too
                }
            }

            msleep(5); //750//Registrator needs small delay between polls, 1 ms is OK too
        }
*/


        //Voltage SET
        msleep(200);
        if (voltageSetCommand==VoltageSetCommand::VOLTSET_SET_CMD)
        {
            uint16_t value_to_write=static_cast<uint16_t>(voltageToSet*1000);

            qDebug() << "mbreader setvoltage=" << value_to_write << "mV";

            modbus_set_slave(ctx, ICP_7024_ADDRESS);
            int write_result=modbus_write_register(ctx,ICP_7024_VOLTAGE_REGISTER,value_to_write);
            //modbus_write_register return 1 if succesfull
            if (write_result==1)
            {
                voltageSetCommand=VoltageSetCommand::VOLTSET_NONE_CMD;// обязательно сбрасываем команду после успешной записи
                    //при неуспешной записи попытка будет повторена
                emit voltageSettedOK();
            }
            else
            {
                emit voltageSettedError();
            }
        }



        msleep(200);  //sleep должен біть после всех emit


        if (CheckThreadStop())
        {
            qDebug() << "stopping modbus reader thread";
            Release();
            return;
        }
    }
}
//=======================================================================

//=======================================================================
void ModbusReader::Release()
{
    /* Close the connection */
    if (ctx!=NULL)
    {
        /*
        //все реле отключаем по очереди, на случай если они на разніх устройствах
        uint16_t reg[200];
        reg[0]=MU110_DO_OFF;
        for(int i=0;i<MU110_NUM_RELES;++i)
        {
            modbus_set_slave(ctx, MU110_DO_ADDRESSES[i]);
            int write_result=modbus_write_registers(ctx,MU110_DO_REGISTERS[i],MU110_DO_NUM_REGISTERS,reg);
            if (write_result!=MU110_DO_NUM_REGISTERS)
            {
                modbus_write_registers(ctx,MU110_DO_REGISTERS[i],MU110_DO_NUM_REGISTERS,reg);
            }
            msleep(5);
        }
        */
        qDebug() << "releasing connection...";
        modbus_close(ctx);
        modbus_free(ctx);
        ctx=NULL;
    }
}
