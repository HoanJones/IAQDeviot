/*
 * i2c.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "i2c.h"
#include "debug.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TEST_I2C_TASK 0

#if(TEST_I2C_TASK == 1)
#define I2C_TASK_STACK_SIZE    (1024 * 2)
#define I2C_TASK_PRIORITY      (150)
#endif
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */
SC_I2C_DEV i2cDev = {.i2c_channel = SC_I2C_CHANNEL0, .i2c_clock = SC_I2C_STANDARD_MODE};


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static UINT8 slave_add = (0X20<<1);
static UINT8 i2c_index = 0;

#if(TEST_I2C_TASK == 1)
static sTaskRef I2CTask = NULL;
static UINT8 I2CTaskStack[I2C_TASK_STACK_SIZE];
#endif
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
#if(TEST_I2C_TASK == 1)
static void Task_I2CProcess(void *ptr);
#endif


/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void i2c_init(void)
{
    DEBUG(__FILENAME__, "_sAPI_I2CConfigInit [%d]\r\n", sAPI_I2CConfigInit(&i2cDev));
#if(TEST_I2C_TASK == 1)
    if(I2CTask == NULL)
    {
        if(sAPI_TaskCreate(&I2CTask, I2CTaskStack, I2C_TASK_STACK_SIZE, I2C_TASK_PRIORITY, (char*)"I2C process", Task_I2CProcess, (void *)0) != SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
        }
        else DEBUG(__FILENAME__, "Task Create ok\r\n");
    }
#endif
}

UINT8 i2c_write(UINT8 reg, UINT8 dat)
{
    UINT8 ret;
    ret = sAPI_I2CWriteEx(i2c_index, slave_add, reg, &dat, 1);
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
#if(TEST_I2C_TASK == 1)
static void Task_I2CProcess(void *ptr)
{

    while(1)
    {      
        DEBUG(__FILENAME__, "i2c_write(0, 0xAA) [%d]\r\n", i2c_write(0, 0xAA));
        sAPI_TaskSleep(200);
    }
}
#endif