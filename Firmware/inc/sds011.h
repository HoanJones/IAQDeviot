#ifndef _SDS011_H_
#define _SDS011_H_

#include "simcom_api.h"
#include <stdio.h>
#include <stdint.h>

/**********Data frame of SDS011 Sensor *****************/

#define messageHeader 0xAA
#define commandNo 0xC0
#define messageTail 0xAB

#define UART_DATA_LEN           128 // max 1024

typedef struct
{
    uint8_t data[UART_DATA_LEN];
    uint8_t flag;
    int len;
} uartTypedef_t;

void uartUser_init(SC_Uart_Port_Number port, SC_UART_BaudRates baudrate, SC_UART_WordLen dataLen, SC_UART_ParityTBits parity, SC_UART_StopBits stopbit);
void SDS011_init(SC_Uart_Port_Number port, SC_UART_BaudRates baudrate, SC_UART_WordLen dataLen, SC_UART_ParityTBits parity, SC_UART_StopBits stopbit);
// void UartCBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve);
// void readData_SDS011(uartTypedef_t uartTypedefRecv);

#endif