/*
 * gpio.h
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

#ifndef _GPIO_H_
#define _GPIO_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */

// #define BUTTON          2  // Boot pin
// #define BUTTON_READ     sAPI_GpioGetValue(BUTTON)

#define NETLIGHT_PIN       SC_MODULE_GPIO_09 // NETLIGHT

// #define STATUS_PIN         SC_MODULE_GPIO_10 // STATUS

#define FLASH_CS_PIN       SC_MODULE_GPIO_12 // IO1
#define FLASH_CS_HIG       sAPI_GpioSetValue(FLASH_CS_PIN, 1)
#define FLASH_CS_LOW       sAPI_GpioSetValue(FLASH_CS_PIN, 0)

#define RF_CS_PIN          SC_MODULE_GPIO_10 // IO2
#define RF_CS_HIG          sAPI_GpioSetValue(RF_CS_PIN, 1)
#define RF_CS_LOW          sAPI_GpioSetValue(RF_CS_PIN, 0)

#define A76xx_D1           SC_MODULE_GPIO_00
#define A76xx_D1_HIG       sAPI_GpioSetValue(A76xx_D1, 1)
#define A76xx_D1_LOW       sAPI_GpioSetValue(A76xx_D1, 0)

#define A76xx_D4           SC_MODULE_GPIO_14
#define A76xx_D4_HIG       sAPI_GpioSetValue(A76xx_D4, 1)
#define A76xx_D4_LOW       sAPI_GpioSetValue(A76xx_D4, 0)

#define A76xx_D5           SC_MODULE_GPIO_13
#define A76xx_D5_HIG       sAPI_GpioSetValue(A76xx_D5, 1)
#define A76xx_D5_LOW       sAPI_GpioSetValue(A76xx_D5, 0)

#define A76xx_D6           SC_MODULE_GPIO_12
#define A76xx_D6_HIG       sAPI_GpioSetValue(A76xx_D6, 1)
#define A76xx_D6_LOW       sAPI_GpioSetValue(A76xx_D6, 0)

#define A76xx_D7           SC_MODULE_GPIO_10
#define A76xx_D7_HIG       sAPI_GpioSetValue(A76xx_D7, 1)
#define A76xx_D7_LOW       sAPI_GpioSetValue(A76xx_D7, 0)

// #define ACC_IRQ         14
// #define ACC_IRQ_READ    sAPI_GpioGetValue(ACC_IRQ)
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */





/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void gpio_Init(void);
#endif
#ifdef __cplusplus
}
#endif
