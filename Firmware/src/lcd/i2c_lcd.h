/*
 * i2c_lcd.h
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

#ifndef _I2C_i2cLCD_H_
#define _I2C_i2cLCD_H_


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
void i2cLCD_init (void);   // initialize lcd
void i2cLCD_send_string (char *str);  // send string to the lcd
void i2cLCD_clear_display (void);	//clear display lcd
void i2cLCD_goto_XY (int row, int col); //set proper location on screen

#endif
#ifdef __cplusplus
}
#endif
