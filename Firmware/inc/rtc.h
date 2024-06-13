/*
 * rtc.h
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
 *
 *      
*/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef RTC_H
#define RTC_H


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */


/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void RTC_setDateTime(int hour, int min, int sec, int weekDay, int monthDay, int month, int year);
void RTC_getDateTime(t_rtc *rtc);
void RTC_setTime(int hour, int min, int sec);
void RTC_setAlarm(int hour, int min, int sec, int weekDay, int monthDay, int month, int year, AlCallback cbFunc);
void RTC_cancelAlarm(void);
void RTC_Init(void);
#endif
#ifdef __cplusplus
}
#endif
