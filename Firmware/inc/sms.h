/*
 * sms.h
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

#ifndef _SMS_H_
#define _SMS_H_


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
typedef enum
{
    SMS_OK = 0,
    SMS_SEND_FAIL,
    SMS_REV_FAIL,
    SMS_REV_TIMEOUT,
    SMS_SEND_TIMEOUT,
} smsStatus_t;
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */
extern sMsgQRef smsUrcMesQ;

/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void sms_Init(void);
smsStatus_t sms_SendNewMsg(UINT8 *address, UINT8 *data, UINT16 dataLen);
#endif
#ifdef __cplusplus
}
#endif
