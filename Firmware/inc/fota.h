/*
 * fota.h
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

#ifndef _FOTA_UPDATE_H_
#define _FOTA_UPDATE_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
#define FLAG_MASK_FOTA_KERNEL 	0x00000040
#define FLAG_MASK_FOTA_APP  	0x00000080
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */
typedef struct 
{
	char targetVer[10];
	char approveVer[10];
	char forceVer[10];
	char urlUpdate[100];
} fotaParams_t;

extern fotaParams_t fotaParams;
extern sFlagRef otaFlags;
/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void FOTA_Init(void);
SCAppDwonLoadReturnCode FOTA_DownloadCusApp(void);
void FOTA_appUpdate(void);
#endif
#ifdef __cplusplus
}
#endif
