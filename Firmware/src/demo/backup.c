#include "simcom_api.h"
#include "simcom_application.h"
#include "debug.h"

#define STARTSECTOR 2
#define ENDSECTOR 15
extern void initLogger2(char *input, INT8 first);
extern struct ConfigStruct g_conf;
extern struct MiscData g_miscdata;
INT16 nb_backup = 0;
INT16 ptr_backup = 0;
extern void PrintfResp(INT8 *format);

void eraseAll() // erase all flash sector used for backup
{
    int i;
    INT8 val[2];
    for (i = STARTSECTOR; i <= ENDSECTOR; i++)
    {
        val[0] = 0xff;
        val[1] = 0xff;
        sAPI_ReadFlash(i * 4096, &val, 2);
        if (val[0] != 0xff || val[1] != 0xff)
            sAPI_EraseFlashSector(4096 * i, 4096);
    }
    ptr_backup = 0;
    nb_backup = 0;
}

void writeMiscData()
{
    if (g_miscdata.nb_backup != nb_backup || g_miscdata.ptr_backup != ptr_backup)
    {
        g_miscdata.nb_backup = nb_backup;
        g_miscdata.ptr_backup = ptr_backup;
        sAPI_EraseFlashSector(4096, 4096); // erase sector 1
        sAPI_TaskSleep(100);
        sAPI_WriteFlash(4096, &g_miscdata, sizeof(g_miscdata));
    }
}

void readMiscData()
{
    sAPI_ReadFlash(4096, &g_miscdata, sizeof(g_miscdata));
    if (g_miscdata.nb_backup < 60000)
    {
        nb_backup = g_miscdata.nb_backup;
        ptr_backup = g_miscdata.ptr_backup;
    }
    else
        writeMiscData();
}

void writeConfig()
{
    sAPI_EraseFlashSector(0, 4096); // erase sector 1
    sAPI_TaskSleep(100);
    sAPI_WriteFlash(0, &g_conf, sizeof(g_conf));
}

void readConfig()
{
    sAPI_ReadFlash(0, &g_conf, sizeof(g_conf));
    //DEBUG(__FILENAME__, "return readflash = %d \r\n", sAPI_ReadFlash(0, &g_conf, sizeof(g_conf)));

    if (g_conf.id[0] == 0xff)
    {
        initLogger2("airlogger", 1);
    }
    //else  DEBUG(__FILENAME__, "Init logger fail!!\r\n");

    readMiscData();
}

void printMem(INT8 *dat, INT32 len)
{
    INT8 buff[512];
    INT32 pos = 0;
    int i;
    for (i = 0; i < len; i++)
    {
        sprintf(buff + pos, "%02x ", dat[i]);
        pos += 3;
        if (pos > 500)
        {
            pos = 0;
            PrintfResp(buff);
            memset(buff, 0, 512);
        }
    }
    PrintfResp(buff);
}

void checkFlash(INT32 adr)
{
    INT8 dat[128];
    sAPI_ReadFlash(adr, &dat, sizeof(dat));
    printMem(dat, 128);
}

DataBackup readData()
{
    DataBackup res;
    // char buff[100];
    UINT32 offset = 4096 * STARTSECTOR; // start from sector
    if (nb_backup > ptr_backup)
        offset += (ptr_backup) * sizeof(res);
    else
        return;
    sAPI_ReadFlash(offset, &res, sizeof(res));
    ptr_backup++;
    if (ptr_backup > 2000 && ptr_backup == nb_backup) // erase once in a while
    {
        eraseAll();
    }
    // sprintf(buff, "nbback : %d, ptrback: %d\n", nb_backup, ptr_backup);
    // PrintfResp(buff);
    return res;
}

void writeData(DataBackup data)
{
    UINT32 offset = 4096 * STARTSECTOR;
    int ret;
    offset += nb_backup * sizeof(data);
    if (offset >= ENDSECTOR * 4096) // full
        return;

    ret = sAPI_WriteFlash(offset, &data, sizeof(data));
    if (ret == 0)
        nb_backup++;
}

