#include "simcom_api.h"
#include "blynkled.h"
#include "gpio.h"
#include "debug.h"

#define TAG "[BLYNK_LED]"
#define TEST_BLYNKLED_TASK 1
#define TEST_OUTPUT 1

#if(TEST_BLYNKLED_TASK == 1)
static sTaskRef BlynlLedTask = NULL;
static UINT8 BlynkLEDTaskStack[1024*3];
static void Task_BlynkLEDProcess(void *ptr);

#endif

void Led_init(void) {
    
    SC_GPIOReturnCode ret;
    SC_GPIOConfiguration gpioCfg;
    /* Config output mode go here */
    gpioCfg.pinDir = SC_GPIO_OUT_PIN;
    gpioCfg.initLv = 0;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_NO_EDGE;
    gpioCfg.isr = NULL;
    gpioCfg.wu = NULL;

    ret = sAPI_GpioConfig(A76xx_D1, gpioCfg);
    DEBUG(__FILENAME__, "Gpio num[%d] config ret [%d]\r\n", A76xx_D1, ret);
    A76xx_D1_HIG;

#if(TEST_BLYNKLED_TASK == 1)
    if(BlynlLedTask == NULL)
    {
        if(sAPI_TaskCreate(&BlynlLedTask, BlynkLEDTaskStack, 1024 * 3, 150, (char*)"BlynkLed process", Task_BlynkLEDProcess, (void *)0) != SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "BlynkLed Task error!\n");
        }
        else DEBUG(__FILENAME__, "BlynkLed Task Create ok\r\n");
    }
#endif
}


#if(TEST_BLYNKLED_TASK == 1)
void Task_BlynkLEDProcess(void* ptr) {
    UINT8 sTick1 = 0;
    unsigned int value1 = 0;
    while(1) {
        #if(TEST_OUTPUT)
            // UINT8 sTick1 = 0;
            // unsigned int value1 = 0;
            sTick1++;
            if(sTick1 >= 1) // 500ms
            {
                sAPI_GpioSetValue(A76xx_D1, value1);
                DEBUG(__FILENAME__, "A76xx_D1: %d \r\n", value1);
                DEBUG(__FILENAME__, "After 3 seconds \r\n");
                value1 = !value1;
                sTick1 = 0;
            }
            sAPI_DelayUs(1000*10000); //5000ms = 5s
        #endif
    }
}
#endif