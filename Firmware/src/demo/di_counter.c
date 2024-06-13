#include "simcom_api.h"

INT32 di_count = 0;
UINT32 tick = 0;
UINT32 tick2 = 0;

void DI1_handler(void)
{
    tick2 = sAPI_GetTicks();
    if (tick2 - tick > 2 && tick2 - tick < 100)
        di_count++;
    tick = tick2;
}

void di_init(void)
{
    sAPI_GpioSetDirection(12, 0); // GPIO_12 input
    sAPI_GpioConfigInterrupt(12, SC_GPIO_TWO_EDGE, DI1_handler);
    sAPI_GpioWakeupEnable(12, SC_GPIO_TWO_EDGE);
}
