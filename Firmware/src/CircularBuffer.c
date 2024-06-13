/*
 * CircularBuffer.c
 *
 *  Created on: Sep 20, 2018
 *      Author: chungnguyena1@gmail.com
 *  2021-08-16:	chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "CircularBuffer.h"
#include <math.h>

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */
//circularBuf_t cBuffer;


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */


/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */
static uint8_t checkFull(circularBuf_t *cBuf);
static uint8_t checkEmpty(circularBuf_t *cBuf);
static void put(circularBuf_t *cBuf, void *data);
static void get(circularBuf_t *cBuf, void *data);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */
/*********************************************************************************************************//**
* @brief   : init circular buffer
* @param	 : cBuf: handle cua circular buffer
* @param	 : address: tro toi buffer duoc dung lam circular buffer
* @param	 : sizeOfElm: kich thuoc cua 1 phan tu circular buffer
* @param	 : size: kich thuoc cua circular buffer
 * @retval  
 ************************************************************************************************************/
void CBUFFER_Init(circularBuf_t *cBuf, void *address, uint16_t sizeOfElm, uint16_t size)
{
	cBuf->buf = (uint8_t*)address;
	cBuf->sizeOfElm = sizeOfElm;
	cBuf->size = size;
	cBuf->tail = 0;
	cBuf->head = 0;
	cBuf->count = 0;
}

uint8_t CBUFFER_Put(circularBuf_t *cBuf, void *data)
{
	if(checkFull(cBuf) == 1) return 0;
	put(cBuf, data);
	return 1;
}

uint8_t CBUFFER_Get(circularBuf_t *cBuf, void *data)
{
	if(checkEmpty(cBuf) == 1) return 0;
	get(cBuf, data);
	return 1;
}

void CBUFFER_Reset(circularBuf_t *cBuf)
{
	cBuf->count = cBuf->tail = cBuf->head = 0;
}

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
static uint8_t checkFull(circularBuf_t *cBuf)
{
	if(cBuf->count == cBuf->size) return 1;
	else return 0;
}

static uint8_t checkEmpty(circularBuf_t *cBuf)
{
	if(cBuf->count == 0) return 1;
	else return 0;
}
	
static void put(circularBuf_t *cBuf, void *data)
{
	uint16_t i;
	uint8_t *p = (uint8_t*)&cBuf->buf[cBuf->sizeOfElm*cBuf->head];
	uint8_t *q = (uint8_t*)data;
	
	for(i = 0; i < cBuf->sizeOfElm; i++) p[i] = q[i];

	cBuf->head = (cBuf->head+1) % cBuf->size;
	cBuf->count++;
}

static void get(circularBuf_t *cBuf, void *data)
{
	uint16_t i;
	uint8_t *p = (uint8_t*)data;
	uint8_t *q = (uint8_t*)&cBuf->buf[cBuf->sizeOfElm*cBuf->tail];
	
	for(i = 0; i < cBuf->sizeOfElm; i++) p[i] = q[i];
	
	cBuf->tail = (cBuf->tail+1) % cBuf->size;
	cBuf->count--;
}
