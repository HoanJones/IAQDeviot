/*
 * rtc.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "rtc.h"
#include "debug.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "\r\n--------------->[RTC]"

#define RTC_TASK_STACK_SIZE    (1024 * 1)
#define RTC_TASK_PRIORITY      (150)
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */



/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static t_rtc sRTC;
static sTaskRef RTCTask = NULL;
static UINT8 RTCTaskStack[RTC_TASK_STACK_SIZE];
static int sec = 0;
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void Task_RTCManager(void *ptr);
static void s_alarm_callback(void);



/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
/* 1970 <= year < 2070 */
void RTC_setDateTime(int hour, int min, int sec, int weekDay, int monthDay, int month, int year)
{
    int ret;
    t_rtc RTC;

    RTC.tm_hour = hour;
    RTC.tm_min = min;
    RTC.tm_sec = sec;
    RTC.tm_wday = weekDay;
    RTC.tm_mday = monthDay;
    RTC.tm_mon = month;
    RTC.tm_year = year;
    ret = sAPI_SetRealTimeClock(&RTC);
    if(ret < 0)
        DEBUG(__FILENAME__, "Set date time failed ret [%d] !\r\n", ret);
    else
        DEBUG(__FILENAME__, "Set date time successed !\r\n");
}

void RTC_getDateTime(t_rtc *rtc)
{
    sAPI_GetRealTimeClock(rtc);
}

void RTC_setTime(int hour, int min, int sec)
{
    int ret;
    t_rtc rtcTemp;

    sAPI_GetRealTimeClock(&rtcTemp);
    rtcTemp.tm_hour = hour;
    rtcTemp.tm_min = min;
    rtcTemp.tm_sec = sec;

    ret = sAPI_SetRealTimeClock(&rtcTemp);
    if(ret < 0)
        DEBUG(__FILENAME__, "Set time failed ret [%d] !\r\n", ret);
    else
        DEBUG(__FILENAME__, "Set time successed !\r\n");
}

void RTC_setAlarm(int hour, int min, int sec, int weekDay, int monthDay, int month, int year, AlCallback cbFunc)
{
    t_rtc RTC;

    RTC.tm_hour = hour;
    RTC.tm_min = min;
    RTC.tm_sec = sec;
    RTC.tm_wday = weekDay;
    RTC.tm_mday = monthDay;
    RTC.tm_mon = month;
    RTC.tm_year = year;

    sAPI_RtcSetAlarm(&RTC);
    sAPI_RtcEnableAlarm(1);
    sAPI_RtcRegisterCB(cbFunc);
}

void RTC_cancelAlarm(void)
{
    sAPI_RtcEnableAlarm(0);
    DEBUG(__FILENAME__, "Cancel alarm\r\n");
}

void RTC_Init(void)
{
    // RTC_setDateTime(11, 30, sec, 7, 4, 12, 2021);
    // sec += 5;
    RTC_setAlarm(11, 30, sec, 7, 4, 12, 2021, s_alarm_callback);

    if(sAPI_TaskCreate(&RTCTask, RTCTaskStack, RTC_TASK_STACK_SIZE, RTC_TASK_PRIORITY, (char*)"RTC process", Task_RTCManager, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
    }
    else DEBUG(__FILENAME__, "Task Create ok\r\n");

}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
static void Task_RTCManager(void *ptr)
{
    uint8_t oldSec = 0;

    sAPI_TaskSleep(3000/5);

    RTC_getDateTime(&sRTC);
    oldSec = sRTC.tm_sec;

    while(1)
    {
        sAPI_TaskSleep(200/5);
        RTC_getDateTime(&sRTC);
        if(oldSec != sRTC.tm_sec)
        {
            oldSec = sRTC.tm_sec;
            DEBUG(__FILENAME__, "%d:%d:%d\r\n", sRTC.tm_hour, sRTC.tm_min, sRTC.tm_sec);
        }
    }
}

static void s_alarm_callback(void)
{
    DEBUG(__FILENAME__, "ALARM\r\n");

    sec += 5;
    RTC_setAlarm(11, 30, sec, 7, 4, 12, 2021, s_alarm_callback);
    if(sec >= 30) RTC_cancelAlarm();
}
