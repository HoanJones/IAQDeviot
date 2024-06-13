#ifndef _DHT11_H_
#define _DHT11_H_

#include "simcom_api.h"
#include <stdint.h>

//Khai bao cac ham
void inputMode(void);
void outputMode(void);
void Request(void);
void Respone(void);
uint8_t receiveData(void);
void delay_us(UINT32 microseconds);
//static void readData_DHT11(void);
//void init_DHT11(void);
void DHT11_init(int);
uint8_t check_Respone(void);

#endif 


