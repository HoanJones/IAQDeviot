#include "simcom_api.h"
#include "simcom_application.h"
#include "../inc/debug.h"
#include "dht11.h"
#include "mq135.h"
#include "sds011.h"
#include "sgp30.h"
#include "uart.h"


extern void PrintfResp(INT8 *format);

sMsgQRef urc_mqtt_msgq_1;
sMsgQRef simcomUI_msgq;
static UINT8 mainProcesserStack[1024 * 30];
static UINT8 debugProcesserStack[1024 * 10];
sTaskRef mainProcesser, debugProcesser;
extern INT32 di_count;
extern struct ConfigStruct g_conf;
extern INT16 nb_backup;
extern INT16 ptr_backup;
extern void readConfig();
extern void process_conf(char *buff, INT8 type);
extern void writeMiscData();
void writeData(DataBackup data); //add
extern INT8 mqttconnected;
extern INT8 mqtt_status;
extern INT8 send_mqtt(DataBackup data);
extern INT8 subscribe_status;
extern void init_mqtt(void);

INT8 force_send = 0;
INT8 powersaving = 0;
INT8 sleepmode = 0;
UINT32 waketime = 0;
INT8 ntpstatus = 0;
UINT32 lostConnectionTime = 0;

int pinDHT = 12;
extern uint8_t I_RH, D_RH, I_Temp, D_Temp, CheckSum;
extern uint8_t temp;
extern uint8_t humdity;
extern void DHT11_init(int pinDHT);

extern void MQ135_init();
extern float CO2_val;

extern void SGP30_Init(void);
extern void SGP30_readData(void);
extern uint8_t eCO2;
extern uint8_t PVOC;

extern uint8_t pm25;
extern uint8_t pm10;
extern void SDS011_init(SC_Uart_Port_Number port, SC_UART_BaudRates baudrate, SC_UART_WordLen dataLen, SC_UART_ParityTBits parity, SC_UART_StopBits stopbit);
extern void Uart2CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve);
//extern void readData_SDS011(uartTypedef_t uartTypedefRecv);
sMsgQRef uartQueueHdl = NULL;
/***************UART Callback*******************/
void Uart2CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve)
{
    uartTypedef_t uartTypedefSend;

    sAPI_UartRead(portNumber, uartTypedefSend.data, len);

    uartTypedefSend.data[len] = 0;
    uartTypedefSend.flag = portNumber;
    uartTypedefSend.len = len;

    sAPI_MsgQSend(uartQueueHdl, (SIM_MSG_T*)&uartTypedefSend);

    return;
}

void debugTask(void *arg);

sTaskRef watchdogTaskRef;
char watchdogTaskStack[1024];
static int level = 0;
void sAPP_CusFeedDog(void *argv)
{
    while (1)
    {
        if (level == 1)
            sAPI_SysReset();
        level = 1;
        sAPI_TaskSleep(120 * 200); // 120s
    }
}

DataBackup packSensorData(SCsysTime_t t)
{
    DataBackup res;
    UINT8 csq = 0;
    sAPI_NetworkGetCsq(&csq);
    res.year = t.tm_year - 2000;
    res.month = t.tm_mon;
    res.day = t.tm_mday;
    res.hour = t.tm_hour;
    res.min = t.tm_min;
    res.di_count = di_count;

    res.temp = temp;
    res.humdity = humdity;
    res.airQua = CO2_val;
    res.pm25 = pm25;
    res.pm10 = pm10;
    
    res.vbat = sAPI_ReadVbat();
    //res.adc0 = sAPI_ReadAdc(0);
    //res.adc1 = sAPI_ReadAdc(1);
    res.csq = csq;
    res.uptime = sAPI_GetTicks() / 200;
    return res;
}

void mainProcess(void *ptr) {

    //UINT8 sTick = 0;
    uartTypedef_t uartTypedefRecv;
    SCsysTime_t now, last_send;
    unsigned int vbat;
    INT8 buffer[200] = {0};
    INT8 period;
    INT8 ret;
    INT8 nbMsg;

    sAPI_Debug("Start collect data from sensors!!! \r\n");

    uartUser_init(SC_UART, SC_UART_BAUD_115200, SC_UART_WORD_LEN_8, SC_UART_NO_PARITY_BITS, SC_UART_ONE_STOP_BIT);
    SDS011_init(SC_UART2, SC_UART_BAUD_9600, SC_UART_WORD_LEN_8, SC_UART_NO_PARITY_BITS, SC_UART_ONE_STOP_BIT);

    if(uartQueueHdl == NULL)
    {
         if(sAPI_MsgQCreate(&uartQueueHdl, "uartQueueHdl", sizeof(uartTypedef_t), 5, SC_FIFO) == SC_SUCCESS)
         {
             //DEBUG(__FILENAME__, "Create Queue ok\r\n");
         }
    }

    //SGP30_Init();

    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL}; // NULL pointer for msgQ_data_recv.arg3 is necessary!
    SCmqttData *sub_data = NULL;
    // enable sleep mode
    // sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_ENABLE);
    sAPI_NetworkSetCtzu(1);

    readConfig();
    sAPI_GpioSetDirection(9, 1); //set GPIO_09 is output
    sAPI_GpioSetValue(9, 1); //=> netlight on
    sAPI_TaskSleep(1000); // wait 5s

    sAPI_GpioSetDirection(0, 1); //first sAPI_GpioSetDirection(0, 1);

    init_mqtt();
    di_init();
    sms_init(); 
    force_send = 1;

    sAPI_TaskSleep(1000);
    updateNtp();
    sAPI_GetSysLocalTime(&last_send);

    while(1) {

        sAPI_GpioSetValue(10, 0);
        sAPI_TaskSleep(10);
        sAPI_GpioSetValue(10, 1);

        level = 0;

        nbMsg = 0;
        sAPI_MsgQPoll(urc_mqtt_msgq_1, &nbMsg); //= 0
        //DEBUG(__FILENAME__, "nbMSG: [%d]\t\r\n", nbMsg);
        while (nbMsg > 0) // have msg
        {
            sAPI_MsgQRecv(urc_mqtt_msgq_1, &msgQ_data_recv, 100);
            sub_data = (SCmqttData *)(msgQ_data_recv.arg3);
            // memset(buffer, 0, sizeof(buffer));
            // sprintf(buffer, "MQTT sub msg: %s\n", (char *)sub_data->payload_P);
            // PrintfResp(buffer);
            process_conf((char *)sub_data->payload_P, 1);
            sAPI_Free(sub_data->topic_P);
            sAPI_Free(sub_data->payload_P);
            sAPI_Free(sub_data);
            sAPI_MsgQPoll(urc_mqtt_msgq_1, &nbMsg);
            sAPI_TaskSleep(20);
        }

        check_sms();

/***********Code process and read data from sensorsld****************/
        DHT11_init(pinDHT);

        MQ135_init();

            //readData_SDS011(uartTypedefRecv);
            
        if(sAPI_MsgQRecv(uartQueueHdl, (SIM_MSG_T*)&uartTypedefRecv, SC_SUSPEND) == SC_SUCCESS)
        {
            switch(uartTypedefRecv.flag)
            {
                case SC_UART:

                    // if((uartTypedefRecv.data[0] == messageHeader) && (uartTypedefRecv.data[1] == commandNo) && (uartTypedefRecv.data[9] == messageTail)) {
                    //     pm25 = (uartTypedefRecv.data[2] + (uartTypedefRecv.data[3]*256))/10.0; //nong do bui pm2.5
                    //     pm10 = (uartTypedefRecv.data[4] + (uartTypedefRecv.data[5]*256))/10.0; //nong do bui pm10
                    // } 
                    break;
                case SC_UART2:
                        if((uartTypedefRecv.data[0] == messageHeader) && (uartTypedefRecv.data[1] == commandNo) && (uartTypedefRecv.data[9] == messageTail)) {
                            pm25 = (uartTypedefRecv.data[2] + (uartTypedefRecv.data[3]*256))/10.0; //nong do bui pm2.5
                            pm10 = (uartTypedefRecv.data[4] + (uartTypedefRecv.data[5]*256))/10.0; //nong do bui pm10
                        }
                    break;
                case SC_UART3:
                    break;
            }
        }

        //SGP30_readData();
        //DataBackup res2;

        // DEBUG(__FILENAME__, "Temp: %d *C \r\n", temp);
        // DEBUG(__FILENAME__, "Humdity: %d %% \r\n", humdity);
        // DEBUG(__FILENAME__, "Quality of CO2: %0.1f PPM\r\n", CO2_val);
        // DEBUG(__FILENAME__, "Quanlity of PM2.5 Dust: %d ug/m3 \r\n", pm25);
        // DEBUG(__FILENAME__, "Quality of PM10 Dust: %d ug/m3 \r\n", pm10);
        // // DEBUG(__FILENAME__, "eCO2: %d \r\n", eCO2);
        // // DEBUG(__FILENAME__, "PVOC: %d \r\n", PVOC);
        // DEBUG(__FILENAME__, "CSQ: %d \r\n", res2.csq);
        sAPI_TaskSleep(800);

/*****************Send data on Server*****************/
        sAPI_GetSysLocalTime(&now);

        if (di_count > 0)
            period = g_conf.period_active;
        if ((period == 60 && now.tm_min == 0 && now.tm_hour != last_send.tm_hour) || (period < 60 && now.tm_min % period == 0 && last_send.tm_min != now.tm_min) || force_send)
        {
            force_send = 0;
            sAPI_GetSysLocalTime(&last_send);
            if (di_count == 0)
                period = g_conf.period_passive;

            DataBackup data = packSensorData(now);
            di_count = 0;

            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "Time arrived: %d:%d %d/%d/%d, DI: %d, Temp: %d, Humdity: %d, CO2: %0.1f, PM2.5: %d, PM10: %d, vbat: %d, CSQ: %d, nb_backup: %d, ptr_backup: %d\r\n",
                    data.hour, data.min, data.day, data.month, data.year, data.di_count, data.temp, data.humdity, data.airQua, data.pm25, data.pm10, data.vbat, data.csq, nb_backup, ptr_backup);
            PrintfResp(buffer);
            DEBUG(__FILENAME__, buffer);
            
            //vbat ~2000
            if (powersaving == 0 && data.vbat < 3300)
                powersaving = 0; // first = 1
            else if (powersaving == 1 && data.vbat > 3500)
                powersaving = 1; //0

            //DEBUG(__FILENAME__, "powersaving = [%d]\r\n", powersaving);

            if (powersaving == 0) 
            {
                ret = send_mqtt(data);
                //DEBUG(__FILENAME__, "ret = [%d]\r\n", ret);
                if (ret == 0)
                {
                    PrintfResp("MQTT send ok\n");
                    DEBUG(__FILENAME__, "MQTT send ok!\t\r");
                    if (ptr_backup < nb_backup)
                        send_mqtt_backup();
                }
                else if (data.csq < 99 && data.csq > 5 && sAPI_GetTicks() - lostConnectionTime > 360000)
                    sAPI_SysReset(); // reset if no connection for 1/2 hour with positive csq
                else if (data.csq == 99 || data.csq < 5)
                    lostConnectionTime = sAPI_GetTicks();
            }
            if (powersaving != 0 || ret != 0) 
                writeData(data);
            writeMiscData();
        }
        // sAPI_SystemAlarmClock2Wakeup(g_conf.period_suspend * 200);
        if (sleepmode == 1 && sAPI_SystemSleepGet() != SC_SYSTEM_SLEEP_ENABLE)
        {
            PrintfResp("start sleep\n");
            DEBUG(__FILENAME__, "Start sleep\r\n");
            sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_ENABLE);
        }
        sAPI_TaskSleep(g_conf.period_suspend * 200);
        if (sleepmode == 0 && sAPI_SystemSleepGet() != SC_SYSTEM_SLEEP_DISABLE)
        {
            PrintfResp("woke up\n");
            DEBUG(__FILENAME__, "Woke up\r\n");
            waketime = sAPI_GetTicks();
            sAPI_SystemSleepSet(SC_SYSTEM_SLEEP_DISABLE);
        }
        if (sleepmode == 0 && sAPI_GetTicks() - waketime > 60000)
            sleepmode = 1;

        if (ntpstatus == 0)
            updateNtp();

        if (now.tm_hour == 1 && now.tm_min == 0)
            ntpstatus = 1;

        if (sAPI_GetTicks() / 200 > 3600 * 24 * 7)
            sAPI_SysReset();

        if (!mqttconnected)
        {
            connect_mqtt();
            if (sAPI_GetTicks() > 1080000) // led on for 90 min max
                sAPI_GpioSetValue(9, 0);
        }

        if (mqtt_status == 1 && subscribe_status == 0)
            subscribe();
        //    do
        //{
        //    SC_SMSReturnCode sms_ret;
        //    sms_ret = sAPI_SmsReadMsg(1, 1, sms_msgq);
        //    if (sms_ret == SC_SMS_SUCESS)
        //{
        //       sAPI_MsgQRecv(sms_msgq, &msgQ_sms_recv, 200);
        //       PrintfResp(msgQ_sms_recv.arg3);
        //       sAPI_SmsDelAllMsg(&msgQ_sms_recv);
        //   }
        //   } while (nbMsg > 0);
        /**/
  
    }
}

void initTask(void) {

    SC_STATUS status;

    status = sAPI_MsgQCreate(&simcomUI_msgq, "simcomUI_msgq", sizeof(SIM_MSG_T), 12, SC_FIFO);

    status = sAPI_MsgQCreate(&urc_mqtt_msgq_1, "urc_mqtt_msgq_1", (sizeof(SIM_MSG_T)), 4, SC_FIFO); // msgQ for subscribed data transfer

    status = sAPI_TaskCreate(&mainProcesser, mainProcesserStack, 1024 * 30, 100, (char*) "mainProcess", mainProcess, (void *)0);

    if(status != SC_SUCCESS) {
        DEBUG(__FILENAME__, "Main Process Task init failed \r\n");
    }
    //DEBUG(__FILENAME__, "Create task success!! \r\n");

    status = sAPI_TaskCreate(&debugProcesser, debugProcesserStack, 1024 * 10, 100, "debugProcesser", debugTask, (void *)0);
    if (SC_SUCCESS != status)
    {
        sAPI_Debug("task create fail");
    }

}