/*
 * CircularBuffer.h
 *
 *  Created on: Sep 20, 2018
 *      Author: chungnguyena1@gmail.com
 *  2021-08-16:	chungnt@epi-tech.com.vn
*/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_



/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "simcom_api.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */
typedef struct
{
	uint8_t *buf;  					// luu dia chi cua mang cBuff can dung
	uint16_t sizeOfElm;			// kich thuoc cua 1 phan tu luu tru
	uint16_t size;          // kich thuoc mang buffer
	uint16_t tail;          // phan tu doc
	uint16_t head;          // phan tu viet
	uint16_t count;         // bien dem so phan tu da ghi
} circularBuf_t;

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* Definition of public (external) data types go here */





/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/* Function prototypes for public (external) functions go here */
void CBUFFER_Init(circularBuf_t *cBuf, void *address, uint16_t sizeOfElm, uint16_t size);
uint8_t CBUFFER_Put(circularBuf_t *cBuf, void *data);
uint8_t CBUFFER_Get(circularBuf_t *cBuf, void *data);
void CBUFFER_Reset(circularBuf_t *cBuf);

#endif
#ifdef __cplusplus
}
#endif
