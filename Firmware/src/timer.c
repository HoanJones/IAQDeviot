/*
 * timer.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "timer.h"
#include "debug.h"
#include "../inc/gpio.h"
#include "defineType.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[TIMER]"

#define TIMER1_PERIOD 0xAA
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */



/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static sTimerRef timerRef;
static UINT8 vLed1 = 0;
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void timer_callback(UINT32 arg)
{
    if(arg == TIMER1_PERIOD) 
    {
#if (GPIO_EN)
        sAPI_GpioSetValue(NETLIGHT_PIN, vLed1);
        vLed1 = !vLed1;
#endif
    }
}


/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void timer_Init(void)
{
    SC_STATUS status;
    status = sAPI_TimerCreate(&timerRef);
    if(status != 0) 
    {
        DEBUG(__FILENAME__, "Timer create err [%d]\r\n", status);
        return;
    }

    DEBUG(__FILENAME__, "Timer create ok\r\n");

    status = sAPI_TimerStart(timerRef, 10, 100, timer_callback, TIMER1_PERIOD); // 200 means 200*5ms = 1s
    DEBUG(__FILENAME__, "sAPI_TimerStart [%d]\r\n", status);
}

/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/