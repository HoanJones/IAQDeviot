/*
 * mqtt.h
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
 *
 *      
*/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _MQTT_H_
#define _MQTT_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
#define SERVER_TYPE_TLS 1

#if (!SERVER_TYPE_TLS)
#define DEFAULT_MQTT_CLIENT_NAME	"chungnt"
#define DEFAULT_MQTT_CLI_IDX		0
#define DEFAULT_MQTT_IPADD			"broker.hivemq.com"
#define DEFAULT_MQTT_PORT			1883
#define DEFAULT_MQTT_KEEP_TIME		60
#define DEFAULT_MQTT_CLEAN_SESSION	1
#define DEFAULT_MQTT_QOS			0
#define DEFAULT_MQTT_USERNAME		"chungnt"
#define DEFAULT_MQTT_PASSWORD		"12345678"

#else
#define DEFAULT_MQTT_CLIENT_NAME	"clientTest"
#define DEFAULT_MQTT_CLI_IDX		0
#define DEFAULT_MQTT_IPADD			"test.mosquitto.org"
#define DEFAULT_MQTT_PORT			8883
#define DEFAULT_MQTT_KEEP_TIME		60
#define DEFAULT_MQTT_CLEAN_SESSION	1
#define DEFAULT_MQTT_QOS			0
// #define DEFAULT_MQTT_USERNAME		"chungnt"
// #define DEFAULT_MQTT_PASSWORD		"12345678"
#define DEFAULT_MQTT_USERNAME		NULL
#define DEFAULT_MQTT_PASSWORD		NULL
#endif

#define TOPIC_DATA				"chungnt/data"		// Pub /* update: pub /IMEI topic */
#define TOPIC_EVENT				"chungnt/event"		// Pub
#define TOPIC_CONTROL			"chungnt/control" 	// Sub /* update: sub /IMEI topic */
#define TOPIC_UPDATE			"chungnt/update"	// Sub
#define TOPIC_POSITION          "chungnt/position"  // Pub

#define TEST_REQUEST_DATA		"{\"test:\"\"abcd\"}"
#define TOPIC_REPONSE			"chungnt/reponse"

#define PUBLISH_DONE 		0
#define PUBLISH_BUSY	    1
#define PUBLISH_ERROR		2
#define PUBLISH_TIMEOUT     3

#define MQTT_RX_LEN 		1024
#define MQTT_TX_LEN 		256
#define MQTT_TOPIC_LEN      20

typedef enum
{
	MQTT_STT_DISCONNECTED = 0,
	MQTT_STT_CONNECTED,
} MQTT_CONNECT_STT_E;

typedef struct
{
	UINT8 connect;
	UINT8 needConnect;
	UINT8 connectStatus;
	UINT8 state;
	
	UINT8 mesIsBeingSent;
	UINT8 retryConnection;
	UINT16 timeoutConnection;	
	UINT16 timeoutPublish;
} Mqtt_Manager_t;

typedef struct
{
	// char RXBuf[MQTT_RX_LEN];
	// UINT16 RXlen;
	// char TXBuf[MQTT_TX_LEN];
	// UINT16 TXlen;
	char bufContent[50];
	/*======================*/
	UINT8 acquireClient[20];
	UINT8 clientIdx;
	char serverAdd[30];
	UINT16 port; 
	UINT8 keepAliveTime;
	UINT8 cleanSession;
	UINT8 qos;
	char* username; // max 256bytes
	char* password; // max 256bytes
} Mqtt_Params_t;

typedef struct 
{
	char *pubTopic;
	UINT16 topicLen;
	char *data;
	UINT16 dataLen;
	UINT8 qos;
} Mqtt_Pub_Params_t;

typedef struct 
{
	char subTopic[1024];
} subscribeTopic_t;

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */





/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void mqtt_Init(void);
SCmqttResultType mqtt_connect(void);
SCmqttResultType mqtt_disconnect(void);
UINT8 mqtt_PublishData(const char *topic, UINT8 qos, const char *data);
void mqtt_setParams(char *input, int len);
#endif
#ifdef __cplusplus
}
#endif
