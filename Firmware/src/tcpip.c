/*
 * tcpip.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "simcom_tcpip.h"
#include "simcom_network.h"
#include "tcpip.h"
#include "debug.h"
// #include "fota.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[TCPIP]"

#define TCPIP_TASK_STACK_SIZE    (1024 * 5)
#define TCPIP_TASK_PRIORITY      (150)

typedef enum
{
	TCP_DEFINE_PDP = 0, 
	TCP_CONFIG_TIMEOUT,
	TCP_NETOPEN,
	TCP_INQUIRY_IPADD,
	TCP_CIPOPEN,
	TCP_CIPCLOSE,
	TCP_NETCLOSE,
    TCP_CONNECT,
    TCP_DISCONNECT,
} Tcp_Step_t;

#define MAX_PDP_IDX 8
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */
sFlagRef tcpipFlags;


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static sTaskRef tcpipTask = NULL;
static UINT8 tcpipTaskStack[TCPIP_TASK_STACK_SIZE];

static UINT8 pdpActiveFlag[MAX_PDP_IDX];
static INT32 clientSocket[MAX_PDP_IDX] = {-1};
static UINT8 isClientConnected[MAX_PDP_IDX] = {TCP_STT_DISCONNECTED};
static TCP_Params_t TCP_Params[MAX_PDP_IDX];
static volatile INT32 rxClientSkId = -1;
// UINT8 pdp_id;
static struct SCipInfo localIpInfo = {TCPIP_PDP_IPV4, 0, {0}};
// UINT8 resp[100] = {0}; 
static TCP_Manager_t TCP_Manager = {.connectStatus = TCP_STT_DISCONNECTED};

static TCP_Save_Last lastParams[2];
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void Task_TcpipManager(void *ptr);
static void TCPIP_CallbackFunc(int s, int evt, uint16_t len);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void tcpip_Init(void)
{
    SC_STATUS ret;
    ret = sAPI_FlagCreate(&tcpipFlags);
    DEBUG(__FILENAME__, "tcpipFlags init ret [%d]\r\n", ret);

    if(sAPI_TaskCreate(&tcpipTask, tcpipTaskStack, TCPIP_TASK_STACK_SIZE, TCPIP_TASK_PRIORITY, (char*)"TCPIP process", Task_TcpipManager, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
    }
    else DEBUG(__FILENAME__, "Task Create ok\r\n");
    return;
}

void tcpip_SwitchState(UINT8 state)
{
    switch(state)
    {
        case TCP_DEFINE_PDP: DEBUG(__FILENAME__, "switch: TCP_DEFINE_PDP\r\n");break;
        case TCP_CIPOPEN: DEBUG(__FILENAME__, "switch: TCP_CIPOPEN\r\n");break;
        case TCP_CIPCLOSE: DEBUG(__FILENAME__, "switch: TCP_CIPCLOSE\r\n");break;
        case TCP_CONNECT: DEBUG(__FILENAME__, "switch: TCP_CONNECT\r\n");break;
        case TCP_DISCONNECT: DEBUG(__FILENAME__, "switch: TCP_DISCONNECT\r\n");break;
    }

    TCP_Manager.state = state;
    // sAPI_FlagSet(sysFlag, FLAG_MASK_TCPIP, SC_FLAG_OR);
}

UINT8 tcpip_definePDP(UINT8 pdpId)
{
    INT8 *s;
    if(sAPI_TcpipPdpActive(pdpId, 1) == SC_TCPIP_SUCCESS)
    {
        DEBUG(__FILENAME__, "Define PDP ok\r\n");
        pdpActiveFlag[pdpId] = 1;

        if(sAPI_TcpipGetSocketPdpAddr(pdpId, 1, &localIpInfo) != SC_TCPIP_SUCCESS)
        {
            DEBUG(__FILENAME__, "PDP get addr err\r\n");
            sAPI_TcpipPdpDeactive(pdpId, 1);
            DEBUG(__FILENAME__, "Init fail\r\n");
            return SC_TCPIP_FAIL;
        }
        DEBUG(__FILENAME__, "Getsocket ok\r\n");

        if(localIpInfo.type == TCPIP_PDP_IPV4)
        {
            s = sAPI_TcpipInet_ntoa(localIpInfo.ip4);
            DEBUG(__FILENAME__, "Addr: %s\r\n", s);
        }
        return SC_TCPIP_SUCCESS;
    }
    else DEBUG(__FILENAME__, "Define PDP err\r\n");

    return SC_TCPIP_FAIL;
}

#define MAX_RETRY 50
UINT8 tcpip_clientOpen(INT8 *ipAdd, UINT16 port, UINT8 pdpId)
{
    SChostent *host_entry = NULL;
    SCsockAddrIn server;
    UINT8 cntRetry = 0;
    UINT8 csq, cfun;
    int cgReg;
    SCcpsiParm cpsi;
    UINT8 ip[4];

    while(cntRetry < MAX_RETRY)
    {
        DEBUG(__FILENAME__, "Retry connect [%d]\r\n", cntRetry);
        sAPI_NetworkGetCfun(&cfun);
        DEBUG(__FILENAME__, "cfun %d\r\n", cfun);
        if(cfun != 1) sAPI_NetworkSetCfun(1);         
        sAPI_NetworkGetCgreg(&cgReg);
        DEBUG(__FILENAME__, "cgReg %d\r\n", cgReg);
        sAPI_NetworkGetCsq(&csq);
        sAPI_NetworkGetCpsi(&cpsi);
        DEBUG(__FILENAME__, "System mode: %s CSQ: %d\r\n", cpsi.networkmode, csq);
        if((cgReg != 1) || (strstr(cpsi.networkmode, "No Service")))
        {
            sAPI_TaskSleep(600); // sleep 3s
            cntRetry++;
            if(cntRetry == MAX_RETRY) 
            {
                DEBUG(__FILENAME__, "Pls check SIM card\r\n");
                return SC_TCPIP_FAIL;
            }
        }
        else
        {
            if(csq < 32)
            {
                break;
            }
        }
    }

    if(tcpip_definePDP(pdpId) != SC_TCPIP_SUCCESS) return SC_TCPIP_FAIL;

    if(pdpActiveFlag[pdpId] == 1)
    {
        memset(TCP_Params[pdpId].ipAdd, 0, sizeof(TCP_Params[pdpId].ipAdd));
        memcpy(TCP_Params[pdpId].ipAdd, ipAdd, strlen((const char *)ipAdd));
        TCP_Params[pdpId].port = port;

        DEBUG(__FILENAME__, "IP Add: %s\r\n", ipAdd);
        host_entry = sAPI_TcpipGethostbyname(ipAdd);
        if(host_entry == NULL)
        {
            DEBUG(__FILENAME__, "Server IP add err\r\n");
            return SC_TCPIP_FAIL;
        }
        ip[0] = host_entry->h_addr_list[0][0]&0xff;
        ip[1] = host_entry->h_addr_list[0][1]&0xff;
        ip[2] = host_entry->h_addr_list[0][2]&0xff;
        ip[3] = host_entry->h_addr_list[0][3]&0xff;
        DEBUG(__FILENAME__, "Get Host ok, IP: %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);

        // clientSocket[pdpId] = sAPI_TcpipSocket(SC_AF_INET, SC_SOCK_STREAM, 0);
        clientSocket[pdpId] = sAPI_TcpipSocket_with_callback(SC_AF_INET, SC_SOCK_STREAM, 0, TCPIP_CallbackFunc);
        if(clientSocket[pdpId] < 0) 
        {
            DEBUG(__FILENAME__, "Create socket err\r\n");
            return SC_TCPIP_FAIL;
        }
        DEBUG(__FILENAME__, "Create socket ok\r\n");

        server.sin_family = SC_AF_INET;
        server.sin_port = sAPI_TcpipHtons(port);
        server.sin_addr.s_addr = *(UINT32 *)host_entry->h_addr_list[0];

        if(sAPI_TcpipConnect(clientSocket[pdpId], (SCsockAddr*)&server, sizeof(SCsockAddr)) != SC_TCPIP_SUCCESS)
        {
            DEBUG(__FILENAME__, "Connect server fail\r\n");   
            return SC_TCPIP_FAIL;
        }
        DEBUG(__FILENAME__, "Connect server ok\r\n");  
        isClientConnected[pdpId] = TCP_STT_CONNECTED;
        
        char buf[100] = {0};
        char len;
        len = sprintf(buf, "SocketID[%d]: Connected\r\n", (int)clientSocket[pdpId]);
        tcpip_send(buf, len, pdpId);

        lastParams[0].pdpId = pdpId;
        lastParams[0].params.port = port;
        memcpy(lastParams[0].params.ipAdd, TCP_Params[pdpId].ipAdd, strlen((const char *)TCP_Params[pdpId].ipAdd));

        return SC_TCPIP_SUCCESS;
    }
    return SC_TCPIP_FAIL;
}

UINT8 tcpip_send(const void *data, INT32 size, UINT8 pdpId)
{
    INT32 ret;
    if(isClientConnected[pdpId] == TCP_STT_CONNECTED)
    {
        ret = sAPI_TcpipSend(clientSocket[pdpId], data, size, SC_MSG_WAITALL);
        if(ret < 0)
        {
            DEBUG(__FILENAME__, "Client send fail !\r\n");
        }
        else
        {
            DEBUG(__FILENAME__, "Client send ok !\r\n");
        }
    }
    else DEBUG(__FILENAME__, "Pls open the client [%d]\r\n", pdpId);

    return SC_TCPIP_SUCCESS;
}

UINT8 tcpip_receive(void *data, INT32 size, UINT8 pdpId)
{
    INT32 ret;
    if(isClientConnected[pdpId] == TCP_STT_CONNECTED)
    {
        ret = sAPI_TcpipRecv(clientSocket[pdpId], data, size, SC_MSG_WAITALL);
        if(ret < 0)
        {
            DEBUG(__FILENAME__, "Client rec fail !\r\n");
        }
        else
        {
            DEBUG(__FILENAME__, "RX data: %s\r\n", (char*)data);
        }     
    }

    return SC_TCPIP_SUCCESS;
}

void tcp_setParams(char *input, int len)
{
    char *msg_token = input;
    char *msg_field[3];
    uint8_t countfield = 0;
    UINT16 port;
    UINT8 pdpId;
    msg_field[0] = msg_token;

    while(*msg_token != '\0')
    {
        if(*msg_token == ':')
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

    port = atoi(msg_field[1]);
    pdpId = atoi(msg_field[2]);
    DEBUG(__FILENAME__, "set: %s - %d - %d\r\n", msg_field[0], port, pdpId);

    tcpip_clientOpen((INT8*)msg_field[0], port, pdpId);
}

/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
static char rxTcpBuf[1024];
/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
static void Task_TcpipManager(void *ptr)
{
    SC_STATUS status;
    UINT32 flags = 0;

    char buf[100] = {0};
    char len;

    while(1)
    {
        status = sAPI_FlagWait(tcpipFlags, FLAG_MASK_TCP_RX, SC_FLAG_OR_CLEAR, &flags, SC_SUSPEND);
        if(status == SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "rxClientSkId [%d]\r\n", rxClientSkId);
            if(rxClientSkId == clientSocket[1])
            {
                tcpip_receive(rxTcpBuf, 1024, 1);
                len = sprintf(buf, "SocketID[%d]: Data --> ok\r\n", (int)clientSocket[1]);
                tcpip_send(buf, len, 1);
            }
            else if(rxClientSkId == clientSocket[2])
            {
                tcpip_receive(rxTcpBuf, 1024, 2);
                len = sprintf(buf, "SocketID[%d]: Data --> ok\r\n", (int)clientSocket[2]);
                tcpip_send(buf, len, 2);
            }
            // if(strstr(rxTcpBuf, "FOTA_UPDATE"))
            // {
            //     sAPI_FlagSet(sysFlag, FLAG_MASK_FOTA, SC_FLAG_OR); 
            // }
        }
    }
}

static void TCPIP_CallbackFunc(int s, int evt, uint16_t len)
{
    // DEBUG(__FILENAME__, "Tcp/ip [%d], [%d], [%d]\r\n", s, evt, len);
    switch(evt)
    {
        case NETCONN_EVT_CONNECTED:
        {
            // DEBUG(__FILENAME__, "Connected\r\n");
            break;
        }
        case NETCONN_EVT_RCVPLUS:
        {         
            if(len > 0) 
            {
                // DEBUG(__FILENAME__, "Data is comming\r\n");
                rxClientSkId = s;
                sAPI_FlagSet(tcpipFlags, FLAG_MASK_TCP_RX, SC_FLAG_OR); 
            }
            break;
        }
        case NETCONN_EVT_CLOSE_WAIT:
        {
            DEBUG(__FILENAME__, "Connection close\r\n");
            // tcpip_clientOpen((INT8*)lastParams[0].params.ipAdd, lastParams[0].params.port, lastParams[0].pdpId);
            break;
        }
    } 
}
