#ifndef CC3100_H
#define CC3100_H

//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rtc.h"

//=========================================================================
// standard_lib
//=========================================================================
#include "string.h"
#include "stdbool.h"

//=========================================================================
// cc3100_lib
//=========================================================================
#include "simplelink.h"
#include "CC3100_EventHandler.h"
#include "socket.h"
#include "wlan.h"
#include "netapp.h"

//=========================================================================
// board_lib
//=========================================================================
#include "usart.h"
#include "BME280.h"

//=========================================================================
// Zugangsdaten STA Mode
//=========================================================================
extern const char SSID_STA_MODE[30];
extern const char PASSWORD_STA_MODE[30];
extern unsigned char SEC_TYPE_STA_MODE;
//=========================================================================
// Zugangsdaten AP Mode
//=========================================================================
extern const char SSID_AP_MODE[30];
extern const char PASSWORD_AP_MODE[30];
extern unsigned char SEC_TYPE_AP_MODE;

//=========================================================================
extern uint32_t  WiFi_RTC;
extern uint32_t CC3100_select_status;
extern unsigned char Stop_CC3100_select;
//=========================================================================


//=========================================================================
// Status Bits des CC3100
//=========================================================================
typedef enum{
    STATUS_BIT_CONNECTION =  0,
    STATUS_BIT_STA_CONNECTED,
    STATUS_BIT_IP_ACQUIRED,
    STATUS_BIT_IP_LEASED,
    STATUS_BIT_CONNECTION_FAILED,
    STATUS_BIT_P2P_NEG_REQ_RECEIVED,
    STATUS_BIT_SMARTCONFIG_DONE,
    STATUS_BIT_SMARTCONFIG_STOPPED
}e_StatusBits;

enum ConnectionType{
    UDP_Client 			=  1,
    UDP_Server 			=  2,
    UDP_ClientServer 	=  3,
    TCP_Client 			=  4,
    TCP_Server 			=  5
};

#define SET_STATUS_BIT(status_variable, bit)    status_variable |= ((unsigned long)1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit)    status_variable &= ~((unsigned long)1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    (0 != (status_variable & ((unsigned long)1<<(bit))))

#define IS_CONNECTED(status_variable)             GET_STATUS_BIT(status_variable, 	STATUS_BIT_CONNECTION)
#define IS_STA_CONNECTED(status_variable)         GET_STATUS_BIT(status_variable, 	STATUS_BIT_STA_CONNECTED)
#define IS_IP_ACQUIRED(status_variable)           GET_STATUS_BIT(status_variable, 	STATUS_BIT_IP_ACQUIRED)
#define IS_IP_LEASED(status_variable)             GET_STATUS_BIT(status_variable, 	STATUS_BIT_IP_LEASED)
#define IS_CONNECTION_FAILED(status_variable)     GET_STATUS_BIT(status_variablle, 	STATUS_BIT_CONNECTION_FAILED)
#define IS_P2P_NEG_REQ_RECEIVED(status_variable)  GET_STATUS_BIT(status_variable, 	STATUS_BIT_P2P_NEG_REQ_RECEIVED)
#define IS_SMARTCONFIG_DONE(status_variable)      GET_STATUS_BIT(status_variable, 	STATUS_BIT_SMARTCONFIG_DONE)
#define IS_SMARTCONFIG_STOPPED(status_variable)   GET_STATUS_BIT(status_variable, 	STATUS_BIT_SMARTCONFIG_STOPPED)


//=========================================================================
// FW Update
//=========================================================================
#define find_min(a,b) (((a) < (b)) ? (a) : (b))


//=========================================================================
// Select
//=========================================================================
#define WLAN_PROFILE_SIZE 		(80)
#define WLAN_ALL_PROFILES_SIZE 	(560)
#define WLAN_PROFILE_COUNT 		(7)
#define NETWORKS_ARRAY_MAX 		(30)
#define CC3100_MAX_SOCK_COUNT 	(10)

#define SUCCESS             0

typedef struct ipInfo {
	int16_t family; 	// e.g. AF_INET
	uint16_t port; 		// e.g. htons(3490)
	unsigned char addr[4];
} ipInfo_t;


extern unsigned char rx_daten[2048]; // TODO auf extern char rx_buf[][2048];
//extern char buffer_out[1024];

//=========================================================================
// Select Sockets
//=========================================================================
extern unsigned long nfds;
extern sockaddr sockets[];
extern unsigned long socket_Handles[];

extern unsigned long handle_mDNS;
extern unsigned long handle_TCP;
extern unsigned long handle_HTTP;
extern unsigned long handle_MQTT;
extern unsigned long handle_NTP;
extern unsigned long handle_Protokoll;
extern unsigned long handle_HTTP_Client;
extern unsigned long handle_NetBIOS;
extern unsigned long handle_DB;
extern int handle_FTP_Command;
extern int handle_FTP_Data;


extern sockaddr my_IP;
extern sockaddr S1_mDNS;
extern sockaddr S2_TCP;
extern sockaddr S3_NTP;
extern sockaddr S3_MQTT;
extern sockaddr S4_unused;
extern sockaddr S_NetBIOS;
extern sockaddr S_ConnectedClient;

extern socklen_t s_ConnectedLen;

extern _Bool S3_WaitForAnswer;
extern _Bool S_ClientConnected;


extern char rx_buf[][2048]; // 	Array der Empfangspuffer
extern char tx_buf[][2048]; // 	Array der Empfangspuffer

//=========================================================================
// Funktionen
//=========================================================================




// Funktionen zur Nutzung der Sockets
//=========================================================================
long CC3100_select(void);

unsigned long CC3100_openSocketul(unsigned long ip_l,
		uint16_t port,
		unsigned char ConType,
		void (*read_cb)(char* rx, uint16_t rx_len, sockaddr* from, uint16_t socket_Pos),
		long (*write_cb)(char* tx, uint16_t tx_len, uint16_t socket_Position),
		void (*except_cb)(uint16_t pos));

unsigned long CC3100_openSocket(
		unsigned char ip1, unsigned char ip2,
		unsigned char ip3, unsigned char ip4,
		uint16_t port,
		unsigned char ConType,
		void (*read_cb)(char* rx, uint16_t rx_len, sockaddr* from, uint16_t socket_Pos),
		long (*write_cb)(char* tx, uint16_t tx_len, uint16_t socket_Position),
		void (*except_cb)(uint16_t pos)
		);

unsigned long CC3100_closeSocket(uint16_t socketNr);




// Funktionen zum setzen der CC3100 Betriebsmodi
//=========================================================================
void CC3100_set_in_STA_Mode(char webserver);	// Webserver 0-aus 1-ein
void CC3100_set_in_AP_Mode(void);

void CC3100_start_STA_Mode_Profil(void);



// Funktionen zum Zugriff auf den internen seriellen Flash des CC3100
//=========================================================================
void CC3100_sf_file_read( unsigned char * datei_name);
void CC3100_sf_file_erase( unsigned char * datei_name);
int	 CC3100_sf_file_write(void);





// Funktionen zum Update der Firmware
//=========================================================================
void CC3100_fw_check(void);
void CC3100_fw_version(void);
int  CC3100_fw_update(void);

int  CC3100_set_time(void);

#endif
