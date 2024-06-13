/*
 * gpio.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "../inc/gpio.h"
#include "debug.h"
// #include "systemFlag.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[GPIO]"

#define TEST_GPIO_TASK 1

#if(TEST_GPIO_TASK == 1)
#define GPIO_TASK_STACK_SIZE    (1024 * 3)
#define GPIO_TASK_PRIORITY      (150)

#define TEST_OUTPUT 0
#define TEST_INPUT  (!TEST_OUTPUT)

#define FLAG_MASK_BTN1 0x00000001
#define FLAG_MASK_BTN2 0x00000002
#define FLAG_MASK_BTN3 0x00000004
#define FLAG_MASK_BTN4 0x00000008
#define FLAG_MASK_BTN5 0x00000010
#define FLAG_MASK_BTN  (FLAG_MASK_BTN1 | FLAG_MASK_BTN2 | FLAG_MASK_BTN3 | FLAG_MASK_BTN4 | FLAG_MASK_BTN5)

#endif
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */
sFlagRef gpioFlags;

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
#if(TEST_GPIO_TASK == 1)
static sTaskRef gpioTask = NULL;
static UINT8 gpioTaskStack[GPIO_TASK_STACK_SIZE];
#endif
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
#if(TEST_GPIO_TASK == 1)
static void Task_GpioProcess(void *ptr);

#if(TEST_INPUT)
static void buttonISR1(void);
static void buttonISR2(void);
static void buttonISR3(void);
static void buttonISR4(void);
static void buttonISR5(void);
#endif

#endif
/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void gpio_Init(void)
{
    SC_GPIOReturnCode ret;
    SC_GPIOConfiguration gpioCfg;
    /* Config output mode go here */
    gpioCfg.pinDir = SC_GPIO_OUT_PIN;
    gpioCfg.initLv = 0;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_NO_EDGE;
    gpioCfg.isr = NULL;
    gpioCfg.wu = NULL;

    ret = sAPI_GpioConfig(NETLIGHT_PIN, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", NETLIGHT_PIN, ret);

    // ret = sAPI_GpioConfig(STATUS_PIN, gpioCfg);
    // DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", STATUS_PIN, ret);

    ret = sAPI_GpioConfig(FLASH_CS_PIN, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", FLASH_CS_PIN, ret);
    FLASH_CS_HIG;

    ret = sAPI_GpioConfig(RF_CS_PIN, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", RF_CS_PIN, ret);
    RF_CS_HIG;
    
    #if(TEST_OUTPUT)
    gpioCfg.pinDir = SC_GPIO_OUT_PIN;
    gpioCfg.initLv = 0;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_NO_EDGE;
    gpioCfg.isr = NULL;
    gpioCfg.wu = NULL;

    ret = sAPI_GpioConfig(A76xx_D1, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D1, ret);
    A76xx_D1_HIG;

    ret = sAPI_GpioConfig(A76xx_D4, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D4, ret);
    A76xx_D4_HIG;

    ret = sAPI_GpioConfig(A76xx_D5, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D5, ret);
    A76xx_D5_HIG;

    ret = sAPI_GpioConfig(A76xx_D6, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D6, ret);
    A76xx_D6_HIG;

    ret = sAPI_GpioConfig(A76xx_D7, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D7, ret);
    A76xx_D7_HIG;
    #endif

    #if(TEST_INPUT)

    SC_STATUS retFlag;
    retFlag = sAPI_FlagCreate(&gpioFlags);
    DEBUG(__FILENAME__, "gpioFlags init ret [%d]\r\n", retFlag);

    /* Config input mode go here */
    gpioCfg.pinDir = SC_GPIO_IN_PIN;
    gpioCfg.initLv = 1;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_FALL_EDGE;
    gpioCfg.isr = buttonISR1;
    gpioCfg.wu = NULL;
    ret = sAPI_GpioConfig(A76xx_D1, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D1, ret);

    gpioCfg.pinDir = SC_GPIO_IN_PIN;
    gpioCfg.initLv = 1;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_FALL_EDGE;
    gpioCfg.isr = buttonISR2;
    gpioCfg.wu = NULL;
    ret = sAPI_GpioConfig(A76xx_D4, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D4, ret);

    gpioCfg.pinDir = SC_GPIO_IN_PIN;
    gpioCfg.initLv = 1;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_FALL_EDGE;
    gpioCfg.isr = buttonISR3;
    gpioCfg.wu = NULL;
    ret = sAPI_GpioConfig(A76xx_D5, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D5, ret);

    gpioCfg.pinDir = SC_GPIO_IN_PIN;
    gpioCfg.initLv = 1;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_FALL_EDGE;
    gpioCfg.isr = buttonISR4;
    gpioCfg.wu = NULL;
    ret = sAPI_GpioConfig(A76xx_D6, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D6, ret);

    gpioCfg.pinDir = SC_GPIO_IN_PIN;
    gpioCfg.initLv = 1;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_FALL_EDGE;
    gpioCfg.isr = buttonISR5;
    gpioCfg.wu = NULL;
    ret = sAPI_GpioConfig(A76xx_D7, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D7, ret);
    #endif

#if(TEST_GPIO_TASK == 1)
    if(gpioTask == NULL)
    {
        if(sAPI_TaskCreate(&gpioTask, gpioTaskStack, GPIO_TASK_STACK_SIZE, GPIO_TASK_PRIORITY, (char*)"GPIO process", Task_GpioProcess, (void *)0) != SC_SUCCESS)
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
#if(TEST_GPIO_TASK == 1)
static void Task_GpioProcess(void *ptr)
{
#if(TEST_OUTPUT)
    UINT8 sTick1 = 0, sTick2 = 0, sTick3 = 0, sTick4 = 0, sTick5 = 0;
    unsigned int value1 = 0, value2 = 0, value3 = 0, value4 = 0, value5 = 0;
#endif

#if(TEST_INPUT)
    SC_STATUS status;
    UINT32 flags = 0;
#endif
    while(1)
    {      
#if(TEST_OUTPUT)
        sTick1++;
        sTick2++;
        sTick3++;
        sTick4++;
        sTick5++;
        if(sTick1 >= 1) // 500ms
        {
            sAPI_GpioSetValue(A76xx_D1, value1);
            value1 = !value1;
            sTick1 = 0;
        }

        if(sTick2 >= 2) // 1000ms
        {
            sAPI_GpioSetValue(A76xx_D4, value2);
            value2 = !value2;
            sTick2 = 0;
        }

        if(sTick3 >= 4) // 2000ms
        {
            sAPI_GpioSetValue(A76xx_D5, value3);
            value3 = !value3;
            sTick3 = 0;
        }

        if(sTick4 >= 8) // 4000ms
        {
            sAPI_GpioSetValue(A76xx_D6, value4);
            value4 = !value4;
            sTick4 = 0;
        }

        if(sTick5 >= 16) // 8000ms
        {
            sAPI_GpioSetValue(A76xx_D7, value5);
            value5 = !value5;
            sTick5 = 0;
        }
        sAPI_TaskSleep(100);
#endif

#if(TEST_INPUT)
        status = sAPI_FlagWait(gpioFlags, FLAG_MASK_BTN, SC_FLAG_OR_CLEAR, &flags, SC_SUSPEND);
        if(status == SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "flags GPIO interupt [%x]\r\n", flags);
            flags = 0;
            // switch(flags)
            // {
            //     case FLAG_MASK_BTN1:
            //     {

            //         break;
            //     }

            //     default: flags = 0; break;
            // }
        }
#endif
    }
}
#endif

#if(TEST_INPUT)
static void buttonISR1(void)
{
    /* User code go here */
#if(TEST_GPIO_TASK == 1)
    sAPI_FlagSet(gpioFlags, FLAG_MASK_BTN1, SC_FLAG_OR);   
#endif
}

static void buttonISR2(void)
{
    /* User code go here */
#if(TEST_GPIO_TASK == 1)
    sAPI_FlagSet(gpioFlags, FLAG_MASK_BTN2, SC_FLAG_OR);   
#endif
}

static void buttonISR3(void)
{
    /* User code go here */
#if(TEST_GPIO_TASK == 1)
    sAPI_FlagSet(gpioFlags, FLAG_MASK_BTN3, SC_FLAG_OR);   
#endif
}

static void buttonISR4(void)
{
    /* User code go here */
#if(TEST_GPIO_TASK == 1)
    sAPI_FlagSet(gpioFlags, FLAG_MASK_BTN4, SC_FLAG_OR);   
#endif
}

static void buttonISR5(void)
{
    /* User code go here */
#if(TEST_GPIO_TASK == 1)
    sAPI_FlagSet(gpioFlags, FLAG_MASK_BTN5, SC_FLAG_OR);   
#endif
}
#endif
