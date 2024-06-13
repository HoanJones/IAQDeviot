/*
 * ssl_server.c
 *
 *  Created on: 
 *      Author: 
*/
/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */
#include "ssl.h"
#include "debug.h"

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */
#define TAG "[SSL]"
/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */
/* Global variables definitions go here */



/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */
/* Definition of private datatypes go here */

const char *domain = "test.mosquitto.org";
const char *port = "8883";
const char *ca_cert =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\r\n"
"BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\r\n"
"A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\r\n"
"BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\r\n"
"by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\r\n"
"BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\r\n"
"MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\r\n"
"dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\r\n"
"KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\r\n"
"UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\r\n"
"Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\r\n"
"s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\r\n"
"3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\r\n"
"E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\r\n"
"MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\r\n"
"6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\r\n"
"BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\r\n"
"6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\r\n"
"+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\r\n"
"sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\r\n"
"LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\r\n"
"m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\r\n"
"-----END CERTIFICATE-----\r\n";

static sslPdp_t sslPdp = { .cid = 1, .channel = 1};
/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */
/* Function prototypes for private (static) functions go here */



/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */
/* Functions definitions go here, organised into sections */


/* ==================================================================== */
/* ========================= private functions ======================== */
/* ==================================================================== */
static int ssl_connect_to_server(const char *pDomain, const char *port)
{
    if (pDomain == NULL)
    {
        DEBUG(__FILENAME__, "The paramter is invalid. Please check input parameter.\r\n");
        return -1;
    }

    int fd = -1;
    struct addrinfo hints, *addr_list;

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = SC_AF_INET;
    hints.ai_socktype = SC_SOCK_STREAM;
    hints.ai_protocol = SC_IPPROTO_TCP;

    if (getaddrinfo(pDomain, port, &hints, &addr_list ) != 0 )
    {
        DEBUG(__FILENAME__, "Dns parse domain fail. Please check domain:%s.\r\n", pDomain);
        return -1;
    }

    BOOL flag = FALSE;
    for (struct addrinfo *cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0)
        {
            DEBUG(__FILENAME__, "create socket fail.\r\n");
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            flag = TRUE;
            DEBUG(__FILENAME__, "connect to server success.\r\n");
            break;
        }

        close(fd);
    }

    freeaddrinfo(addr_list);

    return flag ? fd : -1;
}
/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
sslStatus_t ssl_handshake(void)
{
    if(sAPI_TcpipPdpActive(sslPdp.cid, sslPdp.channel) != 0)
    {
        DEBUG(__FILENAME__, "The network may be unavailable. Please check related equipment.\r\n");
        return SSL_ERR;
    }
    DEBUG(__FILENAME__, "TcpipPdpActive ok\r\n");

    sslPdp.fd = ssl_connect_to_server(domain, port);
    if (sslPdp.fd < 0)
    {
        DEBUG(__FILENAME__, "connect to server:%s fail\r\n", domain);
        return SSL_ERR;
    }

    SCSslCtx_t sslCtx;

    memset(&sslCtx, 0, sizeof(SCSslCtx_t));
    sslCtx.ClientId = 0;
    sslCtx.fd = sslPdp.fd;
    sslCtx.ciphersuitesetflg = 0;
    sslCtx.ssl_version = SC_SSL_CFG_VERSION_TLS12;
    sslCtx.enable_SNI = 0;
    sslCtx.auth_mode = SC_SSL_CFG_VERIFY_MODE_REQUIRED;
    sslCtx.ignore_local_time = 1;
    sslCtx.root_ca = (INT8 *)ca_cert;
    sslCtx.root_ca_len = strlen(ca_cert) + 1;

    if (sAPI_SslHandShake(&sslCtx) != 0)
    {
        DEBUG(__FILENAME__, "ssl handshake fail.\r\n");
        return SSL_ERR;
    }
    DEBUG(__FILENAME__, "ssl handshake ok\r\n");

    // /* download file through https */
    // HTTP_Demo();
    // /* end */

#if 0
    sAPI_SslClose(0);
    close(sslPdp.fd);

    if(sAPI_TcpipPdpDeactive(sslPdp.cid, sslPdp.channel)  != 0)
    {
        DEBUG(__FILENAME__, "TcpipPdpDeactive fail\r\n");
        return - 1;
    }
    DEBUG(__FILENAME__, "TcpipPdpDeactive ok\r\n");
#endif
    return SSL_CONNECTED;
}

/*********************************************************************//**
 * @brief   
 * @param
 * @retval  
 ************************************************************************/
sslStatus_t ssl_disconnect(void)
{
#if 1
    sAPI_SslClose(0);
    close(sslPdp.fd);

    if(sAPI_TcpipPdpDeactive(sslPdp.cid, sslPdp.channel)  != 0)
    {
        DEBUG(__FILENAME__, "TcpipPdpDeactive fail\r\n");
        return SSL_ERR;
    }
    DEBUG(__FILENAME__, "TcpipPdpDeactive ok\r\n");
#endif
    return SSL_CLOESED;
}
