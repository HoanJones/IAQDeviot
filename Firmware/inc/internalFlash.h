/*
 * internalFlash.h
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

#ifndef _INTERNAL_FLASH_H_
#define _INTERNAL_FLASH_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
#define INTERNAL_FLASH_MIN_SECTOR 0
#define INTERNAL_FLASH_MAX_SECTOR 7

typedef enum
{
    FLASH_ERR_TYPE_NONE = 0,
    FLASH_ERR_TYPE_OUTOFSIZE,
    FLASH_ERR_TYPE_ERASE,
    FLASH_ERR_TYPE_WRITE,
    FLASH_ERR_TYPE_READ,
} flashErrorType_E;

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */


/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
flashErrorType_E internalFlash_write(UINT8 sector, void *tx_DATA, UINT32 len);
flashErrorType_E internalFlash_read(UINT8 sector, void *rx_DATA, UINT32 len);
void internalFlash_Test(void);
#endif
#ifdef __cplusplus
}
#endif
