#ifndef DW1000_RANGING_HEADER
#define DW1000_RANGING_HEADER

#include "dw1000_driver.h"
#include "dw1000_user.h"


#define UWBRANGING_MAXPACKAGESIZE 1024
#define UWBRANGING_TIMEOUT_MS 5			// <- auf 20 setzen, wenn Systick=10ms



// *** ENUMERATIONEN ***


// *****************************************************************
// Mögliche Ergebnisse beim Senden eines Text- oder Bytearray-Pakets
// *****************************************************************
typedef enum
{
	UWBRANGING_MESSAGERESULT_OK = 0,
	UWBRANGING_MESSAGERESULT_BUSY
}
uwbranging_messageresults_e;


extern unsigned char ismeasuring;


// *** EREIGNISHANDLER ***


// **********************************************************************
// Definiert den Aufbau des Eventhandlers für eingehende Textmitteilungen
// **********************************************************************
typedef void (*uwbranging_textmessagehandler_f)(unsigned short sender, unsigned char* content, unsigned long long int rxtimestamp, double rxpower, float rxtemperature);


// ***************************************************************************
// Definiert den Aufbau des Eventhandlers für eingehende Bytearraymitteilungen
// ***************************************************************************
typedef void (*uwbranging_bytearraymessagehandler_f)(unsigned short sender, unsigned char* content, int contentlength, unsigned long long int rxtimestamp, double rxpower, float rxtemperature);


// ******************************************************************
// Definiert einen Handler, der nach erfolgreichem TX ausgeführt wird
// ******************************************************************
typedef void (*uwbranging_txhandler_f)(unsigned long long timestamp, float temperature, uwbranging_messageresults_e error);


// *************************************************************************************
// Definiert einen Handler, der nach erfolgreicher Entfernungsbestimmung ausgeführt wird
// *************************************************************************************
typedef void (*uwbranging_rangehandler_f)(unsigned short destination, float range);


// *** ÖFFENTLICHE FUNKTIONEN ***


// ***************************
// Initialisiert das UWB-Modul
// ***************************
void uwbranging_initialize();


// *****************************************************************************************
// Diese Methode sollte innerhalb der MAIN-Schleife aufgerufen werden. Sie handhabt timeouts
// *****************************************************************************************
void uwbranging_tick();


// **********************************************************************************
// Leitet (wenn möglich) eine neue Entfernungsermittlung zum gegebenen Zielknoten ein
// **********************************************************************************
unsigned char uwbranging_startRanging(unsigned short goal);


// *********************************************
// Submits a text message to the remote instance
// *********************************************
unsigned char uwbranging_sendTextMessage(unsigned short remoteaddress, unsigned char* message, void (*handler)(unsigned long long timestamp, float temperature, unsigned char error));


// **************************************************
// Submits a bytearray message to the remote instance
// **************************************************
unsigned char uwbranging_sendBinaryMessage(unsigned short remoteaddress, unsigned char* message, unsigned int messagelength, void (*handler)(unsigned long long timestamp, float temperature, uwbranging_messageresults_e error));


// *******************************************************************************************
// Registriert den Eventhandler, der aufgerufen wird, wenn eine Textmitteilung eingegangen ist
// *******************************************************************************************
void uwbranging_registerTextMessageHandler(void (*handler)(unsigned short, unsigned char*, unsigned long long int, double, float));


// ************************************************************************************************
// Registriert den Eventhandler, der aufgerufen wird, wenn eine Bytearraymitteilung eingegangen ist
// ************************************************************************************************
void uwbranging_registerByteArrayMessageHandler(void (*handler)(unsigned short, unsigned char*, int, unsigned long long int, double, float));


// *****************************************************************************************
// Registriert einen Eventhandler, der aufgerufen wird, wenn eine Entfernung ermittelt wurde
// *****************************************************************************************
void uwbranging_registerRangeHandler(void (*handler)(unsigned short destination, float range));


#endif
