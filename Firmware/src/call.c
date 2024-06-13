/*
 * call.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "call.h"
#include "debug.h"
#include "sms.h"
#include "gpio.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[CALL]"

#define CALL_TASK_STACK_SIZE    (1024 * 5)
#define CALL_TASK_PRIORITY      (150)

#define CALL_URC_RECIVE_TIME_OUT    2000 // 10s

#define CALL_FLAG_MASK          (0x00000001)
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */
sMsgQRef callUrcMesQ;
sFlagRef callFlag;
callManager_t callManager = {.callMode = CALL_MODE_IDLE};

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static sTaskRef callTask = NULL;
static UINT8 callTaskStack[CALL_TASK_STACK_SIZE];

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void taskCall_Process(void *ptr);
static SC_CALLReturnCode Call_Dial(void);
static SC_CALLReturnCode Call_End(void);
static BOOL Call_SendUssdCode(void);    
/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void call_Init(void)
{
    if(sAPI_TaskCreate(&callTask, callTaskStack, CALL_TASK_STACK_SIZE, CALL_TASK_PRIORITY, (char*)"Call process", taskCall_Process, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
    }
    else DEBUG(__FILENAME__, "Task Create ok\r\n");
}

SC_STATUS call_SwitchMode(callMode_t mode, char *phoneNumber)
{
    SC_STATUS status = SC_FAIL;
    if(callFlag != NULL)
    {
        memset(callManager.phoneNumber, 0, MAX_OA_SIZE);
        sAPI_Sprintf(callManager.phoneNumber, "%s", phoneNumber);
        callManager.callMode = mode;
        status = sAPI_FlagSet(callFlag, CALL_FLAG_MASK, SC_FLAG_OR); 
    }
    return status;
}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
static void taskCall_Process(void *ptr)
{
    SC_CALLReturnCode ret;
    SC_STATUS status;
    UINT32 flags = 0;

    status = sAPI_MsgQCreate(&callUrcMesQ, "callUrcMesQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
    if(status != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "callUrcMesQ creat err!\r\n");
    }
    else DEBUG(__FILENAME__, "callUrcMesQ creat ok!\r\n");

    ret = sAPI_FlagCreate(&callFlag);
    DEBUG(__FILENAME__, "callFlag init ret [%d]\r\n", ret);

    while(1)
    {
        status = sAPI_FlagWait(callFlag, CALL_FLAG_MASK, SC_FLAG_OR_CLEAR, &flags, SC_SUSPEND);
        DEBUG(__FILENAME__, "Status %d, mode %d\r\n", status, callManager.callMode);
        switch(callManager.callMode)
        {
            case CALL_MODE_IDLE:
            {
                break;
            }

            case CALL_MODE_DIAL:
            {
                Call_Dial();
                break;
            }

            case CALL_MODE_ANSWER:
            {
                break;
            }

            case CALL_MODE_END:
            {
                if(Call_End() == SC_CALL_SUCESS)
                {
                    // if(btnSttRegUser == BTN_STT_REQUESTING_REG_USER)
                    // {
                    //     btnSttRegUser = BTN_STT_REG_USER_CANCEL;
                    //     sms_ReqRegUserFromCallCmd(callManager.phoneNumber);
                    // }
                    // else sms_SendMsgFromCallCmd(callManager.phoneNumber);
                }
                break;
            }

            case CALL_MODE_GET_STATE:
            {
                break;
            }

            case CALL_MODE_AUTO_ANSWER:
            {
                break;
            }

            case CALL_MODE_REQ_USSD:
            {
                Call_SendUssdCode();
                break;
            }
        }

        callManager.callMode = CALL_MODE_IDLE;
        memset(callManager.phoneNumber, 0, MAX_OA_SIZE);
    }
}

static SC_CALLReturnCode Call_Dial(void)
{
    SC_CALLReturnCode ret = SC_CALL_SUCESS;
    SIM_MSG_T msg;
    SC_STATUS status;
    // INT8 rsp_buff[100];
    // SIM_MSG_T optionMsg ={0,0,0,NULL};  

    ret = sAPI_CallDialMsg((UINT8*)callManager.phoneNumber, callUrcMesQ);
    if(ret == SC_CALL_SUCESS)
    {
    memset(&msg,0,sizeof(msg));
    status = sAPI_MsgQRecv(callUrcMesQ, &msg, CALL_URC_RECIVE_TIME_OUT);
    DEBUG(__FILENAME__, "sAPI_CallDialMsg: resultCode %d, status %d\r\n", msg.arg2, status);
    }

    return ret;
}

static SC_CALLReturnCode Call_End(void)
{
    SC_CALLReturnCode ret = SC_CALL_SUCESS;
    SIM_MSG_T msg;
    SC_STATUS status;
    uint8_t i;

    // sAPI_TaskSleep(2000/5);

    for(i = 0; i < 3; i++)
    {
        ret = sAPI_CallEndMsg(callUrcMesQ);
        if(ret == SC_CALL_SUCESS)
        {
            memset(&msg,0,sizeof(msg));
            status = sAPI_MsgQRecv(callUrcMesQ, &msg, CALL_URC_RECIVE_TIME_OUT);
            DEBUG(__FILENAME__, "sAPI_CallEndMsg: resultCode [%d], status %d\r\n", msg.arg2, status);
            break;
        }
    }

    return ret;
}

static BOOL Call_SendUssdCode(void)
{
    BOOL ret = FALSE;
    char buf[1024];
    uint32_t len;

    memset(buf, 0, sizeof(buf));
    len = sAPI_Sprintf(buf, "AT+CUSD=1,\"%s\",15\r\n", callManager.phoneNumber);
    DEBUG(__FILENAME__, "Ussd code: %s", buf);
    ret = sAPI_AtSend(buf, len);
        
    return ret;
}
