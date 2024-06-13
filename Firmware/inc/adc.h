/*
 * adc.h
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

#ifndef _ADC_H_
#define _ADC_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
#define R5        4700 // oHm
#define R40       1000 // oHm
#define RSUM      (R5+R40) 

#define ADC1_READ (sAPI_ReadAdc(0)*RSUM/R40)
#define ADC2_READ sAPI_ReadVbat()   

typedef enum
{
    A76xx_ADC1,
    A76xx_ADC2,
} A76xx_ADC_Channel_E;
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void ADC_Init(void);

#endif
#ifdef __cplusplus
}
#endif
