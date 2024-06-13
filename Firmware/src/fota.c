/*
 * fota.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "fota.h"
#include "debug.h"
#include "simcom_fota.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[FOTA]"

#define FOTA_TASK_STACK_SIZE    (1024 * 3)
#define FOTA_TASK_PRIORITY      (150)

#define FW_VER100  "VER_1.00.120224"
#define FW_VER101  "VER_1.01.120224"
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */
fotaParams_t fotaParams;
sFlagRef otaFlags;

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */
static sTaskRef FOTATask = NULL;
static UINT8 FOTATaskStack[FOTA_TASK_STACK_SIZE];

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */
static void Task_FOTAManager(void *ptr);

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
void FOTA_Init(void)
{
    SC_STATUS ret;
    ret = sAPI_FlagCreate(&otaFlags);
    DEBUG(__FILENAME__, "otaFlags init ret [%d]\r\n", ret);

    if(sAPI_TaskCreate(&FOTATask, FOTATaskStack, FOTA_TASK_STACK_SIZE, FOTA_TASK_PRIORITY, (char*)"FOTA process", Task_FOTAManager, (void *)0) != SC_SUCCESS)
    {
        DEBUG(__FILENAME__, "sAPI_TaskCreate error!\n");
    }
    else DEBUG(__FILENAME__, "Task Create ok\r\n");
}

SCAppDwonLoadReturnCode FOTA_DownloadCusApp(void)
{
    SCAppDownloadPram param;
    SCAppDwonLoadReturnCode ret;

    param.mod = SC_APP_DOWNLOAD_HTTP_MOD;
    param.url = "http://5bb1-2402-800-62ce-a560-6d2c-324b-9585-e9a6.ngrok-free.app/customer_app.bin";
    // param.url = fotaParams.urlUpdate;

    DEBUG(__FILENAME__, "sAPI_AppDownload start\r\n");
    ret = sAPI_AppDownload(&param);
    if(ret == SC_APP_DOWNLOAD_SUCESSED)
    {
        DEBUG(__FILENAME__, "Download app ok\r\n");
        return ret;
    }

    DEBUG(__FILENAME__, "Download err [%d]\r\n", ret);
    return ret;
}

void FOTA_appUpdate(void)
{
    sAPI_FlagSet(otaFlags, FLAG_MASK_FOTA_APP, SC_FLAG_OR);
}

/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
static void Task_FOTAManager(void *ptr)
{
    SC_STATUS status;
    UINT32 flags = 0;

    while(1)
    {
        status = sAPI_FlagWait(otaFlags, FLAG_MASK_FOTA_APP | FLAG_MASK_FOTA_KERNEL, SC_FLAG_OR_CLEAR, &flags, (5000/5));
        DEBUG(__FILENAME__, "status: %d -- flags: %x\r\n", status, flags);
        // DEBUG(__FILENAME__, "FW Version: %s\r\n", __TIME__);
        DEBUG(__FILENAME__, "FW Version: %s\r\n", FW_VER101);
        if(status == SC_SUCCESS)
        {
            switch(flags)
            {
                case FLAG_MASK_FOTA_APP:
                {
                    if(FOTA_DownloadCusApp() == SC_APP_DOWNLOAD_SUCESSED) 
                    {
                        DEBUG(__FILENAME__, "Reset to load new firmware\r\n");
                        // sAPI_SysReset();
                    }
                    break;
                }

                default: flags = 0; break;
            }
        }
    }
}
