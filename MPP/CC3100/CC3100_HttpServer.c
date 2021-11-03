#include "CC3100_HttpServer.h"



//=========================================================================
// Makro
//=========================================================================
#define SL_STOP_TIMEOUT        0xFF



//=========================================================================
// Variablen
//=========================================================================
char POST_token[] = "__SL_P_L.D";
char GET_token[]  = "__SL_G_LED";

_u8 SecType = 0;

_u8 g_auth_name[MAX_AUTH_NAME_LEN+1];
_u8 g_auth_password[MAX_AUTH_PASSWORD_LEN+1];
_u8 g_auth_realm[MAX_AUTH_REALM_LEN+1];

_u8 g_domain_name[MAX_DOMAIN_NAME_LEN];

_u8 g_device_urn[MAX_DEVICE_URN_LEN];

char Httpbuffer[100] = {0};

char antwort1[] = "led-on";
char antwort2[] = "led-off";

//=========================================================================
// Funktionen
//=========================================================================

//=========================================================================
void start_HttpServer(void)
//=========================================================================
{
	_i32 retVal = 0;
    /* Configure AP mode without security */
    retVal = sl_WlanSet(	SL_WLAN_CFG_AP_ID,
    						WLAN_AP_OPT_SSID,
    						strlen(SSID_AP_MODE),
    						(_u8 *)SSID_AP_MODE
    						);

    SecType = SEC_TYPE_AP_MODE;

    /* Configure the Security parameter in the AP mode */
    retVal = sl_WlanSet(	SL_WLAN_CFG_AP_ID,
    						WLAN_AP_OPT_SECURITY_TYPE,
    						1,
    						(_u8 *)&SecType
    						);

    retVal = sl_WlanSet(	SL_WLAN_CFG_AP_ID,
    						WLAN_AP_OPT_PASSWORD,
    						strlen(PASSWORD_AP_MODE),
    						(_u8 *)PASSWORD_AP_MODE
    						);

    // CC3100 Restart

    WiFi_Status = 0;

    retVal = sl_Stop(SL_STOP_TIMEOUT);
    retVal = sl_Start(0, 0, 0);
    if (ROLE_AP == retVal)
    {
        while(!IS_IP_ACQUIRED(WiFi_Status)) { _SlNonOsMainLoopTask(); }
        usart2_send(" \r\n CC3100 im AP-Mode gewesen \n\r");
    }
    else
    {	//retVal = sl_WlanSetMode(ROLE_AP);
        usart2_send(" \r\n CC3100 in den AP-Mode gesetzt \n\r");
    }

    usart2_send(" Waiting for client to connect\n\r");

    // warten auf client
    while((!IS_IP_LEASED(WiFi_Status)) || (!IS_STA_CONNECTED(WiFi_Status))) { _SlNonOsMainLoopTask(); }

    usart2_send(" Client connected\n\r");

    // Enable the HTTP Authentication
       retVal = set_authentication_check(TRUE);

       /* Get authentication parameters */
       retVal = get_auth_name(g_auth_name);

       retVal = get_auth_password(g_auth_password);

       retVal = get_auth_realm(g_auth_realm);

       retVal = get_domain_name(g_domain_name);

       retVal = get_device_urn (g_device_urn);

       usart2_send("\r\n Authentication parameters: ");

       sprintf(Httpbuffer,"\r\n Name = %s", g_auth_name);
       usart2_send(Httpbuffer);

       sprintf(Httpbuffer,"\r\n Password = %s", g_auth_password);
       usart2_send(Httpbuffer);

       sprintf(Httpbuffer,"\r\n Realm = %s", g_auth_realm);
       usart2_send(Httpbuffer);

       sprintf(Httpbuffer,"\r\n Domain name = %s", g_domain_name);
       usart2_send(Httpbuffer);

       sprintf(Httpbuffer,"\r\n Device URN = %s", g_device_urn);
       usart2_send(Httpbuffer);

       while(1)
       {
           _SlNonOsMainLoopTask();
       }
}



//=========================================================================
void Http_Get_Event(	SlHttpServerEvent_t *pHttpEvent,
        				SlHttpServerResponse_t *pHttpResponse)
//=========================================================================
{
	// Definiere den Head, der
	char tokenhead[] = "__SL_G_U";
	int tokenheadlength = strlen(tokenhead);

	// Setze die Antwort zurück
	*pHttpResponse->ResponseData.token_value.data = 0;
	pHttpResponse->ResponseData.token_value.len = 0;

	// Prüfe, ob der Tokenname bekannt ist
	if (pHttpEvent->EventData.httpTokenName.len >= 10 && strncmp((char*)pHttpEvent->EventData.httpTokenName.data, tokenhead, tokenheadlength) == 0)
	{
		// Schneide den Header ab - es bleiben zwei Buchstaben für die Identifikation des Tokens
		char *tokenid = (char*)&(pHttpEvent->EventData.httpTokenName.data[tokenheadlength]);

		// Prüfe, ob der LED-Status abgefragt wird
		if (strncmp(tokenid, "LS", 2) == 0)
		{
			if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_2) == Bit_SET)
			{
				strcpy((char*)pHttpResponse->ResponseData.token_value.data, "ON");
				pHttpResponse->ResponseData.token_value.len = 2;
			}
			else
			{
				strcpy((char*)pHttpResponse->ResponseData.token_value.data, "OFF");
				pHttpResponse->ResponseData.token_value.len = 3;
			}
		}

		// Prüfe, ob der Tasterstatus abgefragt wurde
		else if (strncmp(tokenid, "TS", 2) == 0)
		{
			if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == Bit_SET)
			{
				strcpy((char*)pHttpResponse->ResponseData.token_value.data, "ON");
				pHttpResponse->ResponseData.token_value.len = 2;
			}
			else
			{
				strcpy((char*)pHttpResponse->ResponseData.token_value.data, "OFF");
				pHttpResponse->ResponseData.token_value.len = 3;
			}
		}
	}
}



//=========================================================================
void Http_Post_Event(	SlHttpServerEvent_t *pHttpEvent,
        				SlHttpServerResponse_t *pHttpResponse)
//=========================================================================
{
	// Definiere den Head, der
	char tokenhead[] = "__SL_P_U";
	int tokenheadlength = strlen(tokenhead);

	// Prüfe, ob der Tokenname bekannt ist
	if (pHttpEvent->EventData.httpPostData.token_name.len >= 10 && strncmp((char*)pHttpEvent->EventData.httpPostData.token_name.data, tokenhead, tokenheadlength) == 0)
	{
		// Schneide den Header ab - es bleiben zwei Buchstaben für die Identifikation des Tokens
		char *tokenid = (char*)&(pHttpEvent->EventData.httpPostData.token_name.data[tokenheadlength]);

		// Prüfe, ob der LED-Status abgefragt wird
		if (strncmp(tokenid, "LS", 2) == 0)
		{
			if (strncmp((char*)pHttpEvent->EventData.httpPostData.token_value.data, "ON", 2) == 0)
			{
				LED_GR_ON;
			}
			else
			{
				LED_GR_OFF;
			}
		}
	}
}



//=========================================================================
void Http_Server_STA_mode(void)
//=========================================================================
{

////	// wenn auf port 80 stoppen
//	_u16 port = 80;
//
//    sl_NetAppStop(	SL_NET_APP_HTTP_SERVER_ID);
//
//    sl_NetAppSet(	SL_NET_APP_HTTP_SERVER_ID,
//    				NETAPP_SET_GET_HTTP_OPT_PORT_NUMBER,
//    				2,
//    				(_u8) * port );
//
//    retVal = sl_Stop(SL_STOP_TIMEOUT);
//    retVal = sl_Start(0, 0, 0);
//
//    sl_NetAppstart(	SL_NET_APP_HTTP_SERVER_ID);


    //
//    // get Socket
//    int socketIndex = getSocket();
//
//    // get socket handle
//    int socketHandle = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_IPPROTO_TCP);
//
//    // bind socket
//
//    //
//    //bind the socket to the requested port and check for success
//    //if failure, gracefully close the socket and return failure
//    //
//    SlSockAddrIn_t portAddress;
//    portAddress.sin_family = SL_AF_INET;
//    portAddress.sin_port = sl_Htons(_port);
//    portAddress.sin_addr.s_addr = 0;
//    int enableOption = 1;
//
//    sl_SetSockOpt(socketHandle, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &enableOption, sizeof(enableOption));
//    sl_SetSockOpt(socketHandle, SL_SOL_SOCKET, SL_SO_KEEPALIVE, &enableOption, sizeof(enableOption));
//
//    int iRet = sl_Bind(socketHandle, (SlSockAddr_t*)&portAddress, sizeof(SlSockAddrIn_t));
//    if (iRet < 0) {
//        sl_Close(socketHandle);
//        return;
//    }
//
//    //
//    //Make the socket start listening for incoming tcp connections
//    //(backlog of length 0)
//    //
//    iRet = sl_Listen(socketHandle, 0);
//    if (iRet < 0) {
//        sl_Close(socketHandle);
//        return;
//    }
//
//    //
//    //set socket operation to be non blocking
//    //
//    long NonBlocking = true;
//    iRet = sl_SetSockOpt(socketHandle, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &NonBlocking, sizeof(NonBlocking));
//    if (iRet < 0) {
//        sl_Close(socketHandle);
//        return;
//    }
//
//    //
//    //Simplelink api calls are done, so set the object's variables
//    //
//    _socketIndex = socketIndex;
//    WiFiClass::_handleArray[socketIndex] = socketHandle;
//    WiFiClass::_portArray[socketIndex] = _port;
//    WiFiClass::_typeArray[socketIndex] = TYPE_TCP_SERVER;



}



//=========================================================================
int set_port_number(_u16 num)
//=========================================================================
{
    _NetAppHttpServerGetSet_port_num_t port_num;
    _i32 status = -1;

    port_num.port_number = num;

    /*Need to restart the server in order for the new port number configuration
     *to take place */
    status = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
    sprintf(Httpbuffer,"set_port_number sl_NetAppStop = %d", (int) status);
    usart2_send(Httpbuffer);

    status  = sl_NetAppSet (SL_NET_APP_HTTP_SERVER_ID, NETAPP_SET_GET_HTTP_OPT_PORT_NUMBER,
                  sizeof(_NetAppHttpServerGetSet_port_num_t), (_u8 *)&port_num);
    sprintf(Httpbuffer,"set_port_number sl_NetAppSet = %d", (int) status);
    usart2_send(Httpbuffer);

    status = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
    sprintf(Httpbuffer,"set_port_number sl_NetAppStart = %d", (int) status);
    usart2_send(Httpbuffer);

    return SUCCESS;
}



//=========================================================================
int set_authentication_check (_u8 enable)
//=========================================================================
{
    _NetAppHttpServerGetSet_auth_enable_t auth_enable;
    _i32 status = -1;

    auth_enable.auth_enable = enable;
    status = sl_NetAppSet(SL_NET_APP_HTTP_SERVER_ID, NETAPP_SET_GET_HTTP_OPT_AUTH_CHECK,
                 sizeof(_NetAppHttpServerGetSet_auth_enable_t), (_u8 *)&auth_enable);
    sprintf(Httpbuffer,"set_authentication_check sl_NetAppSet = %d", (int) status);
    usart2_send(Httpbuffer);

    return SUCCESS;
}



//=========================================================================
int get_auth_name (_u8 *auth_name)
//=========================================================================
{
    _u8 len = MAX_AUTH_NAME_LEN;
    _i32 status = -1;

    status = sl_NetAppGet(SL_NET_APP_HTTP_SERVER_ID, NETAPP_SET_GET_HTTP_OPT_AUTH_NAME,
                 &len, (_u8 *) auth_name);
    sprintf(Httpbuffer,"get_auth_name sl_NetAppGet = %d", (int) status);
    usart2_send(Httpbuffer);

    auth_name[len] = '\0';

    return SUCCESS;
}



//=========================================================================
int get_auth_password (_u8 *auth_password)
//=========================================================================
{
    _u8 len = MAX_AUTH_PASSWORD_LEN;
    _i32 status = -1;

    status = sl_NetAppGet(SL_NET_APP_HTTP_SERVER_ID, NETAPP_SET_GET_HTTP_OPT_AUTH_PASSWORD,
                                                &len, (_u8 *) auth_password);
    sprintf(Httpbuffer,"get_auth_password sl_NetAppGet = %d", (int) status);
    usart2_send(Httpbuffer);

    auth_password[len] = '\0';

    return SUCCESS;
}



//=========================================================================
int get_auth_realm (_u8 *auth_realm)
//=========================================================================
{
    _u8 len = MAX_AUTH_REALM_LEN;
    _i32 status = -1;

    status = sl_NetAppGet(SL_NET_APP_HTTP_SERVER_ID, NETAPP_SET_GET_HTTP_OPT_AUTH_REALM,
                 &len, (_u8 *) auth_realm);
    sprintf(Httpbuffer,"get_auth_realm sl_NetAppGet = %d", (int) status);
    usart2_send(Httpbuffer);

    auth_realm[len] = '\0';

    return SUCCESS;
}



//=========================================================================
int get_device_urn (_u8 *device_urn)
//=========================================================================
{
    _u8 len = MAX_DEVICE_URN_LEN;
    _i32 status = -1;

    status = sl_NetAppGet(SL_NET_APP_DEVICE_CONFIG_ID, NETAPP_SET_GET_DEV_CONF_OPT_DEVICE_URN,
                 &len, (_u8 *) device_urn);
    sprintf(Httpbuffer,"get_device_urn sl_NetAppGet = %d", (int) status);
     usart2_send(Httpbuffer);

    device_urn[len] = '\0';

    return SUCCESS;
}



//=========================================================================
int get_domain_name (_u8 *domain_name)
//=========================================================================
{
    _u8 len = MAX_DOMAIN_NAME_LEN;
    _i32 status = -1;

    status = sl_NetAppGet(SL_NET_APP_DEVICE_CONFIG_ID, NETAPP_SET_GET_DEV_CONF_OPT_DOMAIN_NAME,
                 &len, (_u8 *)domain_name);
    sprintf(Httpbuffer,"get_domain_name sl_NetAppGet = %d", (int) status);
     usart2_send(Httpbuffer);

    domain_name[len] = '\0';

    return SUCCESS;
}



//=========================================================================
int initializeAppVariables(void)
//=========================================================================
{
    WiFi_Status = 0;
    memset(g_auth_name, 0, sizeof(g_auth_name));
    memset(g_auth_password, 0, sizeof(g_auth_name));
    memset(g_auth_realm, 0, sizeof(g_auth_name));
    memset(g_domain_name, 0, sizeof(g_auth_name));
    memset(g_device_urn, 0, sizeof(g_auth_name));
    return SUCCESS;
}



//=========================================================================
void get_wetter(char*host,int port, char *api, char *city)
//=========================================================================
{
//	unsigned long 	ip;
//	int 			sock;
//	SlSockAddrIn_t 	addr;
//	int 			size;
//	char 			buff[256];
//	unsigned char 	value;
//
//	sl_Start(0,0,0);
//
//	sl_NetAppDnsGetHostByName(host, strlen(host), &ip, SL_AF_INET);
//
//	sock = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_SEC_SOCKET);
//
//	value = SL_SO_SEC_METHOD_SSLV3;
//	sl_SetSockOpt(g_SockID, SL_SOL_SOCKET, SL_SO_SECMETHOD, &value, sizeof(value));
//
//	value = SL_SO_SEC_MASK_SSL_RSA_WITH_RC4_128_SHA;
//	sl_SetSockOpt(g_SockID, SL_SOL_SOCKET, SL_SO_SECURE_MASK, &value, sizeof(value));
//
//	addr.sin_family			= SL_AF_INET;
//	addr.sin_port			= sl_Htons(port);
//	addr.sin_addr.s_addr	=sl_Htons(ip);
//
//	size = sizeof(SlSockAddrIn_t);
//
//	sl-Connect(sock, ( SlSockAddr_t *)&addr, size);
//
//	sprintf(buff, api, city);
//	sl_Send(sock, buff, strlen(buff), 0);
//	sl_Recv(sock, &buff[0], sizeof(buff), 0);
//
//	// text
//
//	sl_Stop(100);

}



//=========================================================================
void start_HttpServer_in_STA_Mode(void)
//=========================================================================
{
//
//	// server auf port 80 stoppen
//	sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
//
//	// Test ob Socket verfügbar
//	// ?
//
//	// Socket Handle holen
//	int socketHandle = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_IPPROTO_TCP);
//
//	// binden des Sockets auf den Port
//	SlSockAddrIn_t portAddress;
//	portAddress.sin_family = SL_AF_INET;
//	portAddress.sin_port = sl_Htons(80);
//	portAddress.sin_addr.s_addr = 0;
//	int enableOption = 1;
//
//	sl_SetSockOpt(socketHandle, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &enableOption, sizeof(enableOption));
//	sl_SetSockOpt(socketHandle, SL_SOL_SOCKET, SL_SO_KEEPALIVE, &enableOption, sizeof(enableOption));
//
//	int iRet = sl_Bind(socketHandle, (SlSockAddr_t*)&portAddress, sizeof(SlSockAddrIn_t));
//	if (iRet < 0)
//	{
//		sl_Close(socketHandle);
//		return;
//	}
//
//
//	//
//	//Make the socket start listening for incoming tcp connections
//	//(backlog of length 0)
//	//
//	iRet = sl_Listen(socketHandle, 0);
//	if (iRet < 0)
//	{
//		sl_Close(socketHandle);
//		return;
//	}
//
//	//
//	//set socket operation to be non blocking
//	//
//	long NonBlocking = true;
//	iRet = sl_SetSockOpt(socketHandle, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &NonBlocking, sizeof(NonBlocking));
//	if (iRet < 0)
//	{
//		sl_Close(socketHandle);
//		return;
//	}
//
//	//
//	//Simplelink api calls are done, so set the object's variables
//	//
////	_socketIndex = socketIndex;
////	WiFiClass::_handleArray[socketIndex] = socketHandle;
////	WiFiClass::_portArray[socketIndex] = _port;
////	WiFiClass::_typeArray[socketIndex] = TYPE_TCP_SERVER;
//
//
//
//	//

}
