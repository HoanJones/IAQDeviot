/*
 * urcProcess.c
 *
 *  Created on:
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "urcProcess.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "defineType.h"
// #include "app.h"
#include "debug.h"
// #include "systemFlag.h"
// #include "mqtt.h"
#include "sms.h"
#include "call.h"
// #include "rtc.h"
// #include "simcom_simcard.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[URC]"

#define URC_TASK_STACK_SIZE    (1024 * 10)
#define URC_TASK_PRIORITY      (150)
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static sTaskRef urcTask = NULL;
static UINT8 urcTaskStack[URC_TASK_STACK_SIZE];
static sMsgQRef urcQueueHdl;
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void Task_UrcProcess(void *ptr);


/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
/*********************************************************************************************************//**
 * @brief   Init urc process task
 * @retval  
 * @param
 ************************************************************************************************************/
void urcProcess_Init(void)
{
    if(sAPI_TaskCreate(&urcTask, urcTaskStack, URC_TASK_STACK_SIZE, URC_TASK_PRIORITY, (char*)"URC process", Task_UrcProcess, (void *)0) != SC_SUCCESS)
    {
        urcTask = NULL;
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
        return;
    }

    DEBUG(__FILENAME__, "Init Done\r\n");
}

/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
static void Task_UrcProcess(void *ptr)
{
    SIM_MSG_T msg = {0};
    // UINT8 *Pbbuf = NULL;
    // UINT8 *Netbuf = NULL;

    #if (SMS_EN)
    UINT8 *SMSbuf = NULL;
    #endif

    if(sAPI_MsgQCreate(&urcQueueHdl, "URCQueueHdl", sizeof(SIM_MSG_T), 20, SC_FIFO) != SC_SUCCESS)
    {
        return;      
    }

    if(sAPI_UrcRefRegister(urcQueueHdl, SC_MODULE_ALL) != SC_SUCCESS)
    {
        return;
    }

    DEBUG(__FILENAME__, "Create Queue ok\r\n");

    while(1)
    {
        // sAPI_TaskSleep(1);
        if(sAPI_MsgQRecv(urcQueueHdl, &msg, SC_SUSPEND) != SC_SUCCESS)
        {
            continue;
        }

        if(SRV_URC != msg.msg_id)
        {
            sAPI_Free(msg.arg3);
            continue;
        }

        // DEBUG(__FILENAME__, "arg1: %d, arg2: %d\r\n", msg.arg1, msg.arg2);

        switch(msg.arg1)
        {
            case SC_URC_PB_MASK:
            {
                // DEBUG(__FILENAME__, "Phonebook process!!\r\n");
                if(msg.arg2 == SC_URC_PBDOWN)
                {
                    // Pbbuf = (UINT8 *)msg.arg3;
                    // DEBUG(__FILENAME__, "Phonebook: %s\r\n", Pbbuf);
                }
                break;
            }

            #if (SMS_EN)
            case SC_URC_SMS_MASK:
            {
                // DEBUG(__FILENAME__, "SMS process! sim_urc_code = %d\r\n", msg.arg2);
                // DEBUG(__FILENAME__, "msg.arg3: %p\r\n", msg.arg3);
                
                switch(msg.arg2)
                {
                    case SC_URC_FLASH_MSG: 
                    {
                        /* flash msg recved, +CMT: */
                        UINT8 *p_msgpload = (UINT8*)msg.arg3;
                        SIM_MSG_T SmsUrcProcessReq = {0};
                        if(p_msgpload != NULL)
                        {
                            DEBUG(__FILENAME__, "SC_URC_FLASH_MSG, new msg: %s\r\n", p_msgpload);
                        
                            /*send msg to process task and read this msg*/
                            SmsUrcProcessReq.msg_id = SC_URC_FLASH_MSG; // msgid
                            SmsUrcProcessReq.arg3 = sAPI_Malloc(strlen(msg.arg3)+1); // msg.arg3 will free after this process
                            memset(SmsUrcProcessReq.arg3, 0, (strlen(msg.arg3)+1));
                            memcpy(SmsUrcProcessReq.arg3, (UINT8*)msg.arg3, strlen(msg.arg3)); // msg string
                            /*Trigger urc process thread*/
                            DEBUG(__FILENAME__, "MsgQSend ret[%d]\r\n", sAPI_MsgQSend(smsUrcMesQ, &SmsUrcProcessReq));
                        }
                        break;
                    }
                    case SC_URC_NEW_MSG_IND: 
                    {
                        /* new msg recved, +CMTI: "SM", index */
                        UINT8 *p_smsindex = (UINT8*)msg.arg3;
                        SIM_MSG_T SmsUrcProcessReq = {0};
                        char *tok;
                        UINT8 index;
                        if(p_smsindex != NULL)
                        {
                            DEBUG(__FILENAME__, "SC_URC_NEW_MSG_IND, new sms string: %s\r\n", p_smsindex);
                            tok = strtok((char*)p_smsindex, ":"); // +CMTI:
                            tok = strtok(NULL, ","); // "SM"
                            tok = strtok(NULL, "\r"); // index
                            index = atoi(tok);

                            /*send msg to process task and read this msg*/
                            SmsUrcProcessReq.msg_id = SC_URC_NEW_MSG_IND; // msgid
                            SmsUrcProcessReq.arg1 = index; // msg index
                            /*Trigger urc process thread*/
                            DEBUG(__FILENAME__, "MsgQSend ret[%d]\r\n", sAPI_MsgQSend(smsUrcMesQ, &SmsUrcProcessReq));
                        }
                        break;
                    }
                    case SC_URC_STATUS_REPORT: 
                    {
                        /* sms send status, +CDS: */
                        UINT8 *p_smsStatus = (UINT8*)msg.arg3;
                        if(p_smsStatus != NULL)
                        {
                            DEBUG(__FILENAME__, "sms status report, p_smsStatus: %s\r\n", p_smsStatus);      
                        }
                        break;
                    }
                    case SC_URC_SMSDONE: 
                    {
                        /* SMS DONE? */
                        SMSbuf = (UINT8*)msg.arg3;
                        DEBUG(__FILENAME__, "SC_URC_SMSDONE, msgbuf: %s\r\n", SMSbuf);
                        break;
                    }
                    case SC_URC_SMSFULL: 
                    {
                        /* SMS FULL */
                        UINT8 *p_msgpload = (UINT8*)msg.arg3;
                        if(p_msgpload != NULL)
                        {
                            DEBUG(__FILENAME__, "SC_URC_SMSFULL, p_msgpload: %s\r\n", p_msgpload);

                            if(sAPI_SmsDelAllMsg(smsUrcMesQ) != SC_SMS_SUCESS)
                            {
                                DEBUG(__FILENAME__, "Delete all sms err\r\n");
                            }
                            else 
                            {
                                DEBUG(__FILENAME__, "Delete all sms done\r\n");
                            }
                        }
                        break;
                    }
                }
                break;
            }
            #endif

            #if (CALL_EN)
            case SC_URC_CALL_MASK:
            {
                switch(msg.arg2)
                {
                    case SC_URC_RING_IND:
                    {
                        UINT8 *pCallBuf = (UINT8*)msg.arg3;
                        UINT8 *p = NULL, *tok = NULL;
                        uint8_t len;
                        uint8_t i;
                        char phoneBuf[MAX_OA_SIZE];
                        DEBUG(__FILENAME__, "pCallBuf: %s\r\n", pCallBuf);
                        if(pCallBuf != NULL)
                        {
                            // gCalling = TRUE;

                            p = strstr(pCallBuf, "Digits");
                            DEBUG(__FILENAME__, "p: %s\r\n", p);
                            if(p != NULL)
                            {
                                len = strlen(p);
                                for(i = 0; i < len; i++) 
                                {
                                    if(p[i] == ':')
                                    {
                                        tok = &p[i+2]; // lay sdt bo so 0 o dau
                                        DEBUG(__FILENAME__, "[%d] tok: %s\r\n", __LINE__, tok);
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                p = strstr(pCallBuf, "+CLCC:"); // +CLCC: 1,1,4,0,0,"0xxxxxxxxx",129,""
                                                                // +CLCC: 1,1,4,0,0,"84xxxxxxxxx",145,""
                                len = strlen(p);
                                for(i = 0; i < len; i++)
                                {
                                    if(p[i] == '"')
                                    {
                                        if(p[i+1] == '0') 
                                        {
                                            tok = &p[i+2]; // lay sdt bo so 0 o dau
                                        }
                                        else if((p[i+1] == '8') && (p[i+2] == '4')) 
                                        {
                                            tok = &p[i+3]; // lay sdt bo 84
                                        }
                                        
                                        break;
                                    }
                                }
                                for(i = 0; i < strlen(tok); i++)
                                {
                                    if(tok[i] == '"')
                                    {
                                        tok[i] = 0;
                                        break;
                                    }
                                }
                                DEBUG(__FILENAME__, "[%d] tok: %s\r\n", __LINE__, tok);
                            }
                            memset(phoneBuf, 0, MAX_OA_SIZE);
                            sAPI_Sprintf(phoneBuf, "%s%s", REGION, tok);
                            DEBUG(__FILENAME__, "Incomming => %s\r\n", phoneBuf); // ban tin nay chi xuat hien 1 lan khi co cuoc goi den
                        }
                        break;
                    }
                    case SC_URC_DTMF_IND:
                    {
                        UINT8 *pDtmfBuf = (UINT8*)msg.arg3;
                        if(pDtmfBuf != NULL)
                        {
                            // DEBUG(__FILENAME__, "SC_URC_DTMF_IND: %s\r\n", pDtmfBuf);
                        }
                        break;
                    }

                }
                break;
            }
            #endif

            case SC_URC_SIM_MASK:
            {
                // DEBUG(__FILENAME__, "CPIN process!!\r\n");
                switch(msg.arg2)
                {
                    case SC_URC_CPIN_READY:
                        /*add usercode here*/
                        DEBUG(__FILENAME__, "CPIN ready!!\r\n");
                        #if (SMS_EN)
                        // DEBUG(__FILENAME__, "sAPI_SmsSetFormat(1) [%d]\r\n", sAPI_SmsSetFormat(1));
                        DEBUG(__FILENAME__, "sAPI_SmsSetNewMsgInd (Flash SMS) [%d]\r\n", sAPI_SmsSetNewMsgInd(1,1,2,0,0,0));
                        // sms_SendNewMsg("+84385xxxxxx", "Hello", 5);
                        // sms_SendNewMsg("191", "HDCALL", 6);
                        #endif
                        break;
                    case SC_URC_CPIN_REMOVED:
                        /*add usercode here*/
                        DEBUG(__FILENAME__, "CPIN Removed!!\r\n");

                        break;
                    default:
                        break;
                }
                break;
            }

//             case SC_URC_NETSTATUE_MASK:
//             {
//                 // DEBUG(__FILENAME__, "NET process!!\r\n");
//                 switch(msg.arg2)
//                 {
//                     case SC_URC_NETACTED:
//                     {
//                         Netbuf = (UINT8*)msg.arg3;
//                         // DEBUG(__FILENAME__, "Net act: %s\r\n", Netbuf);
//                         /*add usercode here*/
//                         xSystemData.networkActive = NETWORK_ACTIVE;
//                         break;
//                     }
//                     case SC_URC_NETDIS:
//                     {
//                         Netbuf = (UINT8*)msg.arg3;
//                         // DEBUG(__FILENAME__, "Net dis :%s\r\n", Netbuf);
//                         /*add usercode here*/
//                         xSystemData.networkActive = NETWORK_DEACTIVE;
//                         break;
//                     }
//                     case SC_URC_PDP_ACTIVE:
//                     {
//                         Netbuf = (UINT8*)msg.arg3;
//                         // DEBUG(__FILENAME__, "PDP act: %s\r\n", Netbuf);
//                         /*add usercode here*/
// //                        sms_SendNewMsg("+84385108103", "DEVICE: CONNECTED", strlen("DEVICE: CONNECTED"));
//                         break;
//                     }   
//                     case SC_URC_PDP_DEACT:
//                     {
//                         UINT8 *p_cid = (UINT8*)(msg.arg3);
//                         // DEBUG(__FILENAME__, "PDP deact, cid[%d]\r\n", *p_cid);
//                         break;
//                     }
                        
//                     default:
//                         break;
//                 }
//                 break;
//             }
            #if (1)
            case SC_URC_INTERNAL_AT_RESP_MASK:
            {
                UINT8 *respstr = (UINT8*)msg.arg3;
                if((msg.arg2 != 0) && (respstr != NULL))
                {
                    // DEBUG(__FILENAME__, "Internal AT Rsp: len [%d], str %s\r\n", msg.arg2, respstr);      

                    #if (CALL_EN)
                    if(strstr((const char*)respstr, "+CALLDISCONNECT"))
                    {
                        DEBUG(__FILENAME__, "Call disconnect\r\n");
                    }
                    #endif
                }
                break;
            }
            #endif
        }

        sAPI_Free(msg.arg3);
        msg.arg3 = NULL;
    }
}
