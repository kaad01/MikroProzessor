#include "dw1000_driver.h"
#include <stdio.h>

#ifndef DW1000_OPTIMIZE
#define DW1000_OPTIMIZE
#endif

#ifndef DW1000_INLINE
#define DW1000_INLINE
#endif

char dw1000_temperature = 0;							//!< Letzte gemessene und korrigierte Temperatur in °C
unsigned long int dw1000_lasttemperaturemeasure = 0;	//!< Zeitpunkt der letzten Temperaturmessung in ms
unsigned long int dw1000_receivecounter = 0;			//!< Empfangscounter (bei jedem RX-Interrupt wird er um 1 erhöht)

unsigned char dw1000_reftemperature = 0;				//!< Kalibrierte Referenztemperatur
unsigned char dw1000_lasttemperature = 0;				//!< Letzter gespeicherter Temperaturwert
unsigned char dw1000_refvoltage = 0;					//!< Kalibrierte Referenzspannung


//! Legt die Taktung des Moduls fest
DW1000_OPTIMIZE int dw1000_setClock(unsigned long int clock)
{
	// Bereite den gewünschten Registerinhalt vor
	unsigned char pmscctrl0[2];

	// Lese die Taktungseinstellung aus dem Modul
	dw1000_read(DW1000_PMSC, 2, pmscctrl0, DW1000_PMSC_CTRL0_OFFSET);

	// Prüfe, ob die Systemtaktung angepasst werden muss
	if (clock & DW1000_CLOCK_SYS_MASK)
	{
		switch(clock & DW1000_CLOCK_SYS_MASK)
		{
			case DW1000_CLOCK_SYS_XTI:	pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_SYS_RST) | 0b00000001;	break;
			case DW1000_CLOCK_SYS_PLL:	pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_SYS_RST) | 0b00000010;	break;
			default:					pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_SYS_RST);				break;
		}
	}

	// Prüfe, ob die RX-Taktung angepasst werden muss
	if (clock & DW1000_CLOCK_RX_MASK)
	{
		switch(clock & DW1000_CLOCK_RX_MASK)
		{
			case DW1000_CLOCK_RX_XTI:	pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_RX_RST) | 0b00000100;	break;
			case DW1000_CLOCK_RX_PLL:	pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_RX_RST) | 0b00001000;	break;
			case DW1000_CLOCK_RX_OFF:	pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_RX_RST) | 0b00001100;	break;
			default:					pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_RX_RST);				break;
		}
	}

	// Prüfe, ob die TX-Taktung angepasst werden muss
	if (clock & DW1000_CLOCK_TX_MASK)
	{
		switch(clock & DW1000_CLOCK_TX_MASK)
		{
			case DW1000_CLOCK_TX_XTI:	pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_TX_RST) | 0b00010000;	break;
			case DW1000_CLOCK_TX_PLL:	pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_TX_RST) | 0b00100000;	break;
			case DW1000_CLOCK_TX_OFF:	pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_TX_RST) | 0b00110000;	break;
			default:					pmscctrl0[0] = (pmscctrl0[0] & DW1000_CLOCK_TX_RST);				break;
		}
	}

	// Prüfe, ob die Akkumulator-Taktung manipuliert werden muss
	if (clock & DW1000_CLOCK_ENABLE_ACCUMULATOR)
	{
		pmscctrl0[0] = pmscctrl0[0] | 0b01000000;
		pmscctrl0[1] = pmscctrl0[1] | 0b10000000;
	}
	else if (clock & DW1000_CLOCK_DISABLE_ACCUMULATOR)
	{
		pmscctrl0[0] = pmscctrl0[0] | 0b10111111;
		pmscctrl0[1] = pmscctrl0[1] & 0b01111111;
	}

	// Prüfe, ob die OTP-Taktung manipuliert werden muss
	if (clock & DW1000_CLOCK_ENABLE_OTP) 		pmscctrl0[1] = pmscctrl0[1] | 0b00000010;
	else if (clock & DW1000_CLOCK_DISABLE_OTP)	pmscctrl0[1] = pmscctrl0[1] & 0b11111101;

	// Prüfe, ob die LDU-Taktung manipuliert werden muss
	if (clock & DW1000_CLOCK_ENABLE_LDU) 		pmscctrl0[1] = pmscctrl0[1] | 0b00000001;
	else if (clock & DW1000_CLOCK_DISABLE_LDU)	pmscctrl0[1] = pmscctrl0[1] & 0b11111110;

	// Prüfe, ob die ADC-Taktung manipuliert werden muss
	if (clock & DW1000_CLOCK_ENABLE_ADCCE) 		pmscctrl0[1] = pmscctrl0[1] | 0b00000100;
	else if (clock & DW1000_CLOCK_ENABLE_ADCCE)	pmscctrl0[1] = pmscctrl0[1] & 0b11111011;

	// Schreibe die geänderten Taktungseinstellungen
	dw1000_writeUInt8(DW1000_PMSC, 0, pmscctrl0[0]);
	dw1000_writeUInt8(DW1000_PMSC, 1, pmscctrl0[1]);

	// Success
	return 1;
}

//! Interrupt handler
/*! Muss aufgerufen werden, sobald die Interrupt-Leitung anschlägt */
DW1000_OPTIMIZE DW1000_INLINE unsigned short dw1000_handleInterrupt()
{
	unsigned char handled = 0;
	dw1000_irqactive = 1;

	// Lese das Interrupt Status register
	unsigned int sysstatus = dw1000_readUInt32(DW1000_SYS_STATUS, 0);

	// Fange den Status ab, der den erfolgreichen Transfer eines Pakets bestätigt
	if (sysstatus & DW1000_SYS_STATUS_TX)
	{
		if (dw1000_receiveAfterTX)
		{
			// Setze die Flags zurück
			dw1000_writeUInt16(DW1000_SYS_STATUS, 0, (unsigned long int)DW1000_SYS_STATUS_TX_ALL);

			// Lege den Systemstatus fest
			if (dw1000_devicemode == DW1000_TX_MODE) dw1000_devicemode = DW1000_RX_MODE;
		}
		else if (dw1000_devicemode == DW1000_TX_MODE)
		{
			// Lege den Systemstatus fest
			dw1000_devicemode = DW1000_IDLE_MODE;
		}

		// Rufe den Ereignishandler auf
		dw1000_handleTransmittedPackage();

		// Ereignis gehandhabt
		handled = 1;
	}

	// Prüfe, ob ein Frame komplett empfangen wurde
	else if (sysstatus & (DW1000_SYS_STATUS_RXFCG | DW1000_SYS_STATUS_RXDFR))
	{
		// Ein RX-Ereignis ist eingegangen
		dw1000_receivecounter++;

		// Prüfe, ob der LDE-Algorithms abgeschlossen wurde
		if (sysstatus & DW1000_SYS_STATUS_LDEDONE)
		{
			if (dw1000_doublebuffered && (sysstatus & DW1000_SYS_STATUS_RXOVRR))
			{
				dw1000_writeUInt8(DW1000_SYS_CTRL, DW1000_SYS_CTRL_HRBT_OFFSET, 1);
				dw1000_resetRx(1);
			}
			else
			{
				// Setze alle RX-Flags zurück
				dw1000_writeUInt32(DW1000_SYS_STATUS, 0, (unsigned long int)DW1000_SYS_STATUS_RX_ALL);

				// Gehe in den Idle-Modus, wenn der Doppelpuffer inaktiv ist
				if (!dw1000_doublebuffered) dw1000_devicemode = DW1000_IDLE_MODE;

				// Löse das Empfangsereignis aus
				dw1000_handleIncomingPackage();

				// Wenn der Doppelpuffer aktiv ist, swappe ihn, ansonsten starte den Empfang
				if (dw1000_doublebuffered && (dw1000_devicemode != DW1000_TX_MODE)) dw1000_writeUInt8(DW1000_SYS_CTRL, DW1000_SYS_CTRL_HRBT_OFFSET, 1);
				else if (dw1000_devicemode == DW1000_IDLE_MODE) dw1000_startReceive();

				// Ereignis gehandhabt
				handled = 1;
			}
		}

		// Fehler beim ausführen des LDE-Algorithmus
		else
		{
			dw1000_resetRx(1);
			dw1000_idle();
			dw1000_startReceive();
		}
	}

	// Fange den SFD-Timeout ab - der Empfang wird automatisch neu gestartet
	else if (sysstatus & (DW1000_SYS_STATUS_RXRFTO | DW1000_SYS_STATUS_RXPTO)) dw1000_resetRx(1);

	// Fange das Paket ab, dass ein fehlerhaftes Paket kennzeichnet
	else if (sysstatus & (DW1000_SYS_STATUS_RXFCE))
	{
		// Wenn der Doppelpuffer aktiv ist, swappe ihn, ansonsten starte den Empfang
		if (dw1000_doublebuffered) dw1000_writeUInt8(DW1000_SYS_CTRL, DW1000_SYS_CTRL_HRBT_OFFSET, 1);
		else if (!dw1000_receiverAutoReenable) dw1000_startReceive();
	}

	// Fange alle RX-Fehler ab -> sie führen zum Neustart des Moduls
	else if (sysstatus & DW1000_SYS_STATUS_RX_ERROR)
	{
		if (dw1000_doublebuffered)
		{
			if (sysstatus & DW1000_SYS_STATUS_RXOVRR)
			{
				if (dw1000_doublebuffered) dw1000_writeUInt8(DW1000_SYS_CTRL, DW1000_SYS_CTRL_HRBT_OFFSET, 1);
				dw1000_resetRx(1);
			}
			else if (!dw1000_receiverAutoReenable) dw1000_startReceive();
		}
		else if (!dw1000_receiverAutoReenable) dw1000_startReceive();
	}

	dw1000_irqactive = 0;
	return handled;
}

//! Überprüft den aktuellen Status des Systems, indem Registerinhalte mit dem gepufferten Status verglichen werden.
/*! \returns 1, wenn der gepufferte Status gültig ist, sonst 0 */
unsigned char dw1000_checkstate(void)
{
	unsigned char currentstate = dw1000_readUInt8(0x19, 2);
	switch(currentstate)
	{
		case 0x00: return dw1000_devicemode == DW1000_IDLE_MODE;
		case 0x02:
		case 0x04: return dw1000_devicemode == DW1000_TX_MODE;
		case 0x03:
		case 0x05: return dw1000_devicemode == DW1000_RX_MODE;
	}
	return 0;
}


//! Überprüft, ob der TX-Modus ggf. hängen geblieben ist
/*! \returns 0, wenn der gepufferte Status gültig ist, sonst 1 */
unsigned char dw1000_checktxstuck(void)
{
	if (dw1000_devicemode == DW1000_TX_MODE)
	{
		unsigned char currentstate = dw1000_readUInt8(0x19, 2);
		switch(currentstate)
		{
			case 0x02:
			case 0x04: return dw1000_devicemode != DW1000_TX_MODE;
		}
		return 0;
	}
	return 0;
}


//! Bringt den Chip in den idle modus
DW1000_OPTIMIZE DW1000_INLINE void dw1000_idle(void)
{
	// Deaktiviere die Interrupts
	dw1000_disableInterrupts();

	// Deaktiviere alle Interrupts
	dw1000_writeUInt32(DW1000_SYS_MASK, 0, 0);

	// Prüfe, ob ein Überlauf vorhanden ist
	unsigned char hasoverrun = dw1000_doublebuffered && (dw1000_readUInt8(DW1000_SYS_STATUS, 2) & 0x10);

	// Versetze das Modul in den IDLE Modus
	dw1000_writeUInt8(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_IDLE);

	// Das Modul befindet sich jetzt im idle-Modus
	dw1000_devicemode = DW1000_IDLE_MODE;

	if (hasoverrun)
	{
		// RX-Softreset
		dw1000_writeUInt8(DW1000_SYS_CTRL, DW1000_SYS_CTRL_HRBT_OFFSET, 1);
		dw1000_writeUInt8(DW1000_PMSC, (unsigned short)3, (unsigned char)0xe0);
		dw1000_writeUInt8(DW1000_PMSC, (unsigned short)3, (unsigned char)0xf0);
	}

	// Beende den aktuellen Puffer
	dw1000_writeUInt8(DW1000_SYS_CTRL, DW1000_SYS_CTRL_HRBT_OFFSET, 1);

	// Lese den aktuellen Systemstatus
	unsigned int sysstatus = dw1000_readUInt32(DW1000_SYS_STATUS, 0);

	// Prüfe, ob die Puffer synchronisiert werden müssen
	unsigned char buffer = (unsigned char)((sysstatus >> 24) & 0xff);
	if((buffer & 0x80) != ((buffer & 0x40) << 1)) dw1000_writeUInt8(DW1000_SYS_CTRL, DW1000_SYS_CTRL_HRBT_OFFSET , 0x01) ; // We need to swap RX buffer status reg (write one to toggle internally)

	// Eliminiere alle Sysstatus bits
	dw1000_writeUInt32(DW1000_SYS_STATUS, 0, DW1000_SYS_STATUS_RX_ALL | DW1000_SYS_STATUS_TX_ALL);

	// Maskiere alle relevanten Interrupts
	dw1000_writeUInt32(DW1000_SYS_MASK, 0, DW1000_SYS_MASK_ALL);

	// Aktiviere die Interrupts
	dw1000_enableInterrupts();
}

unsigned char dw1000_getDeviceMode()
{
	return dw1000_devicemode;
}

//! Versetzt das Modul in den Empfangsmodus
DW1000_OPTIMIZE DW1000_INLINE void dw1000_startReceive()
{
	// Need to make sure that the host/IC buffer pointers are aligned before starting RX
	if (dw1000_doublebuffered && (dw1000_readUInt8(DW1000_SYS_STATUS, 2) & 0x10)) dw1000_idle();
	else
	{
		// Prüfe, ob die Puffer synchronisiert werden müssen
		unsigned char buffer = dw1000_readUInt8(DW1000_SYS_STATUS, 3);
		if((buffer & 0x80) != ((buffer & 0x40) << 1))
		{
			dw1000_writeUInt8(DW1000_SYS_CTRL, DW1000_SYS_CTRL_HRBT_OFFSET , 0x01) ; // We need to swap RX buffer status reg (write one to toggle internally)
		}
	}

	// Lege den Systemstatus fest
	dw1000_devicemode = DW1000_RX_MODE;

	// Maskiere alle relevanten Interrupts
	dw1000_writeUInt32(DW1000_SYS_STATUS, 0, 0x3ffffffe);

	// Versetze das Modul in den RX-Modus
	dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_RX);
}


//! Aktiviert den Sniff-Modus mit einer 50:50 Abtastung
DW1000_OPTIMIZE void dw1000_enableSniffMode()
{
    /* Configure ON/OFF times and enable PLL2 on/off sequencing by SNIFF mode. */
    unsigned short sniff_reg = dw1000_sniffmodeoff;
    sniff_reg <<= 8;
    sniff_reg  |= dw1000_sniffmodeon;
    sniff_reg  &= 0x0000FF0FUL;
    dw1000_writeUInt16(DW1000_RX_SNIFF, DW1000_RX_SNIFF_OFFSET, sniff_reg);
    unsigned long int pmsc_reg = dw1000_readUInt32(DW1000_PMSC, DW1000_PMSC_CTRL0_OFFSET);
    pmsc_reg |= DW1000_PMSC_CTRL0_PLL2_SEQ_EN;
    dw1000_writeUInt32(DW1000_PMSC, DW1000_PMSC_CTRL0_OFFSET, pmsc_reg);
}


//! Deaktiviert den Sniff-Modus
DW1000_OPTIMIZE void dw1000_disableSniffMode()
{
	/* Clear ON/OFF times and disable PLL2 on/off sequencing by SNIFF mode. */
	dw1000_writeUInt16(DW1000_RX_SNIFF, DW1000_RX_SNIFF_OFFSET, 0x0000);
	unsigned long int pmsc_reg = dw1000_readUInt32(DW1000_PMSC, DW1000_PMSC_CTRL0_OFFSET);
	pmsc_reg &= ~DW1000_PMSC_CTRL0_PLL2_SEQ_EN;
	dw1000_writeUInt32(DW1000_PMSC, DW1000_PMSC_CTRL0_OFFSET, pmsc_reg);
}


//! Initialisiert das Senden eines Pakets
DW1000_OPTIMIZE DW1000_INLINE unsigned char dw1000_initializeTransmit()
{
	if (dw1000_devicemode != DW1000_TX_MODE)
	{
		// Versetze das Modul in den IDLE-Modus
		dw1000_idle();

		// Starte den TX-Modus
		dw1000_devicemode = DW1000_TX_MODE;

		return 1;
	}

	return 0;
}


//! Sendet das in den Sendepuffer geschriebene Paket
/*! Achtung! Erforderlich ist der Aufruf von dw1000_initializeTransmit() mit anschließendem Schreiben des Sendepuffers bevor dw1000_startTransmit aufgerufen werden kann.
 *  \param timestamp Ist dieser Wert ungleich 0, dann wird das Paket gesendet, sobald die Systemzeit dem timestamp entspricht */
DW1000_OPTIMIZE DW1000_INLINE unsigned char dw1000_startTransmit(unsigned long long int timestamp)
{
	if (dw1000_devicemode == DW1000_TX_MODE)
	{
		// Prüfe, ob ein Zeitstempel angegeben wurde
		if (timestamp)
		{
			// Set the delay
			// NOTE: The low-order 9 bits of the delayed Transmit value programmed into Register file: 0x0A – Delayed
			// Send or Receive Time are ignored giving a time resolution of 8 ns, or more precisely 4 ÷ (499.2×106). To
			// calculate the time of transmission of the RMARKER at the antenna, the low 9 bits of the delayed TX time
			// should be zeroed before adding the TX antenna delay
			unsigned char delayvalue[] =
			{
				(timestamp & 0),
				((timestamp >> 8) & 0xfe),
				((timestamp >> 16) & 0xff),
				((timestamp >> 24) & 0xff),
				((timestamp >> 32) & 0xff)
			};

			// Write the delay to the delay register
			dw1000_write(DW1000_DX_TIME, 5, delayvalue, 0);

			// Start a delayed transmission
			if (dw1000_receiveAfterTX)
			{
				if (dw1000_frameCheck) dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_TX_DELAYED | DW1000_SYS_CTRL_WAIT4RSP);
				else dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_TX_DELAYED | DW1000_SYS_CTRL_SFCST | DW1000_SYS_CTRL_WAIT4RSP);
			}
			else
			{
				if (dw1000_frameCheck) dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_TX_DELAYED);
				else dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_TX_DELAYED | DW1000_SYS_CTRL_SFCST);
			}
		}
		else
		{
			// Start immediate transmission
			if (dw1000_receiveAfterTX)
			{
				if (dw1000_frameCheck) dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_TX | DW1000_SYS_CTRL_WAIT4RSP);
				else dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_TX | DW1000_SYS_CTRL_SFCST | DW1000_SYS_CTRL_WAIT4RSP);
			}
			else
			{
				if (dw1000_frameCheck) dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_TX);
				else dw1000_writeUInt16(DW1000_SYS_CTRL, 0, DW1000_SYS_CTRL_TX | DW1000_SYS_CTRL_SFCST);
			}
		}

		return 1;
	}

	return 0;
}


//! Führt einen Hardwarereset durch
/*! Achtung! Anschließend muss das System neu konfiguriert werden! */
DW1000_OPTIMIZE void dw1000_reset()
{
	dw1000_resetResetPin();								// RST auf LOW
	while (dw1000_readResetPin() == 0) { asm(""); };	// Warte, bis die PLL des Moduls bereit ist
	while((dw1000_readUInt8(DW1000_SYS_STATUS, 0) & 0x02) == 0) asm("");
	dw1000_writeUInt8(DW1000_SYS_STATUS, 0, 0x02);
	dw1000_idle();										// Versetze das Modul in den IDLE-Modus
}


//! Setzt den Empfangspfad zurück
/*! \param startreceiving Wird auf 1 gesetzt, wenn das Modul nach dem Reset in den Empfangsmodus versetzt werden soll. Sonst 0 */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_resetRx(unsigned char startreceiving)
{
	// Versetze das Modul in den IDLE-Modus
	if (dw1000_devicemode != DW1000_IDLE_MODE) dw1000_idle();

	// RX-Softreset
	dw1000_writeUInt8(DW1000_PMSC, (unsigned short)3, (unsigned char)0xe0);
	dw1000_writeUInt8(DW1000_PMSC, (unsigned short)3, (unsigned char)0xf0);

	// Starte den Empfänger neu
	if (startreceiving) dw1000_startReceive();
}

//! Gibt den Zeitstempel aus, an dem der Transfer abgeschlossen wurde (126MHz Basis)
/*! \returns Zeitpunkt, an dem der Anfang des PHY-Headers die Antenne verlassen hat */
DW1000_OPTIMIZE DW1000_INLINE unsigned long long int dw1000_getRawTransmitTimestamp()
{
	unsigned char timeBytes[5];
	dw1000_read(DW1000_TX_TIME, 5, timeBytes, 5);
	return ((unsigned long long int)timeBytes[0]) |
		   ((unsigned long long int)timeBytes[1] << 8) |
		   ((unsigned long long int)timeBytes[2] << 16) |
		   ((unsigned long long int)timeBytes[3] << 24) |
		   ((unsigned long long int)timeBytes[4] << 32);
}

//! Gibt den Zeitstempel aus, an dem der Transfer abgeschlossen wurde
/*! \returns Zeitpunkt, an dem der Anfang des PHY-Headers die Antenne verlassen hat */
DW1000_OPTIMIZE DW1000_INLINE unsigned long long int dw1000_getTransmitTimestamp()
{
	unsigned char timeBytes[5];
	dw1000_read(DW1000_TX_TIME, 5, timeBytes, 0);
	return ((unsigned long long int)timeBytes[0]) |
		   ((unsigned long long int)timeBytes[1] << 8) |
		   ((unsigned long long int)timeBytes[2] << 16) |
		   ((unsigned long long int)timeBytes[3] << 24) |
		   ((unsigned long long int)timeBytes[4] << 32);
}

//! Gibt den Zeitstempel aus, an dem das letzte Paket empfangen wurde (126MHz Basis)
/*! \returns Zeitpunkt, an dem der Anfang des PHY-Headers die Antenne erreicht hat */
DW1000_OPTIMIZE DW1000_INLINE unsigned long long int dw1000_getRawReceiveTimestamp()
{
	unsigned char timeBytes[5];
	dw1000_read(DW1000_RX_TIME, 5, timeBytes, 9);
	return ((unsigned long long int)timeBytes[0]) |
		   ((unsigned long long int)timeBytes[1] << 8) |
		   ((unsigned long long int)timeBytes[2] << 16) |
		   ((unsigned long long int)timeBytes[3] << 24) |
		   ((unsigned long long int)timeBytes[4] << 32);
}

//! Gibt den Zeitstempel aus, an dem das letzte Paket empfangen wurde
/*! \returns Zeitpunkt, an dem der Anfang des PHY-Headers die Antenne erreicht hat */
DW1000_OPTIMIZE DW1000_INLINE unsigned long long int dw1000_getReceiveTimestamp()
{
	unsigned char timeBytes[5];
	dw1000_read(DW1000_RX_TIME, 5, timeBytes, 0);
	return ((unsigned long long int)timeBytes[0]) |
		   ((unsigned long long int)timeBytes[1] << 8) |
		   ((unsigned long long int)timeBytes[2] << 16) |
		   ((unsigned long long int)timeBytes[3] << 24) |
		   ((unsigned long long int)timeBytes[4] << 32);
}

//! Liest die Systemzeit aus dem Modul aus
/*! \returns Aktuelle Systemzeit in Systemeinheiten (ca. 15ps) */
DW1000_OPTIMIZE DW1000_INLINE unsigned long long int dw1000_getSystemTimestamp()
{
	unsigned char timeBytes[5];
	dw1000_read(DW1000_SYS_TIME, 5, timeBytes, 0);
	return ((unsigned long long int)timeBytes[0]) |
		   ((unsigned long long int)timeBytes[1] << 8) |
		   ((unsigned long long int)timeBytes[2] << 16) |
		   ((unsigned long long int)timeBytes[3] << 24) |
		   ((unsigned long long int)timeBytes[4] << 32);
}



//! Diese Methode gibt 1 aus, wenn aktuell übermittelt wird
/*! \returns 1 wenn aktuell gesendet wird, sonst 0 */
DW1000_OPTIMIZE DW1000_INLINE unsigned char dw1000_isInTransmitState() { return dw1000_devicemode == DW1000_TX_MODE; }

//! Diese Methode gibt 1 aus, wenn aktuell empfangen wird
/*! \param haspackage 1: Die Antwort ist nur dann positiv, wenn eine Preambel detektiert wurde; 0: Die Antwort ist positiv, wenn sich das Modul im Empfangsmodus befindet
 	\returns 1 wenn aktuell gesendet wird, sonst 0 */
unsigned char dw1000_isInReceiveState() { return dw1000_devicemode == DW1000_RX_MODE; }


//! Diese Methode gibt den Empfangszähler aus
/*! Der Empfangszähler wird immer dann erhöht, wenn ein RX-Interrupt ausgelöst wurde. Er kann verwendet werden, um zu ermitteln ob seit der letzten Abfrage Pakete eingegangen sind.
 *  \returns Empfangszähler */
unsigned long int dw1000_getReceiveCounter() { return dw1000_receivecounter; }


//! Gibt die Empfangsleistung für das letzte Paket in dBm aus
/*! \returns Empfangsleistung des letzten Pakets in dBm */
DW1000_OPTIMIZE DW1000_INLINE double dw1000_getReceivePower()
{
	volatile double cir_pwr = (double)dw1000_readUInt16(DW1000_RX_FQUAL, 6) * 131072.0;
	volatile double sqrN = pow((double)(dw1000_readUInt16(DW1000_RX_FINFO, 2) >> 4),2);
	if(dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_16MHZ) return 10.0 * log10(cir_pwr / sqrN) - 115.72;
	else return 10.0 * log10(cir_pwr / sqrN) - 121.74;
}

//! Gibt die Empfangsleistung der ersten drei Flanken des PHY-Headers (und das Rauschen auf dem Kanal) aus
/*! Anhand dieser Flanken berechnet sich der Zeitstempel für eingehende Pakete
 *  \returns RAUSCHEN (Bits 18-32); EMPFANGSLEISTUNG (Bits 0-17) */
DW1000_OPTIMIZE DW1000_INLINE unsigned int dw1000_getAccumulatedFirstPathPower()
{
	unsigned short rx_fqual[3];
	unsigned int acccount = (double)(dw1000_readUInt16(DW1000_RX_FINFO, 2) >> 4);
	dw1000_read(DW1000_RX_FQUAL, 6, (unsigned char*)rx_fqual, 0);
	unsigned int f1 = (unsigned int)dw1000_readUInt16(DW1000_RX_TIME, DW1000_FP_AMPL1_OFFSET);
	unsigned int f2 = (unsigned int)rx_fqual[1];
	unsigned int f3 = (unsigned int)rx_fqual[2];
	return (f1 + f2 + f3) / acccount;
}


//! Gibt die Empfangsleistung der ersten drei Flanken des PHY-Headers (und das Rauschen auf dem Kanal) aus
/*! Anhand dieser Flanken berechnet sich der Zeitstempel für eingehende Pakete
 *  \returns RAUSCHEN (Bits 18-32); EMPFANGSLEISTUNG (Bits 0-17) */
DW1000_OPTIMIZE DW1000_INLINE unsigned int dw1000_getFirstPathPower()
{
	unsigned short rx_fqual[3];
	dw1000_read(DW1000_RX_FQUAL, 6, (unsigned char*)rx_fqual, 0);
	unsigned int noise = (unsigned int)rx_fqual[0];
	unsigned int f1 = (unsigned int)dw1000_readUInt16(DW1000_RX_TIME, DW1000_FP_AMPL1_OFFSET);
	unsigned int f2 = (unsigned int)rx_fqual[1];
	unsigned int f3 = (unsigned int)rx_fqual[2];
	return ((f1 + f2 + f3) & 0b00000000000000111111111111111111) | ((noise & 0b11111111111111) << 18);
}


//! LDE Threshold report
/*! \returns The threshold is calculated based on an estimate of the noise made during the LDE algorithm’s analysis of the accumulator data. */
DW1000_OPTIMIZE DW1000_INLINE unsigned short dw1000_getLDEThreshold() { return dw1000_readUInt16(DW1000_LDE_IF, DW1000_LDE_THRESH_OFFSET); }

//! Lädt den RX Time Tracking Offset in ppm
/*! \returns Verzögerung zwischen diesem Knoten und dem Sender des letzten Pakets */
DW1000_OPTIMIZE DW1000_INLINE double dw1000_getTimeTrackingOffset()
{
	unsigned int ttcki_unsigned = dw1000_readUInt32(DW1000_RX_TTCKI, 0);
	signed int ttcko_signed = dw1000_readUInt32(DW1000_RX_TTCKO, 0) & 0b1111111111111111111;
	if (ttcko_signed & 0b1000000000000000000) ttcko_signed = 0b11111111111111000000000000000000 | ttcko_signed;
	//return ttcko_signed * (1000000.0f / ttcki_unsigned);
	return ((double)ttcko_signed / (double)ttcki_unsigned) * 1000000.0;
}


//! Diese Methode gibt an, ob sich der Programmstack aktuell in einem Interrupt befindet
DW1000_OPTIMIZE DW1000_INLINE unsigned char dw1000_isIrqActive() { return dw1000_irqactive; }


//! Diese Funktion aktiviert die Temperatur- und Spannungsmessung
DW1000_OPTIMIZE void dw1000_enableTemperatureAndVoltageMeasurement()
{
	dw1000_writeUInt8(DW1000_RF_CONF, 0x11, 0x80);		// Enable TLD Bias
	dw1000_writeUInt8(DW1000_RF_CONF, 0x12, 0x0A);		// Enable TLD Bias and ADC Bias
	dw1000_writeUInt8(DW1000_RF_CONF, 0x12, 0x0f);		// Enable Outputs (only after Biases are up and running)
	dw1000_writeUInt8(DW1000_TX_CAL, 0, 0);
	dw1000_writeUInt8(DW1000_TX_CAL, 0, 1);

	// Lese die Referenztemperatur und Referenzspannung
	dw1000_reftemperature = dw1000_readOTPUInt8(0x09);
	dw1000_refvoltage = dw1000_readOTPUInt8(0x08);
}

//! Diese Funktion gibt die Temperatur in °C aus
DW1000_OPTIMIZE char dw1000_getTemperature()
{
	// Lesen der Temperatur nur alle 1000ms erlauben
	unsigned long int currenttick = dw1000_getTickCounter();
	if (currenttick < dw1000_lasttemperaturemeasure) dw1000_lasttemperaturemeasure = currenttick;
	else if (currenttick - dw1000_lasttemperaturemeasure >= 1000)
	{
		dw1000_lasttemperaturemeasure = currenttick;
		unsigned char temperature = dw1000_readUInt8(DW1000_TX_CAL, 0x04);
		dw1000_writeUInt8(DW1000_TX_CAL, 0, 0);
		dw1000_writeUInt8(DW1000_TX_CAL, 0, 1);
		if (dw1000_lasttemperature != temperature)
		{
			int newtemperature = (int)round((((int)temperature - (int)dw1000_reftemperature) * 1.14 + 23));
			if (newtemperature >= -40 && newtemperature <= 80) dw1000_temperature = (char)newtemperature;
		}
	}
	return dw1000_temperature;
}

//! Diese Funktion gibt die Spannung in V aus
DW1000_OPTIMIZE int dw1000_getVoltage()
{
	// TODO: implementieren
	return 0;
}


/**
 * Estimate the transmission time of a frame in seconds
 * dwt_config_t   dwt_config  Configuration struct of the DW1000
 * uint16_t       framelength Framelength including the 2-Byte CRC
 * bool           only_rmarker Option to compute only the time to the RMARKER (SHR time)
 */
DW1000_OPTIMIZE float dw1000_estimatePackageTime(unsigned char datarate, unsigned char pulsefrequency, unsigned char preamblelength, unsigned short framelength, unsigned char only_rmarker )
{

  long int tx_time = 0;
  size_t sym_timing_ind = 0;
  unsigned short shr_len = 0;

  const unsigned short DATA_BLOCK_SIZE  = 330;
  const unsigned short REED_SOLOM_BITS  = 48;

  // Symbol timing LUT
  const size_t SYM_TIM_16MHZ = 0;
  const size_t SYM_TIM_64MHZ = 9;
  const size_t SYM_TIM_110K  = 0;
  const size_t SYM_TIM_850K  = 3;
  const size_t SYM_TIM_6M8   = 6;
  const size_t SYM_TIM_SHR   = 0;
  const size_t SYM_TIM_PHR   = 1;
  const size_t SYM_TIM_DAT   = 2;

  const static unsigned short SYM_TIM_LUT[] = {
    // 16 Mhz PRF
    994, 8206, 8206,  // 0.11 Mbps
    994, 1026, 1026,  // 0.85 Mbps
    994, 1026, 129,   // 6.81 Mbps
    // 64 Mhz PRF
    1018, 8206, 8206, // 0.11 Mbps
    1018, 1026, 1026, // 0.85 Mbps
    1018, 1026, 129   // 6.81 Mbps
  };

  // Find the PHR
  switch(pulsefrequency)
  {
    case DW1000_TX_PULSE_FREQ_16MHZ:  sym_timing_ind = SYM_TIM_16MHZ; break;
    case DW1000_TX_PULSE_FREQ_64MHZ:  sym_timing_ind = SYM_TIM_64MHZ; break;
  }

  // Find the preamble length
  switch(preamblelength)
  {
    case DW1000_TX_PREAMBLE_LEN_64:   shr_len = 64;    break;
    case DW1000_TX_PREAMBLE_LEN_128:  shr_len = 128;   break;
    case DW1000_TX_PREAMBLE_LEN_256:  shr_len = 256;   break;
    case DW1000_TX_PREAMBLE_LEN_512:  shr_len = 512;   break;
    case DW1000_TX_PREAMBLE_LEN_1024: shr_len = 1024;  break;
    case DW1000_TX_PREAMBLE_LEN_1536: shr_len = 1536;  break;
    case DW1000_TX_PREAMBLE_LEN_2048: shr_len = 2048;  break;
    case DW1000_TX_PREAMBLE_LEN_4096: shr_len = 4096;  break;
  }

  // Find the datarate
  switch(datarate)
  {
    case DW1000_TRX_RATE_110KBPS:
      sym_timing_ind  += SYM_TIM_110K;
      shr_len         += 64;  // SFD 64 symbols
      break;
    case DW1000_TRX_RATE_850KBPS:
      sym_timing_ind  += SYM_TIM_850K;
      shr_len         += 8;   // SFD 8 symbols
      break;
    case DW1000_TRX_RATE_6800KBPS:
      sym_timing_ind  += SYM_TIM_6M8;
      shr_len         += 8;   // SFD 8 symbols
      break;
  }

  // Add the SHR time
  tx_time = shr_len * SYM_TIM_LUT[sym_timing_ind + SYM_TIM_SHR];

  // If not only RMARKER, calculate PHR and data
  if(!only_rmarker)
  {

    // Add the PHR time (21 bits)
    tx_time  += 21 * SYM_TIM_LUT[ sym_timing_ind + SYM_TIM_PHR ];

    // Bytes to bits
    framelength *= 8;

    // Add Reed-Solomon parity bits
    framelength += REED_SOLOM_BITS * ( framelength + DATA_BLOCK_SIZE - 1 ) / DATA_BLOCK_SIZE;

    // Add the DAT time
    tx_time += framelength * SYM_TIM_LUT[ sym_timing_ind + SYM_TIM_DAT ];

  }

  // Return float seconds
  return (1.0e-9f) * tx_time;

}

DW1000_OPTIMIZE void dw1000_preparesleep()
{
	//dw1000_writeUInt16(DW1000_AON, DW1000_AON_WCFG_OFFSET, DW1000_AON_WCFG_ONW_RADC | DW1000_AON_WCFG_PRES_SLEEP | DW1000_AON_WCFG_ONW_LLDO | DW1000_AON_WCFG_ONW_LLDE | DW1000_AON_WCFG_ONW_LDC);
	//dw1000_writeUInt8(DW1000_AON, DW1000_AON_CFG0_OFFSET, DW1000_WAKE_WK | DW1000_SLP_EN);
}

DW1000_OPTIMIZE void dw1000_entersleep()
{
	dw1000_idle();
	dw1000_disableInterrupts();
	dw1000_writeUInt16(DW1000_AON, DW1000_AON_CTRL_OFFSET, 0x00);
	dw1000_writeUInt16(DW1000_AON, DW1000_AON_WCFG_OFFSET, /*DW1000_AON_WCFG_ONW_RADC |*/ DW1000_AON_WCFG_PRES_SLEEP | DW1000_AON_WCFG_ONW_LLDO | DW1000_AON_WCFG_ONW_LLDE | DW1000_AON_WCFG_ONW_LDC);
	dw1000_writeUInt32(DW1000_AON, DW1000_AON_CFG0_OFFSET, DW1000_WAKE_WK | DW1000_SLP_EN);
	dw1000_writeUInt16(DW1000_AON, DW1000_AON_CFG1_OFFSET, 0);
	dw1000_writeUInt16(DW1000_AON, DW1000_AON_CTRL_OFFSET, 0x02);
}

DW1000_OPTIMIZE void dw1000_wakefromsleep()
{
	// Setze die WAKE-Leitung
	dw1000_setWakePin();

	// Warte, bis sich die PLL stabilisiert hat
	while (dw1000_readResetPin() == 0) { asm(""); };

	// Setze die WAKE-Leitung zurück
	dw1000_resetWakePin();

	// Setze die clock-Leitung zurück
	dw1000_reduceClock();
	if (dw1000_getDeviceId() != 0xdeca0130)
	{
		int i;
		for (i=0; i<1000; i++)
		{
			if (dw1000_getDeviceId() == 0xdeca0130) break;
		}
	}

	// Lege die Antennendelaywerte fest
	dw1000_writeUInt16(DW1000_LDE_IF, DW1000_LDE_RXANTD_OFFSET, (unsigned short)dw1000_antennadelay_rx) ;
	dw1000_writeUInt16(DW1000_TX_ANTD, 0, (unsigned short)dw1000_antennadelay_tx) ;

	dw1000_raiseClock();

	// Gehe in den IDLE-Modus
	dw1000_idle();
}


