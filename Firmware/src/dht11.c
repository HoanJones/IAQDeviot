#include "simcom_api.h"
#include "simcom_gpio.h"
#include "../inc/gpio.h"
#include "simcom_os.h"
#include "dht11.h"
#include "debug.h"
#include "defineType.h"
//chan data cua DHT11 se ket noi voi mot SC_GPIO_00 nao do

#define TAG "[DHT11_Sensor]"
int pinDHT; //Chan du lieu cua DHT11 duoc noi voi D1
uint8_t I_RH, D_RH, I_Temp, D_Temp, CheckSum;

// float temp = 0;
// float humdity = 0;


#define LOG_DHT11_INFOR 1
#define DHT11_RESPONE 1
#define DHT11_REQUEST 1
#define DHT11_RECEIVE 1
#define TEST_OUTPUT 1
#define TEST_INPUT 1

static sTaskRef dhtTask = NULL;
static UINT8 dhtTaskStack[3*1024];
uint8_t temp = 0;
uint8_t humdity = 0;

// #if(LOG_DHT11_INFOR == 1)
// static void readData_DHT11(void* ptr);
// #endif

//Input mode 

void inputMode(void) {

    //SC_GPIOReturnCode ret;
    SC_GPIOConfiguration gpioCfg;
    /* Config input mode go here */
    gpioCfg.pinDir = SC_GPIO_IN_PIN;
    gpioCfg.initLv = 1;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_FALL_EDGE;
    gpioCfg.isr = NULL;
    gpioCfg.wu = NULL;

    sAPI_GpioConfig(pinDHT, gpioCfg);
    //sAPI_GpioSetValue(pinDHT)
    //sAPI_GpioSetDirection(pinDHT, 0); 
}

//Output mode

void outputMode(void) {

    SC_GPIOConfiguration gpioCfg;
    /* Config output mode go here */
    gpioCfg.pinDir = SC_GPIO_OUT_PIN;
    gpioCfg.initLv = 0;
    gpioCfg.pinPull = SC_GPIO_PULLUP_ENABLE;
    gpioCfg.pinEd = SC_GPIO_NO_EDGE;
    gpioCfg.isr = NULL;
    gpioCfg.wu = NULL;

    sAPI_GpioConfig(pinDHT, gpioCfg);
} 


//SIM gui mot request toi DHT11 de bat dau qua trinh truyen du lieu

void Request(void) {

    //SIM on output mode
    outputMode();
    sAPI_GpioSetValue(pinDHT, 0); //keo xung xuong muc 0
    sAPI_DelayUs(18000); //18000us = 18ms
    sAPI_GpioSetValue(pinDHT, 1); //keo len  muc 1
    sAPI_DelayUs(30); //30us
    inputMode();

}

//Ham hoi dap

void Respone(void) {
    int time = 0;
    //inputMode();
    while (sAPI_GpioGetValue(pinDHT) == 1);
    time++;
    sAPI_DelayUs(1);

    //cho xung xuong muc 0

    while (sAPI_GpioGetValue(pinDHT) == 0);
    time++;
    sAPI_DelayUs(1);

    //cho xung len muc cao

    while (sAPI_GpioGetValue(pinDHT) == 1);
    time++;
    sAPI_DelayUs(1);
}

/*************Check Respone tu DHT11*/
//Chua su dung

uint8_t check_Respone(void) {
    uint8_t check = 0;
	sAPI_DelayUs(40);
	if (!(sAPI_GpioGetValue(pinDHT)))
	{
		sAPI_DelayUs(80);
		if (sAPI_GpioGetValue(pinDHT)) check = 1;
		else check = -1;
	}
	while (sAPI_GpioGetValue(pinDHT));   // wait for the pin to go low

	return check;
}

//Truyen du lieu tu DHT
uint8_t receiveData(void)			/* ham nhan du lieu tu DHT11 */
{
    uint8_t c = 0;
	for (int q = 0; q < 8; q++)
	{
		while(sAPI_GpioGetValue(pinDHT) ==  0);  /* cho doan tin hieu xung thap */
		sAPI_DelayUs(30);
		if(sAPI_GpioGetValue(pinDHT))/* neu xung cao dai hon 30us */
		{
			c = (c<<1) | (0x01);	/* thi gia tri logic tra ve co gia tri 1 */
		}
		else			/* neu khong gia tri logic tra ve la 0 */
		{
			c = (c<<1);
		}
		while(sAPI_GpioGetValue(pinDHT));
	}
	return c;
}

void DHT11_init(int pinDHT) {

/**
#if(LOG_DHT11_INFOR)
    if (sAPI_TaskCreate(&dhtTask, dhtTaskStack, 3*1024, 90, (char*)"DHT11 process", 
    readData_DHT11, (void *)0) != SC_SUCCESS ) {
        DEBUG(__FILENAME__, "Create dht11Task fail! \r\n");
    }
    DEBUG(__FILENAME__, "dht11Task ok!, start reveive data from DHT11 \r\n");
#endif
}
**/

/**
static void readData_DHT11(void* ptr) {
    UINT8 sTick1 = 0;

    DEBUG(__FILENAME__, "Truyen du lieu thanh cong \r\n");
    
    while(1) {
        sTick1++;
        if(sTick1 >= 1) {
        //#if(DHT11_REQUEST == 1)
            Request();
        //#endif
        //#if(DHT11_RESPONE == 1)
            Respone();
        //check_Respone();
        //#endif
        //#if(LOG_DHT11_INFOR == 1)
            I_RH = receiveData(); //Do am phan nguyen
            D_RH = receiveData(); //Do am phan thap phan
            I_Temp = receiveData(); //Nhiet do phan nguyen
            D_Temp = receiveData(); //Nhiet do thap phan
            CheckSum = receiveData(); //Checksum

            if ((I_RH  + D_RH + I_Temp + D_Temp)  != CheckSum) {
                DEBUG(__FILENAME__,"Khong co du lieu DHT11 \r\n");
            }
            else {
                temp = I_Temp + (D_Temp/10.0);
                humdity = I_RH + (D_RH/10.0);
                DEBUG(__FILENAME__,"Temp: [%d]  *C \r\n", temp);
                DEBUG(__FILENAME__,"Humdity: [%d] %% \r\n", humdity);
            }
            sTick1 = 0;
            sAPI_TaskSleep(400); //wait 2s
        //#endif
        }
        
    }

*/
    Request();
    Respone();

    I_RH = receiveData(); //Do am phan nguyen
    D_RH = receiveData(); //Do am phan thap phan
    I_Temp = receiveData(); //Nhiet do phan nguyen
    D_Temp = receiveData(); //Nhiet do thap phan
    CheckSum = receiveData(); //Checksum

    //if ((I_RH  + D_RH + I_Temp + D_Temp)  != CheckSum) {
        //DEBUG(__FILENAME__,"Khong co du lieu DHT11 \r\n");
    //}
    //else {
        temp = I_Temp + (D_Temp/10.0);
        humdity = I_RH + (D_RH/10.0);
        // DEBUG(__FILENAME__,"Temp: [%d]  *C \r\n", temp);
        // DEBUG(__FILENAME__,"Humdity: [%d] %% \r\n", humdity);
    ///}


}
