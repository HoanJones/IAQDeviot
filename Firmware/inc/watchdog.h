/*
 * watchdog.h
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

#ifndef _WTD_H_
#define _WTD_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
// If the watchdog threshold is 01s and the value is set to 0x00,
// If the watchdog threshold is set to 02s and the value is set to 0x01,
// If the watchdog threshold is set to 04s and the value is set to 0x02,
// If the watchdog threshold is 08s and the value is set to 0x03,
// If the watchdog threshold is set to 16s and the value is set to 0x04,
// If the watchdog threshold is set to 32s and the value is set to 0x05,
// If the watchdog threshold is set to 48s and the value is set to 0x06,
// If the watchdog threshold is set to 64s and the value is set to 0x07,
// If the watchdog threshold is 96s and the value is set to 0x08,
// The watchdog threshold is 128s. If the value is set to 0x09,
// If the watchdog threshold is set to 192s and the value is set to 0x0a,
// If the watchdog threshold is set to 256s and the value is set to 0x0b,
// If the watchdog threshold is set to 384s and the value is set to 0x0c,
// If the watchdog threshold is 512s and the value is set to 0x0d,
// If the watchdog threshold is set to 768s and the value is set to 0x0e,
// If the watchdog threshold is set to 1024s and the value is set to 0x0f

#define WTD_PERIOD_1S       0x00
#define WTD_PERIOD_2S       0x01
#define WTD_PERIOD_4S       0x02
#define WTD_PERIOD_8S       0x03
#define WTD_PERIOD_16S      0x04
#define WTD_PERIOD_32S      0x05
#define WTD_PERIOD_48S      0x06
#define WTD_PERIOD_64S      0x07
#define WTD_PERIOD_96S      0x08
#define WTD_PERIOD_128S     0x09
#define WTD_PERIOD_192S     0x0A
#define WTD_PERIOD_256S     0x0B
#define WTD_PERIOD_384S     0x0C
#define WTD_PERIOD_512S     0x0D
#define WTD_PERIOD_768S     0x0E
#define WTD_PERIOD_1024S    0x0F

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void WTD_Init(void);
int WTD_Enable(unsigned char period);
int WTD_Disable(void);
#endif
#ifdef __cplusplus
}
#endif
