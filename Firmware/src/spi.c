/*
 * spi.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "spi.h"
#include "debug.h"
#include "gpio.h"
#include "defineType.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[SPI]"


#define TEST_SPI_TASK   1
#define TEST_SPI_ARR    0

#if(TEST_SPI_TASK == 1)
#define SPI_TASK_STACK_SIZE    (1024 * 2)
#define SPI_TASK_PRIORITY      (150)
#endif
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */
static SC_SPI_DEV spiPort; 

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
#if(TEST_SPI_TASK == 1)
static sTaskRef SPITask = NULL;
static UINT8 SPITaskStack[SPI_TASK_STACK_SIZE];
#endif

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
#if(TEST_SPI_TASK == 1)
static void Task_SPIProcess(void *ptr);
#endif


/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */


/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
void spi_Init(SC_SPI_MODE mode)
{
    SC_SPI_ReturnCode ret;

    spiPort.clock = SPI_CLOCK_1625KHz;
    spiPort.mode = mode;
    spiPort.csMode = CUS_CS_MODE;
    spiPort.index = SC_SPI_CHANNEL0;
    ret = sAPI_SpiConfigInitEx(&spiPort);

    DEBUG(__FILENAME__, "Init Ret [%d]\r\n", ret);

#if(TEST_SPI_TASK == 1)
    if(SPITask == NULL)
    {
        if(sAPI_TaskCreate(&SPITask, SPITaskStack, SPI_TASK_STACK_SIZE, SPI_TASK_PRIORITY, (char*)"SPI process", Task_SPIProcess, (void *)0) != SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
        }
        else DEBUG(__FILENAME__, "Task Create ok\r\n");
    }
#endif
}

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
void spi_SendByte(UINT8 data)
{
    SC_SPI_ReturnCode ret;

    ret = sAPI_SpiWriteBytesEx(&spiPort, &data, 1);

    DEBUG(__FILENAME__, "Write Ret [%d]\r\n", ret);
}

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
UINT8 spi_ReadByte(void)
{
    UINT8 data, temp;
    SC_SPI_ReturnCode ret;

    ret = sAPI_SpiReadBytesEx(&spiPort, &temp, 1, &data, 1);

    DEBUG(__FILENAME__, "Read Ret [%d]\r\n", ret);

    return data;
}

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
void spi_SendMulti(UINT8 *p, UINT32 len)
{
    SC_SPI_ReturnCode ret;

    ret = sAPI_SpiWriteBytesEx(&spiPort, p, len);

    DEBUG(__FILENAME__, "Write Ret [%d]\r\n", ret);
}

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
void spi_ReceiveMulti(UINT8 *p, UINT32 len)
{
    SC_SPI_ReturnCode ret;

    ret = sAPI_SpiReadBytesEx(&spiPort, NULL, 0, p, len);

    DEBUG(__FILENAME__, "Read Ret [%d]\r\n", ret);
}

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
void spi_SendRev(UINT8 *datSend, UINT32 sendLen, UINT8 *datRev, UINT32 revLen)
{
    SC_SPI_ReturnCode ret;

    ret = sAPI_SpiReadBytesEx(&spiPort, datSend, sendLen, datRev, revLen);

    DEBUG(__FILENAME__, "Read Ret [%d]\r\n", ret);
}

/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
#if(TEST_SPI_TASK == 1)
static void Task_SPIProcess(void *ptr)
{
    #if(TEST_SPI_ARR == 1)
    UINT8 spiDataTest[5] = {0x12, 0x15, 0x44, 0x58, 0xAA};
    #endif
    while(1)
    {      
#if (GPIO_EN)
        #if(TEST_SPI_ARR == 1)
        FLASH_CS_LOW;
        spi_SendMulti(spiDataTest, 5);
        FLASH_CS_HIG;
        sAPI_TaskSleep(200);       
        #else
        FLASH_CS_LOW;
        spi_SendByte(0x38);
        FLASH_CS_HIG;   
        sAPI_TaskSleep(100);
        RF_CS_LOW;
        spi_SendByte(0x15);
        RF_CS_HIG;
        sAPI_TaskSleep(100);
        #endif
#endif       
    }
}
#endif