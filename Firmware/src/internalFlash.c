/*
 * internalFlash.c
 *
 *  Created on: 
 *      Author: chungnt@epi-tech.com.vn
 * Flash size: 32kBytes, 1 sector: 4kB => 8 sectors
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "internalFlash.h"
#include "debug.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[internalFlash]"

// #define SECTOR_1    0
// #define SECTOR_2    4096
#define ERASE_SIZE  4096
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



/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */
/* sector là bội của 4096, tổng cộng có 8 sectors */
flashErrorType_E internalFlash_write(UINT8 sector, void *tx_DATA, UINT32 len)
{
    int ret = 0;
    UINT32 sectorOffset = 4096*sector;

    if(sector > INTERNAL_FLASH_MAX_SECTOR) return FLASH_ERR_TYPE_OUTOFSIZE;
    ret = sAPI_EraseFlashSector(sectorOffset, ERASE_SIZE);

    char *p;
    p = (char*)tx_DATA;

    if(ret != 0) 
    {
        DEBUG(__FILENAME__, "Erase error [%d]\r\n", ret);
        return FLASH_ERR_TYPE_ERASE;
    }

    ret = sAPI_WriteFlash(sectorOffset, p, len);
    if(ret != 0) 
    {
        DEBUG(__FILENAME__, "Write error [%d]\r\n", ret);
        return FLASH_ERR_TYPE_WRITE;
    }
}

flashErrorType_E internalFlash_read(UINT8 sector, void *rx_DATA, UINT32 len)
{
    int ret = 0;
    UINT32 sectorOffset = 4096*sector;

    if(sector > INTERNAL_FLASH_MAX_SECTOR) return FLASH_ERR_TYPE_OUTOFSIZE;

	char *p;
	p = (char*)rx_DATA;

    ret = sAPI_ReadFlash(sectorOffset, p, len);
    if(ret != 0) 
    {
        DEBUG(__FILENAME__, "Read error [%d]\r\n", ret);
        return FLASH_ERR_TYPE_READ;
    }

    return FLASH_ERR_TYPE_NONE;
}

void internalFlash_Test(void)
{
    #if (0) // lan dau set thanh 1 de ghi vao flash, sau do set ve 0, reset module de test doc du lieu
    uint8_t dataArrTX[5] = {0x12, 0x22, 0x34, 0x64, 0xAA};
    DEBUG(__FILENAME__, "Write to Flash [%d]\r\n", internalFlash_write(0, dataArrTX, 5));
    #else
    uint8_t dataArrRX[5] = {0};
    DEBUG(__FILENAME__, "Read from Flash [%d]\r\n", internalFlash_read(0, dataArrRX, 5));
    DEBUG(__FILENAME__, "dataArrRX: %x, %x, %x, %x, %x\r\n", dataArrRX[0], dataArrRX[1], dataArrRX[2], dataArrRX[3], dataArrRX[4]);
    #endif
    return 0;
}
/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
