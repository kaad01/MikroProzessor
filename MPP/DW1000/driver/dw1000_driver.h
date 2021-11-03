#ifndef DW1000_DRIVER_HEADER
#define DW1000_DRIVER_HEADER

// Definitionen einbinden
#include "dw1000_base.h"
#include "dw1000_def.h"
#include "dw1000_user.h"


// Benötigte Header einbinden
#include "math.h"

#ifndef DW1000_OPTIMIZE
#define DW1000_OPTIMIZE
#endif

#ifndef DW1000_INLINE
#define DW1000_INLINE
#endif

#define DW1000_HANDLECRCERROR (1)

extern unsigned short dw1000_antennadelay_rx;			//!< Legt fest, welche zeitliche Verzögerung zwischen Aus-/Eingang am Funkchip und Aus-/Eingang an der Antenne vorliegt
extern unsigned short dw1000_antennadelay_tx;			//!< Legt fest, welche zeitliche Verzögerung zwischen Aus-/Eingang am Funkchip und Aus-/Eingang an der Antenne vorliegt

extern unsigned char dw1000_extendedFrameLength;		//!< Legt fest, ob die erweiterte Framelänge verwendet werden soll (1: 1023Bytes) oder nicht (0: 127Bytes)
extern unsigned char dw1000_frameCheck;					//!< Legt fest, ob die Frameüberprüfungsfunktionalität verwendet werden soll (1) oder nicht (0)
extern unsigned char dw1000_devicemode;					//!< Speichert den aktuellen Zustand des Moduls (RX,TX,IDLE)
extern unsigned char dw1000_pulseFrequency;				//!< Legt die Pulsfrequenz fest (16 oder 64MHz)
extern unsigned char dw1000_doublebuffered;				//!< Legt fest, ob der Doppelpuffer verwendet werden soll (1) oder nicht (0)
extern unsigned char dw1000_sniffmodeoff;				//!< Gibt an, wie lange das Modul im Sniff-Modus auf eine Preambel hören soll
extern unsigned char dw1000_sniffmodeon;				//!< Gibt an, wie lange das Modul zwischen zwei Preamblesuchvorgängen schlafen soll (in µs)
extern unsigned char dw1000_receiverAutoReenable;		//!< Legt fest, ob sich der Empfänger bei einem Fehler automatisch wieder in den RX-Modus setzen soll (1) oder nicht (0)
extern unsigned char dw1000_receiveAfterTX;				//!< Legt fest, ob direkt nach dem Senden wieder Empfangen werden soll
extern unsigned long int dw1000_tx_fctrl;				//!< Abbild des aktuellen TX_FCTRL-Registerinhalts (Transmit Frame Control)




// **************************************************************************************************************************
// dw1000_driver.c
// **************************************************************************************************************************

//! Legt die Taktung des Moduls fest
int dw1000_setClock(unsigned long int clock);

//! Setzt den Empfangspfad zurück
/*! \param startreceiving Wird auf 1 gesetzt, wenn das Modul nach dem Reset in den Empfangsmodus versetzt werden soll. Sonst 0 */
void dw1000_resetRx(unsigned char startreceiving);





// **************************************************************************************************************************
// dw1000_config.c
// **************************************************************************************************************************

//! Lädt die LDO-Kalibrierungsdaten, wenn notwendig
void dw1000_loadLDOCalibrationData();

//! Liest den XTAL Trimmungswert
void dw1000_loadXTalTrimming();

//! Lädt den leading edge Code in den Geräteram
void dw1000_loadLeadingEdgeAlgorithm();




// Setzt den Kanal des Moduls
void dw1000_setChannel(unsigned char channel);

// ...
double dw1000_getReceiverPower();

// Schreibt die Netzwerkkennung
// ---------------------------------------------------------------------------------------------
// (Re-)set the network identifier which the selected chip should be associated with. This
// setting is important for certain MAC address filtering rules.
void dw1000_setNetworkId(unsigned int val);

// Liest die Netzwerkkennung
unsigned int dw1000_getNetworkId();

// Schreibt die Gerätekennung im Netzwerk
// ---------------------------------------------------------------------------------------------
// (Re-)set the device address (i.e. short address) for the currently selected chip. This
// setting is important for certain MAC address filtering rules.
void dw1000_setDeviceAddress(unsigned int val);

// Liest die Gerätekennung im Netzwerk
unsigned int dw1000_getDeviceAddress();

// Schreibt die EUI
void dw1000_setEUI(unsigned char* val, int len);

// Liest die EUI
void dw1000_getEUI(unsigned char* val, int len);




















// **************************************************************************************************************************
// dw1000_io.c
// **************************************************************************************************************************

//! Schreibt die angegebene Anzahl von Zeichen aus dem übergebenen Buffern an die angegebene Adresse
/*! Die Methode sendet den Datenblock buffer1+buffer2 mit der Länge length1+length2 an die gegebene Registeradresse
 *  \param address Registeradresse
 *  \param length1 Länge des ersten Buffers
 *  \param buffer1 Erster Buffer
 *  \param length2 Länge des zweiten Buffers
 *  \param buffer2 Zweiter Buffer
 *  \returns Anzahl der geschriebenen Zeichen (inkl. Header) */
int dw1000_writeTwoBuffers(unsigned char address, unsigned short length1, unsigned char* buffer1, unsigned short length2, unsigned char* buffer2);


//! Liest einen 8Bit-Wert aus dem OTP-Speicher
/*! \param address Registeradresse
 *  \returns Gelesener Registerinhalt */
unsigned char dw1000_readOTPUInt8(unsigned short address);


//! Liest einen 16Bit-Wert aus dem OTP-Speicher
/*! \param address Registeradresse
 *  \returns Gelesener Registerinhalt */
unsigned short dw1000_readOTPUInt16(unsigned short address);


//! Liest einen 32Bit-Wert aus dem OTP-Speicher
/*! \param address Registeradresse
 *  \returns Gelesener Registerinhalt */
unsigned int dw1000_readOTPUInt32(unsigned short address);




#endif
