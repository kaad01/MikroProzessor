#ifndef HEADER_DW1000_USER
#define HEADER_DW1000_USER



//#define DW1000_OPTIMIZE_CODEELEMENTS (1)

#ifdef DW1000_OPTIMIZE_CODEELEMENTS

//! Manche Hilfsfunktionen können und sollten mit aktivierter Optimierung kompiliert werden. Ist das gewünscht, kann ein Optimierungspräfix hier als Makro angegeben werden
#define DW1000_OPTIMIZE __attribute__((optimize("O3")))


//! Manche Hilfsfunktionen können zur Geschwindigkeitsoptimierung inline aufgerufen werden. Ist das gewünscht, kann ein Inlinepräfix hier als Makro angegeben werden
#define DW1000_INLINE __attribute__((always_inline)) inline

#else

//! Ohne Funktion
#define DW1000_OPTIMIZE

//! Ohne Funktion
#define DW1000_INLINE

#endif


//! Wartet die angegebene Zeit an Millisekunden (gilt aber nur bei 168MHz Taktfrequenz)
void dw1000_delay_mSek(unsigned short ms);


//! Methode zur Initialisierung der Hardwareschnittstellen zum DW1000 Chip
/*!	Vor der Initialisierung des Chips, wird diese Funktion zur Initialisierung der Hardware aufgerufen.<br>
 *  Sie muss implementiert werden, damit alle Leitungen und Bussysteme für die spätere Verwendung bereit sind */
void dw1000_initializehardware(void);


//! Methode zur Aktivierung der Interrupt-Leitung für Statusänderungen des DW1000-Chips
/*!	Es ist mitunter notwendig, die Interrupte zeitweise zu deaktivieren, um Überläufe zu vermeiden.<br>
 *  Diese Methode muss implementiert werden, um eine Funktionalität zur Deaktivierung des Statusinterrupts zu gewährleisten */
void dw1000_enableInterrupts(void);


//! Methode zur Deaktivierung der Interrupt-Leitung für Statusänderungen des DW1000-Chips
/*!	Es ist mitunter notwendig, die Interrupte zeitweise zu deaktivieren, um Überläufe zu vermeiden.<br>
 *  Diese Methode muss implementiert werden, um eine Funktionalität zur Deaktivierung des Statusinterrupts zu gewährleisten */
void dw1000_disableInterrupts(void);


//! Methode zur Reduzierung der SPI-Taktung auf maximal 3MHz
void dw1000_reduceClock(void);


//! Methode zur Anhebung der SPI-Taktung auf maximal 20MHz
void dw1000_raiseClock(void);

//! Diese Methode liest und schreibt die SPI
/*! \param content Der Inhalt dieses Puffers wird auf die SPI geschrieben. Die Antwort wird ebenfalls in diesen Puffer gelesen.
 *  \param contentlength Anzahl der Symbole, die auf die SPI-Schnittstelle geschrieben, bzw. von ihr gelesen werden sollen */
unsigned short dw1000_readWriteSpi(unsigned char* content, unsigned short contentlength);


//! Diese Methode schaltet die Reset-Leitung auf Ausgang und legt einen HIGH-Pegel an.
void dw1000_setResetPin();


//! Diese Methode schaltet die Reset-Leitung auf Ausgang und legt einen LOW-Pegel an.
void dw1000_resetResetPin();


//! Diese Methode schaltet die Reset-Leitung auf Eingang und liest ihren Wert
unsigned char dw1000_readResetPin();


//! Diese Methode legt einen High-Pegel auf den Wake-Pin
void dw1000_setWakePin();


//! Diese Methode legt einen Low-Pegel auf den Wake-Pin
void dw1000_resetWakePin();


//! Diese Methode liest den Pegel, der aktuell am Wake-Pin angelegt wird
unsigned char dw1000_readWakePin();


//! Diese Methode wird aufgerufen, wenn ein Paket empfangen wurde
void dw1000_handleIncomingPackage();


//! Diese Methode wird aufgerufen, wenn ein Paket erfolgreich gesendet wurde
void dw1000_handleTransmittedPackage();


//! Diese Methode gibt die Anzahl der Millisekunden aus, die seit Systemstart vergangen sind
unsigned long int dw1000_getTickCounter();


void dw1000_log(char* str);


#endif
