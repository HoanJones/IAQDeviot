/*
 * spi.h
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

#ifndef _SPI_H_
#define _SPI_H_


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
// void spi_Init(UINT8 spiIndex, SC_SPI_MODE mode);
// void spi_SendByte(UINT8 spiIndex, UINT8 data);
// UINT8 spi_ReadByte(UINT8 spiIndex);
// void spi_SendMulti(UINT8 spiIndex, UINT8 *p, UINT32 len);
// void spi_ReceiveMulti(UINT8 spiIndex, UINT8 *p, UINT32 len);
// void spi_SendRev(UINT8 spiIndex, UINT8 *datSend, UINT32 sendLen, UINT8 *datRev, UINT32 revLen);

void spi_Init(SC_SPI_MODE mode);
void spi_SendByte(UINT8 data);
UINT8 spi_ReadByte(void);
void spi_SendMulti(UINT8 *p, UINT32 len);
void spi_ReceiveMulti(UINT8 *p, UINT32 len);
void spi_SendRev(UINT8 *datSend, UINT32 sendLen, UINT8 *datRev, UINT32 revLen);
#endif
#ifdef __cplusplus
}
#endif
