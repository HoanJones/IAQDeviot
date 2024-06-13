/*
 * config.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "config.h"
#include "debug.h"
#include "CircularBuffer.h"
#include "network.h"
#include "mqtt.h"
#include "tcpip.h"
#include "fota.h"
#include "http.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[CONFIG]"

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */



/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static ERR_E Config_tcp(char* input, char* out);
static ERR_E Config_mqtt(char* input, char* out);
static ERR_E Config_system(char* input, char* out);
static ERR_E Config_gnss(char* input, char* out);
static ERR_E Config_http(char* input, char* out);
/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */
/* Parse cmd */
#define COMMAND_MAX_LEN (20)
#define COMMAND_TERMINAL    ('#')

#define MSG_FIELD_SEPARATOR (',')
/* Message field position */
#define MSG_FIELD_TYPE (0)
#define MSG_FIELD_DATA (1)
#define MSG_MAX_FIELDS (2)

typedef ERR_E(* PACKET_FUNC_PTR)(char* input, char* out);

typedef struct parse_cmd_process_tag
{
    uint8_t				msg_id;
    const char			packet_str[COMMAND_MAX_LEN];
    PACKET_FUNC_PTR		packet_func_ptr;
} PARSE_PACKET_PROCESS_T;

/* ex: mqtt,connect# */
static const PARSE_PACKET_PROCESS_T packet_process_table[] =
{
	{1, "tcp", Config_tcp},
    {2, "mqtt", Config_mqtt},
    {3, "system", Config_system},
    {4, "gnss", Config_gnss},
    {5, "http", Config_http},
};

static uint8_t packet_process_table_size = sizeof(packet_process_table)/sizeof(packet_process_table[0]);
/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */


/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
ERR_E config_process(char* input, int len)
{
    char *buffer = input;
    int lenBuf = len;

    char *msg_field[MSG_MAX_FIELDS];
	ERR_E ret = CMD_NOT_SUPPORT;
	// pointer 
	char *msg_token;
	char data_reply[20] = {0};
    uint8_t numOfField = 0;
	// generic indexers
	uint8_t j;


    if(buffer[lenBuf-1] != COMMAND_TERMINAL) 
    {
        DEBUG(__FILENAME__, "Command not support\r\n");
        return CMD_NOT_SUPPORT;
    }

    DEBUG(__FILENAME__, "Frame: %s\r\n", buffer);

    memset(msg_field, 0, sizeof(msg_field));

    msg_token		 		  = buffer;
    msg_field[MSG_FIELD_TYPE] = msg_token;
    numOfField = MSG_FIELD_DATA;

    while(*msg_token != '\0')
    {
        if(*msg_token == MSG_FIELD_SEPARATOR)
        {
            // terminate string after field separator or end-of-message characters
            *msg_token = '\0';

            // save position of the next token
            msg_field[MSG_FIELD_DATA] = msg_token + 1;
            numOfField++;
            // break;
        }
        msg_token++;
    }
    
    DEBUG(__FILENAME__, "numOfField [%d]\r\n", numOfField);

    if(numOfField != MSG_MAX_FIELDS)
    {
        DEBUG(__FILENAME__, "msg fields: not support\r\n");
        return CMD_MSG_FIELD_ERR;
    }

    for(j = 0; j < packet_process_table_size; j++)
    {
        if(strcmp((char*)packet_process_table[j].packet_str, (char*)msg_field[MSG_FIELD_TYPE]) == 0)
        {
            if(packet_process_table[j].packet_func_ptr != NULL)
            {
                ret = packet_process_table[j].packet_func_ptr((char*)msg_field[MSG_FIELD_DATA], (char*)data_reply);
            }
            break;
        }
    }

    if(j == packet_process_table_size)
    {
        DEBUG(__FILENAME__, "Command not support\r\n");
        return CMD_NOT_SUPPORT;
    }
                
    DEBUG(__FILENAME__, "+%s, ret: %d\r\n", (char*)packet_process_table[j].packet_str, ret);
    return CMD_OK;
}


/* Private functions ---------------------------------------------------------------------------------------*/
static ERR_E Config_tcp(char* input, char* out)
{
#if (TCPIP_EN)
    tcp_setParams(input, strlen(input));
#endif
    return CMD_OK;
}

static ERR_E Config_mqtt(char* input, char* out)
{
#if (MQTT_EN)
    mqtt_setParams(input, strlen(input));
#endif
    return CMD_OK;
}

static ERR_E Config_system(char* input, char* out)
{
    if(strstr((const char*)input, "reset"))
    {
        sAPI_SysReset();
    }
    else if(strstr((const char*)input, "pwroff"))
    {
        sAPI_SysPowerOff();
    }
#if (FOTA_EN)
    else if(strstr((const char*)input, "app_update"))
    {
        FOTA_appUpdate();
    }
#endif
    return CMD_OK;
}

static ERR_E Config_gnss(char* input, char* out)
{
#if (GNSS_EN)
    UINT8 csq;
    int creg, cgreg;
    if(strstr((const char*)input, "agps"))
    {
        sAPI_NetworkGetCsq(&csq);
        sAPI_NetworkGetCreg(&creg);
        sAPI_NetworkGetCgreg(&cgreg);
        DEBUG(__FILENAME__, "csq [%d], creg [%d], cgreg [%d]\r\n", csq, creg, cgreg);
        if(gnss_GetPwronStatus() == 1)
        {
            if((csq > 0) && (csq < 32) && (cgreg == 1))
            {               
                gnss_setAGPS();
            }
        }
        else 
        {
            DEBUG(__FILENAME__, "Pls turn on GNSS\r\n");
            gnss_setAGPS(); // for test
        }
    }
    else if(strstr((const char*)input, "on"))
    {
        gnss_SwitchPower(SC_GNSS_POWER_ON);
        gnss_SetStartMode(SC_GNSS_START_HOT);
        gnss_StartGetNMEA(SC_GNSS_NMEA_DATA_GET_BY_URC);
    }
    else if(strstr((const char*)input, "off"))
    {
        gnss_SwitchPower(SC_GNSS_POWER_OFF);
    }
#endif
    return CMD_OK;
}

static ERR_E Config_http(char* input, char* out)
{
#if (HTTP_EN)
    http_setParams(input, strlen(input));
#endif
    return CMD_OK;
}
