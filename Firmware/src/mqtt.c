/*
 * mqtt.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "mqtt.h"
#include "debug.h"
// #include "systemFlag.h"
// #include "gpio.h"
// #include "system.h"
#include "ssl.h"
// #include "fota.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[MQTT]"

#define MQTTRECV_TASK_STACK_SIZE    (1024 * 10)
#define MQTTRECV_TASK_PRIORITY      (150)

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static sTaskRef mqttRecvTask = NULL;
static UINT8 mqttRecvTaskStack[MQTTRECV_TASK_STACK_SIZE];

static Mqtt_Params_t Mqtt_Params;
static sMsgQRef msgUrcHdl;

static subscribeTopic_t subscribeTopic[] = 
{
    [0].subTopic = TOPIC_UPDATE,
};
static UINT8 numOfSubTopic = sizeof(subscribeTopic)/sizeof(subscribeTopic[0]);
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */


/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
static void connectLostCb(int client_index, int cause);
static void Task_MqttRecv(void *ptr);
#if (JSON_EN)
static INT8 CjsonParseURL(INT8* data);
#endif
/* ==================================================================== */
/* ========================= public functions ========================= */
/* ==================================================================== */
void mqtt_Init(void)
{
    if(sAPI_MsgQCreate(&msgUrcHdl, "msgUrcHdl", sizeof(SIM_MSG_T), 4, SC_FIFO) != SC_SUCCESS)   // msgQ for subscribed data transfer
    {
        DEBUG(__FILENAME__, "msgUrcHdl creat err!\r\n");
        return;
    }
    else DEBUG(__FILENAME__, "msgUrcHdl creat ok\r\n");

    if(sAPI_TaskCreate(&mqttRecvTask, mqttRecvTaskStack, MQTTRECV_TASK_STACK_SIZE, MQTTRECV_TASK_PRIORITY, (char*)"MQTT recev process", Task_MqttRecv, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "Recv task Create error!\r\n");
        return;
    }
    else DEBUG(__FILENAME__, "Recv task Create ok\r\n");

}

SCmqttResultType mqtt_connect(void)
{
    SCmqttResultType ret;

#if(SERVER_TYPE_TLS)
    if(ssl_handshake() == SSL_ERR)
    {
        DEBUG(__FILENAME__, "ssl_handshake fail\r\n");
        return SC_MQTT_RESULT_FAIL;
    }
#endif
    /*==========================================================================*/
    /* Start service */
    /*==========================================================================*/
    ret = sAPI_MqttStart(-1);
    if(ret == SC_MQTT_RESULT_SUCCESS) 
    {
        DEBUG(__FILENAME__, "Start ok\r\n");
    }
    else 
    {
        DEBUG(__FILENAME__, "Start fail, ret[%d]\r\n", ret);
        return ret;
    }
    /*==========================================================================*/
    /* Acquire Client */
    /*==========================================================================*/
    memcpy(Mqtt_Params.acquireClient, DEFAULT_MQTT_CLIENT_NAME, sizeof(DEFAULT_MQTT_CLIENT_NAME));
    Mqtt_Params.clientIdx = DEFAULT_MQTT_CLI_IDX;
#if(!SERVER_TYPE_TLS)
    ret = sAPI_MqttAccq(SC_MQTT_OP_SET, NULL, Mqtt_Params.clientIdx, (char*)Mqtt_Params.acquireClient, 0, msgUrcHdl);
#else
    ret = sAPI_MqttAccq(SC_MQTT_OP_SET, NULL, Mqtt_Params.clientIdx, (char*)Mqtt_Params.acquireClient, 1, msgUrcHdl);
#endif
    if(ret == SC_MQTT_RESULT_SUCCESS) 
    {
        DEBUG(__FILENAME__, "acquireClient ok\r\n");
    }
    else 
    {
        DEBUG(__FILENAME__, "acquireClient fail, ret[%d]\r\n", ret);
        ret = sAPI_MqttStop();
        DEBUG(__FILENAME__, "stop mqtt, ret [%d]\r\n", ret);
        return ret;
    }
    /*==========================================================================*/
    /* Connect server */
    /*==========================================================================*/
    memset(Mqtt_Params.serverAdd, 0, sizeof(Mqtt_Params.serverAdd));
    memcpy(Mqtt_Params.serverAdd, DEFAULT_MQTT_IPADD, sizeof(DEFAULT_MQTT_IPADD));
    Mqtt_Params.port = DEFAULT_MQTT_PORT;
    Mqtt_Params.keepAliveTime = DEFAULT_MQTT_KEEP_TIME;
    Mqtt_Params.cleanSession = DEFAULT_MQTT_CLEAN_SESSION;
    Mqtt_Params.qos = DEFAULT_MQTT_QOS;
    Mqtt_Params.username = DEFAULT_MQTT_USERNAME;
    Mqtt_Params.password = DEFAULT_MQTT_PASSWORD;
    memset(Mqtt_Params.bufContent, 0, sizeof(Mqtt_Params.bufContent));
    sprintf(Mqtt_Params.bufContent, "tcp://%s:%d", Mqtt_Params.serverAdd, Mqtt_Params.port);

#if(SERVER_TYPE_TLS)
    /*==========================================================================*/
    /* SSL config */
    /*==========================================================================*/
    ret = sAPI_MqttSslCfg(SC_MQTT_OP_SET, NULL, Mqtt_Params.clientIdx, 0);
    if(ret == SC_MQTT_RESULT_SUCCESS) 
    {
        DEBUG(__FILENAME__, "MqttSslCfg ok: %s\r\n", Mqtt_Params.bufContent);
    }
    else 
    {
        DEBUG(__FILENAME__, "MqttSslCfg fail, ret[%d]\r\n", ret);
        return ret;
    }
#endif
    DEBUG(__FILENAME__, "Mqtt_Params.keepAliveTime [%d]\r\n", Mqtt_Params.keepAliveTime);
    DEBUG(__FILENAME__, "Mqtt_Params.cleanSession [%d]\r\n", Mqtt_Params.cleanSession);
    DEBUG(__FILENAME__, "Mqtt_Params.qos [%d]\r\n", Mqtt_Params.qos);
    DEBUG(__FILENAME__, "Mqtt_Params.username [%s]\r\n", Mqtt_Params.username);
    DEBUG(__FILENAME__, "Mqtt_Params.password [%s]\r\n", Mqtt_Params.password);
    DEBUG(__FILENAME__, "Connecting to: %s\r\n", Mqtt_Params.bufContent);
    
    ret = sAPI_MqttConnect(SC_MQTT_OP_SET, NULL, Mqtt_Params.clientIdx, Mqtt_Params.bufContent, Mqtt_Params.keepAliveTime, Mqtt_Params.cleanSession, (char*)Mqtt_Params.username, (char*)Mqtt_Params.password);
    // ret = sAPI_MqttConnect(SC_MQTT_OP_SET, NULL, Mqtt_Params.clientIdx, Mqtt_Params.bufContent, Mqtt_Params.keepAliveTime, Mqtt_Params.cleanSession, NULL, NULL);
    if(ret == SC_MQTT_RESULT_SUCCESS) 
    {
        DEBUG(__FILENAME__, "MqttConnect ok, host: %s\r\n", Mqtt_Params.bufContent);
        // Mqtt_Manager.connectStatus = MQTT_STT_CONNECTED;
    }
    else 
    {
        DEBUG(__FILENAME__, "MqttConnect fail, ret[%d]\r\n", ret);
        // Mqtt_Manager.connectStatus = MQTT_STT_DISCONNECTED;
        ret = sAPI_MqttRel(Mqtt_Params.clientIdx);
        DEBUG(__FILENAME__, "release client id, ret [%d]\r\n", ret);
        ret = sAPI_MqttStop();
        DEBUG(__FILENAME__, "stop mqtt, ret [%d]\r\n", ret);
#if(SERVER_TYPE_TLS)
        ssl_disconnect();
#endif
        return ret;
    }

    sAPI_MqttConnLostCb(connectLostCb); 

    /*==========================================================================*/
    /* Sub topic */
    /*==========================================================================*/
    for(UINT8 i = 0; i < numOfSubTopic; i++)
    {
        UINT8 topicLen = strlen(subscribeTopic[i].subTopic);
        /* Input message to sub */
        ret = sAPI_MqttSubTopic(Mqtt_Params.clientIdx, subscribeTopic[i].subTopic, topicLen, Mqtt_Params.qos);
        if(ret == SC_MQTT_RESULT_SUCCESS) 
        {
            DEBUG(__FILENAME__, "input sub ok, topic: %s\r\n", subscribeTopic[i].subTopic);
        }
        else 
        {
            DEBUG(__FILENAME__, "input sub fail, ret[%d]\r\n", ret);
            ret = sAPI_MqttDisConnect(SC_MQTT_OP_SET, NULL, Mqtt_Params.clientIdx, 60);
            DEBUG(__FILENAME__, "disconnect mqtt, ret [%d]\r\n", ret);
            ret = sAPI_MqttRel(Mqtt_Params.clientIdx);
            DEBUG(__FILENAME__, "release client id, ret [%d]\r\n", ret);
            ret = sAPI_MqttStop();
            DEBUG(__FILENAME__, "stop mqtt, ret [%d]\r\n", ret);
            return ret;
        }

        /* Sub topic to server */
        ret = sAPI_MqttSub(Mqtt_Params.clientIdx, NULL, 0, Mqtt_Params.qos, 0);
        if(ret == SC_MQTT_RESULT_SUCCESS)
        {
            DEBUG(__FILENAME__, "sub ok\r\n");
        }
        else 
        {
            DEBUG(__FILENAME__, "sub fail, ret[%d]\r\n", ret);
            ret = sAPI_MqttDisConnect(SC_MQTT_OP_SET, NULL, Mqtt_Params.clientIdx, 60);
            DEBUG(__FILENAME__, "disconnect mqtt, ret [%d]\r\n", ret);
            ret = sAPI_MqttRel(Mqtt_Params.clientIdx);
            DEBUG(__FILENAME__, "release client id, ret [%d]\r\n", ret);
            ret = sAPI_MqttStop();
            DEBUG(__FILENAME__, "stop mqtt, ret [%d]\r\n", ret);
            return ret;
        }
    }

    return ret;
}

SCmqttResultType mqtt_disconnect(void)
{
    SCmqttResultType ret;
    ret = sAPI_MqttDisConnect(SC_MQTT_OP_SET, NULL, Mqtt_Params.clientIdx, 60);
    DEBUG(__FILENAME__, "disconnect mqtt, ret [%d]\r\n", ret);
    ret = sAPI_MqttRel(Mqtt_Params.clientIdx);
    DEBUG(__FILENAME__, "release client id, ret [%d]\r\n", ret);
    ret = sAPI_MqttStop();
    DEBUG(__FILENAME__, "stop mqtt, ret [%d]\r\n", ret);
    return ret;
}

UINT8 mqtt_PublishData(const char *topic, UINT8 qos, const char *data)
{
    SCmqttResultType ret;
    Mqtt_Pub_Params_t Mqtt_Pub_Params;

    Mqtt_Pub_Params.topicLen = strlen(topic);
    DEBUG(__FILENAME__, "Mqtt_Pub_Params.topicLen [%d]\r\n", Mqtt_Pub_Params.topicLen);
    Mqtt_Pub_Params.pubTopic = sAPI_Malloc(Mqtt_Pub_Params.topicLen+1);
    memset(Mqtt_Pub_Params.pubTopic, 0, Mqtt_Pub_Params.topicLen+1);
    memcpy(Mqtt_Pub_Params.pubTopic, topic, Mqtt_Pub_Params.topicLen);

    Mqtt_Pub_Params.qos = qos;

    Mqtt_Pub_Params.dataLen = strlen(data);
    DEBUG(__FILENAME__, "Mqtt_Pub_Params.dataLen [%d]\r\n", Mqtt_Pub_Params.dataLen);
    Mqtt_Pub_Params.data = sAPI_Malloc(Mqtt_Pub_Params.dataLen+1);
    memset(Mqtt_Pub_Params.data, 0, Mqtt_Pub_Params.dataLen+1);
    memcpy(Mqtt_Pub_Params.data, data, Mqtt_Pub_Params.dataLen);

    ret = sAPI_MqttTopic(Mqtt_Params.clientIdx, Mqtt_Pub_Params.pubTopic, Mqtt_Pub_Params.topicLen);
    DEBUG(__FILENAME__, "sAPI_MqttTopic, ret [%d]\r\n", ret);
    ret = sAPI_MqttPayload(Mqtt_Params.clientIdx, Mqtt_Pub_Params.data, Mqtt_Pub_Params.dataLen);
    DEBUG(__FILENAME__, "sAPI_MqttPayload, ret [%d]\r\n", ret);
    ret = sAPI_MqttPub(Mqtt_Params.clientIdx, Mqtt_Pub_Params.qos, 60, 0, 0);
    if(ret == SC_MQTT_RESULT_SUCCESS) 
    {
        DEBUG(__FILENAME__, "Pub ok\r\n");
    }
    else 
    {
        DEBUG(__FILENAME__, "Pub fail, ret[%d]\r\n", ret);
        /* To do: xu ly khi pub fail */

    }
    sAPI_Free(Mqtt_Pub_Params.pubTopic);                                                                                                                  
    sAPI_Free(Mqtt_Pub_Params.data);                                                                                                                 

    return 1;
}

void mqtt_test_pub(void)
{
    mqtt_PublishData(TOPIC_EVENT, 0, TEST_REQUEST_DATA);
}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
static void connectLostCb(int client_index, int cause)
{
    DEBUG(__FILENAME__, "connectLostCb: %d, %d\r\n", client_index, cause);

    mqtt_disconnect();
}

static void Task_MqttRecv(void *ptr)
{
    SIM_MSG_T msgQ_data_recv = {SIM_MSG_INIT, 0, -1, NULL}; // NULL pointer for msgQ_data_recv.arg3 is necessary!
    SCmqttData *sub_data = NULL;
    while(1)
    {
        /*=======================================================================================================================
        *
        *   NOTE: if this data reception cycle too long may cause data loss(data processing slower than data receive from server)
        *
        **=======================================================================================================================*/
        //recv the subscribed topic data, from the message queue: msgUrcHdl, it is set buy sAPI_MqttAccq                                                  
        sAPI_MsgQRecv(msgUrcHdl, &msgQ_data_recv, SC_SUSPEND);                                                                                             
        if((SC_SRV_MQTT != msgQ_data_recv.msg_id) || (0 != msgQ_data_recv.arg1) || (NULL == msgQ_data_recv.arg3))   //wrong msg received               
            continue;                                                                                                                                   
        sub_data = (SCmqttData *)(msgQ_data_recv.arg3);                                                                                               
        /*this part just for test, the payload msg just ascii string*/  
        DEBUG(__FILENAME__, "\r\n");
        DEBUG(__FILENAME__, "/*============= MQTT RX =============*/\r\n");                                                                                         
        DEBUG(__FILENAME__, "client index: [%d]; tpoic_len: [%d]; tpoic: [%s]\r\n", sub_data->client_index, sub_data->topic_len, sub_data->topic_P);                                                                                                                                                    //                     //
        DEBUG(__FILENAME__, "payload_len: [%d]\r\n", sub_data->payload_len);   
        DEBUG(__FILENAME__, "receive topic: %s, len: %d\r\n", sub_data->topic_P, sub_data->topic_len);                                                           
		DEBUG(__FILENAME__, "receive payload: %s, len: %d\r\n", sub_data->payload_P, sub_data->payload_len); 
        DEBUG(__FILENAME__, "/*==================================*/\r\n\r\n");         
        /*************************************************************************************/  
        /* Copy data before processing */

                                                               
        /*************************************************************************************/                                                      
        /*these msg pointer must be free after using, don not change the free order*/                                                                  
        sAPI_Free(sub_data->topic_P);                                                                                                                  
        sAPI_Free(sub_data->payload_P);                                                                                                                 
        sAPI_Free(sub_data);                                                                                                                                                                                 
         
        /*************************************************************************************/
        /* process new msg */


    }
}

void mqtt_setParams(char *input, int len)
{
    if(strstr((const char *)input, "con")) 
    {
        mqtt_connect();
    }
    else if(strstr((const char *)input, "dis")) 
    {
        mqtt_disconnect();
    }
}

#if (JSON_EN)
static INT8 CjsonParseURL(INT8* data)
{
    cJSON* root = NULL;
    cJSON* shared = NULL;
    cJSON* targetFwVer = NULL;
    cJSON* userApproveFwVer = NULL;
    cJSON* targetFwUrl = NULL;

    cJSON_Hooks hooks;
    hooks.malloc_fn = sAPI_Malloc;
    hooks.free_fn = sAPI_Free;
    cJSON_InitHooks(&hooks);

    root = cJSON_Parse(data);
    if(root == NULL)
    {
        DEBUG(__FILENAME__, "json parse fail\r\n");
        return -1;
    }
    // DEBUG(__FILENAME__, "root: %s\r\n", cJSON_Print(root));

    shared = cJSON_GetObjectItem(root, "shared");
    if(shared == NULL)
    {
        DEBUG(__FILENAME__, "shared get fail\r\n");
        return -1;
    }

    targetFwVer = cJSON_GetObjectItem(shared, "targetFwVer");
    if(targetFwVer)
    {
        // DEBUG(__FILENAME__, "targetFwVer: [%s]\r\n", targetFwVer->valuestring);
        memset(fotaParams.targetVer, 0, sizeof(fotaParams.targetVer));
        memcpy(fotaParams.targetVer, targetFwVer->valuestring, strlen(targetFwVer->valuestring));
        DEBUG(__FILENAME__, "targetFwVer: %s\r\n", fotaParams.targetVer);
    }

    userApproveFwVer = cJSON_GetObjectItem(shared, "userApproveFwVer");
    if(userApproveFwVer)
    {
        // DEBUG(__FILENAME__, "userApproveFwVer: [%s]\r\n", userApproveFwVer->valuestring);
        memset(fotaParams.approveVer, 0, sizeof(fotaParams.approveVer));
        memcpy(fotaParams.approveVer, userApproveFwVer->valuestring, strlen(userApproveFwVer->valuestring));
        DEBUG(__FILENAME__, "userApproveFwVer: %s\r\n", fotaParams.approveVer);
    }

    targetFwUrl = cJSON_GetObjectItem(shared, "targetFwUrl");
    if(targetFwUrl)
    {
        // DEBUG(__FILENAME__, "targetFwUrl: [%s]\r\n", targetFwUrl->valuestring);
        memset(fotaParams.urlUpdate, 0, sizeof(fotaParams.urlUpdate));
        memcpy(fotaParams.urlUpdate, targetFwUrl->valuestring, strlen(targetFwUrl->valuestring));
        DEBUG(__FILENAME__, "targetFwUrl: %s\r\n", fotaParams.urlUpdate);
    }

    cJSON_Delete(root);

    return 0;
}

#endif
