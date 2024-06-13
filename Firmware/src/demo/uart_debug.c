#include "simcom_api.h"
#include "simcom_application.h"
#include "debug.h"

extern sMsgQRef simcomUI_msgq;
extern struct ConfigStruct g_conf;
extern INT32 di_count;
extern INT8 force_send;
extern void writeConfig();
extern void checkFlash(INT32 adr);
extern INT8 powersaving;
extern void sendtttb();
INT8 mqtt_debug = 0;
extern int Publish(char *input, char *topic);
extern INT8 sleepmode;
char debugtopic[30] = {0};
sMsgQRef misc_msgq;
extern INT8 ntpstatus;
INT32 mqttdbgtime = 0;

void PrintfResp(UINT8 *format)
{
    UINT32 length = strlen(format);
    if (mqtt_debug == 0)
        sAPI_UartWrite(SC_UART, (UINT8 *)format, length);
        //sAPI_UsbVcomWrite((UINT8*)&format, length);
    else
    {
        Publish(format, debugtopic);
        if (sAPI_GetTicks() - mqttdbgtime > 60000) // 5min
            mqtt_debug = 0;
    }
}

void printInfo()
{
    char buffer[100] = {0};
    char iccid[32] = {0};
    UINT8 csq;
    SCcpsiParm Scpsi;
    UINT8 ret;
    sAPI_NetworkGetCsq(&csq);
    sprintf(buffer, "\r\nID: %s\n", g_conf.id);
    PrintfResp(buffer);
    memset(buffer, 0, 100);
    sprintf(buffer, "URL: %s\n", g_conf.url);
    PrintfResp(buffer);
    memset(buffer, 0, 100);
    sprintf(buffer, "Period: a:%d, p:%d, s:%d, Firm: %d\n",
            g_conf.period_active, g_conf.period_passive, g_conf.period_suspend, FIRMWARE_VER);
    PrintfResp(buffer);
    memset(buffer, 0, 100);
    sprintf(buffer, "DI count: %d CSQ: %d\n", di_count, csq);
    PrintfResp(buffer);
    memset(buffer, 0, 100);
    sprintf(buffer, "NTP: %s\n", g_conf.ntp);
    PrintfResp(buffer);
    ret = sAPI_NetworkGetCpsi(&Scpsi);
    if (ret == SC_NET_SUCCESS)
        sprintf(buffer, "NEmode=%s,Gband=%s,Lband=%s,RSSI=%d\n", Scpsi.networkmode, Scpsi.GSMBandStr, Scpsi.LTEBandStr, Scpsi.Rsrp);
    else
        sprintf(buffer, "Get cpsi falied!\n");
    PrintfResp(buffer);
    ret = sAPI_SysGetIccid(iccid);
    if (ret == SC_SIM_RETURN_SUCCESS)
    {
        memset(buffer, 0, 100);
        sprintf(buffer, "ICCID: %s\n", iccid);
        PrintfResp(buffer);
    }
}

void initLogger2(char *input, INT8 first)
{
    INT32 len = strlen(input);
    memset(g_conf.id, 0, sizeof(g_conf.id));
    memcpy(g_conf.id, input, len);
    g_conf.period_active = 5;
    g_conf.period_passive = 5;
    g_conf.period_suspend = 20;
    memset(g_conf.url, 0, sizeof(g_conf.url));
    //sprintf(g_conf.url, "white.aithings.vn:1883");
    sprintf(g_conf.url, "white.aithings.vn");
    memset(g_conf.ntp, 0, sizeof(g_conf.ntp));
    sprintf(g_conf.ntp, "pool.ntp.org");
    writeConfig();
    if (first == 0)
    {
        printInfo();
        force_send = 1;
    }
}

void seturl(char *input)
{
    INT32 len = strlen(input);
    memset(g_conf.url, 0, sizeof(g_conf.url));
    memcpy(g_conf.url, input, len);
    writeConfig();
    printInfo();
    force_send = 1;
}

void set_period_a(char *input)
{
    g_conf.period_active = atoi(input);
    writeConfig();
    printInfo();
}

void set_period_p(char *input)
{
    g_conf.period_passive = atoi(input);
    writeConfig();
    printInfo();
}

void set_period_s(char *input)
{
    g_conf.period_suspend = atoi(input);
    writeConfig();
    printInfo();
}

void set2gmode(int mode2g)
{
    UINT8 ret = 1;
    if (mode2g == 0)
        ret = sAPI_NetworkSetCnmp(2);
    else if (mode2g == 1)
        ret = sAPI_NetworkSetCnmp(13);
    else if (mode2g == 2)
        ret = sAPI_NetworkSetCnmp(38);

    if (ret == SC_NET_SUCCESS)
    {
        PrintfResp("Setting 2g mode success. Rebooting...\n");
        sAPI_TaskSleep(400);
        sAPI_SysReset();
    }
    else
        PrintfResp("Setting 2g mode failed\n");
}

void getcpsi()
{
    char buffer[100] = {0};
    SCcpsiParm Scpsi;
    UINT8 ret;
    ret = sAPI_NetworkGetCpsi(&Scpsi);
    if (ret == SC_NET_SUCCESS)
        sprintf(buffer, "NEmode=%s,Gband=%s,Lband=%s,RSSI=%d\n", Scpsi.networkmode, Scpsi.GSMBandStr, Scpsi.LTEBandStr, Scpsi.Rsrp);
    else
        sprintf(buffer, "Get cpsi falied!\n");
    if (mqtt_debug == 2)
        send_sms(buffer);
    PrintfResp(buffer);
}

void geticcid()
{
    char iccid[32] = {0};
    UINT8 ret;
    ret = sAPI_SysGetIccid(iccid);
    if (ret == SC_SIM_RETURN_SUCCESS)
        PrintfResp(iccid);
}

void sendAT(char *input)
{
    char *tmp;
    char respStr[300];
    int ret = 0;

    tmp = sAPI_Malloc(strlen(input) + 1);
    sprintf(tmp, "%s\r", input);
    ret = sAPI_SendATCMDWaitResp(10, tmp, 30, NULL, 1, NULL, respStr, sizeof(respStr));
    if (ret == 0)
        PrintfResp(respStr);
    else
        PrintfResp("SendAT fail !!");
    sAPI_Free(tmp);
}

void set_ntp_update(char *input)
{
    INT32 len = strlen(input);
    memset(g_conf.ntp, 0, sizeof(g_conf.ntp));
    memcpy(g_conf.ntp, input, len);
    writeConfig();
    printInfo();
    force_send = 1;
}
void updateNtp()
{
    // INT32 len = strlen(input);
    // memset(g_conf.ntp, 0, sizeof(g_conf.ntp));
    // memcpy(g_conf.ntp, input, len);
    // writeConfig();
    SIM_MSG_T ntp_result = {SC_SRV_NONE, -1, 0, NULL};
    // char buffer[250];
    SC_STATUS status;
    if (strlen(g_conf.ntp) > 0)
    {
        sAPI_NtpUpdate(SC_NTP_OP_SET, g_conf.ntp, 28, NULL);
    }
    // sAPI_HtpSrvConfig(SC_HTP_OP_GET, buffer, "DEL", 0, 80, 1, NULL, 0);
    // PrintfResp(buffer);
    sAPI_NtpUpdate(SC_NTP_OP_EXC, NULL, 0, misc_msgq);

    status = sAPI_MsgQRecv(misc_msgq, &ntp_result, 400);

    if (status == SC_SUCCESS && SC_NTP_OK == ntp_result.arg1) // it means update succeed
    {
        PrintfResp("\r\nNTP Update Time Successful!\r\n");
        DEBUG(__FILENAME__, "NTP Update Time Successful!\r\n");
        ntpstatus = 1;
    }
    else
        PrintfResp("\r\nNTP Update Time Failed!\r\n");
        DEBUG(__FILENAME__, "NTP Update Time Failed!\r\n");
    sAPI_Free(ntp_result.arg3);
}

void process_conf(char *buff, INT8 type)
{
    mqtt_debug = type;
    if (type > 0)
        mqttdbgtime = sAPI_GetTicks();

    while (buff[0] == ' ' || buff[0] == '\r' || buff[0] == '\n' || buff[0] == '\t')
        buff++;
    PrintfResp(buff);
    if (strncmp(buff, "init:", 5) == 0)
        initLogger2(buff + 5, 0);
    else if (strncmp(buff, "url:", 4) == 0)
        seturl(buff + 4);
    else if (strncmp(buff, "setpa:", 6) == 0)
        set_period_a(buff + 6);
    else if (strncmp(buff, "setpp:", 6) == 0)
        set_period_p(buff + 6);
    else if (strncmp(buff, "setps:", 6) == 0)
        set_period_s(buff + 6);
    else if (strncmp(buff, "info", 4) == 0)
        printInfo();
    else if (strncmp(buff, "send", 4) == 0)
        force_send = 1;
    else if (strncmp(buff, "ota:", 4) == 0)
        otaUpdate(buff + 4);
    else if (strncmp(buff, "memrd:", 6) == 0)
        checkFlash(atoi(buff + 6));
    else if (strncmp(buff, "pwrsv:", 6) == 0)
        powersaving = atoi(buff + 6);
    else if (strncmp(buff, "mqttdbg:", 8) == 0)
    {
        mqtt_debug = atoi(buff + 8);
        if (mqtt_debug > 0)
        {
            force_send = 1;
            mqttdbgtime = sAPI_GetTicks();
        }
    }
    else if (strncmp(buff, "at:", 3) == 0)
        sendAT(buff + 3);
    else if (strncmp(buff, "rstnow", 6) == 0)
        sAPI_SysReset();
    else if (strncmp(buff, "sleep:", 6) == 0)
        sleepmode = atoi(buff + 6);
    else if (strncmp(buff, "otaver:", 7) == 0)
        otaUpdateVer(buff + 7);
    else if (strncmp(buff, "eraseflash", 10) == 0)
        eraseAll();
    else if (strncmp(buff, "ntpupdate", 9) == 0)
        set_ntp_update(buff + 10);
    else if (strncmp(buff, "set2gmode:", 10) == 0)
        set2gmode(atoi(buff + 10));
    else if (strncmp(buff, "getcpsi", 7) == 0)
        getcpsi();
    else if (strncmp(buff, "geticcid", 8) == 0)
        geticcid();
}

void debugTask(void *arg)
{
    char buffer[100] = {0};
    SIM_MSG_T optionMsg = {0, 0, 0, NULL};
    INT8 count = 0;
    INT8 pos = 0;
    char inputBuff[100] = {0};
    sAPI_MsgQCreate(&misc_msgq, "misc_msgq", sizeof(SIM_MSG_T), 4, SC_FIFO);

    sAPI_TaskSleep(1000);
    memset(buffer, 0, 100);
    sprintf(buffer, "Welcome to Air Datalogger, Firmware version: %d\n", FIRMWARE_VER);
    PrintfResp(buffer);
    mqtt_debug = 0;
    sprintf(debugtopic, "aithings/%s/debug", g_conf.id);
    while (1)
    {
        memset(inputBuff, 0, sizeof(inputBuff));
        pos = 0;
        do
        {
            sAPI_MsgQRecv(simcomUI_msgq, &optionMsg, SC_SUSPEND);
            memcpy(inputBuff + pos, optionMsg.arg3, optionMsg.arg2);
            pos += optionMsg.arg2;
            sAPI_Free(optionMsg.arg3);
            sAPI_TaskSleep(100);
            sAPI_MsgQPoll(simcomUI_msgq, &count);
        } while (count > 0);

        // PrintfResp(inputBuff);
        if (strncmp(inputBuff, ">cmd_", 5) == 0)
            process_conf(inputBuff + 5, 0);
    }
}