#include "simcom_api.h"
#include "sgp30.h"
#include "debug.h"
#include "simcom_i2c.h"

/* Global variables definitions go here */
//SC_I2C_DEV i2cDev = {.i2c_channel = SC_I2C_CHANNEL0, .i2c_clock = SC_I2C_STANDARD_MODE};

#define slave_add (SGP30_I2C_ADDRESS << 1) // (0x58<<1)
UINT8 i2c_index = 0;

uint8_t eCO2 = 0;
uint8_t PVOC = 0;
//I2C write data
//reg: 8bit register address of slave address
//dat: data length
UINT8 i2c_write(UINT8 reg, UINT8 data, UINT8 dataSize)
{
    UINT8 ret;
    ret = sAPI_I2CWrite(i2c_index, slave_add, reg, &data, dataSize);
    return ret;
}

//I2C read data
UINT8 i2c_read(UINT8 reg, UINT8 data, UINT8 dataSize)
{
    UINT8 ret;
    ret = sAPI_I2CReadEx(i2c_index, slave_add, reg, &data, dataSize);
    return ret;
}

void init_iaq() {
    UINT8 ret;
    ret = i2c_write(0, sgp30_iaq_init, 2);
    if (ret != SC_I2C_RC_OK) {
        sAPI_Debug("Init Air Quality fail!!!\t\n");
    }
}
void SGP30_Init(void) {

    /**************** i2c init ****************/
    SC_I2C_DEV i2cDev;
    /*set i2c clock*/
    i2cDev.i2c_clock = SC_I2C_STANDARD_MODE;
    /* set i2c channel */
    i2cDev.i2c_channel = SC_I2C_CHANNEL0;
    
    if(SC_I2C_RC_OK != sAPI_I2CConfigInit(&i2cDev))
    {
        DEBUG(__FILENAME__,"\r\nI2C config init fail!\r\n");
    }
    else
    {
        DEBUG(__FILENAME__,"\r\nI2C config init successful!\r\n");
    }

    init_iaq();
}

void SGP30_readData(void) {
    UINT8 ret;
    ret = i2c_write(0, sgp30_measure_init, 2);
    if( ret == SC_I2C_RC_OK) {
        DEBUG(__FILENAME__, "SGP30 Measure IAQ Init success! ret = [%d] \r\n", ret);
    }
    sAPI_TaskSleep(200*14); //delay 14s

    uint8_t buffer[6];
    ret = i2c_read(0, buffer, 6);
    if (ret == SC_I2C_RC_NOT_OK) {
        DEBUG(__FILENAME__, "Erorr!");
    }
    else 
        //buffer[0]: byte LSB of CO2
        //buffer[1]: byte MSB of CO2
        //buffer[2]: Checksum
        //buffer[3]: byte LSB of TVOC
        //buffer[4]: byte MSB of TVOC

        eCO2 = ((buffer[0] << 8) | buffer[1]);
        PVOC = ((buffer[3] << 8) | buffer[4]);

}

uint8_t CRC8(uint16_t data)
{
    uint8_t crc = 0xFF; // Khởi tạo với 0xFF

    crc ^= (data >> 8); // XOR byte đầu tiên

    for (uint8_t i = 0; i < 8; i++)
    {
        if ((crc & 0x80) != 0)
            crc = (uint8_t)((crc << 1) ^ 0x31);
        else
            crc <<= 1;
    }
    crc ^= (uint8_t)data; // XOR byte cuối cùng

    for (uint8_t i = 0; i < 8; i++)
    {
        if ((crc & 0x80) != 0)
            crc = (uint8_t)((crc << 1) ^ 0x31);
        else
            crc <<= 1;
    }

    return crc; // Không có output reflection
}

#ifdef SGP30_LOOKUP_TABLE
// Sử dụng lookup table để tạo CRC8 cho SGP30
extern uint8_t _CRC8LookupTable[16][16];

uint8_t CRC8(uint16_t data)
{
    uint8_t CRC = 0xFF; // Giá trị khởi tạo
    CRC ^= (uint8_t)(data >> 8); // Bắt đầu với MSB
    CRC = _CRC8LookupTable[CRC >> 4][CRC & 0x0F]; // Tra bảng [MSnibble][LSnibble]
    CRC ^= (uint8_t)data; // Sử dụng LSB
    CRC = _CRC8LookupTable[CRC >> 4][CRC & 0x0F]; // Tra bảng [MSnibble][LSnibble]
    return CRC;
}
#endif