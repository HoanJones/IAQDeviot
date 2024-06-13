#include "simcom_api.h"
#include "simcom_application.h"
extern void PrintfResp(INT8 *format);

void otaUpdate(INT8 *url)
{
    SCAppDownloadPram pram;
    SCAppDwonLoadReturnCode ret = SC_APP_DOWNLOAD_SUCESSED;
    SCAppPackageInfo gAppUpdateInfo = {0};
    UINT8 pGreg = 0;

    pram.url = url;
    if (strncmp(pram.url, "https://", strlen("https://")) != 0 && strncmp(pram.url, "http://", strlen("http://")) != 0)
        return;

    pram.mod = SC_APP_DOWNLOAD_HTTP_MOD;
    pram.recvtimeout = 20000;
    PrintfResp("Start downloading firmware\n");
    PrintfResp(pram.url);
    ret = sAPI_AppDownload(&pram);
    if (ret != SC_APP_DOWNLOAD_SUCESSED)
    {
        PrintfResp("Firmware download failed\n");
        return;
    }
    PrintfResp("Firmware download OK\n");
    ret = sAPI_AppPackageCrc(&gAppUpdateInfo);
    if (SC_APP_DOWNLOAD_SUCESSED == ret)
    {
        PrintfResp("Firmware CRC ok, resetting...\n");
        sAPI_TaskSleep(200);
        sAPI_SysReset();
    }
    else
        PrintfResp("Firmware CRC failed\n");
}

void otaUpdateVer(INT8 *ver)
{
    INT32 version = atoi(ver);
    INT8 buff[100];
    memset(buff, 0, sizeof(buff));
    if (SIMMODEL == 0)
        sprintf(buff, "http://27.71.237.139:13579/customer_app_%d.bin", version);
    else
        sprintf(buff, "http://27.71.237.139:13579/customer_app_lase_%d.bin", version);
    otaUpdate(buff);
}