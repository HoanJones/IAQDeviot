/*
 * defineType.h
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

#ifndef _DEFINETYPE_H_
#define _DEFINETYPE_H_

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include <string.h>
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
typedef enum
{
    CMD_OK = 0,	
	CMD_ERR,
    CMD_NOT_SUPPORT,
	CMD_MSG_FIELD_ERR,
} ERR_E;

typedef struct
{
	char Data[1024];
	char Byte;
	uint16_t Index;
	uint8_t State;
} buffer_t;
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */
#define TEST_DIF_TIME   (0)
// #define UART_EN       (1)
#define VPC_EN          (0)
#define GPIO_EN         (0)
#define TIMER_EN        (0)
#define RTC_EN          (0)
#define GNSS_EN         (0)
#define RTC_EN          (0)
#define SMS_EN          (0)
#define CALL_EN         (0)
#define MQTT_EN         (0)
#define TCPIP_EN        (0)
#define I2C_EN          (0)
#define SPI_EN          (0)
#define FOTA_EN			(0)
#define FLASH_EN		(0)
#define HTTP_EN			(0)
#define WTD_EN			(0)
#define ADC_EN			(0)
#define DHT_EN 0
#define SDS_EN 0
#define MQ_EN 0

/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */

#endif
#ifdef __cplusplus
}
#endif
