#ifndef __CC3100_EventHandler_H__
#define __CC3100_EventHandler_H__




//=========================================================================
// standard lib
//=========================================================================
#include "string.h"




//=========================================================================
// board_cc3100 lib
//=========================================================================
#include "simplelink.h"
#include "CC3100_User.h"
#include "CC3100_Board.h"
#include "CC3100.h"
#include "CC3100_HttpServer.h"
#include "usart.h"





//=========================================================================
// Variablen
//=========================================================================

extern	uint32_t  WiFi_Status;



//=========================================================================
// Funktions_Deklarationen
//=========================================================================

void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent);

void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent);

void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent, SlHttpServerResponse_t *pHttpResponse);

void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent);

void SimpleLinkSockEventHandler(SlSockEvent_t *pSock);




#endif // __CC3100_H__
