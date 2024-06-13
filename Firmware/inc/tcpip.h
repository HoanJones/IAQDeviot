/*
 * tcpip.h
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

#ifndef _TCPIP_H
#define _TCPIP_H


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */
/* Inclusion of system and local header files goes here */
#include "simcom_os.h"
#include "simcom_common.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
/* #define and enum statements go here */
/** Use to inform the callback function about changes */
/*
* The events have to be seen as events signaling the state of these mboxes/semaphores. For non-blocking
 * connections, you need to know in advance whether a call to a netconn function call would block or not,
 * and these events tell you about that.
 * 
 * RCVPLUS: Safe to perform a potentially blocking call call once more. 
 * They are counted in sockets - three RCVPLUS events for accept mbox means you are safe
 * to call netconn_accept 3 times without being blocked.
 * Same thing for receive mbox.
 * 
 * RCVMINUS: Your call to to a possibly blocking function is "acknowledged".
 * Socket implementation decrements the counter.
 * 
 * For TX, there is no need to count, its merely a flag. 
 * SENDPLUS: you may send something. SENDPLUS occurs when enough data was delivered to peer so netconn_send() can be called again.
 * 
 * SENDMINUS: event occurs when the next call to a netconn_send() would be blocking.
 * 
 * SENDACKED:  The data sent has been confirmed by the peer
 * 
 * CONNECTED: TCP three-way handshake is completed, you can send and receive data
 * 
 * ACCEPTPLUS: has new client access
 */
enum netconn_evt {
  NETCONN_EVT_RCVPLUS = 0,
  NETCONN_EVT_RCVMINUS,
  NETCONN_EVT_SENDPLUS,
  NETCONN_EVT_SENDMINUS,
  NETCONN_EVT_CONNECTED,
  NETCONN_EVT_ACCEPTPLUS, // 5
  NETCONN_EVT_ERROR_CLSD,
  NETCONN_EVT_ERROR_RST,
  NETCONN_EVT_ERROR_ABRT,
  NETCONN_EVT_CLOSE_WAIT,
  NETCONN_EVT_SENDACKED, // 10
  NETCONN_EVT_CLOSE_NORMAL,
  NETCONN_EVT_ERROR
};

typedef enum
{
	TCP_STT_DISCONNECTED = 0,
	TCP_STT_CONNECTED,
} TCP_CONNECT_STT_E;

enum TCP_Err_Type_t
{
  TCP_HAS_ERR = 0,
  TCP_NO_ERR,
};

typedef struct
{
	UINT8 connect;
	UINT8 needConnect;
	UINT8 connectStatus;
	UINT8 state;
 
	UINT16 timeoutConnection;
	char dataBuf[1024];
	char bufContent[50];	
} TCP_Manager_t;

typedef struct 
{
	UINT8 ipAdd[255];
	UINT16 port;
} TCP_Params_t;

typedef struct 
{
  UINT8 pdpId;
  TCP_Params_t params;
} TCP_Save_Last;

#define FLAG_MASK_TCP_RX    0x00000100
/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */
/* Definition of public (external) data types go here */
extern sFlagRef tcpipFlags;

/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */
/* Function prototypes for public (external) functions go here */
void tcpip_Init(void);
UINT8 tcpip_definePDP(UINT8 pdpId);
UINT8 tcpip_clientOpen(INT8 *ipAdd, UINT16 port, UINT8 pdpId);
UINT8 tcpip_send(const void *data, INT32 size, UINT8 pdpId);
void tcp_setParams(char *input, int len); // ex: input = "8.tcp.ngrok.io:12257:1"
#endif
#ifdef __cplusplus
}
#endif
