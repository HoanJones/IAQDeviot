/*
 * http.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "http.h"
#include "debug.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[HTTP]"

#define MAX_PARA_STR_SIZE       256
#define HTTP_TASK_STACK_SIZE    (1024 * 4)
#define HTTP_TASK_PRIORITY      (150)

#define FLAG_MASK_HTTP_SET      0x00000200
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */
sFlagRef httpFlags;

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static struct
{
    char type[MAX_PARA_STR_SIZE];
    char value[MAX_PARA_STR_SIZE];
} http_para;

static sMsgQRef httpsResp_msgq = NULL;

static sTaskRef HTTPTask = NULL;
static UINT8 HTTPTaskStack[HTTP_TASK_STACK_SIZE];

static char urlTest[100] = {0};
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static httpActionData_t httpActionResponse(void);
static void readHttpResponse(http_read_type_t type, int startOffset, int size);
static void headHttpResponse(void);
static void Task_HTTPManager(void *ptr);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void http_Init(void)
{
    SC_STATUS ret;
    ret = sAPI_FlagCreate(&httpFlags);
    DEBUG(__FILENAME__, "httpFlags init ret [%d]\r\n", ret);

    if(sAPI_TaskCreate(&HTTPTask, HTTPTaskStack, HTTP_TASK_STACK_SIZE, HTTP_TASK_PRIORITY, (char*)"HTTP process", Task_HTTPManager, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
    }
    else DEBUG(__FILENAME__, "Task Create ok\r\n");
    return;
}

int http_sessionInit(int channel)
{
    SC_STATUS status;
    SC_HTTP_RETURNCODE ret;

    if(httpsResp_msgq == NULL)
    {
        status = sAPI_MsgQCreate(&httpsResp_msgq, "httpsResp_msgq", sizeof(SIM_MSG_T), 8, SC_FIFO);
        if(status != SC_SUCCESS) 
        {
            DEBUG(__FILENAME__, "sAPI_MsgQCreate err\r\n");
            return 0;
        }
        DEBUG(__FILENAME__, "sAPI_MsgQCreate ok\r\n");
    }

    ret = sAPI_HttpInit(channel, httpsResp_msgq);
    if(ret != SC_HTTPS_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_HttpInit err\r\n");
        return -1;
    }
    DEBUG(__FILENAME__, "sAPI_HttpInit ok\r\n");

    return 1;
}

int http_sessionTerm(void)
{
    SC_HTTP_RETURNCODE ret;
    ret = sAPI_HttpTerm();
    if(ret != SC_HTTPS_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_HttpTerm err\r\n");
        return -1;
    }
    DEBUG(__FILENAME__, "sAPI_HttpTerm ok\r\n");
    return 1;
}

/*********************************************************************//**
 * @brief: set http parameters
 * @param: type - string type: URL, CONNECTTO, RECVTO, CONTECT, ACCEPT, SSLCFG, USERDATA, READMODE
 * @retval  
 * @example: http_Para("URL", "http://abcd.net");
 ************************************************************************/
int http_Para(char *type, char *param)
{
    SC_HTTP_RETURNCODE ret;
    ret = sAPI_HttpPara(type, param);
    if(ret != SC_HTTPS_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_HttpPara err\r\n");
        return -1;
    }
    DEBUG(__FILENAME__, "sAPI_HttpPara ok\r\n");
    return 1;
}

httpActionData_t http_Action(http_action_type_t method)
{
    httpActionData_t actionData;
    SC_HTTP_RETURNCODE ret;
    // DEBUG(__FILENAME__, "Action 1\r\n");
    ret = sAPI_HttpAction(method);
    // DEBUG(__FILENAME__, "Action 2\r\n");
    if(ret != SC_HTTPS_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_HttpAction err\r\n");
        actionData.statusCode = -1;
        return actionData;
    }

    DEBUG(__FILENAME__, "sAPI_HttpAction ok, getting status code\r\n");
    return httpActionResponse();
}

int http_Read(http_read_type_t readType, int startOffset, int size)
{
    SC_HTTP_RETURNCODE ret;
    ret = sAPI_HttpRead(readType, startOffset, size);
    if(ret != SC_HTTPS_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_HttpRead err [%d]\r\n", ret);
        return -1;
    }

    readHttpResponse(readType, startOffset, size);
    DEBUG(__FILENAME__, "sAPI_HttpRead ok\r\n");
    
    return 1;
}

int http_Head(void)
{
    SC_HTTP_RETURNCODE ret;
    ret = sAPI_HttpHead();
    if(ret != SC_HTTPS_SUCCESS)
    {
        DEBUG(__FILENAME__, "http_Head err\r\n");
        return -1;
    }

    headHttpResponse();
    DEBUG(__FILENAME__, "http_Head ok\r\n");
    
    return 1;
}

void http_Demo(void)
{
    httpActionData_t actionData;
    http_sessionInit(1);
    // http_Para("URL", "https://6300-2402-9d80-28d-c089-b8bd-1efe-b95f-c9e1.ngrok-free.app/test.txt");
    http_Para("URL", urlTest);
    actionData = http_Action(HTTP_ACTION_GET);
    if(actionData.statusCode == 200)
    {
        DEBUG(__FILENAME__, "actionData.dataLen: %d\r\n", actionData.dataLen);
        http_Head();
        http_Read(HTTP_READ_TYPE_OFFSET, 0, actionData.dataLen);
        // http_Read(HTTP_READ_TYPE_OFFSET, 0, 5);
        // http_Read(HTTP_READ_TYPE_OFFSET, 0, actionData.dataLen-5);
    }
    http_sessionTerm();
}

void http_setParams(char *input, int len)
{
    char *msg_token = input;
    char *msg_field[3];
    uint8_t countfield = 0;
    char *url;
    char *fileName;
    UINT8 method;
    msg_field[0] = msg_token;

    while(*msg_token != '\0')
    {
        if(*msg_token == ',')
        {
            // terminate string after field separator or end-of-message characters
            *msg_token = '\0';

            // save position of the next token
            countfield++;
            msg_field[countfield] = msg_token + 1;
        }
        msg_token++;
    }

    if(countfield != 2)
    {
        DEBUG(__FILENAME__, "params are not correct\r\n");
        return;
    }

    url = msg_field[0];
    fileName = msg_field[1];
    method = atoi(msg_field[2]);
    memset(urlTest, 0, sizeof(urlTest));
    sprintf(urlTest, "%s/%s", url, fileName);
    DEBUG(__FILENAME__, "set: %s - %d\r\n", urlTest, method);

    sAPI_FlagSet(httpFlags, FLAG_MASK_HTTP_SET, SC_FLAG_OR); 
}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
static httpActionData_t httpActionResponse(void)
{
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    sAPI_MsgQRecv(httpsResp_msgq, &msgQ_data_recv, SC_SUSPEND);

    SChttpApiTrans *sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);
    httpActionData_t actionData;
    actionData.statusCode = sub_data->status_code;
    actionData.dataLen = sub_data->action_content_len;

    DEBUG(__FILENAME__, "Status-code=%d content-length=%d\r\n", sub_data->status_code, sub_data->action_content_len);

    sAPI_Free(sub_data->data);
    sAPI_Free(sub_data);

    return actionData;
}

static void readHttpResponse(http_read_type_t type, int startOffset, int size)
{
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    SChttpApiTrans *sub_data = NULL;

    sAPI_MsgQRecv(httpsResp_msgq, &msgQ_data_recv, SC_SUSPEND);
    sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);

    if (type == 0)
    {
        DEBUG(__FILENAME__, "Content-length=%d\r\n", sub_data->action_content_len);
    }
    else
    {
        sub_data->data[size] = 0;
        DEBUG(__FILENAME__, "Sub data: %s\r\n", (UINT8*)sub_data->data);
        // sAPI_Free(sub_data->data);
    }

    sAPI_Free(sub_data); 
}

static void headHttpResponse(void)
{
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL};
    SChttpApiTrans *sub_data = NULL;

    sAPI_MsgQRecv(httpsResp_msgq, &msgQ_data_recv, SC_SUSPEND);

    sub_data = (SChttpApiTrans *)(msgQ_data_recv.arg3);

    if (sub_data->data != NULL && sub_data->dataLen > 0)
    {
        DEBUG(__FILENAME__, "Sub data: %s, data len: %d\r\n", (UINT8 *)sub_data->data, sub_data->dataLen);
    }

    sAPI_Free(sub_data->data);
    sAPI_Free(sub_data);
}

static void Task_HTTPManager(void *ptr)
{
    SC_STATUS status;
    UINT32 flags = 0;

    while(1)
    {
        status = sAPI_FlagWait(httpFlags, FLAG_MASK_HTTP_SET, SC_FLAG_OR_CLEAR, &flags, SC_SUSPEND);
        if(status == SC_SUCCESS)
        {
            if(flags == FLAG_MASK_HTTP_SET)
            {
                http_Demo();
            }
        }
    }
}
