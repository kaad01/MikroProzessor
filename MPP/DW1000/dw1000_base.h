#ifndef DW1000_HEADER
#define DW1000_HEADER



#include "dw1000_user.h"



// ---------------+------------------------+-----------------+-----------------------------+----------------------------
// Channel number | Centre frequency (MHz) | Bandwidth (MHz) | Preamble Codes (16 MHz PRF) | Preamble Codes (64 MHz PRF)
// ---------------+------------------------+-----------------+-----------------------------+----------------------------
// 1              | 3494.4 				   | 499.2           | 1, 2                        | 9, 10, 11, 12
// 2              | 3993.6 				   | 499.2           | 3,                          | 4 9, 10, 11, 12
// 3              | 4492.8 				   | 499.2           | 5, 6                        | 9, 10, 11, 12
// 4              | 3993.6 				   | 1331.2          | 7, 8                        | 17, 18, 19, 20
// 5              | 6489.6 				   | 499.2           | 3, 4                        | 9, 10, 11, 12
// 7              | 6489.6 				   | 1081.6          | 7, 8                        | 17, 18, 19, 20
// ---------------+------------------------+-----------------+-----------------------------+----------------------------




// Optionen für die Framefilter-Konfiuration
#define DW1000_FRAMEFILTER_DISABLED			0b00000000	/*!< \brief Frame Filtering Disabled */
#define DW1000_FRAMEFILTER_COORDINATOR		0b00000001	/*!< \brief Frame Filtering Behave as a Coordinator */
#define DW1000_FRAMEFILTER_ALLOW_BEACON		0b00000010	/*!< \brief Frame Filtering Allow Beacon frame reception */
#define DW1000_FRAMEFILTER_ALLOW_DATA		0b00000100	/*!< \brief Frame Filtering Allow Data frame reception */
#define DW1000_FRAMEFILTER_ALLOW_ACK		0b00001000	/*!< \brief Frame Filtering Allow Acknowledgment frame reception */
#define DW1000_FRAMEFILTER_ALLOW_MAC		0b00010000	/*!< \brief Frame Filtering Allow MAC command frame reception */
#define DW1000_FRAMEFILTER_ALLOW_RESERVED	0b00100000	/*!< \brief Frame Filtering Allow Reserved frame types */
#define DW1000_FRAMEFILTER_ALLOW_TYPE4		0b01000000	/*!< \brief Frame Filtering Allow frames with frame type field of 4 */
#define DW1000_FRAMEFILTER_ALLOW_TYPE5		0b10000000	/*!< \brief Frame Filtering Allow frames with frame type field of 5 */


// transmission/reception bit rate
#define DW1000_TRX_RATE_110KBPS 			0x00		/*!< \brief Wert für eine Datenrate von 110kb/sek */
#define DW1000_TRX_RATE_850KBPS 			0x01		/*!< \brief Wert für eine Datenrate von 850kb/sek */
#define DW1000_TRX_RATE_6800KBPS 			0x02		/*!< \brief Wert für eine Datenrate von 6800kb/sek */


// transmission pulse frequency
// 0x00 is 4MHZ, but receiver in DW1000 does not support it (!??)
#define DW1000_TX_PULSE_FREQ_16MHZ 			0x01		/*!< \brief Wert für eine Pulsfrequenz von 16MHz */
#define DW1000_TX_PULSE_FREQ_64MHZ 			0x02		/*!< \brief Wert für eine Pulsfrequenz von 64MHz */


// preamble length (PE + TXPSR bits)
#define DW1000_TX_PREAMBLE_LEN_64   		0b00000001	/*!< \brief Wert für eine Preambellänge von 64 Symbolen */
#define DW1000_TX_PREAMBLE_LEN_128  		0b00000101	/*!< \brief Wert für eine Preambellänge von 128 Symbolen */
#define DW1000_TX_PREAMBLE_LEN_256  		0b00001001	/*!< \brief Wert für eine Preambellänge von 256 Symbolen */
#define DW1000_TX_PREAMBLE_LEN_512  		0b00001101	/*!< \brief Wert für eine Preambellänge von 512 Symbolen */
#define DW1000_TX_PREAMBLE_LEN_1024 		0b00000010	/*!< \brief Wert für eine Preambellänge von 1024 Symbolen */
#define DW1000_TX_PREAMBLE_LEN_1536 		0b00000110	/*!< \brief Wert für eine Preambellänge von 1536 Symbolen */
#define DW1000_TX_PREAMBLE_LEN_2048 		0b00001010	/*!< \brief Wert für eine Preambellänge von 2048 Symbolen */
#define DW1000_TX_PREAMBLE_LEN_4096 		0b00000011	/*!< \brief Wert für eine Preambellänge von 4096 Symbolen */


/* preamble codes. */
#define DW1000_PREAMBLE_CODE_16MHZ_1 		1			/*!< \brief Wert für Preambelcode 1 bei einer Pulsfrequenz von 16MHz */
#define DW1000_PREAMBLE_CODE_16MHZ_2 		2			/*!< \brief Wert für Preambelcode 2 bei einer Pulsfrequenz von 16MHz */
#define DW1000_PREAMBLE_CODE_16MHZ_3 		3			/*!< \brief Wert für Preambelcode 3 bei einer Pulsfrequenz von 16MHz */
#define DW1000_PREAMBLE_CODE_16MHZ_4 		4			/*!< \brief Wert für Preambelcode 4 bei einer Pulsfrequenz von 16MHz */
#define DW1000_PREAMBLE_CODE_16MHZ_5 		5			/*!< \brief Wert für Preambelcode 5 bei einer Pulsfrequenz von 16MHz */
#define DW1000_PREAMBLE_CODE_16MHZ_6 		6			/*!< \brief Wert für Preambelcode 6 bei einer Pulsfrequenz von 16MHz */
#define DW1000_PREAMBLE_CODE_16MHZ_7 		7			/*!< \brief Wert für Preambelcode 7 bei einer Pulsfrequenz von 16MHz */
#define DW1000_PREAMBLE_CODE_16MHZ_8 		8			/*!< \brief Wert für Preambelcode 8 bei einer Pulsfrequenz von 16MHz */
#define DW1000_PREAMBLE_CODE_64MHZ_9 		9			/*!< \brief Wert für Preambelcode 9 bei einer Pulsfrequenz von 64MHz */
#define DW1000_PREAMBLE_CODE_64MHZ_10 		10			/*!< \brief Wert für Preambelcode 10 bei einer Pulsfrequenz von 64MHz */
#define DW1000_PREAMBLE_CODE_64MHZ_11 		11			/*!< \brief Wert für Preambelcode 11 bei einer Pulsfrequenz von 64MHz */
#define DW1000_PREAMBLE_CODE_64MHZ_12 		12			/*!< \brief Wert für Preambelcode 12 bei einer Pulsfrequenz von 64MHz */
#define DW1000_PREAMBLE_CODE_64MHZ_17 		17			/*!< \brief Wert für Preambelcode 17 bei einer Pulsfrequenz von 64MHz */
#define DW1000_PREAMBLE_CODE_64MHZ_18 		18			/*!< \brief Wert für Preambelcode 18 bei einer Pulsfrequenz von 64MHz */
#define DW1000_PREAMBLE_CODE_64MHZ_19 		19			/*!< \brief Wert für Preambelcode 19 bei einer Pulsfrequenz von 64MHz */
#define DW1000_PREAMBLE_CODE_64MHZ_20 		20			/*!< \brief Wert für Preambelcode 20 bei einer Pulsfrequenz von 64MHz */


/* channel of operation. */
#define DW1000_CHANNEL_1 					1			/*!< \brief Wert für Kanal 1 (3494.4MHz mit Bandbreite von 499.2MHz) */
#define DW1000_CHANNEL_2 					2			/*!< \brief Wert für Kanal 2 (3993.6MHz mit Bandbreite von 499.2MHz) */
#define DW1000_CHANNEL_3 					3			/*!< \brief Wert für Kanal 3 (4492.8MHz mit Bandbreite von 499.2MHz) */
#define DW1000_CHANNEL_4 					4			/*!< \brief Wert für Kanal 4 (3993.6MHz mit Bandbreite von 1081.6MHz) */
#define DW1000_CHANNEL_5 					5			/*!< \brief Wert für Kanal 5 (6489.6MHz mit Bandbreite von 499.2MHz) */
#define DW1000_CHANNEL_7 					7			/*!< \brief Wert für Kanal 7 (6489.6MHz mit Bandbreite von 1081.6MHz) */





//! Initialisiert das Modul
/*! Diese Funktion richtet das Modul ein und bringt es in einer Standardkonfiguration.<br>
 *  Der Aufruf dieser Funktion ist vor der Verwendung des Moduls erforderlich.<br>
 *  Die individuelle Konfiguration hat im Anschluss zu erfolgen. */
void dw1000_initialize(void);


//! Leitet einen Konfigurationsprozess ein
/*! Damit nicht jede Änderung sofort Wirkung zeigt, führt der Aufruf der Funktion dw1000_newConfiguration()
 *  zu einer Blockade der Änderungen solange, bis dw1000_commitConfiguration() aufgerufen wurde
 *  \param reset Setze alle Einstellung auf ihren Standardwert zurück */
void dw1000_newConfiguration(unsigned char reset);


//! Schließt einen Konfigurationsprozess ab und schreibt alle Einstellungen auf das Modul
/*! Damit nicht jede Änderung sofort Wirkung zeigt, führt der Aufruf der Funktion dw1000_newConfiguration()
 *  zu einer Blockade der Änderungen solange, bis dw1000_commitConfiguration() aufgerufen wurde */
void dw1000_commitConfiguration(void);


//! Interrupt handler
/*! Muss aufgerufen werden, sobald die Interrupt-Leitung anschlägt */
unsigned short dw1000_handleInterrupt();



//! Liest die Chip-ID, bestehend aus Revision, Version, Model und einem konstanten 0xdeca Wert
/*! \returns Gerätekennung [0xdeca (16-31); MODEL (8-15); VER (4-7); REV (0-3)] */
unsigned int dw1000_getDeviceId(void);


//! Aktiviert oder deaktiviert die Smart Power Funktionalität (nur bei 6.8MBit)
/*! Smart TX power control applies at the 6.8 Mbps data rate. When sending short data frames at this rate (and providing that the frame transmission rate is < 1 frame per millisecond) it is possible to increase the transmit power and still remain within regulatory power limits which are typically specified as average power per millisecond.
 *  \param val aktiv:1; inaktiv:0 */
void dw1000_setSmartPower(unsigned char val);


//! Gibt aus, ob die Smart Power Funktionalität aktiviert oder deaktiviert ist
/*! Smart TX power control applies at the 6.8 Mbps data rate. When sending short data frames at this rate (and providing that the frame transmission rate is < 1 frame per millisecond) it is possible to increase the transmit power and still remain within regulatory power limits which are typically specified as average power per millisecond.
 *  \returns 0 wenn Smart Power deaktiviert ist; ungleich 0 wenn Smart Power aktiv ist */
unsigned char dw1000_getSmartPower(void);


//! Legt die Flagmaske des Framefilters fest
/*! Frame filtering is a feature of the DW1000 IC that can parse the received data of a frame that complies with the MAC encoding standard, identifying the frame type and its destination address fields, match these against the IC’s own address information, and only accept frames that pass the filtering rules.<br><br>
 *  The frame filtering functionality allows the IC to be placed into receive mode and only interrupt the host processor when a frame arrives that passes the frame filtering criteria. When frame filtering is disabled all frames with good CRC are accepted, typically to interrupt the host with event status indicating a frame has been received with good CRC<br><br>
 *  Mögliche Flags sind
 *		<ul>
 *			<li>DW1000_FRAMEFILTER_DISABLED: Frame Filtering Disabled
 *			<li>DW1000_FRAMEFILTER_COORDINATOR: Frame Filtering Behave as a Coordinator
 *			<li>DW1000_FRAMEFILTER_ALLOW_BEACON: Frame Filtering Allow Beacon frame reception
 *			<li>DW1000_FRAMEFILTER_ALLOW_DATA: Frame Filtering Allow Data frame reception
 *			<li>DW1000_FRAMEFILTER_ALLOW_ACK: Frame Filtering Allow Acknowledgment frame reception
 *			<li>DW1000_FRAMEFILTER_ALLOW_MAC: Frame Filtering Allow MAC command frame reception
 *			<li>DW1000_FRAMEFILTER_ALLOW_RESERVED: Frame Filtering Allow Reserved frame types
 *			<li>DW1000_FRAMEFILTER_ALLOW_TYPE4: Frame Filtering Allow frames with frame type field of 4
 *			<li>DW1000_FRAMEFILTER_ALLOW_TYPE5: Frame Filtering Allow frames with frame type field of 5
 *		</ul>
 *  \param val Flagmaske des Framefilters */
void dw1000_setFrameFilter(unsigned char val);


//! Gibt das aktuelle Flagmaske des Framefilters aus
/*! Frame filtering is a feature of the DW1000 IC that can parse the received data of a frame that complies with the MAC encoding standard, identifying the frame type and its destination address fields, match these against the IC’s own address information, and only accept frames that pass the filtering rules.<br><br>
 *  The frame filtering functionality allows the IC to be placed into receive mode and only interrupt the host processor when a frame arrives that passes the frame filtering criteria. When frame filtering is disabled all frames with good CRC are accepted, typically to interrupt the host with event status indicating a frame has been received with good CRC<br><br>
 *  Mögliche Flags sind
 *		<ul>
 *			<li>DW1000_FRAMEFILTER_DISABLED: Frame Filtering Disabled
 *			<li>DW1000_FRAMEFILTER_COORDINATOR: Frame Filtering Behave as a Coordinator
 *			<li>DW1000_FRAMEFILTER_ALLOW_BEACON: Frame Filtering Allow Beacon frame reception
 *			<li>DW1000_FRAMEFILTER_ALLOW_DATA: Frame Filtering Allow Data frame reception
 *			<li>DW1000_FRAMEFILTER_ALLOW_ACK: Frame Filtering Allow Acknowledgment frame reception
 *			<li>DW1000_FRAMEFILTER_ALLOW_MAC: Frame Filtering Allow MAC command frame reception
 *			<li>DW1000_FRAMEFILTER_ALLOW_RESERVED: Frame Filtering Allow Reserved frame types
 *			<li>DW1000_FRAMEFILTER_ALLOW_TYPE4: Frame Filtering Allow frames with frame type field of 4
 *			<li>DW1000_FRAMEFILTER_ALLOW_TYPE5: Frame Filtering Allow frames with frame type field of 5
 *		</ul>
 *  \returns Flagmaske des Framefilters  */
unsigned char dw1000_getFrameFilter(void);


//! Aktiviert oder deaktiviert das Double Buffering
/*! This DW1000 has a pair of receive buffers offering the capability to receive into one of the pair while the host system is reading previously received data from the other buffer of the pair. This is useful in a TDOA RTLS anchor node where it is desired to have the receiver on as much as possible to avoid missing any tag blink messages.
 *  \param val 0: deaktivieren; 1: aktivieren */
void dw1000_setDoubleBuffering(unsigned char val);


//! Gibt aus, ob das Double Buffering aktiviert oder deaktiviert ist
/*! This DW1000 has a pair of receive buffers offering the capability to receive into one of the pair while the host system is reading previously received data from the other buffer of the pair. This is useful in a TDOA RTLS anchor node where it is desired to have the receiver on as much as possible to avoid missing any tag blink messages.
 *  \returns 0: deaktiviert; 1: aktiviert */
unsigned char dw1000_getDoubleBuffering(unsigned char val);


//! Aktiviert oder deaktiviert die automatische Aktivierung des Empfängers
/*! \param val 0: deaktivieren; 1: aktivieren */
void dw1000_setReceiverAutoReenable(unsigned char val);


//! Gibt aus, ob die automatische Aktivierung des Empfängers aktiviert oder deaktiviert ist
/*! \returns 0: deaktiviert; 1: aktiviert */
unsigned char dw1000_getReceiverAutoReenable(void);


//! Aktiviert oder deaktiviert das automatische Empfangen direkt nach dem Senden
/*! \param val 0: deaktivieren; 1: aktivieren */
void dw1000_setReceiveAfterTX(unsigned char val);


//! Gibt aus, ob die nach dem Senden automatisch empfangen werden soll
/*! \returns 0: deaktiviert; 1: aktiviert */
unsigned char dw1000_getReceiveAfterTX(void);


//! Aktiviere oder deaktiviere die erweiterte Framegröße (1023 statt 127 Bytes)
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  \param val 0: deaktivieren; 1: aktivieren */
void dw1000_setExtendedFrameLength(unsigned char val);


//! Gibt aus, ob die erweiterte Framegröße (1023 statt 127 Bytes) aktiviert oder deaktiviert ist
/*! \returns 0: deaktiviert; 1: aktiviert */
unsigned char dw1000_getExtendedFrameLength(void);


//! Legt die Datenrate fest
/*! Mögliche Werte sind:
 *		<ul>
 *			<li>DW1000_TRX_RATE_110KBPS: 110kb/sek
 *			<li>DW1000_TRX_RATE_850KBPS: 850kb/sek
 *			<li>DW1000_TRX_RATE_6800KBPS: 6.8Mb/sek
 *		</ul>
 *  \param rate Einzustellende Datenrate */
void dw1000_setDataRate(unsigned char rate);

//! Gibt die aktuelle Datenrate aus
/*! Mögliche Werte sind:
 *		<ul>
 *			<li>DW1000_TRX_RATE_110KBPS: 110kb/sek
 *			<li>DW1000_TRX_RATE_850KBPS: 850kb/sek
 *			<li>DW1000_TRX_RATE_6800KBPS: 6.8Mb/sek
 *		</ul>
 *  \returns Aktuelle Datenrate */
unsigned char dw1000_getDataRate(void);


//! Legt die Pulsfrequenz des Moduls fest
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  Specifies the pulse repetition frequency (PRF) of data transmissions with the DW1000. Either
 *  <ul>
 *		<li>DW1000_TX_PULSE_FREQ_16MHZ: 16MHz
 *		<li>DW1000_TX_PULSE_FREQ_64MHZ: 64MHz
 *	</ul>
 *  has to be chosen.<br>
 *  Note that the 16 MHz setting is more power efficient, while the 64 MHz setting requires more
 *  power, but also delivers slightly better transmission performance (i.e. on communication range and
 *  timestamp accuracy) (see DWM1000 User Manual, section 9.3). */
void dw1000_setPulseFrequency(unsigned char freq);


//! Gibt die Pulsfrequenz des Moduls aus
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  Specifies the pulse repetition frequency (PRF) of data transmissions with the DW1000. Either
 *  <ul>
 *		<li>DW1000_TX_PULSE_FREQ_16MHZ: 16MHz
 *		<li>DW1000_TX_PULSE_FREQ_64MHZ: 64MHz
 *	</ul>
 *  has to be chosen.<br>
 *  Note that the 16 MHz setting is more power efficient, while the 64 MHz setting requires more
 *  power, but also delivers slightly better transmission performance (i.e. on communication range and
 *  timestamp accuracy) (see DWM1000 User Manual, section 9.3).
 *  \returns Aktuelle Pulsfrequenz */
unsigned char dw1000_getPulseFrequency(void);


//! Legt die Preamblelänge des Moduls fest
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  Mögliche Werte sind:
 * 	<ul>
 *		<li>DW1000_TX_PREAMBLE_LEN_64
 * 		<li>DW1000_TX_PREAMBLE_LEN_128
 *		<li>DW1000_TX_PREAMBLE_LEN_256
 *		<li>DW1000_TX_PREAMBLE_LEN_512
 *		<li>DW1000_TX_PREAMBLE_LEN_1024
 *		<li>DW1000_TX_PREAMBLE_LEN_1536
 *		<li>DW1000_TX_PREAMBLE_LEN_2048
 *		<li>DW1000_TX_PREAMBLE_LEN_4096
 *	</ul>
 *	\param prealen Einzustellende Preambellänge (Achtung! Sollte die Länge inkompatibel zu den restlichen Einstellungen sein, wird sie auf einen Standardwert gesetzt) */
void dw1000_setPreambleLength(unsigned char prealen);


//! Gibt die aktuelle Preamblelänge des Moduls aus
/*! Mögliche Werte sind:
 * 	<ul>
 *		<li>DW1000_TX_PREAMBLE_LEN_64
 * 		<li>DW1000_TX_PREAMBLE_LEN_128
 *		<li>DW1000_TX_PREAMBLE_LEN_256
 *		<li>DW1000_TX_PREAMBLE_LEN_512
 *		<li>DW1000_TX_PREAMBLE_LEN_1024
 *		<li>DW1000_TX_PREAMBLE_LEN_1536
 *		<li>DW1000_TX_PREAMBLE_LEN_2048
 *		<li>DW1000_TX_PREAMBLE_LEN_4096
 *	</ul>
 *	\param prealen Aktuelle Preambellänge */
unsigned char dw1000_getPreambleLength(void);



//! Legt die Preambelkodierung des Moduls fest
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  Mögliche Werte sind:
 * 	<ul>
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_1
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_2
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_3
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_4
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_5
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_6
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_7
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_8
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_9
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_10
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_11
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_12
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_17
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_18
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_19
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_20
 *	</ul>
 *	\param prealen Einzustellende Preambelkodierung (Achtung! Sollte die Kodierung inkompatibel zu den restlichen Einstellungen sein, wird sie auf einen Standardwert gesetzt) */
void dw1000_setPreambleCode(unsigned char preacode);


//! Gibt die aktuell verwendete Preambelkodierung aus
/*!  Mögliche Werte sind:
 * 	<ul>
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_1
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_2
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_3
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_4
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_5
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_6
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_7
 *		<li>DW1000_PREAMBLE_CODE_16MHZ_8
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_9
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_10
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_11
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_12
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_17
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_18
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_19
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_20
 *	</ul>
 *	\returns prealen Aktuelle Preambelkodierung */
unsigned char dw1000_getPreambleCode(void);


//! Legt den Kanal fest, auf dem kommuniziert wird
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  Mögliche Werte sind:
 * 	<ul>
 *		<li>DW1000_CHANNEL_1 (3494.4MHz mit Bandbreite von 499.2MHz)
 *		<li>DW1000_CHANNEL_2 (3993.6MHz mit Bandbreite von 499.2MHz)
 *		<li>DW1000_CHANNEL_3 (4492.8MHz mit Bandbreite von 499.2MHz)
 *		<li>DW1000_CHANNEL_4 (3993.6MHz mit Bandbreite von 1081.6MHz)
 *		<li>DW1000_CHANNEL_5 (6489.6MHz mit Bandbreite von 499.2MHz)
 *		<li>DW1000_CHANNEL_7 (6489.6MHz mit Bandbreite von 1081.6MHz)
 *	</ul>
 *	\param newchannel Einzustellender Funkkanal */
void dw1000_setChannel(unsigned char newchannel);


//! Gibt den Kanal aus, auf dem kommuniziert wird
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  Mögliche Werte sind:
 * 	<ul>
 *		<li>DW1000_CHANNEL_1 (3494.4MHz mit Bandbreite von 499.2MHz)
 *		<li>DW1000_CHANNEL_2 (3993.6MHz mit Bandbreite von 499.2MHz)
 *		<li>DW1000_CHANNEL_3 (4492.8MHz mit Bandbreite von 499.2MHz)
 *		<li>DW1000_CHANNEL_4 (3993.6MHz mit Bandbreite von 1081.6MHz)
 *		<li>DW1000_CHANNEL_5 (6489.6MHz mit Bandbreite von 499.2MHz)
 *		<li>DW1000_CHANNEL_7 (6489.6MHz mit Bandbreite von 1081.6MHz)
 *	</ul>
 *	\Returns Aktueller Funkkanal */
unsigned char dw1000_getChannel(void);


//! Legt die Verzögerung zwischen Funkchip und Antenne fest
/*! \param delay Antennenverzögerung (in Systemtakten - ca. 15ps) */
void dw1000_setAntennaDelay(unsigned short rxdelay, unsigned short txdelay);


//! Gibt die aktuelle Sendeleistung aus
/*! \returns Sendeleistung in dBm */
float dw1000_getTxPower();


//! Legt die Sendeleistung fest
/*! \param power Sendeleistung in dBm */
float dw1000_setTxPower(float power);






//! Führt einen Hardwarereset durch
/*! Achtung! Anschließend muss das System neu konfiguriert werden! */
void dw1000_reset();


//! Bringt den Chip in den idle modus
void dw1000_idle(void);


//! Überprüft den aktuellen Status des Systems, indem Registerinhalte mit dem gepufferten Status verglichen werden.
/*! \returns 1, wenn der gepufferte Status gültig ist, sonst 0 */
unsigned char dw1000_checkstate(void);


//! Überprüft, ob der TX-Modus ggf. hängen geblieben ist
/*! \returns 0, wenn der gepufferte Status gültig ist, sonst 1 */
unsigned char dw1000_checktxstuck(void);


unsigned char dw1000_getDeviceMode();

//! Versetzt das Modul in den Empfangsmodus
void dw1000_startReceive();


//! Aktiviert den Sniff-Modus mit einer 50:50 Abtastung
void dw1000_enableSniffMode();


//! Deaktiviert den Sniff-Modus
void dw1000_disableSniffMode();


//! Initialisiert das Senden eines Pakets
unsigned char dw1000_initializeTransmit();


//! Sendet das in den Sendepuffer geschriebene Paket
/*! Achtung! Erforderlich ist der Aufruf von dw1000_initializeTransmit() mit anschließendem Schreiben des Sendepuffers bevor dw1000_startTransmit aufgerufen werden kann.
 *  \param timestamp Ist dieser Wert ungleich 0, dann wird das Paket gesendet, sobald die Systemzeit dem timestamp entspricht */
unsigned char dw1000_startTransmit(unsigned long long int timestamp);


//! Diese Methode gibt an, ob sich der Programmstack aktuell in einem Interrupt befindet
unsigned char dw1000_isIrqActive();


//! Gibt den Zeitstempel aus, an dem der Transfer abgeschlossen wurde (126MHz Basis)
/*! \returns Zeitpunkt, an dem der Anfang des PHY-Headers die Antenne verlassen hat */
unsigned long long int dw1000_getRawTransmitTimestamp();


//! Gibt den Zeitstempel aus, an dem der Transfer abgeschlossen wurde
/*! \returns Zeitpunkt, an dem der Anfang des PHY-Headers die Antenne verlassen hat */
unsigned long long int dw1000_getTransmitTimestamp();


//! Gibt den Zeitstempel aus, an dem das letzte Paket empfangen wurde (126MHz Basis)
/*! \returns Zeitpunkt, an dem der Anfang des PHY-Headers die Antenne erreicht hat */
unsigned long long int dw1000_getRawReceiveTimestamp();


//! Gibt den Zeitstempel aus, an dem das letzte Paket empfangen wurde
/*! \returns Zeitpunkt, an dem der Anfang des PHY-Headers die Antenne erreicht hat */
unsigned long long int dw1000_getReceiveTimestamp();


//! Liest die Systemzeit aus dem Modul aus
/*! \returns Aktuelle Systemzeit in Systemeinheiten (ca. 15ps) */
unsigned long long int dw1000_getSystemTimestamp();


//! LDE Threshold report
/*! \returns The threshold is calculated based on an estimate of the noise made during the LDE algorithm’s analysis of the accumulator data. */
unsigned short dw1000_getLDEThreshold();


//! Lädt den RX Time Tracking Offset in ppm
/*! \returns Verzögerung zwischen diesem Knoten und dem Sender des letzten Pakets */
double dw1000_getTimeTrackingOffset();


//! Diese Methode gibt 1 aus, wenn aktuell übermittelt wird
/*! \returns 1 wenn aktuell gesendet wird, sonst 0 */
unsigned char dw1000_isInTransmitState();


//! Diese Methode gibt 1 aus, wenn sich das Modul aktuell im Empfangsmodus befindet
/*! \returns 1, wenn sich das Modul aktuell im Empfangsmodus befindet */
unsigned char dw1000_isInReceiveState();


//! Diese Methode gibt den Empfangszähler aus
/*! Der Empfangszähler wird immer dann erhöht, wenn ein RX-Interrupt ausgelöst wurde. Er kann verwendet werden, um zu ermitteln ob seit der letzten Abfrage Pakete eingegangen sind.
 *  \returns Empfangszähler */
unsigned long int dw1000_getReceiveCounter();


//! Gibt die Empfangsleistung für das letzte Paket in dBm aus
/*! \returns Empfangsleistung des letzten Pakets in dBm */
double dw1000_getReceivePower();


//! Gibt die Empfangsleistung der ersten drei Flanken des PHY-Headers (und das Rauschen auf dem Kanal) aus
/*! Anhand dieser Flanken berechnet sich der Zeitstempel für eingehende Pakete
 *  \returns RAUSCHEN (Bits 18-32); EMPFANGSLEISTUNG (Bits 0-17) */
unsigned int dw1000_getAccumulatedFirstPathPower();


//! Liest die angegebene Anzahl von Zeichen an der angegebenen Adresse und schreibt sie in den übergebenen Buffer
/*! \param address Registeradresse
 *  \param length Anzahl der zu lesenden Zeichen
 *  \param buffer Puffer, in den gelesen werden soll
 *  \param offset Registeroffset (siehe Doku.)
 *  \returns Anzahl der transferrierten Zeichen (inkl. Header) */
int dw1000_read(unsigned char address, unsigned short length, unsigned char* buffer, unsigned short offset);


//! Liest eine vorzeichenlose 32Bit Zahl aus dem angegebenen Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \returns Gelesener Registerinhalt */
unsigned int dw1000_readUInt32(unsigned char address, unsigned short offset);


//! Liest eine vorzeichenlose 16Bit Zahl aus dem angegebenen Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \returns Gelesener Registerinhalt */
unsigned short dw1000_readUInt16(unsigned char address, unsigned short offset);


//! Liest eine vorzeichenlose 8Bit Zahl aus dem angegebenen Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \returns Gelesener Registerinhalt */
unsigned char dw1000_readUInt8(unsigned char address, unsigned short offset);


//! Schreibt einen 32Bit Wert in das angegebene Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \param value Zu schreibender Wert */
void dw1000_writeUInt32(unsigned char address, unsigned short offset, unsigned int value);


//! Schreibt einen 16Bit Wert in das angegebene Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \param value Zu schreibender Wert */
void dw1000_writeUInt16(unsigned char address, unsigned short offset, unsigned short value);


//! Schreibt einen 8Bit Wert in das angegebene Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \param value Zu schreibender Wert */
void dw1000_writeUInt8(unsigned char address, unsigned short offset, unsigned char value);


//! Schreibt die angegebene Anzahl von Zeichen aus dem übergebenen Buffer an die angegebene Adresse
/*! \param address Registeradresse
 *  \param length Anzahl der zu schreibenden Daten
 *  \param buffer Zu schreibende Daten
 *  \param offset Registeroffset (siehe Doku)
 *  \returns Anzahl der geschriebenen Zeichen (inkl. Header) */
int dw1000_write(unsigned char address, unsigned short length, unsigned char* buffer, unsigned short offset);


//! Liest den Akkumulatorspeicher aus
/*! Achtung! Das erste Symbol wird ignoriert - Dokumentation lesen
 *  \param buffer Buffer, in den der Speicherinhalt geschrieben werden soll
 *  \param length Anzahl der gelesenen Zeichen (Achtung! das erste wird stets ignoriert)
 *  \returns Anzahl der gelesenen Zeichen */
unsigned short dw1000_getAccMem(unsigned char* buffer, unsigned short length);


//! Liest den RX Buffer in das gegebene Byte Array
/*! \param array Array, in das der RX-Puffer geschrieben werden soll
 *  \param len Anzahl der Zeichen, die gelesen werden sollen */
void dw1000_readRxBufferToByteArray(unsigned char* array, int len);


//! Gibt die Anzahl der Zeichen im RX Buffer oder TX Buffer (abhängig vom Modus) aus
/*! \returns Anzahl der Zeichen im RX-Buffer */
unsigned short dw1000_getRxTxBufferLen();


//! Schreibt das angegebene Byte Array in den TX Buffer
/*! \param array Array, das zu übermitteln ist
 *  \param len Anzahl der Symbole aus dem Array, die zu übermitteln sind */
void dw1000_writeByteArrayToTxBuffer(unsigned char* array, int len);


//! Schreibt das angegebene Byte Array sowie Headerinformationen in den TX Buffer
/*! \param header Bytearray mit Headerinformationen
 *  \param headerlength Länge des Headers
 *  \param array Array, das zu übermitteln ist
 *  \param len Anzahl der Symbole aus dem Array, die zu übermitteln sind */
void dw1000_writeByteArrayAndHeaderToTxBuffer(unsigned char* header, int headerlength, unsigned char* array, int arraylength);


//! Schreibt die gegebene Zeichenkette in den TX Buffer
/*! \param array Null-terminierte Zeichenkette, die kopiert werden soll */
void dw1000_writeStringTTxBuffer(unsigned char* array);


//! Schreibt die gegebene Zeichenkette sowie Headerinformationen in den TX Buffer
/*! \param header Bytearray mit Headerinformationen
 *  \param headerlength Länge des Headers
 *  \param array Null-terminierte Zeichenkette, die kopiert werden soll */
void dw1000_writeStringAndHeaderToTxBuffer(unsigned char* header, int headerlength, unsigned char* array);


//! Diese Funktion aktiviert die Temperatur- und Spannungsmessung
void dw1000_enableTemperatureAndVoltageMeasurement();


//! Diese Funktion gibt die Temperatur in °C aus
char dw1000_getTemperature();


//! Diese Funktion gibt die Spannung in V aus
int dw1000_getVoltage();


//! Diese Funktion schätzt die benötigte Paketdauer für das angegebene Paket
/*! \param datarate Eingestellte Datenrate (0.11MBit; 0.85MBit oder 6.80MBit)
 *  \param pulsefrequency Pulsfrequenz (16MHz oder 64MHz)
 *  \param preamblelength Preambellänge (128 - 2048 Symbole)
 *  \param framelength Anzahl der zu übermittelnden Symbole (inkl. 2CRC bytes)
 *  \param only_rmarker Wenn die Nutzdaten ignoriert werden sollen, dann 1 */
float dw1000_estimatePackageTime(unsigned char datarate, unsigned char pulsefrequency, unsigned char preamblelength, unsigned short framelength, unsigned char only_rmarker);



void dw1000_preparesleep();

void dw1000_entersleep();

void dw1000_wakefromsleep();

unsigned char dw1000_getClockOffset();

unsigned char dw1000_setClockOffset(unsigned char offset);


#endif
