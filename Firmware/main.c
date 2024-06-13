/*
 * sc_application.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "simcom_api.h"
#include "simcom_application.h"
//#include "defineType.h"
#include "debug.h"
// #include "uart.h"
// #include "../inc/gpio.h"
// #include "timer.h"
#include "urcProcess.h"
// #include "network.h"
// #include "sms.h"
// #include "call.h"
// #include "mqtt.h"
// #include "tcpip.h"
// #include "i2c.h"
// #include "i2c_lcd.h"
// #include "spi.h"
// #include "fota.h"
// #include "internalFlash.h"
// #include "http.h"
// #include "watchdog.h"
// #include "..\inc\adc.h"
// #include "dht11.h"
// #include "blynkled.h"
//#include "sds011.h"
//#include "simcom_uart.h"
// #include "mq135.h"
// #include "simcom_application.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[MAIN]"

#define LOGO "\
\r\n/*==========================================================*/ \r\n  \
      ___       _______      _______    _______    _______     \r\n \
     /   \\     |_____  |   |  _____|  |_____  |  |_____  |   \r\n \
    / /_\\ \\         / /   | |_____        / /    _____| |  \r\n \
   / _____ \\       / /     |  ___  |      / /    |  _____|   \r\n \
  / /     \\ \\     | |     | |___| |     | |    | |_____   \r\n \
 /_/       \\_\\    |_|     |_______|     |_|    |_______|  \r\n \
/*==========================================================*/\r\n"

#define HEADER "\
\r\nRead Data_Logger in the Air\r\n\
SIMcom Air Logger By Pham Van Hoan\r\n"
#define version 1.0
/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
/* 1tick = 5ms */

extern void initTask();

typedef void (*app_t)(void * argv);
typedef struct
{
	app_t app_entry;

} appRegItem_t;
#define _appRegTable_attr_ __attribute__((unused, section(".appRegTable")))

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void Application(void * argv)
{
  /* Do not comment the initialization code below */
  unsigned long *apiTable = (unsigned long*)argv;
  if(apiTable != NULL)
  {
      get_sAPI(apiTable);
      sAPI_Debug("sc_Init successfull...");

  }
  /*----------------------------------------------*/

  sAPI_TaskSleep(200);

  //uartUser_Init(DEBUG_PORT, SC_UART_BAUD_115200, SC_UART_WORD_LEN_8, SC_UART_NO_PARITY_BITS, SC_UART_ONE_STOP_BIT);  // SC_UART2 dung cho debug nen can init truoc
#if (VPC_EN)
  virtualCom_Init();
#endif
  DEBUG(__FILENAME__, "%s", HEADER);
  DEBUG(__FILENAME__, "Air Logger version %0.1f\r\n", version);
  // DEBUG(__FILENAME__, "sAPI_GetPowerUpEvent [%d]\r\n", sAPI_GetPowerUpEvent());
  // DEBUG(__FILENAME__, "sAPI_GetPowerDownEvent [%d]\r\n", sAPI_GetPowerDownEvent());

  //urcProcess_Init();

  //network_Init();
  sAPP_UrcTask();
  initTask();

#if (GPIO_EN)
  gpio_Init();
#endif

#if (TIMER_EN)
  timer_Init();
#endif

#if (RTC_EN)
  RTC_Init();
#endif

#if (SMS_EN)
  sms_Init();
#endif

#if (CALL_EN)
  call_Init();
#endif

#if (MQTT_EN)
  mqtt_Init();
#endif

#if (TCPIP_EN)
  tcpip_Init();
#endif

#if (I2C_EN)
  i2c_init();
  i2cLCD_init();
#endif

#if (SPI_EN)
  spi_Init(SPI_MODE_PH0_PO0);
#endif

#if (FOTA_EN)
  FOTA_Init();
#endif

#if (FLASH_EN)
  internalFlash_Test();
#endif

#if (HTTP_EN)
  http_Init();
#endif

#if (WTD_EN)
  WTD_Init();
#endif

#if (ADC_EN)
  ADC_Init();
#endif

}

appRegItem_t openSDK_entry _appRegTable_attr_ = {.app_entry = Application};
