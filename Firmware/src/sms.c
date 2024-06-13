/*
 * sms.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "sms.h"
#include "debug.h"
#include "gpio.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[SMS]"


#define SMS_TASK_STACK_SIZE    (1024 * 6)
#define SMS_TASK_PRIORITY      (100)

#define SMS_URC_RECIVE_TIME_OUT 1000
#define MAX_SMS_INPUT_DATA_LEN  400
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */
sMsgQRef smsUrcMesQ;


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static sTaskRef smsTask = NULL;
static UINT8 smsTaskStack[SMS_TASK_STACK_SIZE];

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void taskSMS_Process(void *arg);


/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
void sms_Init(void)
{
    if(sAPI_TaskCreate(&smsTask, smsTaskStack, SMS_TASK_STACK_SIZE, SMS_TASK_PRIORITY, (char*)"SMS process", taskSMS_Process, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
    }
    else DEBUG(__FILENAME__, "Task Create ok\r\n");
}

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 * @example: sms_SendNewMsg("+84xxxxxxxxx", "Hello", 5); 
 * @NOTE: data < 160bytes
 ************************************************************************/
smsStatus_t sms_SendNewMsg(UINT8 *address, UINT8 *data, UINT16 dataLen)
{
    SC_SMSReturnCode ret = SC_SMS_SUCESS;
    SIM_MSG_T msg_rsp;

    ret = sAPI_SmsSendMsg(1, data, dataLen, address, smsUrcMesQ); // text mode
    DEBUG(__FILENAME__, "sms_SendNewMsg: add[%s], dat[%s]\r\n", address, data);
    if(ret == SC_SMS_SUCESS)
    {
        memset(&msg_rsp, 0, sizeof(msg_rsp));
        sAPI_MsgQRecv(smsUrcMesQ, &msg_rsp, SMS_URC_RECIVE_TIME_OUT);
        DEBUG(__FILENAME__, "sAPI_SmsSendMsg, primID[%d] resultCode[%d], rspStr: %s \r\n", msg_rsp.arg1, msg_rsp.arg2, msg_rsp.arg3);
        sAPI_Free(msg_rsp.arg3);
    }
    else
    {
       DEBUG(__FILENAME__, "sAPI_SmsSendMsg failed!\r\n");
       return SMS_SEND_FAIL;
    }

    return SMS_OK;
}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
static void taskSMS_Process(void *arg)
{
    SC_SMSReturnCode ret = SC_SMS_SUCESS;
    SC_STATUS status;
    SIM_MSG_T SmsUrcProcessReq;
    SIM_MSG_T msg_rsp;

    if(sAPI_MsgQCreate(&smsUrcMesQ, "smsUrcMesQ", sizeof(SIM_MSG_T), 4, SC_FIFO) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "smsUrcMesQ creat err!\r\n");
        return;
    }
    else DEBUG(__FILENAME__, "smsUrcMesQ creat ok!\r\n");

    while(1)
    {
        memset(&SmsUrcProcessReq, 0, sizeof(SIM_MSG_T));
        /*msgQ send from urc*/
        sAPI_MsgQRecv(smsUrcMesQ, &SmsUrcProcessReq, SC_SUSPEND);
        /*operations related to a valid cmd type*/
        switch(SmsUrcProcessReq.msg_id)
        {
            case SC_URC_NEW_MSG_IND:
            {
                ret = sAPI_SmsReadMsg(1, SmsUrcProcessReq.arg1, smsUrcMesQ); // 1: text mode
                if(ret == SC_SMS_SUCESS)
                {
                    memset(&msg_rsp, 0, sizeof(msg_rsp));
                    status = sAPI_MsgQRecv(smsUrcMesQ, &msg_rsp, SMS_URC_RECIVE_TIME_OUT);
                    if(status == SC_SUCCESS)
                    {
                        DEBUG(__FILENAME__, "sAPI_SmsReadMsg, primID[%d] resultCode[%d], rspStr: %s\r\n", msg_rsp.arg1, msg_rsp.arg2, msg_rsp.arg3);
                        // copy msg_res.arg3 -> free pointer 

                        /*must free this response buffer*/
                        sAPI_Free(msg_rsp.arg3);
                        /* process new msg */

                    }
                    else 
                    {
                        DEBUG(__FILENAME__, "sAPI_SmsReadMsg fail, ret [%d]\r\n", ret);
                        return;
                    }

                    /*delete one msg*/
                    ret = sAPI_SmsDelOneMsg(SmsUrcProcessReq.arg1, smsUrcMesQ);
                    if(ret == SC_SMS_SUCESS)
                    {
                        memset(&msg_rsp, 0, sizeof(msg_rsp));
                        status = sAPI_MsgQRecv(smsUrcMesQ, &msg_rsp, SMS_URC_RECIVE_TIME_OUT);
                        if(status == SC_SUCCESS)
                        {
                            DEBUG(__FILENAME__, "sAPI_SmsDelOneMsg suscess\r\n");
                        }
                        /*must free this response buffer*/
                        sAPI_Free(msg_rsp.arg3);
                    }
                    else
                    {
                        DEBUG(__FILENAME__, "sAPI_SmsDelOneMsg failed!\r\n");
                    }
                }
                else
                {
                   DEBUG(__FILENAME__, "sAPI_SmsReadMsg failed!\r\n");
                }
                #if(0)
                sAPI_TaskSleep(10000/5); // test DEBUG MsgQSend ret 
                DEBUG(__FILENAME__, "exit sleep test\r\n");
                #endif
                break;
            }
            case SC_URC_FLASH_MSG:
            {
                // int txtLen;
                // char *phoneNum = NULL, *txt = NULL;
                /*recv new text msg string here, if sAPI_SmsSetNewMsgInd(1,1,2,0,0,0) has been set when booting*/
                DEBUG(__FILENAME__, "SC_URC_FLASH_MSG: %s\r\n", (UINT8*)SmsUrcProcessReq.arg3);
                // copy msg_res.arg3 -> free pointer

                /*must free this response buffer*/
                sAPI_Free(msg_rsp.arg3);
                /* process new msg */
                // sms_SendNewMsg("+84385xxxxxx", "Hello", 5);

                #if(0)
                sAPI_TaskSleep(10000/5); // test DEBUG MsgQSend ret
                DEBUG(__FILENAME__, "exit sleep test\r\n");
                #endif
                break;
            }
        }
    }
}
