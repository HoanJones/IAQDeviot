#include "simcom_api.h"
#include "simcom_uart.h"
#include "..\inc\debug.h"
#include "simcom_common.h"
#include "..\inc\sds011.h"

uint8_t pm25 = 0;
uint8_t pm10 = 0;

static sMsgQRef uartQueueHdl = NULL;

//SC_Uart_Port_Number port;

void Uart2CBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve);
void UartCBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve);

void uartUser_init(SC_Uart_Port_Number port, SC_UART_BaudRates baudrate, SC_UART_WordLen dataLen, SC_UART_ParityTBits parity, SC_UART_StopBits stopbit)
{
    SCuartConfiguration uartConfig;

    /*************************Configure UART again*********************************/
    /*******The user can modify the initialization configuratin of UART in here.***/ 
    /******************************************************************************/
    uartConfig.BaudRate  =  baudrate;
    uartConfig.DataBits  =  dataLen;
    uartConfig.ParityBit =  parity;
    uartConfig.StopBits  =  stopbit;
    if(sAPI_UartSetConfig(port, &uartConfig) == SC_UART_RETURN_CODE_ERROR)
    {
        DEBUG(__FILENAME__, "%s: Configure UART failure!!" ,__func__);
        return;
    }

    sAPI_UartRegisterCallbackEX(port, UartCBFuncEx, (void *)"UartCBFuncEx");

    DEBUG(__FILENAME__, "Uart[%d] to debug init Done\r\n", port);
}


void SDS011_init(SC_Uart_Port_Number port, SC_UART_BaudRates baudrate, SC_UART_WordLen dataLen, SC_UART_ParityTBits parity, SC_UART_StopBits stopbit) {

    SCuartConfiguration uartConfig;

    /*************************Configure UART again*********************************/
    /*******The user can modify the initialization configuratin of UART in here.***/ 
    /******************************************************************************/
    uartConfig.BaudRate  =  baudrate;
    uartConfig.DataBits  =  dataLen;
    uartConfig.ParityBit =  parity;
    uartConfig.StopBits  =  stopbit;

    if(sAPI_UartSetConfig(port, &uartConfig) == SC_UART_RETURN_CODE_ERROR)
    {
        DEBUG(__FILENAME__, "%s: Configure UART failure!!" ,__func__);
        return;
    }

    sAPI_UartRegisterCallbackEX(port, Uart2CBFuncEx, (void *)"Uart2CBFuncEx");

    //DEBUG(__FILENAME__, "Uart[%d] Init Done!! \r\n", port);
    //DEBUG(__FILENAME__,"Truyen du lieu tu SDS011 qua UART[%d] \r\n", port);
    sAPI_Debug("Ready to receive data from SDS011 = Uart[%d]\r\n", port);
}

/*****************************UART CALL BACK**************************************************/
void UartCBFuncEx(SC_Uart_Port_Number portNumber, int len, void *reserve)
{
    uartTypedef_t uartTypedefSend;

    sAPI_UartRead(portNumber, uartTypedefSend.data, len);

    uartTypedefSend.data[len] = 0;
    uartTypedefSend.flag = portNumber;
    uartTypedefSend.len = len;

    sAPI_MsgQSend(uartQueueHdl, (SIM_MSG_T*)&uartTypedefSend);

    return;
}
