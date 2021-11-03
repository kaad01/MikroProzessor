#include "CC3100_EventHandler.h"


//=========================================================================
uint32_t  WiFi_Status = 0;
//=========================================================================
// folgende Zustände sind für das WiFi Modul vorgesehen:
// STATUS_BIT_CONNECTION
// STATUS_BIT_STA_CONNECTED
// STATUS_BIT_IP_ACQUIRED
// STATUS_BIT_IP_LEASED
// STATUS_BIT_CONNECTION_FAILED
// STATUS_BIT_P2P_NEG_REQ_RECEIVED
// STATUS_BIT_SMARTCONFIG_DONE
// STATUS_BIT_SMARTCONFIG_STOPPED



//=========================================================================
// Funktionen
//=========================================================================



//=========================================================================
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
//=========================================================================
{

    usart2_send(" [WLAN EVENT]\n\r");

    if(pWlanEvent == NULL) { usart2_send(" [WLAN EVENT] NULL Pointer Error \n\r"); }

    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {	// STA or P2P client connection indication event
			// pWlanEvent->EventData.STAandP2PModeWlanConnected main fields:
			//	- ssid_name
			//	- ssid_len
			//	- bssid
			//	- go_peer_device_name
			//	- go_peer_device_name_len
            usart2_send(" [WLAN EVENT] SL_WLAN_CONNECT_EVENT \n\r");
            SET_STATUS_BIT(WiFi_Status, STATUS_BIT_CONNECTION);

            //memcpy(g_AP_Name, pWlan->EventData.STAandP2PModeWlanConnected.ssid_name, pWlan->EventData.STAandP2PModeWlanConnected.ssid_len);

            /*
             * Information about the connected AP (like name, MAC etc) will be
             * available in 'slWlanConnectAsyncResponse_t' - Applications
             * can use it if required
             *
             * slWlanConnectAsyncResponse_t *pEventData = NULL;
             * pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
             *
             */
        }
        break;


        case SL_WLAN_DISCONNECT_EVENT:
        {	// STA or P2P client disconnection event
        	// pWlanEvent->EventData.STAandP2PModeDisconnected main fields:
        	//	- ssid_name
        	//	- ssid_len
        	//	- reason_code
            usart2_send(" [WLAN EVENT] SL_WLAN_DISCONNECT_EVENT \n\r");

            CLR_STATUS_BIT(WiFi_Status, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(WiFi_Status, STATUS_BIT_IP_ACQUIRED);

            slWlanConnectAsyncResponse_t*  pEventData = NULL;
            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
            	usart2_send(" Device disconnected from the AP on application's request \n\r");
            }
            else
            {
                usart2_send(" Device disconnected from the AP on an ERROR..!! \n\r");
            }
        }
        break;


        case SL_WLAN_STA_CONNECTED_EVENT:
        {	// AP/P2P(Go) connected STA/P2P(Client)
        	// pWlanEvent->EventData.APModeStaConnected fields:
			//	- go_peer_device_name
			//	- mac
			//	- go_peer_device_name_len
			//	- wps_dev_password_id
			//	- own_ssid:  relevant for event sta-connected only
			//	- own_ssid_len:  relevant for event sta-connected only
            usart2_send(" [WLAN EVENT] SL_WLAN_STA_CONNECTED_EVENT \n\r");

        	SET_STATUS_BIT(WiFi_Status, STATUS_BIT_STA_CONNECTED);
        }
        break;


        case SL_WLAN_STA_DISCONNECTED_EVENT:
        { 	// AP/P2P(Go) disconnected STA/P2P(Client)
        	// pWlanEvent->EventData.APModestaDisconnected fields:
        	//	- go_peer_device_name
			//  - mac
			//  - go_peer_device_name_len
			//  - wps_dev_password_id
			//  - own_ssid:  relevant for event sta-connected only
			//  - own_ssid_len:  relevant for event sta-connected only
        	usart2_send(" [WLAN EVENT] SL_WLAN_STA_DISCONNECTED_EVENT \n\r");

        	CLR_STATUS_BIT(WiFi_Status, STATUS_BIT_STA_CONNECTED);
            CLR_STATUS_BIT(WiFi_Status, STATUS_BIT_IP_LEASED);
        }
        break;


        case SL_WLAN_SMART_CONFIG_COMPLETE_EVENT:
        { 	// pSlWlanEvent->EventData.smartConfigStartResponse fields:
        	//	- status
        	//	- ssid_len
        	//	- ssid
        	//	- private_token_len
        	//	- private_token
        	usart2_send(" [WLAN EVENT] SL_WLAN_SMART_CONFIG_COMPLETE_EVENT \n\r");
        }
        break;


        case SL_WLAN_SMART_CONFIG_STOP_EVENT:
        { 	// pWlanEvent->EventData.smartConfigStopResponse fields:
        	// 	- status
        	usart2_send(" [WLAN EVENT] SL_WLAN_SMART_CONFIG_STOP_EVENT \n\r");
        }
        break;


        case SL_WLAN_P2P_DEV_FOUND_EVENT:
        { 	// pWlanEvent->EventData.P2PModeDevFound fields:
        	// 	 go_peer_device_name
        	// 	- mac
        	// 	- go_peer_device_name_len
        	// 	- wps_dev_password_id
        	// 	- own_ssid:  relevant for event sta-connected only
        	// 	- own_ssid_len:  relevant for event sta-connected only
        	usart2_send(" [WLAN EVENT] SL_WLAN_P2P_DEV_FOUND_EVENT \n\r");
        }
        break;


        case SL_WLAN_P2P_NEG_REQ_RECEIVED_EVENT:
        { 	// pWlanEvent->EventData.P2PModeNegReqReceived fields:
        	//	- go_peer_device_name
        	//  	- mac
        	//  	- go_peer_device_name_len
        	//  	- wps_dev_password_id
        	//  	- own_ssid:  relevant for event sta-connected only
        	usart2_send(" [WLAN EVENT] SL_WLAN_P2P_NEG_REQ_RECEIVED_EVENT \n\r");
        }
        break;


        case SL_WLAN_CONNECTION_FAILED_EVENT:
        { 	// P2P only
        	// pWlanEvent->EventData.P2PModewlanConnectionFailure fields:
        	//	- status
        	usart2_send(" [WLAN EVENT] SL_WLAN_CONNECTION_FAILED_EVENT \n\r");
        }
        break;


        default:
        {
            usart2_send(" [WLAN EVENT] Unexpected event \n\r");
        }
        break;
    }
}




//=========================================================================
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
//=========================================================================
{

    usart2_send(" [NETAPP EVENT]\n\r");

    if(pNetAppEvent == NULL) { usart2_send(" [NETAPP EVENT] NULL Pointer Error \n\r"); }

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {	// pSlWlanEvent->EventData.ipAcquiredV4 fields:
            // - ip
            // - gateway
            // - dns
        	usart2_send(" [NETAPP EVENT] SL_NETAPP_IPV4_IPACQUIRED_EVENT \n\r");

            SET_STATUS_BIT(WiFi_Status, STATUS_BIT_IP_ACQUIRED);

            //SlIpV4AcquiredAsync_t *pEventData = NULL;
            //pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
            //g_GatewayIP = pEventData->gateway;
            //g_GatewayIP = pEventData->ip;
            //g_GatewayIP = pEventData->dns;
        }
        break;


        case SL_NETAPP_IP_LEASED_EVENT:
        {   // pSlWlanEvent->EventData.ipLeased fields:
            //	- ip_address
            //	- lease_time
            //	- mac
        	usart2_send(" [NETAPP EVENT] SL_NETAPP_IP_LEASED_EVENT \n\r");

        	SET_STATUS_BIT(WiFi_Status, STATUS_BIT_IP_LEASED);
        }
        break;


        case SL_NETAPP_IP_RELEASED_EVENT:
        {	// pSlWlanEvent->EventData.ipReleased fields:
        	//	- ip_address
        	//  - mac
        	//  - reason
            usart2_send(" [NETAPP EVENT] SL_NETAPP_IP_RELEASED_EVENT\n\r");

        	SET_STATUS_BIT(WiFi_Status, STATUS_BIT_IP_LEASED);
        }
        break;


        default:
        {
            usart2_send(" [NETAPP EVENT] Unexpected event \n\r");
        }
        break;
    }
}




//=========================================================================
void SimpleLinkSockEventHandler(SlSockEvent_t *pSockEvent)
//=========================================================================
{
    usart2_send(" [SOCK EVENT]\n\r");

    if(pSockEvent == NULL) {usart2_send(" [SOCK EVENT] NULL Pointer Error \n\r");}

    switch( pSockEvent->Event )
    {
        case SL_SOCKET_TX_FAILED_EVENT:
        {	// pSockEvent->EventData fields:
        	//    - sd
        	//    - status
        	//SlSockEventData_t *pEventData = NULL;
        	//pEventData = &pSockEvent->EventData;

        	switch( pSockEvent->socketAsyncEvent.SockTxFailData.status)
            {
                case SL_ECLOSE:
                {
                    usart2_send(" [SOCK EVENT] SL_SOCKET_TX_FAILED_EVENT - failed to transmit all queued packets\n\r");
                }
                break;

                default:
                {
                usart2_send(" [SOCK EVENT] SL_SOCKET_TX_FAILED_EVENT - Unexpected event \n\r");
                }
                break;
            }
        }
        break;


        case SL_SOCKET_ASYNC_EVENT:
        {	// pSlSockEvent->EventData fields:
			//	- sd
			//	- type: SSL_ACCEPT  or RX_FRAGMENTATION_TOO_BIG or OTHER_SIDE_CLOSE_SSL_DATA_NOT_ENCRYPTED
			//	- val
        	usart2_send(" [SOCK EVENT] SL_SOCKET_ASYNC_EVENT \n\r");
        }
        break;

        default:
        {
            usart2_send(" [SOCK EVENT] Unexpected event \n\r");
        }
        break;
    }
}



//=========================================================================
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDeviceEvent)
//=========================================================================
{
	char tout[100] = {0x00};
    usart2_send(" [GENERAL EVENT] \n\r");
    switch(pDeviceEvent->Event)
    {
    case SL_DEVICE_FATAL_ERROR_EVENT:
        	{	// pDeviceEvent->EventData.deviceEvent fields:
        		//	- status: An error code indication from the device
                //	- sender: The sender originator which is based on
        		//		SlErrorSender_e enum
          	sprintf(tout," [GENERAL EVENT] SL_DEVICE_FATAL_ERROR_EVENT - ID=%d Sender=%d\n\n",
        	           pDeviceEvent->EventData.deviceEvent.status,  // status of the general event
        	           pDeviceEvent->EventData.deviceEvent.sender);
        	usart2_send(tout);
        	}
        	break;
	default:
	break;
    }
}



//=========================================================================
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
                                  SlHttpServerResponse_t *pHttpResponse)
//=========================================================================
{
    usart2_send(" [HTTP EVENT]\n\r");

    if(pHttpEvent == NULL || pHttpResponse == NULL){
    	usart2_send(" [HTTP EVENT] NULL Pointer Error \n\r");
    	}

    switch (pHttpEvent->Event)
    {
    case SL_NETAPP_HTTPGETTOKENVALUE_EVENT:
    	{	// pHttpEvent->EventData fields:
			//	 - httpTokenName
			//		 - data
			//		 - len
			// pHttpResponse->ResponseData fields:
			//		 - data
			//		 - len

    	// SET_STATUS_BIT(WiFi_Status, SL_NETAPP_HTTPGETTOKENVALUE_EVENT);
   		usart2_send(" [HTTP EVENT] SL_NETAPP_HTTPGETTOKENVALUE_EVENT \n\r");
   		Http_Get_Event(pHttpEvent,pHttpResponse);
    	}
    	break;


    case SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT:
		{	// pHttpEvent->EventData.httpPostData fields:
			//	 - action
			//	 - token_name
			//	 - token_value
			// pHttpResponse->ResponseData fields:
			//	 - data
			//	 - len

		// SET_STATUS_BIT(WiFi_Status, SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT);
		usart2_send(" [HTTP EVENT] SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT \n\r");
	    Http_Post_Event(pHttpEvent,pHttpResponse);
		}
		break;

    	default:
    	break;
    }
}
