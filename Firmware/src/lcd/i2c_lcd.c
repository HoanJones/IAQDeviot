/*
 * i2c_lcd.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "i2c.h"
#include "i2c_lcd.h"
#include "debug.h"
#include <string.h>
#include "adc.h"
#include "defineType.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define SLAVE_ADDRESS_LCD 	(0X20<<1)

#define TEST_I2C_LCD_TASK 1

#if(TEST_I2C_LCD_TASK == 1)
#define I2C_LCD_TASK_STACK_SIZE    (1024 * 2)
#define I2C_LCD_TASK_PRIORITY      (150)
#endif
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */



/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static int i2c_lcd_channel;

#if(TEST_I2C_LCD_TASK == 1)
static sTaskRef I2C_LCDTask = NULL;
static UINT8 I2C_LCDTaskStack[I2C_LCD_TASK_STACK_SIZE];
#endif
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void i2cLCD_send_cmd (char cmd);
static void i2cLCD_send_data (char data);

#if(TEST_I2C_LCD_TASK == 1)
static void Task_I2C_LCDProcess(void *ptr);
#endif
/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void i2cLCD_init (void)
{
	i2c_lcd_channel = i2cDev.i2c_channel;

	i2cLCD_send_cmd (0x33); /* set 4-bits interface */
	i2cLCD_send_cmd (0x32);
	sAPI_TaskSleep(10);
	i2cLCD_send_cmd (0x28); /* start to set LCD function */
	sAPI_TaskSleep(10);
	i2cLCD_send_cmd (0x01); /* clear display */
	sAPI_TaskSleep(10);
	i2cLCD_send_cmd (0x06); /* set entry mode */
	sAPI_TaskSleep(10);
	i2cLCD_send_cmd (0x0c); /* set display to on */	
	sAPI_TaskSleep(10);
	i2cLCD_send_cmd (0x02); /* move cursor to home and set data address to 0 */
	sAPI_TaskSleep(10);
	i2cLCD_send_cmd (0x80);
	i2cLCD_send_string("A76xx Arduino");

#if(TEST_I2C_LCD_TASK == 1)
    if(I2C_LCDTask == NULL)
    {
        if(sAPI_TaskCreate(&I2C_LCDTask, I2C_LCDTaskStack, I2C_LCD_TASK_STACK_SIZE, I2C_LCD_TASK_PRIORITY, (char*)"I2C LCD process", Task_I2C_LCDProcess, (void *)0) != SC_SUCCESS)
        {
            DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
        }
        else DEBUG(__FILENAME__, "Task Create ok\r\n");
    }
#endif
}

void i2cLCD_send_string (char *str)
{
	while (*str) i2cLCD_send_data (*str++);
}

void i2cLCD_clear_display (void)
{
	i2cLCD_send_cmd (0x01); //clear display
}

void i2cLCD_goto_XY (int row, int col)
{
	uint8_t pos_Addr;
	if(row == 1) 
	{
		pos_Addr = 0x80 + row - 1 + col;
	}
	else
	{
		pos_Addr = 0x80 | (0x40 + col);
	}
	i2cLCD_send_cmd(pos_Addr);
}

/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
static void i2cLCD_send_cmd (char cmd)
{
  	char data_u, data_l;
	uint8_t data_t[4];
	SC_I2C_ReturnCode ret;

	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	ret = sAPI_I2CWrite2(i2c_lcd_channel, SLAVE_ADDRESS_LCD, (UINT8 *)data_t, 4);
	// DEBUG(__FILENAME__, "i2cLCD_send_cmd [%x], ret [%d]\r\n", cmd, ret);
}

static void i2cLCD_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	SC_I2C_ReturnCode ret;

	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=0
	data_t[1] = data_u|0x09;  //en=0, rs=0
	data_t[2] = data_l|0x0D;  //en=1, rs=0
	data_t[3] = data_l|0x09;  //en=0, rs=0
	ret = sAPI_I2CWrite2(i2c_lcd_channel, SLAVE_ADDRESS_LCD, (UINT8 *)data_t, 4);
	// DEBUG(__FILENAME__, "i2cLCD_send_data [%x], ret [%d]\r\n", data, ret);
}

#if(TEST_I2C_LCD_TASK == 1)
static void Task_I2C_LCDProcess(void *ptr)
{
	char stringTest[17];
	int count = 0;
    while(1)
    {      
		memset(stringTest, 0, 17);
		#if (ADC_EN)
		// sprintf(stringTest, "ADC: %d (mv)", ADC2_READ());
		sprintf(stringTest, "ADC: %d (mv)", ADC1_READ);
		#else
		sprintf(stringTest, "Count: %d", count);
		#endif
        i2cLCD_goto_XY(2, 0);
		i2cLCD_send_string(stringTest);
        sAPI_TaskSleep(200);
		count++;
    }
}
#endif
