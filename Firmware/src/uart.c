/*
 * uartUser.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include <stdarg.h>
#include "uart.h"
#include "..\inc\debug.h"
#include "config.h"
//#include "sds011.h"
// #include "http.h"
// #include "systemFlag.h"
// #include "fota.h"
// #include "rtc.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[UART]"

#define UART_TASK_STACK_SIZE    (1024 * 10)
#define UART_TASK_PRIORITY      (100)

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static sTaskRef uartTask = NULL;
static UINT8 uartTaskStack[UART_TASK_STACK_SIZE];
static sMsgQRef uartQueueHdl = NULL;

static uint8_t pm25 = 0;
static uint8_t pm10 = 0;
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void UartCBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve);
static void Task_UartProcess(void *ptr);
/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */

/*********************************************************************************************************//**
 * @brief   Init uart
 * @paramtf
 * @retval  
 ************************************************************************************************************/
void uartUser_Init(SC_Uart_Port_Number port, SC_UART_BaudRates baudrate, SC_UART_WordLen dataLen, SC_UART_ParityTBits parity, SC_UART_StopBits stopbit)
{
    SCuartConfiguration uartConfig;

    /*************************Configure UART again*********************************/
    /*******The user can modify the initialization configuratin of UART in here.***/ 
    /******************************************************************************/
    uartConfig.BaudRate  =  baudrate;
    uartConfig.DataBits  =  dataLen;
    uartConfig.ParityBit =  parity;
    uartConfig.StopBits  =  stopbit;
    if(sAPI_UartSetConfig(port, &uartConfig) == SC_UART_RETURN_CODE_ERROR)
    {
        DEBUG(__FILENAME__, "%s: Configure UART failure!!" ,__func__);
        return;
    }

    sAPI_UartRegisterCallbackEX(port, UartCBFuncEx, (void *)"UartCBFuncEx");

    // if(uartTask == NULL)
    // {
    //     if(sAPI_TaskCreate(&uartTask, uartTaskStack, UART_TASK_STACK_SIZE, UART_TASK_PRIORITY, (char*)"uart process", Task_UartProcess, (void *)0) != SC_SUCCESS)
    //     {
    //         DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
    //         return;
    //     }
    // }
    // else DEBUG(__FILENAME__, "Task already\r\n");

    DEBUG(__FILENAME__, "Uart[%d] Init Done\r\n", port);
}

/*********************************************************************************************************//**
 * @brief   for debug uart
 * @paramtf
 * @retval  
 ************************************************************************************************************/
void uartUser_Debug(const char *tag, const char *format,...)
{
    static char str[1024];
    UINT16 len;

    va_list argList;
    memset(str, 0, sizeof(str));

    va_start(argList, format);
    #if (A7672)
    len = sAPI_Sprintf(str, "[%s]: ", tag);
    len += sAPI_Vsnprintf(&str[len], sizeof(str)-len, format, argList);
    #else
    len = sprintf(str, "%s: ", tag);
    len += vsnprintf(&str[len], sizeof(str)-len, format, argList);
    #endif
    va_end(argList);
 
    sAPI_UartWriteString(DEBUG_PORT, (UINT8*)&str);
}

void _Debug(const char *format,...)
{
    static char str[1024];
    UINT16 len = 0;

    va_list argList;
    memset(str, 0, sizeof(str));

    va_start(argList, format);
    // len = sAPI_Sprintf(str, format, argList);
    #if (A7672)
    len += sAPI_Vsnprintf(&str[len], sizeof(str)-len, format, argList);
    #else
    len += vsnprintf(&str[len], sizeof(str)-len, format, argList);
    #endif
    va_end(argList);
 
    sAPI_UartWriteString(DEBUG_PORT, (UINT8*)&str);
}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************************************************//**
 * @brief   callback uart 
 * @paramtf
 * @retval  
 ************************************************************************************************************/
static void UartCBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve)
{
    uartTypedef_t uartTypedefSend;

    sAPI_UartRead(portNumber, uartTypedefSend.data, len);

    uartTypedefSend.data[len] = 0;
    uartTypedefSend.flag = portNumber;
    uartTypedefSend.len = len;

    sAPI_MsgQSend(uartQueueHdl, (SIM_MSG_T*)&uartTypedefSend);

    return;
}
/*********************************************************************************************************//**
 * @brief   uart task - process received data
 * @paramtf
 * @retval  
 ************************************************************************************************************/
static void Task_UartProcess(void *ptr)
{
    uartTypedef_t uartTypedefRecv;

    if(uartQueueHdl == NULL)
    {
        if(sAPI_MsgQCreate(&uartQueueHdl, "uartQueueHdl", sizeof(uartTypedef_t), 5, SC_FIFO) == SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "Create Queue ok\r\n");
        }
    }
    else DEBUG(__FILENAME__, "Queue already\r\n");

    while(1)
    {
        /* wait 1 tick = 5ms */
        if(sAPI_MsgQRecv(uartQueueHdl, (SIM_MSG_T*)&uartTypedefRecv, SC_SUSPEND) == SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "RX UART[%d] %s\r\n", uartTypedefRecv.flag, (const char *)uartTypedefRecv.data);    
            // sAPI_Debug("%s\r\n", (const char*)uartTypedefRecv.data);

            switch(uartTypedefRecv.flag)
            {
                // case DEBUG_PORT:
                //     if(strstr((const char *)uartTypedefRecv.data, "reset")) 
                //     {
                //         sAPI_SysReset();
                //         return;
                //     }
                //     break;
                case DEBUG_PORT:
                    config_process((char*)uartTypedefRecv.data, uartTypedefRecv.len);
                    // if(strstr((const char *)uartTypedefRecv.data, "reset")) 
                    // {
                    //     sAPI_SysReset();
                    //     return;
                    // }
                    break;
                case CONGIF_PORT:
                    // static uint8_t pm25 = 0;
                    // static uint8_t pm10 = 0;

                    //if (uartTypedefRecv.data[0] == messageHeader & uartTypedefRecv.data[1] == commandNo & uartTypedefRecv.data[9] == messageTail) {
                        pm25 = (uartTypedefRecv.data[2] + (uartTypedefRecv.data[3]*256))/10.0; //nong do bui pm2.5
                        pm10 = (uartTypedefRecv.data[4] + (uartTypedefRecv.data[5]*256))/10.0;

                        DEBUG(__FILENAME__, "Nong do bui PM2.5: [%d] \r\n", pm25 );
                        DEBUG(__FILENAME__, "Nong do bui PM10: [%d] \r\n", pm10);
                    //}

                    // pm25 = (uartTypedefRecv.data[2] + (uartTypedefRecv.data[3]*256))/10.0; //nong do bui pm2.5
                    // pm10 = (uartTypedefRecv.data[4] + (uartTypedefRecv.data[5]*256))/10.0;

                    // DEBUG(__FILENAME__, "Nong do bui PM2.5: [%d] \r\n", pm25 );
                    // DEBUG(__FILENAME__, "Nong do bui PM10: [%d] \r\n", pm10);

                default:
                    break;
            }
#if (TEST_EN)
            if(strstr(uartTypedefRecv.data, "FOTA_UPDATE"))
            {
                sAPI_FlagSet(sysFlag, FLAG_MASK_FOTA, SC_FLAG_OR); 
            }
            else if(strstr(uartTypedefRecv.data, "KERNEL_UPDATE"))
            {
                sAPI_FlagSet(sysFlag, FLAG_MASK_FOTA_KERNEL, SC_FLAG_OR); 
            }
            else if(strstr(uartTypedefRecv.data, "http://"))
            {
                sAPI_FlagSet(sysFlag, FLAG_MASK_HTTP, SC_FLAG_OR); 
            }

            else if(strstr(uartTypedefRecv.data, "RESET")) 
            {
                sAPI_SysReset();
                return;
            }

            else if(strstr(uartTypedefRecv.data, "SET_RTC"))  // SET_RTC:h:m:s#
            {
                char *frame1 = NULL, *tok;
                UINT8 h, m, s;

                frame1 = strstr(uartTypedefRecv.data, "SET_RTC"); // SET_RTC:h:m:s#
                if(frame1 != NULL)
                {                         
                    tok = strtok(frame1, ":");              // SET_RTC 
                    DEBUG(__FILENAME__, "tok: %s\r\n", tok);
                    tok = strtok(NULL, ":");                // h
                    h = atoi(tok);          
                    DEBUG(__FILENAME__, "hour: %d\r\n", h);
                    tok = strtok(NULL, ":");                // m
                    m = atoi(tok);          
                    DEBUG(__FILENAME__, "min: %d\r\n", m);
                    tok = strtok(NULL, ":");                // s
                    s = atoi(tok);          
                    DEBUG(__FILENAME__, "sec: %d\r\n", s);

                    RTC_setTime(h, m, s, 2, 1, 2, 2022);
                }
            }
            // else if(strstr(uartTypedefRecv.data, "LAT"))
            // {
            //     // mqtt_PublishData("chungnt/lat", 2, "");
            // }

            else if(strstr(uartTypedefRecv.data, "GPS_OFF")) 
            {
                // gnss_GetGPSInfo(0);
                gnss_StopGetNMEA(SC_GNSS_NMEA_DATA_GET_BY_URC);
                gnss_SwitchPower(SC_GNSS_POWER_OFF);
            }
            else if(strstr(uartTypedefRecv.data, "GPS_ON")) 
            {
                gnss_SwitchPower(SC_GNSS_POWER_ON);
                gnss_StartGetNMEA(SC_GNSS_NMEA_DATA_GET_BY_URC);
                // gnss_SetStartMode(SC_GNSS_START_HOT);
                // gnss_GetGPSInfo(3);
            }
#endif
        }

        sAPI_TaskSleep(1);
    }
}
