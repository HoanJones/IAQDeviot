/*
 * ssl.h
 *
 *  Created on: 
 *      Author: 
 *
 *      
*/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SSL_H
#define SSL_H


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_api.h"

typedef enum
{
    SSL_OK = 0,
    SSL_ERR,
    SSL_CONNECTED,
    SSL_CLOESED,
} sslStatus_t;

typedef struct
{
    int cid;
    int channel;
    int fd;
} sslPdp_t;
/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */

/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */


/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
sslStatus_t ssl_handshake(void);
sslStatus_t ssl_disconnect(void);
#endif
#ifdef __cplusplus
}
#endif
