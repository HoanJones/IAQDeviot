#include "simcom_api.h"
#include <stdint.h>

#define FIRMWARE_VER 120
#define SIMMODEL 0
#pragma pack(push, 1)
struct ConfigStruct
{
    uint8_t id[16];
    uint8_t url[30];
    uint8_t period_active;   // in minutes
    uint8_t period_passive;  // in minutes
    uint16_t period_suspend; // in seconds
    uint8_t ntp[20];
} g_conf;

struct MiscData
{
    uint32_t nb_backup;
    uint32_t ptr_backup;
} g_miscdata;

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint16_t di_count;

    uint8_t temp; //nhiet do
    uint8_t humdity; //do am
    float airQua; //nong do CO2
    uint8_t pm25; //bui pm2.5
    uint8_t pm10; //bui pm10

    uint16_t vbat;
    //unsigned int adc0;
    //unsigned int adc1;
    uint8_t csq;
    unsigned int uptime;
} DataBackup;

#pragma pack(pop)