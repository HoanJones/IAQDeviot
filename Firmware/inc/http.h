/*
 * http.h
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

#ifndef _HTTP_H_
#define _HTTP_H_


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
    HTTP_ACTION_GET = 0,
    HTTP_ACTION_POST,
    HTTP_ACTION_HEAD,
    HTTP_ACTION_DEL,
} http_action_type_t;

typedef enum
{
    HTTP_READ_TYPE_GETSIZE = 0,
    HTTP_READ_TYPE_OFFSET,
} http_read_type_t;

typedef struct 
{
    /* data */
    int statusCode;
    int dataLen;
} httpActionData_t;

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */


/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void http_Init(void);
int http_sessionInit(int channel);
int http_sessionTerm(void);
int http_Para(char *type, char *param);
httpActionData_t http_Action(http_action_type_t method);
int http_Read(http_read_type_t readType, int startOffset, int size);
void http_setParams(char *input, int len); // http,https://6300-2402-9d80-28d-c089-b8bd-1efe-b95f-c9e1.ngrok-free.app,test.txt,1
#endif
#ifdef __cplusplus
}
#endif
