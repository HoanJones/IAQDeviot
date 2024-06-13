#include "simcom_api.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_application.h"
#include "simcom_mqtts_client.h"
#include "debug.h"

extern void PrintfResp(INT8 *format);
extern DataBackup readData();
extern sMsgQRef urc_mqtt_msgq_1;
extern struct ConfigStruct g_conf;
extern INT16 nb_backup;
extern INT16 ptr_backup;
extern UINT32 lostConnectionTime;
char client_id[20] = {0};
char usrName[20] = {0};
char usrPwd[20] = {0};
char host[100] = {0};
INT8 mqtt_status = 0;
INT8 subscribe_status = 0;
char payload[200] = {0};
char loggertopic[100] = {0};
char subtopic[50];
INT8 connect_retry = 0;
INT8 mqttconnected = 0;
char host2[100] = {0};
int Publish(char *input, char *topic)
{
    INT32 topic_len = strlen(topic);
    INT32 ret;

    if (mqtt_status == 0)
        connect_mqtt();

    if (mqtt_status > 0)
    {
        ret = sAPI_MqttTopic(0, topic, topic_len);
        ret = sAPI_MqttPayload(0, input, strlen(input));
        ret = sAPI_MqttPub(0, 1, 60, 0, 0);
        if (ret == SC_MQTT_RESULT_SUCCESS)
            return 0;
        else
            lost_connection_handler(0, 0);
    }
    return 1;
}

void lost_connection_handler(int client_index, int cause)
{
    PrintfResp("MQTT connection lost\n");
    if (mqtt_status = 1)
    {
        mqtt_status = 0;
        lostConnectionTime = sAPI_GetTicks();
    }
    subscribe_status = 0;
}

void makePayload(DataBackup d)
{
    memset(payload, 0, sizeof(payload));
    sprintf(payload, "%04d%02d%02d%02d%02d00\tTemp:%d\tHumidity:%d\tCO2:%0.1f\tPM2.5:%d\tPM10:%d\tCSQ:%d\tUptime:%d\tTYPE:99",
            d.year + 2000, d.month, d.day, d.hour, d.min, d.temp, d.humdity, d.airQua, d.pm25, d.pm10, d.csq, d.uptime);
}

INT8 send_mqtt(DataBackup data)
{
    PrintfResp("Sending mqtt\n");
    DEBUG(__FILENAME__, "Sending mqtt\r\n");
    memset(loggertopic, 0, sizeof(loggertopic));
    sprintf(loggertopic, "aithings/%s/data", g_conf.id);
    if (mqtt_status == 0)
        connect_mqtt();
    if (mqtt_status == 0)
        return 1;
    else if (subscribe_status == 0)
        subscribe();

    makePayload(data);
    return Publish(payload, loggertopic);
}

void send_mqtt_backup()
{
    INT8 ret;
    DataBackup data;
    INT8 count = 0;
    while (ptr_backup < nb_backup)
    {
        sAPI_TaskSleep(20);
        count++;
        data.year = 0;
        data = readData();

        if (data.year >= 23 && data.year < 100)
        {
            makePayload(data);
            ret = Publish(payload, loggertopic);
            if (ret != 0)
            {
                PrintfResp("MQTT send backup failed\n");
                break;
            }
            else
                PrintfResp("MQTT send backup ok\n");
        }
        if (count > 72)
            break;
    }
}

void subscribe()
{
    int ret;
    memset(subtopic, 0, sizeof(subtopic));
    sprintf(subtopic, "aithings/%s/config", g_conf.id);
    ret = sAPI_MqttSub(0, subtopic, strlen(subtopic), 0, 0);
    if (ret == SC_MQTT_RESULT_SUCCESS)
    {
        subscribe_status = 1;
        connect_retry = 0;
        PrintfResp("MQTT subscribed ok");
        DEBUG(__FILENAME__, "MQTT subscribed ok\r\n");
    }
    else
    {
        connect_retry++;
        if (connect_retry < 3)
        {
            sAPI_TaskSleep(400);
            subscribe();
        }
        else
            connect_retry = 0;
    }
}

void connect_mqtt()
{
    SCmqttReturnCode ret;
    PrintfResp("Connecting mqtt\r\n");
    // Start
    ret = sAPI_MqttStart(-1); ///chua khoi dong duoc mqtt
    if(ret == SC_MQTT_RESULT_SUCCESS) 
    {
        DEBUG(__FILENAME__, "Start ok\r\n");
    }
    else 
        DEBUG(__FILENAME__, "Start fail, ret[%d]\r\n", ret);
    // Connect
    memset(host2, 0, sizeof(host2));
    sprintf(host2, "%s", "tcp://white.aithings.vn");
    ret = sAPI_MqttAccq(0, NULL, 0, client_id, 0, urc_mqtt_msgq_1);
    ret = sAPI_MqttConnect(0, NULL, 0, host2, 60, 1, usrName, usrPwd);
    //ret = sAPI_MqttConnect(0, NULL, 0, host2, 60, 1, usrName, usrPwd);

    // if (SC_MQTT_RESULT_SUCCESS == ret)
    //     {
    //         sAPI_Free(usrName);
    //         sAPI_Free(usrPwd);
    //         sAPI_Free(client_id);
    //     }

    DEBUG(__FILENAME__, "Cilent ID: %s\r\n", client_id);
    DEBUG(__FILENAME__, "Host: %s\r\n", host2);
    if (ret == SC_MQTT_RESULT_SUCCESS)
    {
        PrintfResp("MQTT connected\n");
        DEBUG(__FILENAME__, "MQTT connected!!!\r\n");
        connect_retry = 0;
        sAPI_MqttConnLostCb(lost_connection_handler);
        sAPI_TaskSleep(100);
        subscribe();
        sAPI_TaskSleep(100);
        mqtt_status = 1;
        mqttconnected = 1;
        sAPI_GpioSetValue(9, 0);
    }
    else
    {
        PrintfResp("MQTT connection failed, retrying...\n");
        DEBUG(__FILENAME__, "MQTT connection failed, retrying...\r\n");
        connect_retry++;
        if (connect_retry < 3)
        {
            sAPI_TaskSleep(400);
            connect_mqtt();
        }
        else
        {
            PrintfResp("MQTT connection failed\n");
            DEBUG(__FILENAME__, "MQTT connection failed\r\n");
            connect_retry = 0;
        }
    }
}

void init_mqtt(void)
{
    INT8 status = 0;

    INT32 ret;
    SIM_MSG_T optionMsg = {0, 0, 0, NULL};

    memset(usrName, 0, sizeof(usrName));
    snprintf(usrName, 100, "%s", "hoan");
    memset(usrPwd, 0, sizeof(usrPwd));
    snprintf(usrPwd, 100, "%s", "123456");
    memset(client_id, 0, sizeof(client_id));
    snprintf(client_id, 100, "r_%s", g_conf.id);
    memset(host, 0, sizeof(host));
    sprintf(host, "tcp://%s", g_conf.url);
}
