#include "dw1000_def.h"
#include "dw1000_ranging.h"
#include "dw1000_user.h"
#include "dw1000_base.h"
//#include "CC3100.h"
#include "global.h"
#include "usart.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "stm32f4xx_flash.h"
#include "ff.h"


//#define DW1000_DEBUG_RANGING (1)

/* __packed keyword used to decrease the data type alignment to 1-byte */
#ifndef __packed
	#if defined (__CC_ARM)         /* ARM Compiler */
	  #define __packed    __packed
	#elif defined (__ICCARM__)     /* IAR Compiler */
	  #define __packed    __packed
	#elif defined   ( __GNUC__ )   /* GNU Compiler */
	  #define __packed    __attribute__ ((__packed__))
	#elif defined   (__TASKING__)  /* TASKING Compiler */
	  #define __packed    __unaligned
	#endif /* __CC_ARM */
#endif


// ****************************************************************
// Definiert die möglichen Paketinhalte einer Entfernungsberechnung
// ****************************************************************
typedef enum
{
	UWBRANGING_PACKAGETYPE_SOURCE_TX1 = 1,
	UWBRANGING_PACKAGETYPE_SOURCE_TX2 = 3,
	UWBRANGING_PACKAGETYPE_SOURCE_RESULT = 5,
	UWBRANGING_PACKAGETYPE_DESTINATION_TX1 = 2,
	UWBRANGING_PACKAGETYPE_DESTINATION_RESULT = 4,
	UWBRANGING_PACKAGETYPE_TEXTMESSAGE = 128,
	UWBRANGING_PACKAGETYPE_BYTEARRAYMESSAGE = 129
}
uwbranging_packagetypes_e;


// **************************************************************************************************
// Definiert die möglichen Rollen, die ein Knoten innerhalb einer Entfernungsermittlung annehmen kann
// **************************************************************************************************
typedef enum
{
	UWBRANGING_ROLE_NONE = 0,
	UWBRANGING_ROLE_SOURCE,
	UWBRANGING_ROLE_DESTINATION
} uwbranging_role_e;


// ********************************************************************
// Beinhaltet alle, für die Entfernungsermittlung notwendigen Parameter
// ********************************************************************
typedef struct
{
	unsigned long long timestamps[3];		/* Ermittelte Zeitstempel (abhängig von Rolle) */
	float temperaturevalues[3];				/* Ermittelte Chiptemperaturen, zu den Zeitstempeln */
	float rxpowervalues[3];					/* Ermittelte Empfangsstärken zu den Zeitstempeln */
	unsigned char rangingid;				/* Systemübergreifend eindeutige Kennung der Prozesses */
	unsigned char txtimeindex;				/* Arrayindex für den TX Timestamp */
	uwbranging_role_e rangingrole;			/* Rolle des aktuellen Knotens innerhalb der Entfernungsberechnung */
	unsigned long lasttimestamp;			/* Letzer Übermittlungszeitstempel */
} __packed uwbranging_process_t;


// ****************************************
// Beschreibt den Aufbau eines Paketheaders
// ****************************************
typedef struct
{
	unsigned char packagetype;				/* Pakettyp: uwbranging_packagetypes_e */
	unsigned short source;					/* Netzwerkkennung des Quellknotens */
	unsigned short destination;				/* Netzwerkkennung des Zielknotens */
	unsigned char rangingid;				/* Kennung der aktuellen Entfernungsmessung */
} __packed uwbranging_packetheader_t;


// ********************************************************************
// Beinhaltet alle, für die Sensorknotenkonfiguration notwendigen Werte
// ********************************************************************
typedef struct
{
	unsigned short networkid;
	unsigned short antennadelay;
	unsigned char uwbchannel;
	unsigned char uwbspeed;
	unsigned char uwbpulsefrequency;
	unsigned short uwbtxpower;
} __packed uwbranging_config_t;

unsigned short dw1000_id = 0;
unsigned char ismeasuring = 0;

// *********************************
// PRIVATE UND ÖFFENTLICHE VARIABLEN
// *********************************
uwbranging_process_t uwbranging_process;									/* Aktueller Entfernungsermittlungsprozess */
uwbranging_config_t uwbranging_config;										/* Aktuelle Konfiguration des Sensorknotens */
unsigned char uwbranging_rxbuffer[UWBRANGING_MAXPACKAGESIZE];				/* Empfangspuffer */
unsigned char uwbranging_rangingid = 0;										/* Kennung der letzten Entfernungsberechnung */


// ***************
// Ereignishandler
// ***************
uwbranging_textmessagehandler_f uwbranging_textmessagehandler = 0;			/* Wird bei eingehender Textmitteilung aufgerufen */
uwbranging_bytearraymessagehandler_f uwbranging_bytemessagehandler = 0;		/* Wird bei eingehender Bytearraymitteilung aufgerufen */
uwbranging_txhandler_f uwbranging_txhandler = 0;							/* Wird nach dem Senden aufgerufen */
uwbranging_rangehandler_f uwbranging_rangehandler = 0;						/* Wird nach Entfernungsermittlung aufgerufen */





// Lädt die aktuelle Konfiguration von der SD-Karte
void uwbranging_loadconfig()
{
	// Setze die aktuelle Konfiguration auf vordefinierte (oder bewusst falsche) Werte,
	// um eine spätere Korrektur zu ermöglichen
	uwbranging_config.antennadelay = 16500;
	uwbranging_config.networkid = 1;
	uwbranging_config.uwbchannel = DW1000_CHANNEL_1;
	uwbranging_config.uwbpulsefrequency = DW1000_TX_PULSE_FREQ_64MHZ;
	uwbranging_config.uwbspeed = DW1000_TRX_RATE_6800KBPS;
	uwbranging_config.uwbtxpower = 165;

	FIL configfile;
	FRESULT fileresult = f_open(&configfile, "0:uwbconfig.dat",  FA_OPEN_EXISTING | FA_READ);
	if (fileresult == FR_OK)
	{
		char symbol;
		unsigned int output = 0;
		unsigned int bufferposition = 0;
		char result;
		char filebuffer[256];
		while(1)
		{
			result = f_read(&configfile, &symbol, 1, &output);
			if (result == FR_OK)
			{
				if (symbol == 13 || symbol == 10 || output == 0)
				{
					if (bufferposition > 0)
					{
						filebuffer[bufferposition++] = 0;
						switch(filebuffer[0])
						{
							case 'A':
							{
								if (strncmp(filebuffer, "ANTENNADELAY", strlen("ANTENNADELAY")) == 0 && filebuffer[strlen("ANTENNADELAY")] == '=')
								{
									char* filecontent = &(filebuffer[strlen("ANTENNADELAY")+1]);
									uwbranging_config.antennadelay = atoi(filecontent);
								}
								break;
							}
							case 'C':
							{
								if (strncmp(filebuffer, "CHANNEL", strlen("CHANNEL")) == 0 && filebuffer[strlen("CHANNEL")] == '=')
								{
									char* filecontent = &(filebuffer[strlen("CHANNEL")+1]);
									int value = atoi(filecontent);
									switch(value)
									{
										case 1: uwbranging_config.uwbchannel = DW1000_CHANNEL_1; break;
										case 2: uwbranging_config.uwbchannel = DW1000_CHANNEL_2; break;
										case 3: uwbranging_config.uwbchannel = DW1000_CHANNEL_3; break;
										case 4: uwbranging_config.uwbchannel = DW1000_CHANNEL_4; break;
										case 5: uwbranging_config.uwbchannel = DW1000_CHANNEL_5; break;
										case 7: uwbranging_config.uwbchannel = DW1000_CHANNEL_7; break;
									}
								}
							}
							break;
							case 'N':
							{
								if (strncmp(filebuffer, "NODEID", strlen("NODEID")) == 0 && filebuffer[strlen("NODEID")] == '=')
								{
									char* filecontent = &(filebuffer[strlen("NODEID")+1]);
									uwbranging_config.networkid = atoi(filecontent);
								}
							}
							break;
							case 'P':
							{
								if (strncmp(filebuffer, "PULSEFREQ", strlen("PULSEFREQ")) == 0 && filebuffer[strlen("PULSEFREQ")] == '=')
								{
									char* filecontent = &(filebuffer[strlen("PULSEFREQ")+1]);
									int value = atoi(filecontent);
									if (value == 64) uwbranging_config.uwbpulsefrequency = DW1000_TX_PULSE_FREQ_64MHZ;
								}
							}
							break;
							case 'S':
							{
								if (strncmp(filebuffer, "SPEED", strlen("SPEED")) == 0 && filebuffer[strlen("SPEED")] == '=')
								{
									char* filecontent = &(filebuffer[strlen("SPEED")+1]);
									int value = atoi(filecontent);
									if (value == 110) uwbranging_config.uwbspeed = DW1000_TRX_RATE_110KBPS;
									else if (value == 850) uwbranging_config.uwbspeed = DW1000_TRX_RATE_850KBPS;
									else if (value == 6800) uwbranging_config.uwbspeed = DW1000_TRX_RATE_6800KBPS;
								}
							}
							break;
							case 'T':
							{
								if (strncmp(filebuffer, "TXPOWER", strlen("TXPOWER")) == 0 && filebuffer[strlen("TXPOWER")] == '=')
								{
									char* filecontent = &(filebuffer[strlen("TXPOWER")+1]);
									uwbranging_config.uwbtxpower = atoi(filecontent);
									if (uwbranging_config.uwbtxpower > 335) uwbranging_config.uwbtxpower = 335;
								}
							}
							break;
						}
					}
					bufferposition = 0;
					if (output == 0) break;
				}
				else if (bufferposition < 255)
				{
					filebuffer[bufferposition++] = symbol;
				}
			}
			else break;
		}
	}
	f_close(&configfile);
}



// FUNKTIONSRÜMPFE
// ---------------


// **************************************************************************
// Führt die Entfernungsberechnung anhand aller verfügbaren Zeitstempel durch
// **************************************************************************
void uwbranging_calculateDistance(unsigned short remoteid, uwbranging_process_t* source, uwbranging_process_t* destination)
{
	// Ohne dass ein Ereignishandler registriert wurde, macht die
	// Berechnung der Entfernung keinen Sinn. Also führe zunächst
	// eine Prüfung durch
	if (uwbranging_rangehandler)
	{
		// Extrahiere alle relevanten Zeitstempel
		unsigned long long t0 = source->timestamps[0];
		unsigned long long t1 = destination->timestamps[0];
		unsigned long long t2 = destination->timestamps[1];
		unsigned long long t3 = source->timestamps[1];
		unsigned long long t4 = source->timestamps[2];
		unsigned long long t5 = destination->timestamps[2];

		// Ermittle die relevanten Zeitspannen
		long long tflight1 = (t3 > t0) ? t3 - t0 : t3 + 0xffffffffff - t0;
		long long treact1 = (t2 > t1) ? t2 - t1 : t2 + 0xffffffffff - t1;
		long long tflight2 = (t5 > t2) ? t5 - t2 : t5 + 0xffffffffff - t2;
		long long treact2 = (t4 > t3) ? t4 - t3 : t4 + 0xffffffffff - t3;

		// Ermittle die Entfernung
		//if (tflight1 > treact1 && tflight2 > treact2)
		//{
			double tof_ticks = ((tflight1 - treact1) + (tflight2 - treact2)) / 4;		// Laufzeit in Ticks
			double tof = tof_ticks * 15.6500400641;										// Laufzeit in Picosekunden

			// Rufe den Ereignishandler auf
			(*uwbranging_rangehandler)(remoteid, (float)(tof / 3335.64095198));
		//}
	}
}


// ***************************
// Initialisiert das UWB-Modul
// ***************************
void uwbranging_initialize()
{
	// Lade die Konfiguration
	uwbranging_loadconfig();

	// Initialisiere das UWB Paket
	uwbranging_process.rangingrole = UWBRANGING_ROLE_NONE;

	// Initialisiere den UWB Chip und richte ihn ein
	dw1000_initialize();
	dw1000_newConfiguration(1);
	dw1000_setDataRate(uwbranging_config.uwbspeed);
	dw1000_setChannel(uwbranging_config.uwbchannel);
	dw1000_setPulseFrequency(uwbranging_config.uwbpulsefrequency);
	dw1000_setReceiveAfterTX(1);
	dw1000_setReceiverAutoReenable(1);
	dw1000_setDoubleBuffering(0);
	dw1000_commitConfiguration();
	dw1000_setTxPower(uwbranging_config.uwbtxpower);

	// TODO: Temp-abh
	// Lege die Konfigurationsdaten fest
	if (uwbranging_config.antennadelay != 0) dw1000_setAntennaDelay(uwbranging_config.antennadelay,uwbranging_config.antennadelay);

	// Messe die Temperatur
	dw1000_delay_mSek(1);

	// Führe eine Temperaturmessung durch
	dw1000_enableTemperatureAndVoltageMeasurement();

	//dw1000_enableSniffMode();
	// Start receiving
	dw1000_idle();
	dw1000_startReceive();
}


// ********************************
// Bricht die Entfernungsmessung ab
// ********************************
void uwbranging_cancel()
{
#ifdef DW1000_DEBUG_RANGING

	{
		usart2_send("UWBRANGING: Zurücksetzen\r\n");
	}

#endif

	// Setze den Ranging-Prozess zurück
	uwbranging_process.rangingrole = UWBRANGING_ROLE_NONE;
	uwbranging_process.rangingid = 0;
	uwbranging_process.txtimeindex = 0;
	uwbranging_process.temperaturevalues[0] = 0;
	uwbranging_process.temperaturevalues[1] = 0;
	uwbranging_process.temperaturevalues[2] = 0;
	uwbranging_process.rxpowervalues[0] = 0;
	uwbranging_process.rxpowervalues[1] = 0;
	uwbranging_process.rxpowervalues[2] = 0;

	// Messung abgebrochen
	ismeasuring = 0;

	//dw1000_idle();
	//dw1000_entersleep();


}


// ************************************************************
// Diese Methode wird aufgerufen, wenn ein Paket gesendet wurde
// ************************************************************
void dw1000_handleTransmittedPackage()
{
	//usart2_send("TX\r\n");

	//uwb_send = 0;

	// Prüfe den Pakettypen
	switch(uwbranging_process.rangingrole)
	{
		// Innerhalb einer Routine zur Entfernungsberechnung,
		// werden die Zeitstempel und Temperaturen ermittelt
		case UWBRANGING_ROLE_SOURCE:
			{
#ifdef DW1000_DEBUG_RANGING

				usart2_send("UWBRANGING: TX done\r\n");

#endif

				// Bei Paketen für die reine Entfernungsberechnung, notiere den
				// TX-Zeitstempel und die Temperatur.
				if (uwbranging_process.txtimeindex <= 2)
				{
					uwbranging_process.timestamps[uwbranging_process.txtimeindex] = dw1000_getTransmitTimestamp();
					uwbranging_process.rxpowervalues[uwbranging_process.txtimeindex] = 0;
					uwbranging_process.temperaturevalues[uwbranging_process.txtimeindex] = dw1000_getTemperature();
					uwbranging_process.lasttimestamp = systickcounter;
				}
				else
				{
					uwbranging_cancel();
				}
			}
			break;

		// Innerhalb einer Routine zur Entfernungsberechnung,
		// werden die Zeitstempel und Temperaturen ermittelt
		case UWBRANGING_ROLE_DESTINATION:
			{
#ifdef DW1000_DEBUG_RANGING

				usart2_send("UWBRANGING: TX done\r\n");

#endif

				// Bei Paketen für die reine Entfernungsberechnung, notiere den
				// TX-Zeitstempel und die Temperatur.
				if (uwbranging_process.txtimeindex <= 2)
				{
					uwbranging_process.timestamps[uwbranging_process.txtimeindex] = dw1000_getTransmitTimestamp();
					uwbranging_process.rxpowervalues[uwbranging_process.txtimeindex] = 0;
					uwbranging_process.temperaturevalues[uwbranging_process.txtimeindex] = dw1000_getTemperature();
					uwbranging_process.lasttimestamp = systickcounter;
				}
				else if (uwbranging_process.txtimeindex > 3)
				{
					uwbranging_cancel();
				}
			}
			break;

		// Bei Paketen, die nicht zur Entfernungsmessung gehören,
		// werden Zeitstempel nur auf Wunsch ausgegeben
		default:
			{
				// Rufe den Ergebnishandler auf (wenn angegeben) und übergebe den TX-Zeitstempel und die letzte gemessene Temperatur
				if (uwbranging_txhandler) (*uwbranging_txhandler)(dw1000_getTransmitTimestamp(), dw1000_getTemperature(), 0);
				uwbranging_txhandler = 0;

				// Sicherheitshalber abbrechen
				uwbranging_cancel();
			}
			break;
	}

	// Gehe in den Empfangsmodus
	//dw1000_startReceive();
}


// ***************************************************************************************
// Diese Funktion wird aufgerufen, wenn das UWB-Modul ein eingegangenes Paket signalisiert
// ***************************************************************************************
void dw1000_handleIncomingPackage()
{
	// Lese alle Daten ein, die für die Auswertung des Pakets relevant sein könnten
	volatile unsigned long long int receivetimestamp = dw1000_getReceiveTimestamp();
	volatile double receivepower = dw1000_getReceivePower();
	volatile float receivetemperature = dw1000_getTemperature();

	// Diese Variable wird zurück gesetzt, wenn kein neuer Empfang gestartet werden soll
	unsigned char startreceiving = 1;

	// Lese die Paketlänge (beschleunigte Abfrage - alternativ: dw1000_readRxTxBufferLen();)
	volatile unsigned short packagelength = dw1000_getRxTxBufferLen();

	// Überprüfe, ob sich die Paketlänge in einem gültigen Rahmen bewegt
	if (packagelength >= sizeof(uwbranging_packetheader_t) && packagelength < UWBRANGING_MAXPACKAGESIZE)
	{
		// Lese den Paketinhalt (beschleunigte Anfrage - alternativ: dw1000_readRxBufferToByteArray(uwbranging_rxbuffer, packagelength);)
		dw1000_read(DW1000_RX_BUFFER, packagelength, uwbranging_rxbuffer, 0);

		// Extrahiere den Header aus dem empfangenen Payload
		uwbranging_packetheader_t *header = (uwbranging_packetheader_t*)uwbranging_rxbuffer;

		// Prüfe, ob das Paket an einen gültigen Empfänger gerichtet ist
		if (header->destination != 0)
		{
			// Prüfe, ob das Paket an diesen Sensorknoten gerichtet ist
			if (header->destination == uwbranging_config.networkid)
			{
				// Untersuche den Pakettyp
				switch(header->packagetype)
				{
					// Das erste Paket des Quellknotens wurde empfangen
					case UWBRANGING_PACKAGETYPE_SOURCE_TX1:

#ifdef DW1000_DEBUG_RANGING

					{
						char tmp[128];
						sprintf(tmp, "UWBRANGING: %d -> PS0 -> %d (%d)\r\nUWBRANGING: %d -> PD1 -> %d\r\n", header->source, uwbranging_config.networkid, header->rangingid, uwbranging_config.networkid, header->source);
						usart2_send(tmp);
					}

#endif

						// Dieser Knoten ist nun ein Zielknoten
						uwbranging_process.rangingrole = UWBRANGING_ROLE_DESTINATION;

						// Sende das Antwortpaket so schnell wie möglich
						uwbranging_packetheader_t response;
						response.source = uwbranging_config.networkid;
						response.destination = header->source;
						response.packagetype = UWBRANGING_PACKAGETYPE_DESTINATION_TX1;
						response.rangingid = header->rangingid;

						// Übermittle die Antwort so schnell wie möglich
						dw1000_initializeTransmit();
						dw1000_writeByteArrayToTxBuffer((unsigned char*)&response, sizeof(uwbranging_packetheader_t));
						dw1000_startTransmit(0);

						// Speichere alle relevanten Informationen ab
						uwbranging_process.rangingid = header->rangingid;
						uwbranging_process.timestamps[0] = receivetimestamp;
						uwbranging_process.rxpowervalues[0] = receivepower;
						uwbranging_process.temperaturevalues[0] = receivetemperature;
						uwbranging_process.txtimeindex = 1;
						uwbranging_process.lasttimestamp = systickcounter;

						// Nicht erneut empfangen, da eine Antwort gesendet wurde
						startreceiving = 0;

						break;

					// Das zweite Paket des Quellknotens wurde empfangen
					case UWBRANGING_PACKAGETYPE_SOURCE_TX2:

						// Prüfe, ob sich der aktuelle Knoten im korrekten Zustand befindet
						if (uwbranging_process.rangingrole == UWBRANGING_ROLE_DESTINATION && header->rangingid == uwbranging_process.rangingid)
						{
#ifdef DW1000_DEBUG_RANGING

							{
								char tmp[128];
								sprintf(tmp, "UWBRANGING: %d -> PS2 -> %d (%d)\r\nUWBRANGING: %d -> PD3 [RESULT] -> %d\r\n", header->source, uwbranging_config.networkid, header->rangingid, uwbranging_config.networkid, header->source);
								usart2_send(tmp);
							}

#endif

							// Sende das Antwortpaket so schnell wie möglich
							uwbranging_packetheader_t response;
							response.source = uwbranging_config.networkid;
							response.destination = header->source;
							response.packagetype = UWBRANGING_PACKAGETYPE_DESTINATION_RESULT;
							response.rangingid = header->rangingid;

							// Dieser Knoten ist nun ein Zielknoten
							uwbranging_process.timestamps[2] = receivetimestamp;
							uwbranging_process.rxpowervalues[2] = dw1000_getReceivePower();
							uwbranging_process.temperaturevalues[2] = receivetemperature;
							uwbranging_process.txtimeindex = 3;
							uwbranging_process.lasttimestamp = systickcounter;

							// Übermittle die Antwort so schnell wie möglich
							dw1000_initializeTransmit();
							dw1000_writeByteArrayAndHeaderToTxBuffer((unsigned char*)&response, sizeof(uwbranging_packetheader_t), (unsigned char*)&uwbranging_process, 48);
							dw1000_startTransmit(0);

							// Nicht erneut empfangen, da eine Antwort gesendet wurde
							startreceiving = 0;
						}

						break;

					// Dieses Paket enthält die Informationen rund um die Entfernungsmessung, die
					// vom Quellknoten ermittelt wurden. Anhand dieser Daten kann die Entfernung
					// bestimmt werden
					case UWBRANGING_PACKAGETYPE_SOURCE_RESULT:

						// Prüfe, ob das Paket lang genug und gültig ist
						if (packagelength - sizeof(uwbranging_packetheader_t) >= 48 && uwbranging_process.rangingrole == UWBRANGING_ROLE_DESTINATION && header->rangingid == uwbranging_process.rangingid)
						{
#ifdef DW1000_DEBUG_RANGING

							{
								char tmp[128];
								sprintf(tmp, "UWBRANGING: %d -> PS4 [RESULT] -> %d (%d)\r\n", header->source, uwbranging_config.networkid, header->rangingid);
								usart2_send(tmp);
							}

#endif

							// Speichere die relevanten Daten des entfernten Knotens
							uwbranging_process_t remoteprocess;
							memcpy((void*)&remoteprocess, (void*)&uwbranging_rxbuffer[sizeof(uwbranging_packetheader_t)], 48);

							// Ermittle die Entfernung
							uwbranging_calculateDistance(header->source, &remoteprocess, &uwbranging_process);

							// Setze die Entfernungsermittlung zurück
							uwbranging_cancel();
						}

						break;

					// Das erste Paket des Zielknotens wurde empfangen
					case UWBRANGING_PACKAGETYPE_DESTINATION_TX1:
						if (uwbranging_process.rangingrole == UWBRANGING_ROLE_SOURCE && header->rangingid == uwbranging_process.rangingid)
						{
#ifdef DW1000_DEBUG_RANGING

							{
								char tmp[128];
								sprintf(tmp, "UWBRANGING: %d -> PD1 -> %d (%d)\r\nUWBRANGING: %d -> PS2 -> %d\r\n", header->source, uwbranging_config.networkid, header->rangingid, uwbranging_config.networkid, header->source);
								usart2_send(tmp);
							}

#endif

							// Sende das Antwortpaket so schnell wie möglich
							uwbranging_packetheader_t response;
							response.source = uwbranging_config.networkid;
							response.destination = header->source;
							response.packagetype = UWBRANGING_PACKAGETYPE_SOURCE_TX2;
							response.rangingid = header->rangingid;

							// Übermittle die Antwort so schnell wie möglich
							dw1000_initializeTransmit();
							dw1000_writeByteArrayToTxBuffer((unsigned char*)&response, sizeof(uwbranging_packetheader_t));
							dw1000_startTransmit(0);

							// Nicht erneut empfangen, da eine Antwort gesendet wird
							startreceiving = 0;

							// Speichere die empfangenen Daten
							uwbranging_process.timestamps[1] = receivetimestamp;
							uwbranging_process.rxpowervalues[1] = receivepower;
							uwbranging_process.temperaturevalues[1] = receivetemperature;
							uwbranging_process.txtimeindex = 2;
							uwbranging_process.lasttimestamp = systickcounter;
						}
						break;

					// Dieses Paket enthält die Informationen rund um die Entfernungsmessung, die
					// vom Zielknoten ermittelt wurden. Anhand dieser Daten kann die Entfernung
					// bestimmt werden
					case UWBRANGING_PACKAGETYPE_DESTINATION_RESULT:

						// Prüfe, ob das Paket lang genug und gültig ist
						if (packagelength - sizeof(uwbranging_packetheader_t) >= 48 && uwbranging_process.rangingrole == UWBRANGING_ROLE_SOURCE && header->rangingid == uwbranging_process.rangingid)
						{
#ifdef DW1000_DEBUG_RANGING

							{
								char tmp[128];
								sprintf(tmp, "UWBRANGING: %d -> PD3 [RESULT] -> %d (%d)\r\nUWBRANGING: %d -> PS4 [RESULT] -> %d\r\n", header->source, uwbranging_config.networkid, header->rangingid, uwbranging_config.networkid, header->source);
								usart2_send(tmp);
							}

#endif

							// Sende das Antwortpaket so schnell wie möglich
							uwbranging_packetheader_t response;
							response.source = uwbranging_config.networkid;
							response.destination = header->source;
							response.packagetype = UWBRANGING_PACKAGETYPE_SOURCE_RESULT;
							response.rangingid = header->rangingid;

							// Speichere die empfangenen Daten
							uwbranging_process.txtimeindex = 3;
							uwbranging_process.lasttimestamp = systickcounter;

							// Übermittle die Antwort so schnell wie möglich
							dw1000_initializeTransmit();
							dw1000_writeByteArrayAndHeaderToTxBuffer((unsigned char*)&response, sizeof(uwbranging_packetheader_t), (unsigned char*)&uwbranging_process, 48);
							dw1000_startTransmit(0);

							// Nicht erneut empfangen, da eine Antwort gesendet wurde
							startreceiving = 0;

							// Speichere die relevanten Daten des entfernten Knotens
							uwbranging_process_t remoteprocess;
							memcpy((void*)&remoteprocess, (void*)&uwbranging_rxbuffer[sizeof(uwbranging_packetheader_t)], 48);

							// Ermittle die Entfernung
							uwbranging_calculateDistance(header->source, &uwbranging_process, &remoteprocess);
						}

						break;

					// Eine Nachricht wurde empfangen, die Klartext enthält
					case UWBRANGING_PACKAGETYPE_TEXTMESSAGE:
						{
							// Terminiere die Zeichenkette durch Einfügen einer abschließenden 0 und führe den Ereignishandler aus
							uwbranging_rxbuffer[packagelength - 1] = 0;
							if (uwbranging_textmessagehandler)
							{
								(*uwbranging_textmessagehandler)(
									header->source,
									&uwbranging_rxbuffer[sizeof(uwbranging_packetheader_t) - 1],
									receivetimestamp,
									receivepower,
									receivetimestamp);
							}
						}
						break;

					// Eine Nachricht wurde empfangen, die reine Nutzdaten enthält
					case UWBRANGING_PACKAGETYPE_BYTEARRAYMESSAGE:
						{
							// Ermittle die Länge des Dateninhalts und führe den Ereignishandler aus
							unsigned long payloadlength = packagelength - sizeof(uwbranging_packetheader_t) - 2;
							if (uwbranging_bytemessagehandler)
							{
								(*uwbranging_bytemessagehandler)(
									header->source,
									&uwbranging_rxbuffer[sizeof(uwbranging_packetheader_t) - 1],
									payloadlength,
									receivetimestamp,
									receivepower,
									receivetemperature);
							}
						}
						break;
				}
			}
			else
			{
				// *****************************************************************************************************
				// Die Kommunikation zwischen zwei fremden Knoten wurde mitgehört. Hier können Sonderaktionen ausgeführt
				// werden. Zum Beispiel können alle aktiven Knoten in Reichweite ermittelt werden.
				// *****************************************************************************************************

				// Beispiel: Höre die Broadcastadresse 0xffff ab
				if (header->destination == 0xfffff)
				{
					// Untersuche den Pakettyp
					switch(header->packagetype)
					{
						// Eine Nachricht wurde empfangen, die Klartext enthält
						case UWBRANGING_PACKAGETYPE_TEXTMESSAGE:
							{
								// Terminiere die Zeichenkette durch Einfügen einer abschließenden 0 und führe den Ereignishandler aus
								uwbranging_rxbuffer[packagelength - sizeof(uwbranging_packetheader_t) - 2] = 0;
								if (uwbranging_textmessagehandler)
								{
									(*uwbranging_textmessagehandler)(
										header->source,
										&uwbranging_rxbuffer[sizeof(uwbranging_packetheader_t) - 1],
										receivetimestamp,
										receivepower,
										receivetimestamp);
								}
							}
							break;

						// Eine Nachricht wurde empfangen, die reine Nutzdaten enthält
						case UWBRANGING_PACKAGETYPE_BYTEARRAYMESSAGE:
							{
								// Ermittle die Länge des Dateninhalts und führe den Ereignishandler aus
								unsigned long payloadlength = packagelength - sizeof(uwbranging_packetheader_t) - 2;
								if (uwbranging_bytemessagehandler)
								{
									(*uwbranging_bytemessagehandler)(
										header->source,
										&uwbranging_rxbuffer[sizeof(uwbranging_packetheader_t) - 1],
										payloadlength,
										receivetimestamp,
										receivepower,
										receivetemperature);
								}
							}
							break;
					}
				}
			}
		}
		else
		{
			// *****************************************************************************************************
			// Das Paket wurde an die ungültige Adresse 0x00 gesentet. Hier können Sonderaktionen ausgeführt werden,
			// sodass diese Adresse beispielsweise genutzt wird, um allen Knoten in Reichweite zu signalisieren,
			// dass man selbst aktiv ist (beacon)
			// *****************************************************************************************************
		}
	}

	// Sofern nicht unterbunden: starte den Empfang eines weiteren Pakets
	if (startreceiving)
	{
		dw1000_startReceive();
	}

}


// *****************************************************************************************
// Diese Methode sollte innerhalb der MAIN-Schleife aufgerufen werden. Sie handhabt timeouts
// *****************************************************************************************
O3 INLINE void uwbranging_tick()
{
	// Lese den aktuellen Systick
	volatile unsigned long currentsystemtick = systickcounter;

	// Prüfe, ob ein Systemtimeout aufgetreten ist
	if (uwbranging_process.rangingrole == UWBRANGING_ROLE_SOURCE || uwbranging_process.rangingrole == UWBRANGING_ROLE_DESTINATION)
	{
		if (uwbranging_process.lasttimestamp > currentsystemtick) uwbranging_process.lasttimestamp = currentsystemtick;
		else if (currentsystemtick - uwbranging_process.lasttimestamp > UWBRANGING_TIMEOUT_MS)
		{
#ifdef DW1000_DEBUG_RANGING

		usart2_send("UWBRANGING: Timeout\r\n");

#endif

			//usart2_send("TIMEOUT\r\n"); // ToDo
			dw1000_idle();
			uwbranging_cancel();
			dw1000_startReceive();
			ismeasuring = 0;
		}
	}
	else if (uwbranging_process.rangingrole == UWBRANGING_ROLE_NONE && ismeasuring)
	{
		usart2_send("Messung beendet???\r\n");
		ismeasuring = 0;
	}
}


// **********************************************************************************
// Leitet (wenn möglich) eine neue Entfernungsermittlung zum gegebenen Zielknoten ein
// **********************************************************************************
unsigned char uwbranging_startRanging(unsigned short goal)
{
	// Prüfe, ob eine Entfernungsermittlung eingeleitet wurde
	unsigned char notbusy = !dw1000_isInTransmitState() && uwbranging_process.rangingrole == UWBRANGING_ROLE_NONE;
	unsigned char rightgoal = uwbranging_config.networkid > 0 && goal != 0 && goal != uwbranging_config.networkid;
	if (notbusy && rightgoal)
	{
		dw1000_wakefromsleep();

		// Erhöhe den Zähler für die Entfernungsermittlung
		uwbranging_rangingid = (uwbranging_rangingid == 0xff) ? 1 : uwbranging_rangingid + 1;

#ifdef DW1000_DEBUG_RANGING

		char tmp[128];
		sprintf(tmp, "UWBRANGING: %d -> PS0 -> %d (%d)\r\n", uwbranging_config.networkid, goal, uwbranging_rangingid);
		usart2_send(tmp);

#endif

		// Setze den Prozess auf "NONRANGING"
		uwbranging_process.rangingrole = UWBRANGING_ROLE_SOURCE;
		uwbranging_process.lasttimestamp = systickcounter;
		uwbranging_process.txtimeindex = 0;
		uwbranging_process.rangingid = uwbranging_rangingid;

		// Erstelle den Header
		uwbranging_packetheader_t header;
		header.destination = goal;
		header.source = uwbranging_config.networkid;
		header.packagetype = UWBRANGING_PACKAGETYPE_SOURCE_TX1;
		header.rangingid = uwbranging_rangingid;

		// Schreibt das angegebene Byte Array in den TX Buffer
		dw1000_initializeTransmit();
		dw1000_writeByteArrayToTxBuffer((unsigned char*)&header, sizeof(uwbranging_packetheader_t));
		dw1000_startTransmit(0);

		// Erfolgreich eingeleitet
		return 1;
	}
#ifdef DW1000_DEBUG_RANGING
	else
	{
		if (!notbusy) usart2_send("UWBRANGING: TX busy [Ranging not possible]\r\n");
		else if (!rightgoal) usart2_send("UWBRANGING: Goal address invalid [Ranging not possible]\r\n");
	}
#endif

	// Entfernungsermittlung kann nicht eingeleitet werden
	return 0;
}


// *********************************************
// Submits a text message to the remote instance
// *********************************************
unsigned char uwbranging_sendTextMessage(unsigned short goal, unsigned char* message, void (*handler)(unsigned long long timestamp, float temperature, unsigned char error))
{
	// Prüfe, ob eine Textmitteilung übermittelt werden kann
	if (!dw1000_isInTransmitState() && uwbranging_process.rangingrole == UWBRANGING_ROLE_NONE && uwbranging_config.networkid > 0 && goal != 0 && goal != uwbranging_config.networkid && message)
	{
		// Speichere den Handler
		uwbranging_txhandler = handler;

		// Erstelle den Header
		uwbranging_packetheader_t header;
		header.destination = goal;
		header.source = uwbranging_config.networkid;
		header.packagetype = UWBRANGING_PACKAGETYPE_TEXTMESSAGE;

		// Schreibt das angegebene Byte Array in den TX Buffer
		dw1000_initializeTransmit();
		dw1000_writeByteArrayAndHeaderToTxBuffer((unsigned char*)&header, sizeof(uwbranging_packetheader_t) - 1, message, strlen((const char*)message)+1);
		dw1000_startTransmit(0);

		// Erfolgreich eingeleitet
		return 1;
	}
	else if (handler) (*handler)(0, 0, UWBRANGING_MESSAGERESULT_BUSY);


	// Textmitteilung kann nicht gesendet werden
	return 0;
}


// **************************************************
// Submits a bytearray message to the remote instance
// **************************************************
unsigned char uwbranging_sendBinaryMessage(unsigned short goal, unsigned char* message, unsigned int messagelength, void (*handler)(unsigned long long timestamp, float temperature, uwbranging_messageresults_e error))
{
	// Prüfe, ob eine Textmitteilung übermittelt werden kann
	if (!dw1000_isInTransmitState() && uwbranging_process.rangingrole == UWBRANGING_ROLE_NONE && uwbranging_config.networkid > 0 && goal != 0 && goal != uwbranging_config.networkid && message && messagelength <= 128)
	{
		// Speichere den Handler
		uwbranging_txhandler = handler;

		// Erstelle den Header
		uwbranging_packetheader_t header;
		header.destination = goal;
		header.source = uwbranging_config.networkid;
		header.packagetype = UWBRANGING_PACKAGETYPE_BYTEARRAYMESSAGE;

		// Schreibt das angegebene Byte Array in den TX Buffer
		dw1000_initializeTransmit();
		dw1000_writeByteArrayAndHeaderToTxBuffer((unsigned char*)&header, sizeof(uwbranging_packetheader_t) - 1, message, messagelength);
		dw1000_startTransmit(0);

		// Erfolgreich eingeleitet
		return 1;
	}
	else if (handler) (*handler)(0, 0, UWBRANGING_MESSAGERESULT_BUSY);

	// Textmitteilung kann nicht gesendet werden
	return 0;
}


// *******************************************************************************************
// Registriert den Eventhandler, der aufgerufen wird, wenn eine Textmitteilung eingegangen ist
// *******************************************************************************************
void uwbranging_registerTextMessageHandler(void (*handler)(unsigned short, unsigned char*, unsigned long long int, double, float)) { uwbranging_textmessagehandler = handler; }


// ************************************************************************************************
// Registriert den Eventhandler, der aufgerufen wird, wenn eine Bytearraymitteilung eingegangen ist
// ************************************************************************************************
void uwbranging_registerByteArrayMessageHandler(void (*handler)(unsigned short, unsigned char*, int, unsigned long long int, double, float)) { uwbranging_bytemessagehandler = handler; }


// *****************************************************************************************
// Registriert einen Eventhandler, der aufgerufen wird, wenn eine Entfernung ermittelt wurde
// *****************************************************************************************
void uwbranging_registerRangeHandler(void (*handler)(unsigned short destination, float range)) { uwbranging_rangehandler = handler; }



