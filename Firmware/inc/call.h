/*
 * call.h
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

#ifndef CALL_H
#define CALL_H


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
#define MAX_OA_SIZE (20) // max length of Originating-Address
#define REGION      "+84"

typedef enum
{
    CALL_MODE_IDLE = 0,
    CALL_MODE_DIAL,
    CALL_MODE_ANSWER,
    CALL_MODE_END,
    CALL_MODE_GET_STATE,
    CALL_MODE_AUTO_ANSWER,
    CALL_MODE_REQ_USSD,
} callMode_t;

typedef struct 
{
    callMode_t callMode;
    char phoneNumber[MAX_OA_SIZE];
} callManager_t;

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */
extern sMsgQRef callUrcMesQ;
extern sFlagRef callFlag;
extern callManager_t callManager;

/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void call_Init(void);
SC_STATUS call_SwitchMode(callMode_t mode, char *phoneNumber);
#endif
#ifdef __cplusplus
}
#endif
