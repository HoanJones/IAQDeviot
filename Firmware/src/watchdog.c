/*
 * watchdog.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "watchdog.h"
#include "debug.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TEST_WTD_TASK 1

#if(TEST_WTD_TASK == 1)
#define WTD_TASK_STACK_SIZE    (1024 * 2)
#define WTD_TASK_PRIORITY      (150)
#endif
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */



/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
#if(TEST_WTD_TASK == 1)
static sTaskRef WTDTask = NULL;
static UINT8 WTDTaskStack[WTD_TASK_STACK_SIZE];
#endif
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
#if(TEST_WTD_TASK == 1)
static void Task_WTDProcess(void *ptr);
#endif


/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void WTD_Init(void)
{
#if(TEST_WTD_TASK == 1)
    WTD_Enable(WTD_PERIOD_4S);

    if(WTDTask == NULL)
    {
        if(sAPI_TaskCreate(&WTDTask, WTDTaskStack, WTD_TASK_STACK_SIZE, WTD_TASK_PRIORITY, (char*)"WTD process", Task_WTDProcess, (void *)0) != SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
        }
        else DEBUG(__FILENAME__, "Task Create ok\r\n");
    }
#endif
}

int WTD_Enable(unsigned char period)
{
    int ret = 0;
    ret = sAPI_SetWtdTimeOutPeriod(period);
    ret |= sAPI_FalutWakeEnable(1);
    ret |= sAPI_SoftWtdEnable(1);
    ret |= sAPI_FeedWtd();

    DEBUG(__FILENAME__, "WTD_Enable [%d]\r\n", ret);
    return ret;
}

int WTD_Disable(void)
{
    int ret = 0;
    ret |= sAPI_FalutWakeEnable(0);
    ret |= sAPI_SoftWtdEnable(0);

    DEBUG(__FILENAME__, "WTD_Disable [%d]\r\n", ret);
    return ret;
}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
#if(TEST_WTD_TASK == 1)
static void Task_WTDProcess(void *ptr)
{

    while(1)
    {      
        DEBUG(__FILENAME__, "sAPI_FeedWtd: [%d]\r\n", sAPI_FeedWtd());
        sAPI_TaskSleep(1000/5);
    }
}
#endif
