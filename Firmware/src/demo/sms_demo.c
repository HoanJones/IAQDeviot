#include "simcom_api.h"
#include "simcom_application.h"

extern void PrintfResp(INT8 *format);

extern struct ConfigStruct g_conf;
extern INT8 mqtt_debug;
extern sMsgQRef misc_msgq;
sMsgQRef g_sms_demo_urc_process_msgQ;
sMsgQRef g_sms_demo_msgQ;
sMsgQRef g_sms_demo_urc_rsp_msgQ;
INT8 lastnb[13] = {0};

void sms_init()
{
    sAPI_MsgQCreate(&g_sms_demo_msgQ, "g_sms_demo_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);

    /*msgQ prepared for urc processor*/
    if (g_sms_demo_urc_process_msgQ == NULL)
        sAPI_MsgQCreate(&g_sms_demo_urc_process_msgQ, "g_sms_demo_urc_process_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);

    /*msgQ for urc process response*/
    sAPI_MsgQCreate(&g_sms_demo_urc_rsp_msgQ, "g_sms_demo_urc_rsp_msgQ", sizeof(SIM_MSG_T), 4, SC_FIFO);
}

void sms_process(INT8 *msg)
{
    int pos = 0;
    while (pos < strlen(msg) - 2)
    {
        if (msg[pos] == '+' && msg[pos + 1] == '8' && msg[pos + 2] == '4')
        {
            memset(lastnb, 0, 13);
            memcpy(lastnb, msg + pos, 12);
        }
        if (msg[pos] == '\r' || msg[pos] == '\n')
        {
            process_conf(msg + pos + 1, 2);
            break;
        }
        pos++;
    }
}

void send_sms(INT8 *payload)
{
    PrintfResp(lastnb);
    if (lastnb[0] == '+')
    {
        SC_SMSReturnCode ret = SC_SMS_SUCESS;
        SIM_MSG_T msg;
        ret = sAPI_SmsSendMsg(1, payload, strlen(payload), lastnb, misc_msgq);
        if (ret == SC_SMS_SUCESS)
        {
            INT8 rsp_buff[200] = {0};
            memset(&msg, 0, sizeof(msg));
            sAPI_MsgQRecv(misc_msgq, &msg, SC_SUSPEND);
            sAPI_Debug("[sms], sAPI_SmsSendMsg, primID[%d] resultCode[%d],rspStr[%s]", msg.arg1, msg.arg2, msg.arg3);
            sprintf(rsp_buff, "\r\n\r\nsAPI_SmsSendMsg:\r\n\tprimID[%d] \r\n\tresultCode[%d]\r\n\trspStr[%s]\r\n", msg.arg1, msg.arg2, (INT8 *)msg.arg3);
            PrintfResp(rsp_buff);
            sAPI_Free(msg.arg3);
        }
    }
}

void check_sms()
{
    char *rsp_buff;
    UINT32 nbMsg = 0;
    SC_SMSReturnCode ret = SC_SMS_SUCESS;
    SC_STATUS status;
    SIM_MSG_T SmsUrcProcessReq;
    SIM_MSG_T msg_rsp;

    memset(&SmsUrcProcessReq, 0, sizeof(SIM_MSG_T));
    sAPI_MsgQPoll(g_sms_demo_urc_process_msgQ, &nbMsg);
    if (nbMsg == 0)
        return;
    else
        PrintfResp("have sms msg\n");
    status = sAPI_MsgQRecv(g_sms_demo_urc_process_msgQ, &SmsUrcProcessReq, 200);
    if (status == SC_SUCCESS)
    {
        /*operations related to a valid cmd type*/
        switch (SmsUrcProcessReq.msg_id)
        {
        case SC_URC_NEW_MSG_IND:
        {
            ret = sAPI_SmsReadMsg(1, SmsUrcProcessReq.arg1, g_sms_demo_urc_rsp_msgQ);
            if (ret == SC_SMS_SUCESS)
            {
                memset(&msg_rsp, 0, sizeof(msg_rsp));
                sAPI_MsgQRecv(g_sms_demo_urc_rsp_msgQ, &msg_rsp, 200);
                sms_process((INT8 *)msg_rsp.arg3);
                if (mqtt_debug)
                {
                    rsp_buff = sAPI_Malloc(400);
                    sprintf(rsp_buff, "primID[%d] resultCode[%d] rspStr[%s]\r\n", msg_rsp.arg1, msg_rsp.arg2, (INT8 *)msg_rsp.arg3);
                    PrintfResp((UINT8 *)rsp_buff); /*show msg to uart*/
                    sAPI_Free(rsp_buff);
                }
                /*must free this response buffer*/
                sAPI_Free(msg_rsp.arg3);
            }
            else
            {
                PrintfResp("SMS read failed!\r\n");
            }
            break;
        }
        case SC_URC_FLASH_MSG:
        {
            /*recv new text msg string here, if sAPI_SmsSetNewMsgInd(1,1,2,0,0,0) has been set when booting*/
            rsp_buff = sAPI_Malloc(400);
            sprintf(rsp_buff, "%s", (INT8 *)SmsUrcProcessReq.arg3);
            PrintfResp((UINT8 *)rsp_buff); /*show msg to uart*/
            sAPI_Free(SmsUrcProcessReq.arg3);
            sAPI_Free(rsp_buff);
            break;
        }
        }
    }
}