/*
 * Copyright (C) 2014 Markus Hoffmann <mackone@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. This LICENSE in it's most up to date version can be
 * found under: https://www.gnu.org/licenses/lgpl-2.1.html
 */


#include "CC3100.h"
#include "CC3100_Firmware.h"

//=========================================================================
// Zugangsdaten für STA- und AP- Mode
//=========================================================================
//SlSecParams_t 			WLAN_STA_Passwort 	= {0};
//Sl_WlanNetworkEntry_t	WLAN_STA_Zugang 	= {{0}};
//unsigned char SNTPserver[30] = "nist1-nj2.ustiming.org";

//=========================================================================
// Zugangsdaten STA Mode
//=========================================================================
const char SSID_STA_MODE[30] 		= "MPP_IoT";
const char PASSWORD_STA_MODE[30] 	= "#cc3100#";
unsigned char SEC_TYPE_STA_MODE		= SL_SEC_TYPE_WPA_WPA2;
//=========================================================================
// Zugangsdaten AP Mode
//=========================================================================
const char SSID_AP_MODE[30]			= "hwp1";
const char PASSWORD_AP_MODE[30]		= "fu1234";
unsigned char SEC_TYPE_AP_MODE		= SL_SEC_TYPE_OPEN;


//=========================================================================
uint32_t  WiFi_RTC = 0;
uint32_t CC3100_select_status = 0;
unsigned char Stop_CC3100_select = 0;
//=========================================================================

//========================================================================
// Select Infos
//========================================================================
CC_fd_set readds;
CC_fd_set writeds;
CC_fd_set exceptsds;

timeval timeout;
ipInfo_t senderIP; 							//	Info des Senders (IP und port)


sockaddr sockets[CC3100_MAX_SOCK_COUNT]; 	//	Array mit allen Socket
sockaddr from[CC3100_MAX_SOCK_COUNT];		// 	Array mit allen IP Adressen
sockaddr my_IP;


char rx_buf[CC3100_MAX_SOCK_COUNT][2048]; 	// 	Array der Empfangspuffer
char tx_buf[CC3100_MAX_SOCK_COUNT][2048]; 	// 	Array der Empfangspuffer
unsigned char rx_daten[2048];				// Array der Empfangsdaten


//	Array mit den Socket Handles
unsigned long 	socket_Handles[CC3100_MAX_SOCK_COUNT] = {99,99,99,99,99,99,99,99,99,99};

//	Array für den socket Typ (UDP, TCP, RAW...)
unsigned short	socket_Type[CC3100_MAX_SOCK_COUNT] = {0};

// 	Arrays mit den callback Funktionen für select
void (*read_cbs[CC3100_MAX_SOCK_COUNT])(char* rx, uint16_t rx_len, sockaddr* from, uint16_t socket_Pos);
long (*write_cbs[CC3100_MAX_SOCK_COUNT])(char* tx, uint16_t tx_len, uint16_t socket_Position);
void (*except_cbs[CC3100_MAX_SOCK_COUNT])(uint16_t socket_Pos);


int handle_FTP_Command = 99;
int handle_FTP_Data = 99;

//unsigned long handle_mDNS = 99;		// SocketHandler #1
//sockaddr S1_mDNS; 				// Broadcast Socket on 224.0.0.251:5353

//unsigned long handle_HTTP = 99; 	// SocketHandler #2
//sockaddr S2_TCP; 					// TCP Socket for listening on Port 80

unsigned long handle_TCP = 99; 		// SocketHandler #3 TCP

unsigned long handle_NTP = 99; 		// SocketHandler #3 NTP
//sockaddr S3_NTP; 					// Für NTP Abfrage

unsigned long handle_MQTT = 99; 		// SocketHandler #3 NTP
//sockaddr S3_NTP; 					// Für NTP Abfrage

_Bool S3_WaitForAnswer = false; 	// Wird auf eine Anfrage gewartet

//unsigned long handle_Protokoll = 99;// Socket Handler #4
//sockaddr S4_unused; 				// TCP Socket for ServerConnection on Port 1075
//_Bool S4_WaitForAnswer = false;

//unsigned long handle_NetBIOS = 99;	// SocketHandler #2
//sockaddr S_NetBIOS; 				// TCP Socket for listening on Port 80

//unsigned long handle_HTTP_Client = 99;// Handle für Clients die sich mit dem Modul verbinden

//sockaddr 	S_ConnectedClient;
//socklen_t	s_ConnectedLen = sizeof(sockaddr);
//_Bool 		S_ClientConnected = false;

//unsigned long handle_DB = 99; 		// Handle für die Datenbank Verbindung





//=========================================================================
unsigned long CC3100_openSocketul(
				unsigned long ip_l,
				uint16_t port,
				unsigned char ConnectionType,
				void (*read_cb)(char* rx, uint16_t rx_len, sockaddr* from, uint16_t socket_Pos),
				long (*write_cb)(char* tx, uint16_t tx_len, uint16_t socket_Position),
				void (*except_cb)(uint16_t pos))
//=========================================================================
{
	unsigned long ret = 0;
	unsigned char ip1 = (ip_l & 0xFF000000) >> 24; 	// First octet of destination IP
	unsigned char ip2 = (ip_l & 0x00FF0000) >> 16; 	// Second Octet of destination IP
	unsigned char ip3 = (ip_l & 0x0000FF00) >> 8; 	// Third Octet of destination IP
	unsigned char ip4 = (ip_l & 0x000000FF); 		// Fourth Octet of destination IP
	ret = CC3100_openSocket(
				ip1, ip2, ip3, ip4,
				port,
				ConnectionType,
				read_cb,
				write_cb,
				except_cb
				);
	return ret ;
}





//=========================================================================
unsigned long CC3100_openSocket	(
				unsigned char ip1,
				unsigned char ip2,
				unsigned char ip3,
				unsigned char ip4,
				uint16_t port,
				unsigned char ConnectionType,
				void (*read_cb)(char* rx, uint16_t rx_len, sockaddr* from, uint16_t socket_Pos),
				long (*write_cb)(char* tx, uint16_t tx_len, uint16_t socket_Position),
				void (*except_cb)(uint16_t pos) )
//=========================================================================
{
	long res = 0;
	//int i = 0;
	sockaddr s;
	memset(&s, 0, sizeof(sockaddr));
	int handle = 99;
	int handle_index = 0;


	//=========================================================================
	//==== handle für Socket holen
	//=========================================================================
	if (ConnectionType == TCP_Client || ConnectionType == TCP_Server)
	// handle = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	{
		handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//TODO
	}
	else
	{
		handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	if (handle < 0)
	{
		usart2_printf("=> Fehler: Socket konnte nicht erstellt werden\r\n");
		return 99;
	}

	//=========================================================================
	//==== Socket handle und Protokolltype eintragen
	//=========================================================================
	handle_index = handle;
	if (handle >= 16) { handle_index = handle - 16;	}

	socket_Handles[handle_index] = handle;

	if (ConnectionType == TCP_Client || ConnectionType == TCP_Server){
		socket_Type[handle_index] = IPPROTO_TCP;
	}
	else{
		socket_Type[handle_index] = IPPROTO_UDP;
	}

	//=========================================================================
	//==== Socket Address family and Protocol- specific address information
	//=========================================================================
	sockets[handle_index].sa_family	= AF_INET;
	sockets[handle_index].sa_data[0]	= (port & 0xFF00) >> 8;
	sockets[handle_index].sa_data[1]	= (port & 0x00FF);
	sockets[handle_index].sa_data[2]	= ip1;
	sockets[handle_index].sa_data[3]	= ip2;
	sockets[handle_index].sa_data[4]	= ip3;
	sockets[handle_index].sa_data[5]	= ip4;

	//=========================================================================
	//==== Socket Callbacks anmelden
	//=========================================================================
	read_cbs[handle_index] = read_cb;
	write_cbs[handle_index] = write_cb;
	except_cbs[handle_index] = except_cb;

	//=========================================================================
	//==== Socket connect für UDP_Client und TCP_Client durchführen
	//=========================================================================
	if (  ConnectionType == UDP_Client ||  ConnectionType == TCP_Client) {
		res = connect(handle, &sockets[handle_index], sizeof(sockaddr));
		if (res < 0) {
			usart2_printf("Fehler: Socket(%d) nicht verbunden\r\n", handle);
			return 99;
		}
		else {
			usart2_printf("=>Socket(%d) geöffnet\r\n", handle);
			return handle;
		}
	}

	//=========================================================================
	//==== Socket bind
	//=========================================================================
	if ( ConnectionType != TCP_Server )	{
		usart2_printf("=> Socket (%d) geöffnet \r\n", handle);
		return handle;
	}

	//=========================================================================
	//==== für nicht TCP Sockets
	//=========================================================================
	if ((ConnectionType != TCP_Client) && (ConnectionType != TCP_Server) ) {
		sockets[handle_index].sa_data[2] = 0;
		sockets[handle_index].sa_data[3] = 0;
		sockets[handle_index].sa_data[4] = 0;
		sockets[handle_index].sa_data[5] = 0;
	}

	res = bind(handle, &sockets[handle], sizeof(sockaddr));
	if (res < 0){
		usart2_printf("Fehler: Socket(%d) nicht gebunden\r\n", handle);
		return 99;
	}
	else{
		usart2_printf("==> Socket (%d) gebunden an IP:%d.%d.%d.%d : %d\r\n",
				handle, ip1, ip2, ip3, ip4, port);
	}

	memset(&sockets[handle_index].sa_data[2], 0, 12);

	if ( (ConnectionType == UDP_Server) || (ConnectionType == TCP_Server) )	{
		res = listen(handle, 0);
		if (res < 0){
			usart2_printf("Fehler: Auf Socket (%d) kann nicht gehört werden\r\n", handle);
			return 99;
		}
		else {
			usart2_printf("==> Socket (%d) bereit zum akzeptieren auf Port %d\r\n", handle, port);
		}
	}

	sockets[handle_index].sa_data[2] = ip1;
	sockets[handle_index].sa_data[3] = ip2;
	sockets[handle_index].sa_data[4] = ip3;
	sockets[handle_index].sa_data[5] = ip4;

	return handle;
}





//=========================================================================
unsigned long CC3100_closeSocket(uint16_t socketNr)
//=========================================================================
{
	long res = 0;
	if (socketNr != 99) {
		res = close(socketNr);
		if (res == 0) {
			if (socketNr >= 16) {socketNr = socketNr - 16;}
			usart2_printf("=> Socket (%d) geschlossen \r\n", socket_Handles[socketNr]);
			socket_Handles[socketNr] = 99;
			memset(&sockets[socketNr], 0, sizeof(sockaddr));
			socket_Type[socketNr] 	= 0;
			read_cbs[socketNr] 		= 0;
			write_cbs[socketNr] 	= 0;
			except_cbs[socketNr] 	= 0;
		}
	}
	return 99;
}





//=========================================================================
long CC3100_select(void)
//=========================================================================
{
	//char buf[20];
	unsigned short i;
	long result 			= 0;
	long readresult 		= 0;
	long sendresult 		= 0;
	long send_length 		= 0;
	unsigned long maxSocket = 0;
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;
	socklen_t fromlen = sizeof(sockaddr);

	CC_FD_ZERO(&readds);
	CC_FD_ZERO(&writeds);
	CC_FD_ZERO(&exceptsds);

	for (i = 0; i < CC3100_MAX_SOCK_COUNT; i++)
	{
		if (socket_Handles[i] != 99)
		{
			CC_FD_SET(socket_Handles[i], &readds);
			CC_FD_SET(socket_Handles[i], &writeds);
			CC_FD_SET(socket_Handles[i], &exceptsds);
			if (i>maxSocket)  maxSocket = i;
		}
	}
	//======================================================================
	result = select(maxSocket + 1, &readds, &writeds, &exceptsds, &timeout);
	// Todo war wohl falsch da wenn 0 dann folgt blockierendes lesen
	// <0 fehler
	// 0 keine Daten
	// 1 Socket hat daten
	//======================================================================
	// deshalb reulst > 0
	if (result>0)
	{
		//##### SOCKET Handling #############
		for (i = 0; i < CC3100_MAX_SOCK_COUNT; i++)
		{
			if (socket_Handles[i] != 99)
			{
				//##############################################
				if (CC_FD_ISSET(socket_Handles[i],&exceptsds))
				{
					if (except_cbs[i] != 0)
					{
						(*except_cbs[i])(i);
					}
				}
				//##############################################
				if (CC_FD_ISSET(socket_Handles[i],&readds))
				{	// Puffer leeren
					memset(rx_buf[i], 0, sizeof(rx_buf[i]));
					// Daten in Puffer füllen
					switch (socket_Type[i])
					{
						case IPPROTO_TCP:
							readresult = recv(socket_Handles[i], &rx_buf[i], sizeof(rx_buf[i]), 0);
							if (readresult == 0) {
								uart_send("\r\n###>Gegenstelle hat den Socket geschlossen\r\n");
								CC3100_closeSocket(socket_Handles[i]);
								}
							else{
							memcpy(&from[i], &sockets[i], sizeof(sockaddr));
							}
							break;

						case IPPROTO_UDP:
							readresult = recvfrom(socket_Handles[i], &rx_buf[i], sizeof(rx_buf[i]), 0, &from[i], &fromlen);
							break;

						case IPPROTO_RAW:

							break;

						default:
							usart2_send(" FEHLER: RX-Protokoll nicht unterstützt\r\n");
							continue;
					}
					if (readresult > 0 && readresult <= sizeof(rx_buf[i]))
					{
						if (read_cbs[i] != 0)
						{ 	// Aufruf der angemeldeten Callback Funktion
							// mit Übergabe Puffer, Länge, IP und Index
							(*read_cbs[i])((char*) rx_buf[i], readresult, &from[i], i);
						}
					}
					else
					{
						//sprintf(buf,"###>read=%d\r\n",readresult);
						//uart_send("\r\n###>Fehler beim Lesen des Pakets\r\n");
						//uart_send(buf);
					}
				}
				//##############################################
				if (CC_FD_ISSET(socket_Handles[i],&writeds))
				{
					if (write_cbs[i] != 0)
					{	// Callback wird aufgerufen und liefert zu sendende Länge zurück
						send_length = (*write_cbs[i])(tx_buf[i], sizeof(tx_buf[i]), i);
						if (send_length > 0)
						{
							switch (socket_Type[i])
							{
							case IPPROTO_TCP:
								sendresult = send(		socket_Handles[i],
														&tx_buf[i],
														send_length,
														0
													);
								break;
							case IPPROTO_UDP:
								sendresult = sendto(	socket_Handles[i],
														&tx_buf[i],
														send_length,
														0,
														&from[i],
														fromlen
													);

							case IPPROTO_RAW:
								break;

								break;
							default:
								usart2_send(
										" FEHLER: Protokoll beim Senden nicht unterstützt\r\n");
								continue;
							}
						}
						if (sendresult >= 0)
						{
							memset(tx_buf[i],0,sizeof(tx_buf[i]));
						}
						send_length = 0;
					}
				}
			}
		}
	}
	CC3100_select_status = 0;
	return result;
}











//=========================================================================
void CC3100_set_in_AP_Mode()
//=========================================================================
{

	int retVal = 0;
	usart2_send("#### CC3100_set_AP_Mode()\r\n");

	retVal = sl_Stop(100);
	retVal = sl_Start(0, 0, 0);

	if (retVal != ROLE_STA)
	{
		usart2_send("warten auf AP fertig\r\n");
		if (retVal == ROLE_AP)
		{
			while ( 0 == IS_IP_ACQUIRED(WiFi_Status)) {;}
		}
	}

	retVal = sl_WlanSetMode(ROLE_AP);
	retVal = sl_Stop(100);
	retVal = sl_Start(0, 0, 0);

	if (retVal==ROLE_AP)
		{
		usart2_send("AP-Mode aktiv\r\n");
		}
	//start_HttpServer();
}






//=========================================================================
void CC3100_set_in_STA_Mode(char webserver)
//=========================================================================
{
	int retVal = 0;

	retVal = sl_Start(NULL, NULL, NULL);

	if (retVal != ROLE_STA)
	{
		if (retVal == ROLE_AP)
		{
			while ( 0 == IS_IP_ACQUIRED(WiFi_Status)) {;}
		}
	}

	retVal = sl_WlanSetMode(ROLE_STA);

	retVal = sl_WlanPolicySet (SL_POLICY_SCAN, 0 , 0 , 0) ;

	retVal = sl_WlanPolicySet(SL_POLICY_CONNECTION, SL_CONNECTION_POLICY(0,0,0,0,0),0,0);

	retVal = sl_WlanProfileDel(0xFF);

	unsigned char val = 1;
	retVal = sl_NetCfgSet(	SL_IPV4_STA_P2P_CL_DHCP_ENABLE,
							IPCONFIG_MODE_ENABLE_IPV4,
							sizeof(unsigned char),
							&val);

	unsigned char power = 0;
	retVal = sl_WlanSet(	SL_WLAN_CFG_GENERAL_PARAM_ID,
							WLAN_GENERAL_PARAM_OPT_STA_TX_POWER,
							sizeof(power),
							(_u8 *)&power);
	//retVal = sl_WlanPolicySet(SL_POLICY_PM , SL_LONG_SLEEP_INTERVAL_POLICY, NULL, 0);

	retVal = sl_WlanPolicySet(SL_POLICY_PM , SL_NORMAL_POLICY, NULL, 0);

	retVal = sl_NetAppMDNSUnRegisterService(0, 0);

	sl_Stop(100);
	sl_Start(NULL, NULL, NULL);

	SlSecParams_t Zugangsdaten;
	Zugangsdaten.Key = (_i8*)PASSWORD_STA_MODE;
	Zugangsdaten.KeyLen = strlen(PASSWORD_STA_MODE);
	Zugangsdaten.Type = SEC_TYPE_STA_MODE;

	retVal = sl_WlanConnect((_i8*)SSID_STA_MODE, strlen(SSID_STA_MODE), 0, &Zugangsdaten, 0);

	while((!IS_CONNECTED(WiFi_Status)) || (!IS_IP_ACQUIRED(WiFi_Status))) {;}

	if (webserver) { sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID); }

	usart2_send("Verbindung OK\r\n");

}





//=========================================================================
void CC3100_start_STA_Mode_Profil()
//=========================================================================
{

	int retVal = 0;
	usart2_send("STA-Mode mit Profil starten\r\n");
	retVal = sl_Start(NULL, NULL, NULL);
		if (retVal==ROLE_STA_ERR || retVal==ROLE_AP_ERR || retVal==ROLE_P2P_ERR) {
			usart2_send("STA-Mode Fehler\r\n");
		}
		if (retVal == ROLE_STA) {
			usart2_send("STA-Mode aktiv\r\n");
			}
}




//=========================================================================
void CC3100_set_STA_Mode_Profil()
//=========================================================================
{

	int retVal = 0;
	usart2_send("===>STA-Mode Profil setzen\r\n");

	usart2_send("sl_Start\r\n");
	retVal = sl_Start(NULL, NULL, NULL);
		if (retVal==ROLE_STA_ERR || retVal==ROLE_AP_ERR || retVal==ROLE_P2P_ERR) {
			usart2_send("STA-Mode Fehler\r\n");
			}
		if (retVal == ROLE_STA) {

			usart2_send("STA-Mode aktiv\r\n");

			usart2_send("sl_WlanProfileDel\r\n");
			retVal = sl_WlanProfileDel(0xFF);
			if (retVal !=0){usart2_send("sl_WlanProfileDel ERROR\r\n");}


		usart2_send("warte auf Verbindung\r\n");
		while((!IS_CONNECTED(WiFi_Status)) || (!IS_IP_ACQUIRED(WiFi_Status))) { _SlNonOsMainLoopTask(); }
		usart2_send("Verbindung OK\r\n");
		while(1) {
			wait_uSek_CC3100(20000);
			usart2_send(".");
			_SlNonOsMainLoopTask();
		}


}

}


//=========================================================================
void CC3100_sf_file_read( unsigned char * datei_name)
//=========================================================================
{
	SlFsFileInfo_t  FsFileInfo;

	_i32	fileHandle 	= -1;
	_u32    Token 		= 0;

	_i32 offset = 0;
	_i32 status = 0;

	char datei_daten[200] = {0};
	_u32 rd_bytes = sizeof(datei_daten);

	Stop_CC3100_select = 1;

    status = sl_FsGetInfo(	(_u8 *)datei_name,
							Token,
							&FsFileInfo);
	if (status == 0)
	{
		sprintf(datei_daten,"\r\nSF File lesen:");
		usart2_send(datei_daten);

		sprintf (datei_daten,"\r\nDatei=%s\r\nFlags=%d\r\nAllocatedLen=%d\r\nFileLen=%d\r\n",
										datei_name,
										(int)FsFileInfo.flags,
										(int)FsFileInfo.AllocatedLen,
										(int)FsFileInfo.FileLen);
		usart2_send(datei_daten);

		if (FsFileInfo.FileLen <= rd_bytes) { rd_bytes = FsFileInfo.FileLen;}

		status = sl_FsOpen( 	(_u8 *)datei_name,
								FS_MODE_OPEN_READ,
								&Token,
								&fileHandle);
		if (status == 0)
		{
			while (1)
			{
				memset(datei_daten,0x00,sizeof(datei_daten));

				status = sl_FsRead(	 fileHandle,
									(_u32)(offset),
									(unsigned char *)datei_daten,
									rd_bytes);
				if ((status < 0)){ break; }

				usart2_send((char*)datei_daten);

				offset = offset + rd_bytes;

				if ((FsFileInfo.FileLen - offset) < rd_bytes)
				{
					rd_bytes = FsFileInfo.FileLen - offset;
					if ((rd_bytes <= 0)){ break; }
				}

			}
		}
		status = sl_FsClose(fileHandle, 0, 0, 0);
		if (status < 0) {usart2_send("\r\nerror\r\n");}
		usart2_send("\r\n");

		Stop_CC3100_select = 0;
	}
}




//=========================================================================
void CC3100_sf_file_erase( unsigned char * datei_name)
//=========================================================================
{
	SlFsFileInfo_t  FsFileInfo;

	_i32	fileHandle 	= -1;
	_u32    Token 		= 0;

	_i32 offset = 0;
	_i32 status = 0;

	char datei_daten[100] = {0};
	_u32 rd_bytes = sizeof(datei_daten);

	Stop_CC3100_select = 1;

	status = sl_FsGetInfo(	(_u8 *)datei_name,
							Token,
							&FsFileInfo);

	if (status == 0)
	{
		sprintf(datei_daten,"\r\nSF File löschen:\r\n");
		usart2_send(datei_daten);

		sprintf (datei_daten,"Datei=%s\r\nFlags=%d\r\nAllocatedLen=%d\r\nFileLen=%d\r\n",
							datei_name,
							(int)FsFileInfo.flags,
							(int)FsFileInfo.AllocatedLen,
							(int)FsFileInfo.FileLen);
		usart2_send(datei_daten);

		if (FsFileInfo.FileLen <= rd_bytes) { rd_bytes = FsFileInfo.FileLen; }

		status = sl_FsOpen( (_u8 *)datei_name,
							FS_MODE_OPEN_READ,
							&Token,
							&fileHandle);
		if (status == 0)
		{
			while (1)
			{
				memset(datei_daten,0x00,sizeof(datei_daten));
				status = sl_FsRead(	fileHandle,
						(_u32)(offset),
						(_u8*)datei_daten,
						rd_bytes);
				if (status < 0)
				{
					sprintf(datei_daten,"\r\nError %d bei File_Read\r\n",(int)status);
					usart2_send(datei_daten);
					break;
				}

				//usart2_send((char *)datei_daten);
				offset = offset + rd_bytes;

				if ((FsFileInfo.FileLen - offset) < rd_bytes)
				{
					rd_bytes = FsFileInfo.FileLen - offset;
					if ((rd_bytes <= 0)){ break; }
				}
			}
		}
		else
		{
			sprintf(datei_daten,"\r\nError %d bei File_Open\r\n",(int)status);
			usart2_send(datei_daten);
		}

		status = sl_FsClose(fileHandle, 0, 0, 0);
		if (status < 0)
		{
			sprintf(datei_daten,"\r\nError %d bei File_Close\r\n",(int)status);
			usart2_send(datei_daten);
		}

		status = sl_FsDel((_u8 *)datei_name, Token);
		if (status <0)
		{
			sprintf(datei_daten,"\r\nError %d bei File_Del\r\n",(int)status);
			usart2_send(datei_daten);
		}
		else
		{
			sprintf(datei_daten,"OK!\r\n");
			usart2_send(datei_daten);
		}

	}
	else
	{
		sprintf(datei_daten,"\r\nError %d bei File_Info zu %s\r\n",(int)status , datei_name);
		usart2_send(datei_daten);
	}

	Stop_CC3100_select = 0;
}




//=========================================================================
int CC3100_set_time(void)
//=========================================================================
{
	int status = 0;

	SlDateTime_t dateTime= {0};
	RTC_TimeTypeDef RTC_Time_Aktuell;
	RTC_DateTypeDef RTC_Date_Aktuell;

	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);
	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);

	dateTime.sl_tm_day	= (_u32) RTC_Date_Aktuell.RTC_Date;
	dateTime.sl_tm_mon	= (_u32) RTC_Date_Aktuell.RTC_Month;
	dateTime.sl_tm_year	= (_u32)(RTC_Date_Aktuell.RTC_Year + 2000);
	dateTime.sl_tm_hour	= (_u32) RTC_Time_Aktuell.RTC_Hours;
	dateTime.sl_tm_min	= (_u32) RTC_Time_Aktuell.RTC_Minutes;
	dateTime.sl_tm_sec	= (_u32) RTC_Time_Aktuell.RTC_Seconds;

	Stop_CC3100_select = 1;

	sl_DevSet(	SL_DEVICE_GENERAL_CONFIGURATION,
				SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
				sizeof(SlDateTime_t),
				(_u8 *)(&dateTime));

	Stop_CC3100_select = 0;

	return status;
}




//=========================================================================
int CC3100_sf_file_write(void)
//=========================================================================
{
    _i32 lFileHandle;
    _u32 ulToken;
    _u32 offset = 0;
    _i32 status;
    _i32 buf_len = 0;
	char daten[100] = {0};
	char name[20] = {0};

	Stop_CC3100_select = true;

	sprintf(name,"www/1tt.txt");
	sprintf(daten,"aaaaaaaa\r\n");
	buf_len = strlen(daten);

    status = sl_FsOpen((_u8*)name, FS_MODE_OPEN_CREATE(1*1024, 0 ), &ulToken, &lFileHandle);
    if (status == 0)
    {
    status = sl_FsWrite(lFileHandle, offset, (_u8 *)daten, buf_len);
    }
    status = sl_FsClose(lFileHandle, 0, 0, 0);

	Stop_CC3100_select = false;

    return 1;
}


//=========================================================================
void CC3100_fw_check(void)
//=========================================================================
{
//##### alte Version #####
//	CC3100R device
//	NWP 2.0.7.0
//	MAC 31.0.0.4.1
//	PHY 1.5.3.3

//##### aktuelle Version #####
//	CC3100R device
//	NWP 2.4.0.2
//	MAC 31.1.3.0.1
//	PHY 1.0.3.34

//	char modul_version[] 	= "CC3100R";
	char nwp_version[] 		= "NWP 2.4.0.2";
	char mac_version[] 		= "MAC 31.1.3.0.1";
	char phy_version[] 		= "PHY 1.0.3.34";
	bool	update = false;
	SlVersionFull 		ver;
    _u8 				pConfigOpt, pConfigLen;
	_i32         		retVal = 0;
	char		  printBuffer[200];

	pConfigOpt = SL_DEVICE_GENERAL_VERSION;
	pConfigLen = sizeof(SlVersionFull);

	Stop_CC3100_select = 1;
    sl_Start(0, 0, 0);

	retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION,&pConfigOpt,&pConfigLen,(_u8 *)(&ver));
	if(retVal < 0)
		{
		sprintf(printBuffer, "\r\nRead FW Version Error Code= %d\r\n", (int)retVal);
		usart2_send(printBuffer);
		}

	sprintf(printBuffer, "NWP %d.%d.%d.%d",
				(_u8)ver.NwpVersion[0],
				(_u8)ver.NwpVersion[1],
				(_u8)ver.NwpVersion[2],
				(_u8)ver.NwpVersion[3]);
	if ( (strcmp(nwp_version, printBuffer) != 0) || !(update == false ) ) {update = true;}

	sprintf(printBuffer, "MAC 31.%d.%d.%d.%d",
				(_u8)ver.ChipFwAndPhyVersion.FwVersion[0],
				(_u8)ver.ChipFwAndPhyVersion.FwVersion[1],
				(_u8)ver.ChipFwAndPhyVersion.FwVersion[2],
				(_u8)ver.ChipFwAndPhyVersion.FwVersion[3]);
	if ( (strcmp(mac_version, printBuffer) != 0) || !(update == false ) ) {update = true;}

	sprintf(printBuffer, "PHY %d.%d.%d.%d",
				(_u8)ver.ChipFwAndPhyVersion.PhyVersion[0],
				(_u8)ver.ChipFwAndPhyVersion.PhyVersion[1],
				(_u8)ver.ChipFwAndPhyVersion.PhyVersion[2],
				(_u8)ver.ChipFwAndPhyVersion.PhyVersion[3]);
	if ( (strcmp(phy_version, printBuffer) != 0) || !(update == false ) ) {update = true;}

	sl_Stop(0xFF);

	Stop_CC3100_select = 0;

	if (update == true)
		{
		usart2_send(" ... Firmware ist nicht aktuell\r\n");
		CC3100_format_serial_flash();
		CC3100_fw_update();
		}
	else
		{
		usart2_send(" ... Firmware ist aktuell\r\n");
		}
}


//=========================================================================
void CC3100_fw_version(void)
//=========================================================================
{
	SlVersionFull 		ver;
    _u8 				pConfigOpt, pConfigLen;
	_i32         		retVal = 0;
	char		  printBuffer[200];

	pConfigOpt = SL_DEVICE_GENERAL_VERSION;
	pConfigLen = sizeof(SlVersionFull);
	retVal = sl_DevGet(SL_DEVICE_GENERAL_CONFIGURATION,&pConfigOpt,&pConfigLen,(_u8 *)(&ver));

	if(retVal < 0)
		{
		sprintf(printBuffer, "\r\nRead FW Version Error Code= %d\r\n", (int)retVal);
		usart2_send(printBuffer);
		}

	if (ver.ChipFwAndPhyVersion.ChipId & 0x10)
		{
		usart2_send("CC3200");
		}
	else
		{
		usart2_send("CC3100");
		}

	if (ver.ChipFwAndPhyVersion.ChipId & 0x2)
		{
		usart2_send("Z device\r\n");
		}
	else
		{
		usart2_send("R device\r\n");
		}

	sprintf(printBuffer, "NWP %d.%d.%d.%d\n\rMAC 31.%d.%d.%d.%d\n\rPHY %d.%d.%d.%d\n\r\n\r", \
				(_u8)ver.NwpVersion[0],
				(_u8)ver.NwpVersion[1],
				(_u8)ver.NwpVersion[2],
				(_u8)ver.NwpVersion[3],
				(_u8)ver.ChipFwAndPhyVersion.FwVersion[0],
				(_u8)ver.ChipFwAndPhyVersion.FwVersion[1],
				(_u8)ver.ChipFwAndPhyVersion.FwVersion[2],
				(_u8)ver.ChipFwAndPhyVersion.FwVersion[3],
				(_u8)ver.ChipFwAndPhyVersion.PhyVersion[0],
				(_u8)ver.ChipFwAndPhyVersion.PhyVersion[1],
				(_u8)ver.ChipFwAndPhyVersion.PhyVersion[2],
				(_u8)ver.ChipFwAndPhyVersion.PhyVersion[3]);

	usart2_send(printBuffer);
}

//=========================================================================
int CC3100_fw_update(void)
//=========================================================================
{

	_u32	Token = 0;
	_i32 	fileHandle = -1;
	_i32    retVal = 0;

	_u32    remainingLen;
	_u32	movingOffset;
	_u32	chunkLen;

	char	printBuffer[200];



	Stop_CC3100_select = 1;

	CC3100_init();

    usart2_send("\r\n======CC3100==FW==UPDATE======\r\n");

    sl_Start(0, 0, 0);

    usart2_send("\r\naktuelle FW Version:\r\n");
    CC3100_fw_version();



    usart2_send("1. Servicepackfile öffnen\r\n");
    //=========================================================================
	// ! "/sys/servicepack.ucf"
    retVal = sl_FsOpen((_u8*)"/sys/servicepack.ucf",
					   FS_MODE_OPEN_CREATE(131072, _FS_FILE_OPEN_FLAG_SECURE|_FS_FILE_OPEN_FLAG_COMMIT|_FS_FILE_PUBLIC_WRITE),
					   &Token,
					   &fileHandle);

	if(retVal < 0) { usart2_send("Error sl_FsOpen\r\n"); return -1; }

	//=========================================================================


	remainingLen = sizeof(servicepack_ucf);
	movingOffset = 0;
	chunkLen = (_u32)find_min(1024, remainingLen);



    usart2_send("2. Servicepackfile schreiben\r\n");
	//=========================================================================
	// ! servicepack_ucf
    do
	{
		retVal = sl_FsWrite(fileHandle, movingOffset, (_u8 *)&servicepack_ucf[movingOffset], chunkLen);
		if (retVal < 0)
		{
			sprintf(printBuffer, "Write mit Error Code: %d\r\n", (int)retVal);
			 usart2_send(printBuffer);
			return -1;
		}

		remainingLen -= chunkLen;
		movingOffset += chunkLen;
		chunkLen = (_u32)find_min(1024, remainingLen);
	}while (chunkLen > 0);
	//=========================================================================



	usart2_send("3. Servicepackfile schließen\r\n");

	//=========================================================================
	retVal = sl_FsClose(fileHandle, 0, (_u8 *)servicePackImageSig, sizeof(servicePackImageSig));
	// !!! man beachte servicePackImageSig
	if (retVal < 0)
	{
		sprintf(printBuffer, "Close mit Error code: %d\r\n", (int)retVal);
		 usart2_send(printBuffer);
		return -1;
	}
	//=========================================================================



	usart2_send("=> Update - OK\r\n");
	usart2_send("\r\n=>CC3100 Restart\r\n");


	sl_Stop(0xFF);
	sl_Start(0, 0, 0);


    usart2_send("\r\nneue FW Version:\r\n");
    CC3100_fw_version();


	Stop_CC3100_select = 0;

	CC3100_init();

	return 0;
}
