/*
 * debug.h
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

#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <string.h>
//#include "uart.h"
#include "virtualCom.h"
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
#define USE_DEBUG_UART_PORT 0

#if(USE_DEBUG_UART_PORT)
#define DEBUG(TAG, format, ...) uartUser_Debug(TAG, format, ##__VA_ARGS__)
#define LOG_NMEA(format, ...) _Debug(format, ##__VA_ARGS__)
#else
#define DEBUG(TAG, format, ...) virtualCom_Debug(TAG, format, ##__VA_ARGS__)
#define LOG_NMEA(format, ...) usb_Debug(format, ##__VA_ARGS__)
#endif

#define __FILENAME__    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define A7672 (1)
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */

#endif
#ifdef __cplusplus
}
#endif
