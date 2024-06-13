/*
 * virtualCom.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "virtualCom.h"
#include "debug.h"
#include "CircularBuffer.h"
// #include "ssl_server.h"
// #include "mqtt.h"
// #include "systemFlag.h"
// #include "fota.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[VUSB]"

#define USE_RX_USB_PORT 0

#define MAX_FIFO_VUSB_SIZE 1024


#define VUSB_TASK_STACK_SIZE    (1024 * 10)
#define VUSB_TASK_PRIORITY      (150)

#define COMMAND_MAX_LEN (20)

#define MSG_FIELD_SEPARATOR (',')
/* Message field position */
#define MSG_FIELD_TYPE (0)
#define MSG_FIELD_DATA (1)
#define MSG_MAX_FIELDS (2)
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */



/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */

static sTaskRef VUSBTask = NULL;
static UINT8 VUSBTaskStack[VUSB_TASK_STACK_SIZE];



static uint8_t sVUSBFIFO[MAX_FIFO_VUSB_SIZE];
static circularBuf_t sVUSBHdl_FIFO;
static buffer_t bufferRxCfg;
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
#if(USE_RX_USB_PORT)
void UsbVcomCBFunc(void *para);
static void Task_VUSBProcess(void *ptr);
#endif

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */

void virtualCom_Init(void)
{
#if(USE_RX_USB_PORT)
    // sAPI_UsbVcomRegisterCallbackEX(UsbVcomCBFuncEx, (void*)"UsbVcomCBFuncEx");
    sAPI_UsbVcomRegisterCallback(UsbVcomCBFunc);
    CBUFFER_Init(&sVUSBHdl_FIFO, &sVUSBFIFO, sizeof(uint8_t), MAX_FIFO_VUSB_SIZE);

    if(sAPI_TaskCreate(&VUSBTask, VUSBTaskStack, VUSB_TASK_STACK_SIZE, VUSB_TASK_PRIORITY, (char*)"VUSB process", Task_VUSBProcess, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
        return;
    }
    DEBUG(__FILENAME__, "sAPI_TaskCreate ok!\n");
#endif
}

/*********************************************************************************************************//**
 * @brief   for debug usb virtual com port
 * @param
 * @retval  
 ************************************************************************************************************/
void virtualCom_Debug(const char *tag, const char *format,...)
{
    char str[1024];
    UINT16 len;

    va_list argList;
    memset(str, 0, sizeof(str));

    va_start(argList, format);
    len = sAPI_Sprintf(str, "%s:", tag);
    len += sAPI_Vsnprintf(&str[len], sizeof(str)-len, format, argList);
    va_end(argList);
 
    sAPI_UsbVcomWrite((UINT8*)&str, len);
}

void usb_Debug(const char *format,...)
{
    static char str[1024];
    UINT16 len = 0;

    va_list argList;
    memset(str, 0, sizeof(str));

    va_start(argList, format);
    // len = sAPI_Sprintf(str, format, argList);
    len += sAPI_Vsnprintf(&str[len], sizeof(str)-len, format, argList);
    va_end(argList);
 
    sAPI_UsbVcomWrite((UINT8*)&str, len);
}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
#if(USE_RX_USB_PORT)
void UsbVcomCBFunc(void *para)
{
    int readLen = 0;
    char *usbVCcomData = sAPI_Malloc(MAX_FIFO_VUSB_SIZE);
    uint16_t i;

    readLen = sAPI_UsbVcomRead((UINT8*)usbVCcomData, MAX_FIFO_VUSB_SIZE);
    
    // sAPI_UsbVcomWrite((UINT8*)usbVCcomData, readLen);

    usbVCcomData[readLen] = 0;
    for(i = 0; i < readLen; i++) CBUFFER_Put(&sVUSBHdl_FIFO, &usbVCcomData[i]);

    sAPI_Free((void*)usbVCcomData);
}

static void Task_VUSBProcess(void *ptr)
{
#if 0
    char *msg_field[MSG_MAX_FIELDS];
	ERR_E ret = CMD_NOT_SUPPORT;
	// pointer 
	char *msg_token;
	char data_reply[20] = {0};
	// generic indexers
	uint8_t j;
#endif
    uint8_t ch;

    while(1)
    {
        /* wait 1 tick = 5ms */
        if(CBUFFER_Get(&sVUSBHdl_FIFO, &ch) == 1)
        {
            // if(ch == '$') 
		    // {
            //     memset(&bufferRxCfg, 0, sizeof(bufferRxCfg));
            //     continue;
            // }

            if(ch != '#')
            {
                bufferRxCfg.Data[bufferRxCfg.Index++] = ch;	
            }
            else
            {
                bufferRxCfg.Data[bufferRxCfg.Index] = '\0';

                DEBUG(__FILENAME__, "Frame: %s\r\n", bufferRxCfg.Data);
#if (VCP_TEST)
                if(strstr(bufferRxCfg.Data, "ssl_connect"))
                {
                    ssl_server_handshake();
                }
                else if(strstr(bufferRxCfg.Data, "ssl_disconnect"))
                {
                    ssl_server_disconnect();
                }
                else if(strstr(bufferRxCfg.Data, "reset"))
                {
                    sAPI_SysReset();
                }
                else if(strstr(bufferRxCfg.Data, "mqtt_connect"))
                {
                    mqtt_connect();
                }
                else if(strstr(bufferRxCfg.Data, "mqtt_disconnect"))
                {
                    mqtt_disconnect();
                }
                else if(strstr(bufferRxCfg.Data, "mqtt_request"))
                {
                    mqtt_test_pub();
                }
                else if(strstr(bufferRxCfg.Data, "json_test"))
                {
                    // sAPP_CjsonTest();
                }
                else if(strstr(bufferRxCfg.Data, "switch_sim_2"))
                {
                    DEBUG(__FILENAME__, "ret [%d]\r\n", sAPI_SimcardHotSwapMsg(SC_HOTSWAP_SET_SWITCH, 1, NULL));
                    sAPI_SimcardSwitchMsg(1, NULL);
                    DEBUG(__FILENAME__, "switch to sim 2\r\n");
                }
                else if(strstr(bufferRxCfg.Data, "switch_sim_1"))
                {
                    DEBUG(__FILENAME__, "ret [%d]\r\n", sAPI_SimcardHotSwapMsg(SC_HOTSWAP_SET_SWITCH, 1, NULL));
                    sAPI_SimcardSwitchMsg(0, NULL);
                    DEBUG(__FILENAME__, "switch to sim 1\r\n");
                }
                else if(strstr(bufferRxCfg.Data, "check_iccid"))
                {
                    char iccid[30];
                    memset(iccid, 0, 30);
                    sAPI_SysGetIccid(iccid);
                    DEBUG(__FILENAME__, "ICCID: %s\r\n", iccid);
                }
                else if(strstr(bufferRxCfg.Data, "kernel_update"))
                {
                    sAPI_FlagSet(sysFlag, FLAG_MASK_FOTA_KERNEL, SC_FLAG_OR); 
                }
                else if(strstr(bufferRxCfg.Data, "app_update"))
                {
                    sAPI_FlagSet(sysFlag, FLAG_MASK_FOTA_APP, SC_FLAG_OR);
                }                
                else if(strstr(bufferRxCfg.Data, "GPS_OFF")) 
                {
                    // gnss_GetGPSInfo(0);
                    gnss_StopGetNMEA(SC_GNSS_NMEA_DATA_GET_BY_URC);
                    gnss_SwitchPower(SC_GNSS_POWER_OFF);
                }
                else if(strstr(bufferRxCfg.Data, "GPS_ON")) 
                {
                    // gnss_Init();

                    gnss_SwitchPower(SC_GNSS_POWER_ON);
                    gnss_StartGetNMEA(SC_GNSS_NMEA_DATA_GET_BY_URC);;
                }
#endif
                CBUFFER_Reset(&sVUSBHdl_FIFO);
                memset(&bufferRxCfg, 0, sizeof(bufferRxCfg));

                // memset(msg_field, 0, sizeof(msg_field));

                // msg_token		 		  = bufferRxCfg.Data;
                // msg_field[MSG_FIELD_TYPE] = msg_token;
                
                // while(*msg_token != '\0')
                // {
                //     if(*msg_token == MSG_FIELD_SEPARATOR)
                //     {
                //         // terminate string after field separator or end-of-message characters
                //         *msg_token = '\0';

                //         // save position of the next token
                //         msg_field[MSG_FIELD_DATA] = msg_token + 1;
                //         break;
                //     }
                //     msg_token++;
                // }
                
                // for(j = 0; j < packet_process_table_size; j++)
                // {
                //     if(strcmp((char*)packet_process_table[j].packet_str, (char*)msg_field[MSG_FIELD_TYPE]) == 0)
                //     {
                //         if(packet_process_table[j].packet_func_ptr != NULL)
                //         {
                //             ret = packet_process_table[j].packet_func_ptr((char*)msg_field[MSG_FIELD_DATA], (char*)data_reply);
                //         }
                //         break;
                //     }
                // }
                
                // if(j == packet_process_table_size)
                // {
                //     DEBUG(__FILENAME__, "Command not support\r\n");
                //     memset(&bufferRxCfg, 0, sizeof(bufferRxCfg));
                //     continue;;
                // }
                            
                // DEBUG(__FILENAME__, "+%s, ret: %d\r\n", (char*)packet_process_table[j].packet_str, ret);
            }
        
        }
        sAPI_TaskSleep(1);
    }
}
#endif
