/*
 * network.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "network.h"
#include "debug.h"
#include "defineType.h"
#include "mqtt.h"
// #include "systemFlag.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[NETWORK]" 

#define LOG_NETWORK_INFOR          (1)
#define NETWORK_TASK_STACK_SIZE    (1024 * 3)
#define NETWORK_TASK_PRIORITY      (150)
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
#if(LOG_NETWORK_INFOR)
static sTaskRef networkTask = NULL;
static UINT8 networkTaskStack[NETWORK_TASK_STACK_SIZE];
#endif
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
#if(LOG_NETWORK_INFOR)
static void Task_NetworkManager(void *ptr);
#endif

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void network_Init(void)
{
    UINT8 cfun, sysmode = CNMP_MODE_LTE;
    int ret;

    sAPI_NetworkInit();

    sAPI_NetworkGetCfun(&cfun);
    DEBUG(__FILENAME__, "cfun %d\r\n", cfun);
    if(cfun != 1)
        DEBUG(__FILENAME__, "sAPI_NetworkSetCfun(1), ret: %d\r\n", sAPI_NetworkSetCfun(1));
    DEBUG(__FILENAME__, "Set System Mode(%d), ret: %d\r\n", sysmode, sAPI_NetworkSetCnmp(sysmode));

    // sAPI_AtSend((UINT8*)"AT+COPS=2\r", strlen("AT+COPS=2\r"));
    // sAPI_TaskSleep(1000/5);
    // sAPI_AtSend((UINT8*)"AT+CTZU=1\r", strlen("AT+CTZU=1\r"));
    // sAPI_TaskSleep(1000/5);
    // sAPI_AtSend((UINT8*)"AT+COPS=0\r", strlen("AT+COPS=0\r"));
    // sAPI_TaskSleep(1000/5);

#if(LOG_NETWORK_INFOR)
    if(sAPI_TaskCreate(&networkTask, networkTaskStack, NETWORK_TASK_STACK_SIZE, NETWORK_TASK_PRIORITY, (char*)"Network process", Task_NetworkManager, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
    }
    else DEBUG(__FILENAME__, "Task Create ok\r\n");
#endif
}

/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
#if(LOG_NETWORK_INFOR)
static void Task_NetworkManager(void *ptr)
{
    UINT8 csq;
    // int creg;
    int cgreg;
    SCcpsiParm cpsi;

    while(1)
    {
        sAPI_NetworkGetCsq(&csq);
        DEBUG(__FILENAME__, "sAPI_NetworkGetCsq [%d]\r\n", csq);
        // sAPI_NetworkGetCreg(&creg);
        // DEBUG(__FILENAME__, "sAPI_NetworkGetCreg [%d]\r\n", creg);
        sAPI_NetworkGetCgreg(&cgreg);
        DEBUG(__FILENAME__, "sAPI_NetworkGetCgreg [%d]\r\n", cgreg);

        sAPI_NetworkGetCpsi(&cpsi);
        DEBUG(__FILENAME__, "\r\n======================\r\nnetworkmode [%s], \r\nLTEBandStr [%s], \
                        \r\nGSMBandStr [%s], \r\nCellID [%d], \r\nRssi[%d], \r\nMnc_Mcc[%s]\r\n======================\r\n",
            cpsi.networkmode, cpsi.LTEBandStr, cpsi.GSMBandStr, cpsi.CellID, cpsi.Rssi, cpsi.Mnc_Mcc);

        sAPI_TaskSleep(1000);
    }
}
#endif
