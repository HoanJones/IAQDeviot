/*
 * adc.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "adc.h"
#include "../inc/debug.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TEST_ADC_TASK 1

#if(TEST_ADC_TASK == 1)
#define ADC_TASK_STACK_SIZE    (1024 * 2)
#define ADC_TASK_PRIORITY      (150)
#endif
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */



/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
#if(TEST_ADC_TASK == 1)
static sTaskRef ADCTask = NULL;
static UINT8 ADCTaskStack[ADC_TASK_STACK_SIZE];
#endif
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
#if(TEST_ADC_TASK == 1)
static void Task_ADCProcess(void *ptr);
#endif


/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void ADC_Init(void)
{
#if(TEST_ADC_TASK == 1)
    if(ADCTask == NULL)
    {
        if(sAPI_TaskCreate(&ADCTask, ADCTaskStack, ADC_TASK_STACK_SIZE, ADC_TASK_PRIORITY, (char*)"ADC process", Task_ADCProcess, (void *)0) != SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
        }
        else DEBUG(__FILENAME__, "Task Create ok\r\n");
    }
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
#if(TEST_ADC_TASK == 1)
static void Task_ADCProcess(void *ptr)
{

    while(1)
    {      
        // DEBUG(__FILENAME__, "Read ADC1: %d\r\n", ADC1_READ);
        
        DEBUG(__FILENAME__, "Read ADC2: %d\r\n", ADC2_READ);
        sAPI_TaskSleep(1000/5);
    }
}
#endif
