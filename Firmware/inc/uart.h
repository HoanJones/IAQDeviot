/*
 * uart.h
 *
 *  Created on: Nov 01, 2020
 *      Author: chungnt@epi-tech.com.vn
 *
 *      
*/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _UART_H_
#define _UART_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "simcom_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */
#define UART_DATA_LEN           128 // max 1024

typedef struct
{
    uint8_t data[UART_DATA_LEN];
    uint8_t flag;
    int len;
} uartTypedef_t2;

#define CONGIF_PORT SC_UART
#define DEBUG_PORT SC_UART2
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* Definition of public (external) data types go here */



/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/* Function prototypes for public (external) functions go here */
void uartUser_Init(SC_Uart_Port_Number port, SC_UART_BaudRates baudrate, SC_UART_WordLen dataLen, SC_UART_ParityTBits parity, SC_UART_StopBits stopbit);
void uartUser_Debug(const char *tag, const char *format,...);
void _Debug(const char *format,...);
#endif
#ifdef __cplusplus
}
#endif

