#include "dw1000_driver.h"
#include "dw1000_user.h"


unsigned char dw1000_extendedFrameLength = 0;			//!< Legt fest, ob die erweiterte Framelänge verwendet werden soll (1: 1023Bytes) oder nicht (0: 127Bytes)
unsigned char dw1000_pacSize = 0;						//!< Legt fest, wie viele Symbole der Präambel beim Empfangen detektiert werden mÃ¼ssen. PAC-Size hängt von der erwarteten Präambellänge ab - sollte stets kÃ¼rter sein
unsigned char dw1000_pulseFrequency = 0;				//!< Legt die Pulsfrequenz fest (16 oder 64MHz)
unsigned char dw1000_dataRate = 0;						//!< Legt die Datenrate fest (110kbps, 850kbps, 6.4Mbps)
unsigned char dw1000_preambleLength = 0;				//!< Legt die Präambellänge fest
unsigned char dw1000_preambleCode = 0;					//!< Legt die Präambelcodierung fest
unsigned char dw1000_channel = 0;						//!< Legt den Funkkanal fest
unsigned char dw1000_frameCheck = 0;					//!< Legt fest, ob die FrameÃ¼berprÃ¼fungsfunktionalität verwendet werden soll (1) oder nicht (0)
unsigned char dw1000_smartPower = 0;					//!< Legt fest, ob die Smartpowerfunktionalität (mehr Leistung, wenn wenig Pakete gesendet werden) verwendet werden soll (1) oder nicht (0)
unsigned char dw1000_doublebuffered = 0;				//!< Legt fest, ob der Doppelpuffer verwendet werden soll (1) oder nicht (0)
unsigned char dw1000_framefilter = 0;					//!< Legt fest, welche Framefilterungsmechanismen verwendet werden sollen
unsigned char dw1000_receiverAutoReenable = 0;			//!< Legt fest, ob sich der Empfänger bei einem Fehler automatisch wieder in den RX-Modus setzen soll (1) oder nicht (0)
unsigned char dw1000_receiveAfterTX = 0;				//!< Legt fest, ob direkt nach dem Senden wieder Empfangen werden soll
unsigned char dw1000_devicemode = 0;					//!< Speichert den aktuellen Zustand des Moduls (RX,TX,IDLE)
unsigned char dw1000_irqactive = 0;						//!< Diese Variable wird auf 1 gesetzt, wenn der SPI-Zugriff aus einem Interrupt heraus geschieht
unsigned char dw1000_isinitializeing = 0;				//!< Legt fest, ob das Modul aktuell initialisiert wird
unsigned char dw1000_clockoffset = 0;
unsigned char dw1000_ldoparameter = DW1000_OTP_SF_SEL_DEFAULT;

unsigned short dw1000_antennadelay_rx = 0;				//!< Legt fest, welche zeitliche Verzögerung zwischen Aus-/Eingang am Funkchip und Aus-/Eingang an der Antenne vorliegt
unsigned short dw1000_antennadelay_tx = 0;				//!< Legt fest, welche zeitliche Verzögerung zwischen Aus-/Eingang am Funkchip und Aus-/Eingang an der Antenne vorliegt

unsigned long int dw1000_sys_cfg;						//!< Abbild des aktuellen SYS_CFG-Registerinhalts (Systemkonfiguration)
unsigned long int dw1000_tx_fctrl;						//!< Abbild des aktuellen TX_FCTRL-Registerinhalts (Transmit Frame Control)
unsigned long int dw1000_chan_ctrl;						//!< Abbild des aktuellen CHAN_CTRL-Registerinhalts (Channel Control Register)

unsigned char dw1000_sniffmodeoff = 0;					//!< Gibt an, wie lange das Modul im Sniff-Modus auf eine Preambel hören soll
unsigned char dw1000_sniffmodeon = 0;					//!< Gibt an, wie lange das Modul zwischen zwei Preamblesuchvorgängen schlafen soll (in Âµs)




//! Initialisiert das Modul
/*! Diese Funktion richtet das Modul ein und bringt es in einer Standardkonfiguration.<br>
 *  Der Aufruf dieser Funktion ist vor der Verwendung des Moduls erforderlich.<br>
 *  Die individuelle Konfiguration hat im Anschluss zu erfolgen. */
DW1000_OPTIMIZE void dw1000_initialize(void)
{
	// Initialisiere die SPI
	dw1000_initializehardware();

	// Falls das Modul schläft: wecke es auf
	dw1000_wakefromsleep();

	// Lege die Startkonfiguration fest
	dw1000_sys_cfg 				= DW1000_SYS_CFG_HIRQ_POL;
	dw1000_tx_fctrl 			= 0;
	dw1000_chan_ctrl 			= 0;
	dw1000_pulseFrequency 		= DW1000_TX_PULSE_FREQ_64MHZ;
	dw1000_dataRate 			= DW1000_TRX_RATE_110KBPS;
	dw1000_preambleLength 		= DW1000_TX_PREAMBLE_LEN_2048;
	dw1000_preambleCode 		= DW1000_PREAMBLE_CODE_16MHZ_4;
	dw1000_channel 				= DW1000_CHANNEL_1;
	dw1000_extendedFrameLength 	= DW1000_FRAME_LENGTH_NORMAL;	// Keine erhöhte Framegröße (max. 127Byte)
	dw1000_doublebuffered		= 0;							// Deaktiviere Double Buffering
	dw1000_smartPower			= 0;							// Kein Smart-Power
	dw1000_receiverAutoReenable = 0;							// Nach Fehlern automatisch wieder in RX Modus gehen

}


//! Leitet einen Konfigurationsprozess ein
/*! Damit nicht jede Änderung sofort Wirkung zeigt, fÃ¼hrt der Aufruf der Funktion dw1000_newConfiguration()
 *  zu einer Blockade der Änderungen solange, bis dw1000_commitConfiguration() aufgerufen wurde
 *  \param reset Setze alle Einstellung auf ihren Standardwert zurÃ¼ck */
DW1000_OPTIMIZE void dw1000_newConfiguration(unsigned char reset)
{
	// Device is initializing
	dw1000_isinitializeing = (reset) ? 2 : 1;

	// Lege die Startkonfiguration fest, wenn gewÃ¼nscht
	if (reset)
	{
		dw1000_sys_cfg 				= DW1000_SYS_CFG_HIRQ_POL;
		dw1000_tx_fctrl 			= 0;
		dw1000_chan_ctrl 			= 0;
		dw1000_pulseFrequency 		= DW1000_TX_PULSE_FREQ_16MHZ;
		dw1000_dataRate 			= DW1000_TRX_RATE_110KBPS;
		dw1000_preambleLength 		= DW1000_TX_PREAMBLE_LEN_2048;
		dw1000_preambleCode 		= DW1000_PREAMBLE_CODE_16MHZ_4;
		dw1000_channel 				= DW1000_CHANNEL_1;
		dw1000_framefilter			= 0;
		dw1000_extendedFrameLength 	= DW1000_FRAME_LENGTH_NORMAL;	// Keine erhöhte Framegröße (max. 127Byte)
		dw1000_doublebuffered		= 0;							// Deaktiviere Double Buffering
		dw1000_smartPower			= 0;							// Kein Smart-Power
		dw1000_receiverAutoReenable = 0;							// Nach Fehlern automatisch wieder in RX Modus gehen
		dw1000_ldoparameter 		= DW1000_OTP_SF_SEL_DEFAULT;
	}
}


//! Schließt einen Konfigurationsprozess ab und schreibt alle Einstellungen auf das Modul
/*! Damit nicht jede Änderung sofort Wirkung zeigt, fÃ¼hrt der Aufruf der Funktion dw1000_newConfiguration()
 *  zu einer Blockade der Änderungen solange, bis dw1000_commitConfiguration() aufgerufen wurde */
DW1000_OPTIMIZE void dw1000_commitConfiguration()
{
	// Variablen vorbereiten
	unsigned char 	lengthOfSFD = 0;		// Die Länge der SFD-Sequenz
	unsigned short 	timeoutOfSFD = 0;		// Der Timeout, der sich aus der SFD-Länge und der Preambellänge ergibt

	// Reduziere die Taktung
	dw1000_reduceClock();

	// Lese die Gerätekennung
	unsigned int deviceid = dw1000_getDeviceId();
	if (deviceid != DW1000_DEVICEID) dw1000_reset();
	else if (dw1000_isinitializeing == 2)
	{
		// FÃ¼hre ein Softreset durch
		dw1000_setClock(DW1000_CLOCK_ENABLE_OTP | DW1000_CLOCK_ENABLE_LDU | DW1000_CLOCK_SYS_XTI | DW1000_CLOCK_RX_OFF | DW1000_CLOCK_TX_OFF);
	    dw1000_writeUInt8(DW1000_EXT_SYNC, DW1000_EC_CTRL_OFFSET, DW1000_EC_CTRL_PLLLCK);
	    dw1000_writeUInt8(DW1000_SYS_STATUS, 0, 0x02);
		dw1000_writeUInt16(DW1000_PMSC, DW1000_PMSC_CTRL1_OFFSET, DW1000_PMSC_CTRL1_PKTSEQ_DISABLE);	// Disable PMSC ctrl of RF and RX clk blocks
		dw1000_writeUInt16(DW1000_AON, DW1000_AON_WCFG_OFFSET, 0x00);									// Clear any AON auto download bits (as reset will trigger AON download)
		dw1000_writeUInt8(DW1000_AON, DW1000_AON_CFG0_OFFSET, 0x00);									// Clear the wake-up configuration
		dw1000_writeUInt8(DW1000_AON, DW1000_AON_CTRL_OFFSET, 0x00); 									// Clear the register
		dw1000_writeUInt8(DW1000_AON, DW1000_AON_CTRL_OFFSET, 0x02);									// Upload the new configuration
		dw1000_writeUInt8(DW1000_PMSC, DW1000_PMSC_CTRL0_SOFTRESET_OFFSET, 0x00);						// Reset HIF, TX, RX and PMSC
		while((dw1000_readUInt8(DW1000_SYS_STATUS, 0) & 0x02) == 0) asm("");
		dw1000_writeUInt8(DW1000_SYS_STATUS, 0, 0x02);
		dw1000_writeUInt8(DW1000_PMSC, DW1000_PMSC_CTRL0_SOFTRESET_OFFSET, 0xF0);						// Clear reset
		dw1000_idle();
	}

	// Übernehme die folgende Taktung
	//  - Takt für den OTP-Speicher
	//  - Takt für den LDU-Bereich (undokumentiert)
	//  - Systemtakt auf XTI
	//  - Empfangstakt aus
	//  - Sendetakt aus
	dw1000_setClock(DW1000_CLOCK_ENABLE_OTP | DW1000_CLOCK_SYS_XTI | DW1000_CLOCK_RX_AUTO | DW1000_CLOCK_TX_AUTO);

	// Configure the CPLL lock detect
    dw1000_writeUInt8(DW1000_EXT_SYNC, DW1000_EC_CTRL_OFFSET, DW1000_EC_CTRL_PLLLCK);

	// Lade alle Grundkonfigurationen in die dafür vorgesehenen Register
    dw1000_loadLDOCalibrationData();
    dw1000_loadXTalTrimming();

    dw1000_setClock(DW1000_CLOCK_ENABLE_LDU);
    dw1000_loadLeadingEdgeAlgorithm();
	dw1000_delay_mSek(5);
	dw1000_setClock(DW1000_CLOCK_DISABLE_LDU);

	// Übernehme die folgende Taktung
	//  - Systemtakt auf "PLL, wenn bereit"
	//  - Empfangstakt "PLL, wenn notwendig"
	//  - Sendetakt "PLL, wenn notwendig"
	dw1000_setClock(DW1000_CLOCK_SYS_AUTO | DW1000_CLOCK_RX_AUTO | DW1000_CLOCK_TX_AUTO);

	// Warte auf die PLL
	dw1000_delay_mSek(5);	// TODO: Warte auf LOCK-BIT

	// Passe die SPI-Geschwindigkeiten an
	dw1000_raiseClock();

	// PrÃ¼fe, ob die Gerätekennung nach dem Aufwecken korrekt gelesen wurde
	if (dw1000_getDeviceId() != DW1000_DEVICEID)
	{
		while(1) { asm(""); }
	}

	// Deaktiviere Schlafmodus
	dw1000_writeUInt8(DW1000_AON, DW1000_AON_CFG1_OFFSET, 0x00);									// Clear the wake-up configuration

	// Lege die Konfigurationen fest, die vom Kanal abhängen
    // Tabelle 34, 35, 37, 40 und 41
	switch(dw1000_channel)
	{
		case DW1000_CHANNEL_2:

			// Channel | Centre          | Bandwidth | Preamble Codes | Preamble Codes
			// number  | frequency (MHz) | (MHz)     | (16 MHz PRF)   | (64 MHz PRF)
			// --------+-----------------+-----------+----------------+----------------
			// 1       | 3494.4          | 499.2     | 1, 2           | 9, 10, 11, 12
			// 2       | 3993.6          | 499.2     | 3, 4           | 9, 10, 11, 12   <---------- Kanal 2
			// 3       | 4492.8          | 499.2     | 5, 6           | 9, 10, 11, 12
			// 4       | 3993.6          | 1331.2    | 7, 8           | 17, 18, 19, 20
			// 5       | 6489.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 7       | 6489.6          | 1081.6    | 7, 8           | 17, 18, 19, 20
			switch(dw1000_preambleCode)
			{
				case DW1000_PREAMBLE_CODE_16MHZ_3:
				case DW1000_PREAMBLE_CODE_16MHZ_4:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_9;
					break;
				case DW1000_PREAMBLE_CODE_64MHZ_9:
				case DW1000_PREAMBLE_CODE_64MHZ_10:
				case DW1000_PREAMBLE_CODE_64MHZ_11:
				case DW1000_PREAMBLE_CODE_64MHZ_12:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_64MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_3;
					break;
				default:
					if (dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_3;
					else dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_9;
					break;
			}

			// Operating | 32-bit value to program to
			// Channel   | Sub-Register 0x2B:07 â€“ FS_PLLCFG
			// ----------+----------------------------------
			// 1         | 0x09000407
			// 2,4       | 0x08400508    <---------- Kanal 2
			// 3         | 0x08401009
			// 5, 7      | 0x0800041D
			dw1000_writeUInt32(DW1000_FS_CTRL, DW1000_FS_PLLCFG_OFFSET, 0x08400508);

			// Operating | 8-bit value to program to
			// Channel   | Sub-Register 0x2B:0B â€“ FS_PLLTUNE
			// ----------+----------------------------------
			// 1         | 0x1E
			// 2,4       | 0x26    <---------- Kanal 2
			// 3         | 0x56
			// 5,7       | 0xBE
			dw1000_writeUInt8(DW1000_FS_CTRL, DW1000_FS_PLLTUNE_OFFSET, 0x26);

			// TX        | 32-bit value to program to
			// Channel   | Sub-Register 0x28:0Câ€“ RF_TXCTRL
			// ----------+----------------------------------
			// 1         | 0x00005C40
			// 2         | 0x00045CA0    <---------- Kanal 2
			// 3         | 0x00086CC0
			// 4         | 0x00045C80
			// 5         | 0x001E3FE0
			// 7         | 0x001E7DE0
			dw1000_writeUInt32(DW1000_RF_CONF, DW1000_RF_TXCTRL_OFFSET, 0x00045CA0);

			// RX        | 8-bit value to program to
			// Channel   | Sub-Register 0x28:0Bâ€“RF_RXCTRLH
			// ----------+----------------------------------
			// 1, 2, 3, 5| 0xD8    <---------- Kanal 2
			// 4, 7      | 0xBC
			dw1000_writeUInt8(DW1000_RF_CONF, DW1000_RF_RXCTRLH_OFFSET, 0xD8);

			// TX        | 8-bit value to program to
			// Channel   | Sub-Register 0x2A:0B â€“ TC_PGDELAY
			// ----------+----------------------------------
			// 1         | 0xC9
			// 2         | 0xC2    <---------- Kanal 2
			// 3         | 0xC5
			// 4         | 0x95
			// 5         | 0xC0
			// 7         | 0x93
			dw1000_writeUInt8(DW1000_TX_CAL, DW1000_TC_PGDELAY_OFFSET, 0xc2);
			break;

		case DW1000_CHANNEL_3:

			// Channel | Centre          | Bandwidth | Preamble Codes | Preamble Codes
			// number  | frequency (MHz) | (MHz)     | (16 MHz PRF)   | (64 MHz PRF)
			// --------+-----------------+-----------+----------------+----------------
			// 1       | 3494.4          | 499.2     | 1, 2           | 9, 10, 11, 12
			// 2       | 3993.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 3       | 4492.8          | 499.2     | 5, 6           | 9, 10, 11, 12   <---------- Kanal 3
			// 4       | 3993.6          | 1331.2    | 7, 8           | 17, 18, 19, 20
			// 5       | 6489.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 7       | 6489.6          | 1081.6    | 7, 8           | 17, 18, 19, 20
			switch(dw1000_preambleCode)
			{
				case DW1000_PREAMBLE_CODE_16MHZ_5:
				case DW1000_PREAMBLE_CODE_16MHZ_6:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_9;
					break;
				case DW1000_PREAMBLE_CODE_64MHZ_9:
				case DW1000_PREAMBLE_CODE_64MHZ_10:
				case DW1000_PREAMBLE_CODE_64MHZ_11:
				case DW1000_PREAMBLE_CODE_64MHZ_12:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_64MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_5;
					break;
				default:
					if (dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_5;
					else dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_9;
					break;
			}

			// Operating | 32-bit value to program to
			// Channel   | Sub-Register 0x2B:07 â€“ FS_PLLCFG
			// ----------+----------------------------------
			// 1         | 0x09000407
			// 2,4       | 0x08400508
			// 3         | 0x08401009    <---------- Kanal 3
			// 5, 7      | 0x0800041D
			dw1000_writeUInt32(DW1000_FS_CTRL, DW1000_FS_PLLCFG_OFFSET, 0x08401009);

			// Operating | 8-bit value to program to
			// Channel   | Sub-Register 0x2B:0B â€“ FS_PLLTUNE
			// ----------+----------------------------------
			// 1         | 0x1E
			// 2,4       | 0x26
			// 3         | 0x56    <---------- Kanal 3
			// 5,7       | 0xBE
			dw1000_writeUInt8(DW1000_FS_CTRL, DW1000_FS_PLLTUNE_OFFSET, 0x56);

			// TX        | 32-bit value to program to
			// Channel   | Sub-Register 0x28:0Câ€“ RF_TXCTRL
			// ----------+----------------------------------
			// 1         | 0x00005C40
			// 2         | 0x00045CA0
			// 3         | 0x00086CC0    <---------- Kanal 3
			// 4         | 0x00045C80
			// 5         | 0x001E3FE0
			// 7         | 0x001E7DE0
			dw1000_writeUInt32(DW1000_RF_CONF, DW1000_RF_TXCTRL_OFFSET, 0x00086CC0);

			// RX        | 8-bit value to program to
			// Channel   | Sub-Register 0x28:0Bâ€“RF_RXCTRLH
			// ----------+----------------------------------
			// 1, 2, 3, 5| 0xD8    <---------- Kanal 3
			// 4, 7      | 0xBC
			dw1000_writeUInt8(DW1000_RF_CONF, DW1000_RF_RXCTRLH_OFFSET, 0xD8);

			// TX        | 8-bit value to program to
			// Channel   | Sub-Register 0x2A:0B â€“ TC_PGDELAY
			// ----------+----------------------------------
			// 1         | 0xC9
			// 2         | 0xC2
			// 3         | 0xC5    <---------- Kanal 3
			// 4         | 0x95
			// 5         | 0xC0
			// 7         | 0x93
			dw1000_writeUInt8(DW1000_TX_CAL, DW1000_TC_PGDELAY_OFFSET, 0xc5);
			break;

		case DW1000_CHANNEL_4:

			// Channel | Centre          | Bandwidth | Preamble Codes | Preamble Codes
			// number  | frequency (MHz) | (MHz)     | (16 MHz PRF)   | (64 MHz PRF)
			// --------+-----------------+-----------+----------------+----------------
			// 1       | 3494.4          | 499.2     | 1, 2           | 9, 10, 11, 12
			// 2       | 3993.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 3       | 4492.8          | 499.2     | 5, 6           | 9, 10, 11, 12
			// 4       | 3993.6          | 1331.2    | 7, 8           | 17, 18, 19, 20  <---------- Kanal 4
			// 5       | 6489.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 7       | 6489.6          | 1081.6    | 7, 8           | 17, 18, 19, 20
			switch(dw1000_preambleCode)
			{
				case DW1000_PREAMBLE_CODE_16MHZ_7:
				case DW1000_PREAMBLE_CODE_16MHZ_8:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_20;
					break;
				case DW1000_PREAMBLE_CODE_64MHZ_17:
				case DW1000_PREAMBLE_CODE_64MHZ_18:
				case DW1000_PREAMBLE_CODE_64MHZ_19:
				case DW1000_PREAMBLE_CODE_64MHZ_20:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_64MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_7;
					break;
				default:
					if (dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_7;
					else dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_20;
					break;
			}

			// Operating | 32-bit value to program to
			// Channel   | Sub-Register 0x2B:07 â€“ FS_PLLCFG
			// ----------+----------------------------------
			// 1         | 0x09000407
			// 2,4       | 0x08400508    <---------- Kanal 4
			// 3         | 0x08401009
			// 5, 7      | 0x0800041D
			dw1000_writeUInt32(DW1000_FS_CTRL, DW1000_FS_PLLCFG_OFFSET, 0x08400508);

			// Operating | 8-bit value to program to
			// Channel   | Sub-Register 0x2B:0B â€“ FS_PLLTUNE
			// ----------+----------------------------------
			// 1         | 0x1E
			// 2,4       | 0x26    <---------- Kanal 4
			// 3         | 0x56
			// 5,7       | 0xBE
			dw1000_writeUInt8(DW1000_FS_CTRL, DW1000_FS_PLLTUNE_OFFSET, 0x26);

			// TX        | 32-bit value to program to
			// Channel   | Sub-Register 0x28:0Câ€“ RF_TXCTRL
			// ----------+----------------------------------
			// 1         | 0x00005C40
			// 2         | 0x00045CA0
			// 3         | 0x00086CC0
			// 4         | 0x00045C80    <---------- Kanal 4
			// 5         | 0x001E3FE0
			// 7         | 0x001E7DE0
			dw1000_writeUInt32(DW1000_RF_CONF, DW1000_RF_TXCTRL_OFFSET, 0x00045C80);

			// RX        | 8-bit value to program to
			// Channel   | Sub-Register 0x28:0Bâ€“RF_RXCTRLH
			// ----------+----------------------------------
			// 1, 2, 3, 5| 0xD8
			// 4, 7      | 0xBC    <---------- Kanal 4
			dw1000_writeUInt8(DW1000_RF_CONF, DW1000_RF_RXCTRLH_OFFSET, 0xBC);

			// TX        | 8-bit value to program to
			// Channel   | Sub-Register 0x2A:0B â€“ TC_PGDELAY
			// ----------+----------------------------------
			// 1         | 0xC9
			// 2         | 0xC2
			// 3         | 0xC5
			// 4         | 0x95    <---------- Kanal 4
			// 5         | 0xC0
			// 7         | 0x93
			dw1000_writeUInt8(DW1000_TX_CAL, DW1000_TC_PGDELAY_OFFSET, 0x95);
			break;

		case DW1000_CHANNEL_5:

			// Channel | Centre          | Bandwidth | Preamble Codes | Preamble Codes
			// number  | frequency (MHz) | (MHz)     | (16 MHz PRF)   | (64 MHz PRF)
			// --------+-----------------+-----------+----------------+----------------
			// 1       | 3494.4          | 499.2     | 1, 2           | 9, 10, 11, 12
			// 2       | 3993.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 3       | 4492.8          | 499.2     | 5, 6           | 9, 10, 11, 12
			// 4       | 3993.6          | 1331.2    | 7, 8           | 17, 18, 19, 20
			// 5       | 6489.6          | 499.2     | 3, 4           | 9, 10, 11, 12   <---------- Kanal 5
			// 7       | 6489.6          | 1081.6    | 7, 8           | 17, 18, 19, 20
			switch(dw1000_preambleCode)
			{
				case DW1000_PREAMBLE_CODE_16MHZ_3:
				case DW1000_PREAMBLE_CODE_16MHZ_4:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_9;
					break;
				case DW1000_PREAMBLE_CODE_64MHZ_9:
				case DW1000_PREAMBLE_CODE_64MHZ_10:
				case DW1000_PREAMBLE_CODE_64MHZ_11:
				case DW1000_PREAMBLE_CODE_64MHZ_12:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_64MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_3;
					break;
				default:
					if (dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_3;
					else dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_9;
					break;
			}

			// Operating | 32-bit value to program to
			// Channel   | Sub-Register 0x2B:07 â€“ FS_PLLCFG
			// ----------+----------------------------------
			// 1         | 0x09000407
			// 2,4       | 0x08400508
			// 3         | 0x08401009
			// 5, 7      | 0x0800041D    <---------- Kanal 5
			dw1000_writeUInt32(DW1000_FS_CTRL, DW1000_FS_PLLCFG_OFFSET, 0x0800041D);

			// Operating | 8-bit value to program to
			// Channel   | Sub-Register 0x2B:0B â€“ FS_PLLTUNE
			// ----------+----------------------------------
			// 1         | 0x1E
			// 2,4       | 0x26
			// 3         | 0x56
			// 5,7       | 0xBE    <---------- Kanal 5
			dw1000_writeUInt8(DW1000_FS_CTRL, DW1000_FS_PLLTUNE_OFFSET, 0xBE);

			// TX        | 32-bit value to program to
			// Channel   | Sub-Register 0x28:0Câ€“ RF_TXCTRL
			// ----------+----------------------------------
			// 1         | 0x00005C40
			// 2         | 0x00045CA0
			// 3         | 0x00086CC0
			// 4         | 0x00045C80
			// 5         | 0x001E3FE0    <---------- Kanal 5
			// 7         | 0x001E7DE0
			dw1000_writeUInt32(DW1000_RF_CONF, DW1000_RF_TXCTRL_OFFSET, 0x001E3FE0);

			// RX        | 8-bit value to program to
			// Channel   | Sub-Register 0x28:0Bâ€“RF_RXCTRLH
			// ----------+----------------------------------
			// 1, 2, 3, 5| 0xD8    <---------- Kanal 5
			// 4, 7      | 0xBC
			dw1000_writeUInt8(DW1000_RF_CONF, DW1000_RF_RXCTRLH_OFFSET, 0xD8);

			// TX        | 8-bit value to program to
			// Channel   | Sub-Register 0x2A:0B â€“ TC_PGDELAY
			// ----------+----------------------------------
			// 1         | 0xC9
			// 2         | 0xC2
			// 3         | 0xC5
			// 4         | 0x95
			// 5         | 0xC0    <---------- Kanal 5
			// 7         | 0x93
			dw1000_writeUInt8(DW1000_TX_CAL, DW1000_TC_PGDELAY_OFFSET, 0xc0);
			break;

		case DW1000_CHANNEL_7:

			// Channel | Centre          | Bandwidth | Preamble Codes | Preamble Codes
			// number  | frequency (MHz) | (MHz)     | (16 MHz PRF)   | (64 MHz PRF)
			// --------+-----------------+-----------+----------------+----------------
			// 1       | 3494.4          | 499.2     | 1, 2           | 9, 10, 11, 12
			// 2       | 3993.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 3       | 4492.8          | 499.2     | 5, 6           | 9, 10, 11, 12
			// 4       | 3993.6          | 1331.2    | 7, 8           | 17, 18, 19, 20
			// 5       | 6489.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 7       | 6489.6          | 1081.6    | 7, 8           | 17, 18, 19, 20  <---------- Kanal 7
			switch(dw1000_preambleCode)
			{
				case DW1000_PREAMBLE_CODE_16MHZ_7:
				case DW1000_PREAMBLE_CODE_16MHZ_8:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_17;
					break;
				case DW1000_PREAMBLE_CODE_64MHZ_17:
				case DW1000_PREAMBLE_CODE_64MHZ_18:
				case DW1000_PREAMBLE_CODE_64MHZ_19:
				case DW1000_PREAMBLE_CODE_64MHZ_20:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_64MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_7;
					break;
				default:
					if (dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_7;
					else dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_17;
					break;
			}

			// Operating | 32-bit value to program to
			// Channel   | Sub-Register 0x2B:07 â€“ FS_PLLCFG
			// ----------+----------------------------------
			// 1         | 0x09000407
			// 2,4       | 0x08400508
			// 3         | 0x08401009
			// 5, 7      | 0x0800041D    <---------- Kanal 7
			dw1000_writeUInt32(DW1000_FS_CTRL, DW1000_FS_PLLCFG_OFFSET, 0x0800041D);

			// Operating | 8-bit value to program to
			// Channel   | Sub-Register 0x2B:0B â€“ FS_PLLTUNE
			// ----------+----------------------------------
			// 1         | 0x1E
			// 2,4       | 0x26
			// 3         | 0x56
			// 5,7       | 0xBE    <---------- Kanal 7
			dw1000_writeUInt8(DW1000_FS_CTRL, DW1000_FS_PLLTUNE_OFFSET, 0xBE);

			// TX        | 32-bit value to program to
			// Channel   | Sub-Register 0x28:0Câ€“ RF_TXCTRL
			// ----------+----------------------------------
			// 1         | 0x00005C40
			// 2         | 0x00045CA0
			// 3         | 0x00086CC0
			// 4         | 0x00045C80
			// 5         | 0x001E3FE0
			// 7         | 0x001E7DE0    <---------- Kanal 7
			dw1000_writeUInt32(DW1000_RF_CONF, DW1000_RF_TXCTRL_OFFSET, 0x001E7DE0);

			// RX        | 8-bit value to program to
			// Channel   | Sub-Register 0x28:0Bâ€“RF_RXCTRLH
			// ----------+----------------------------------
			// 1, 2, 3, 5| 0xD8
			// 4, 7      | 0xBC    <---------- Kanal 7
			dw1000_writeUInt8(DW1000_RF_CONF, DW1000_RF_RXCTRLH_OFFSET, 0xBC);

			// TX        | 8-bit value to program to
			// Channel   | Sub-Register 0x2A:0B â€“ TC_PGDELAY
			// ----------+----------------------------------
			// 1         | 0xC9
			// 2         | 0xC2
			// 3         | 0xC5
			// 4         | 0x95
			// 5         | 0xC0
			// 7         | 0x93    <---------- Kanal 7
			dw1000_writeUInt8(DW1000_TX_CAL, DW1000_TC_PGDELAY_OFFSET, 0x93);
			break;

		case DW1000_CHANNEL_1:
		default:

			// Standardkanal ist 1
			dw1000_channel = DW1000_CHANNEL_1;

			// Channel | Centre          | Bandwidth | Preamble Codes | Preamble Codes
			// number  | frequency (MHz) | (MHz)     | (16 MHz PRF)   | (64 MHz PRF)
			// --------+-----------------+-----------+----------------+----------------
			// 1       | 3494.4          | 499.2     | 1, 2           | 9, 10, 11, 12   <---------- Kanal 1
			// 2       | 3993.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 3       | 4492.8          | 499.2     | 5, 6           | 9, 10, 11, 12
			// 4       | 3993.6          | 1331.2    | 7, 8           | 17, 18, 19, 20
			// 5       | 6489.6          | 499.2     | 3, 4           | 9, 10, 11, 12
			// 7       | 6489.6          | 1081.6    | 7, 8           | 17, 18, 19, 20
			switch(dw1000_preambleCode)
			{
				case DW1000_PREAMBLE_CODE_16MHZ_1:
				case DW1000_PREAMBLE_CODE_16MHZ_2:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_9;
					break;
				case DW1000_PREAMBLE_CODE_64MHZ_9:
				case DW1000_PREAMBLE_CODE_64MHZ_10:
				case DW1000_PREAMBLE_CODE_64MHZ_11:
				case DW1000_PREAMBLE_CODE_64MHZ_12:
					if (dw1000_pulseFrequency != DW1000_TX_PULSE_FREQ_64MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_1;
					break;
				default:
					if (dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_16MHZ) dw1000_preambleCode = DW1000_PREAMBLE_CODE_16MHZ_1;
					else dw1000_preambleCode = DW1000_PREAMBLE_CODE_64MHZ_9;
					break;
			}

			// Operating | 32-bit value to program to
			// Channel   | Sub-Register 0x2B:07 â€“ FS_PLLCFG
			// ----------+----------------------------------
			// 1         | 0x09000407    <---------- Kanal 1
			// 2,4       | 0x08400508
			// 3         | 0x08401009
			// 5, 7      | 0x0800041D
			dw1000_writeUInt32(DW1000_FS_CTRL, DW1000_FS_PLLCFG_OFFSET, 0x09000407);

			// Operating | 8-bit value to program to
			// Channel   | Sub-Register 0x2B:0B â€“ FS_PLLTUNE
			// ----------+----------------------------------
			// 1         | 0x1E    <---------- Kanal 1
			// 2,4       | 0x26
			// 3         | 0x56
			// 5,7       | 0xBE
			dw1000_writeUInt8(DW1000_FS_CTRL, DW1000_FS_PLLTUNE_OFFSET, 0x1E);

			// TX        | 32-bit value to program to
			// Channel   | Sub-Register 0x28:0Câ€“ RF_TXCTRL
			// ----------+----------------------------------
			// 1         | 0x00005C40    <---------- Kanal 1
			// 2         | 0x00045CA0
			// 3         | 0x00086CC0
			// 4         | 0x00045C80
			// 5         | 0x001E3FE0
			// 7         | 0x001E7DE0
			dw1000_writeUInt32(DW1000_RF_CONF, DW1000_RF_TXCTRL_OFFSET, 0x00005C40);

			// RX        | 8-bit value to program to
			// Channel   | Sub-Register 0x28:0Bâ€“RF_RXCTRLH
			// ----------+----------------------------------
			// 1, 2, 3, 5| 0xD8    <---------- Kanal 1
			// 4, 7      | 0xBC
			dw1000_writeUInt8(DW1000_RF_CONF, DW1000_RF_RXCTRLH_OFFSET, 0xD8);

			// TX        | 8-bit value to program to
			// Channel   | Sub-Register 0x2A:0B â€“ TC_PGDELAY
			// ----------+----------------------------------
			// 1         | 0xC9    <---------- Kanal 1
			// 2         | 0xC2
			// 3         | 0xC5
			// 4         | 0x95
			// 5         | 0xC0
			// 7         | 0x93
			dw1000_writeUInt8(DW1000_TX_CAL, DW1000_TC_PGDELAY_OFFSET, 0xc9);
			break;
	}

	// Lege die Konfigurationen fest, die von der Datenrate abhängen
	switch(dw1000_dataRate)
	{

		case DW1000_TRX_RATE_850KBPS:

			// Data Rate | Recommended preamble sequence length
			// ----------+-------------------------------------
			// 6.8 Mbps  | 64 or 128 or 256
			// 850 kbps  | 256 or 512 or 1024    <---------- 850
			// 110kbps   | 2048 or 4096
			switch(dw1000_preambleLength)
			{
			    case DW1000_TX_PREAMBLE_LEN_256:
				case DW1000_TX_PREAMBLE_LEN_512:
				case DW1000_TX_PREAMBLE_LEN_1024: break;
				default:
					dw1000_preambleLength = DW1000_TX_PREAMBLE_LEN_256;
					break;
			}

			// Note: The selection SFD sequences other than the IEEE 802.15.4-2011
			// UWB standard compliant SFD sequence may improve performance, but it will
			// of course make it impossible to interwork with a DW1000 configured to
			// use the standard SFD (or with a third party standard compliant device).
			lengthOfSFD = 8;

			// Schreibe die Länge der SFD
			dw1000_writeUInt8(DW1000_USR_SFD, DW1000_SFD_LENGTH_OFFSET, lengthOfSFD);		// Setze die Länge des SFD auf 16 (Nach Tabelle 20)

			// Lege die erweiterte SFD fest (Nach Tabelle 20)
			//dw1000_chan_ctrl |=  DW1000_CHAN_CTRL_DWSFD;
			dw1000_chan_ctrl &= ~DW1000_CHAN_CTRL_DWSFD;
			dw1000_chan_ctrl &= ~DW1000_CHAN_CTRL_TNSSFD;
			dw1000_chan_ctrl &= ~DW1000_CHAN_CTRL_RNSSFD;

			// Schreibe das DRX_TUNI0b Register
			dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE0b_OFFSET, 0x0001);	// Standard SFD
			//dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE0b_OFFSET, 0x0006);	// Non-Standard

			// Receiver Mode 110 kbps data rate. This configuration when set to 1 will cause the receiver
			// to look for long SFD and process the PHY Header and RX data as per the 110 kbps frame
			// mode. When this configuration is 0, (the default), the receiver will look for short SFD and
			// will determine the RX data rate from the PHY header as either 850 kbps or 6.8 Mbps
			dw1000_sys_cfg &= ~DW1000_SYS_CFG_RXM110K;

			break;
		case DW1000_TRX_RATE_6800KBPS:

			// Data Rate | Recommended preamble sequence length
			// ----------+-------------------------------------
			// 6.8 Mbps  | 64 or 128 or 256      <---------- 6800
			// 850 kbps  | 256 or 512 or 1024
			// 110kbps   | 2048 or 4096
			switch(dw1000_preambleLength)
			{
				case DW1000_TX_PREAMBLE_LEN_64:
				case DW1000_TX_PREAMBLE_LEN_128: break;
				case DW1000_TX_PREAMBLE_LEN_256: break;
				default:
					dw1000_preambleLength = DW1000_TX_PREAMBLE_LEN_256;
					break;
			}

			// Note: The selection SFD sequences other than the IEEE 802.15.4-2011
			// UWB standard compliant SFD sequence may improve performance, but it will
			// of course make it impossible to interwork with a DW1000 configured to
			// use the standard SFD (or with a third party standard compliant device).
			lengthOfSFD = 8;

			// Setze die Länge des SFD auf 8 (Nach Tabelle 20)
			dw1000_writeUInt8(DW1000_USR_SFD, DW1000_SFD_LENGTH_OFFSET, 8);

			// Lege die erweiterte SFD fest (Nach Tabelle 20)
			dw1000_chan_ctrl &= ~DW1000_CHAN_CTRL_DWSFD;
			dw1000_chan_ctrl &= ~DW1000_CHAN_CTRL_TNSSFD;
			dw1000_chan_ctrl &= ~DW1000_CHAN_CTRL_RNSSFD;

			// Schreibe das DRX_TUNI0b Register
			dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE0b_OFFSET, 0x0001);	// Standard SFD
			//dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE0b_OFFSET, 0x0002);	// Non-Standard

			// Receiver Mode 110 kbps data rate. This configuration when set to 1 will cause the receiver
			// to look for long SFD and process the PHY Header and RX data as per the 110 kbps frame
			// mode. When this configuration is 0, (the default), the receiver will look for short SFD and
			// will determine the RX data rate from the PHY header as either 850 kbps or 6.8 Mbps
			dw1000_sys_cfg &= ~DW1000_SYS_CFG_RXM110K;

			break;

		case DW1000_TRX_RATE_110KBPS:
		default:

			// FÃ¼r den Fall, dass default anspringt
			dw1000_dataRate = DW1000_TRX_RATE_110KBPS;

			// Data Rate | Recommended preamble sequence length
			// ----------+-------------------------------------
			// 6.8 Mbps  | 64 or 128 or 256
			// 850 kbps  | 256 or 512 or 1024
			// 110kbps   | 2048 or 4096          <---------- 110
			switch(dw1000_preambleLength)
			{
				case DW1000_TX_PREAMBLE_LEN_2048:
				case DW1000_TX_PREAMBLE_LEN_4096: break;
				default:
					dw1000_preambleLength = DW1000_TX_PREAMBLE_LEN_2048;
					break;
			}

			// Note: The selection SFD sequences other than the IEEE 802.15.4-2011
			// UWB standard compliant SFD sequence may improve performance, but it will
			// of course make it impossible to interwork with a DW1000 configured to
			// use the standard SFD (or with a third party standard compliant device).
			lengthOfSFD = 64;

			// Setze die Länge des SFD auf 64 (Nach Tabelle 20)
			//dw1000_writeUInt8(DW1000_USR_SFD, DW1000_SFD_LENGTH_OFFSET, lengthOfSFD);

			// Lege die erweiterte SFD fest (Nach Tabelle 20)
			dw1000_chan_ctrl |=  DW1000_CHAN_CTRL_DWSFD;
			dw1000_chan_ctrl |=  DW1000_CHAN_CTRL_TNSSFD;
			dw1000_chan_ctrl |=  DW1000_CHAN_CTRL_RNSSFD;

			// Schreibe das DRX_TUNI0b Register
			//dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE0b_OFFSET, 0x000a);	// Standard SFD
			dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE0b_OFFSET, 0x0016);	// Non-Standard SFD

			// Receiver Mode 110 kbps data rate. This configuration when set to 1 will cause the receiver
			// to look for long SFD and process the PHY Header and RX data as per the 110 kbps frame
			// mode. When this configuration is 0, (the default), the receiver will look for short SFD and
			// will determine the RX data rate from the PHY header as either 850 kbps or 6.8 Mbps
			dw1000_sys_cfg |= DW1000_SYS_CFG_RXM110K;

			break;
	}

	// Calculate the size of the PHY Header, depending on the Preamblelength (Table 6)
	switch(dw1000_preambleLength)
	{
		case DW1000_TX_PREAMBLE_LEN_64:
			dw1000_pacSize = DW1000_PAC_SIZE_8;
			timeoutOfSFD = 64 + lengthOfSFD + 1;
			dw1000_sniffmodeon = 2;
			dw1000_sniffmodeoff = ((dw1000_sniffmodeon + 1) * DW1000_PAC_SIZE_8);
			break;
		case DW1000_TX_PREAMBLE_LEN_128:
			dw1000_pacSize = DW1000_PAC_SIZE_8;
			timeoutOfSFD = 128 + lengthOfSFD + 1;
			dw1000_sniffmodeon = 2;
			dw1000_sniffmodeoff = ((dw1000_sniffmodeon + 1) * DW1000_PAC_SIZE_8);
			break;
		case DW1000_TX_PREAMBLE_LEN_256:
			dw1000_pacSize = DW1000_PAC_SIZE_16;
			timeoutOfSFD = 256 + lengthOfSFD + 1;
			dw1000_sniffmodeon = 2;
			dw1000_sniffmodeoff = ((dw1000_sniffmodeon + 1) * DW1000_PAC_SIZE_8);
			break;
		case DW1000_TX_PREAMBLE_LEN_512:
			dw1000_pacSize = DW1000_PAC_SIZE_16;
			timeoutOfSFD = 512 + lengthOfSFD + 1;
			dw1000_sniffmodeon = 2;
			dw1000_sniffmodeoff = ((dw1000_sniffmodeon + 1) * DW1000_PAC_SIZE_8);
			break;
		case DW1000_TX_PREAMBLE_LEN_1024:
			dw1000_pacSize = DW1000_PAC_SIZE_32;
			timeoutOfSFD = 1024 + lengthOfSFD + 1;
			dw1000_sniffmodeon = 1;
			dw1000_sniffmodeoff = 255;//((dw1000_sniffmodeon + 1) * DW1000_PAC_SIZE_8);
			break;
		case DW1000_TX_PREAMBLE_LEN_1536:
			dw1000_pacSize = DW1000_PAC_SIZE_64;
			timeoutOfSFD = 1536 + lengthOfSFD + 1;
			dw1000_sniffmodeon = 2;
			dw1000_sniffmodeoff = ((dw1000_sniffmodeon + 1) * DW1000_PAC_SIZE_8);
			break;
		case DW1000_TX_PREAMBLE_LEN_2048:
			dw1000_pacSize = DW1000_PAC_SIZE_64;
			timeoutOfSFD = 2048 + lengthOfSFD + 1;
			dw1000_sniffmodeon = 2;
			dw1000_sniffmodeoff = ((dw1000_sniffmodeon + 1) * DW1000_PAC_SIZE_8);
			break;
		case DW1000_TX_PREAMBLE_LEN_4096:
			dw1000_pacSize = DW1000_PAC_SIZE_64;
			timeoutOfSFD = 4096 + lengthOfSFD + 1;
			dw1000_sniffmodeon = 2;
			dw1000_sniffmodeoff = ((dw1000_sniffmodeon + 1) * DW1000_PAC_SIZE_8);
			break;
	}

	// Replica Coefficient: Leading Edge Detection Interface, sub-register 0x2804 is a 16-bit configuration register
	// for setting the replica avoidance coefficient. The accumulator operates on the preamble sequence to give
	// the channel impulse response. This works because of the perfect periodic auto-correlation property of the
	// IEEE 802.15.4 UWB preamble sequences.  The auto-correlation is not perfect where there is a significant
	// clock offset between the remote transmitter and the local receiver. In these circumstances small amplitude
	// replicas of the channel impulse response appear repeatedly throughout the accumulator span. The
	// magnitude of this effect is dependent on the clock offset and on the preamble code being employed.  To
	// avoid the LDE erroneously seeing one of these replica signals as the leading edge the threshold used for
	// detecting the first path is artificially raised by a factor depending on the measured clock offset. For optimum
	// performance this factor also needs to be dependent on the preamble code selected in the receiver. To
	// achieve this, the LDE_REPC configuration needs to be set depending on the receiver preamble code
	// configuration.
	unsigned short dw1000_lde_replicaCoeff[] =
	{
		0,		0x5998,	0x5998,	0x51EA,	0x428E,
		0x451E,	0x2E14,	0x8000,	0x51EA,	0x28F4,
		0x3332,	0x3AE0,	0x3D70,	0x3AE0,	0x35C2,
		0x2B84,	0x35C2,	0x3332,	0x35C2,	0x35C2,
		0x47AE,	0x3AE0,	0x3850,	0x30A2,	0x3850
	};
	dw1000_writeUInt16(DW1000_LDE_IF, DW1000_LDE_REPC_OFFSET, (dw1000_dataRate == DW1000_TRX_RATE_110KBPS) ? (unsigned short)(dw1000_lde_replicaCoeff[dw1000_preambleCode] >> 3) : dw1000_lde_replicaCoeff[dw1000_preambleCode]);

	// Konfiguriere die LDE (Noise Threshold Multiplier und Peak Multiplier)
    dw1000_writeUInt16(DW1000_LDE_IF, DW1000_LDE_CFG1_OFFSET, DW1000_PEAK_MULTPLIER | DW1000_N_STD_FACTOR);

    // Lege "Leading Edge Detection Interface" und "Digital receiver configuration" fest
    if(dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_16MHZ)
    {
        // Konfiguriere die LDE (Tuning register)
        // Leading Edge Detection Interface, sub-register 0x1806 is a 16-bit LDE configuration
        // tuning register.  The value here needs to change depending on the RXPRF configuration.
    	// RXPRF configuration | Value to program to Sub-Register 0x2E:1806â€“ LDE_CFG2
    	// --------------------+-----------------------------------------------------
    	// (1) = 16 MHz PRF    | 0x1607    <--------- 16
    	// (2) = 64 MHz PRF    | 0x0607
    	dw1000_writeUInt16(DW1000_LDE_IF, DW1000_LDE_CFG2_OFFSET, 0x1607);

    	// Register file: 0x27 â€“ Digital receiver configuration, sub-register 0x04 is a 16-bit
    	// tuning register.  The value here needs to change depending on the RXPRF configuration.
    	// The values needed are given in Table 29 below.  Please take care not to write other values
    	// to this register as doing so may cause the DW1000 to malfunction.
    	// RXPRF configuration | Value to program to Sub-Register 0x27:04 â€“	DRX_TUNE1a
    	// --------------------+-----------------------------------------------------
    	// (1) = 16 MHz PRF    | 0x0087    <--------- 16
    	// (2) = 64 MHz PRF    | 0x008D
    	dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE1a_OFFSET, 0x0087);
    }
    else
    {
        // Konfiguriere die LDE (Tuning register)
        // Leading Edge Detection Interface, sub-register 0x1806 is a 16-bit LDE configuration
        // tuning register.  The value here needs to change depending on the RXPRF configuration.
    	// RXPRF configuration | Value to program to Sub-Register 0x2E:1806â€“ LDE_CFG2
		// --------------------+-----------------------------------------------------
		// (1) = 16 MHz PRF    | 0x1607
		// (2) = 64 MHz PRF    | 0x0607    <--------- 64
    	dw1000_writeUInt16(DW1000_LDE_IF, DW1000_LDE_CFG2_OFFSET, 0x0607);

    	// Register file: 0x27 â€“ Digital receiver configuration, sub-register 0x04 is a 16-bit
		// tuning register.  The value here needs to change depending on the RXPRF configuration.
		// The values needed are given in Table 29 below.  Please take care not to write other values
		// to this register as doing so may cause the DW1000 to malfunction.
    	// RXPRF configuration | Value to program to Sub-Register 0x27:04 â€“	DRX_TUNE1a
    	// --------------------+-----------------------------------------------------
    	// (1) = 16 MHz PRF    | 0x0087
    	// (2) = 64 MHz PRF    | 0x008D    <--------- 64
    	dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE1a_OFFSET, 0x008d);
    }

    if (
    	(dw1000_preambleLength == DW1000_TX_PREAMBLE_LEN_1536) ||
    	(dw1000_preambleLength == DW1000_TX_PREAMBLE_LEN_2048) ||
    	(dw1000_preambleLength == DW1000_TX_PREAMBLE_LEN_4096)
    ){
        // Digital receiver configuration, sub-register 0x06 is a 16-bit tuning register. The value
        // here needs to change depending on use case. The values needed are given in Table 30 below. Please take
        // care not to write other values to this register as doing so may cause the DW1000 to malfunction.
    	// Use case                                                                  | Value to program to Sub-Register 0x27:06 â€“ DRX_TUNE1b
    	// --------------------------------------------------------------------------+------------------------------------------------------
    	// Preamble lengths > 1024 symbols, for 110 kbps operation                   | 0x0064    <---------
    	// Preamble lengths 128 to 1024 symbols, for 850 kbps and 6.8 Mbps operation | 0x0020
    	// Preamble length = 64 symbols, for 6.8 Mbps operation                      | 0x0010
    	dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE1b_OFFSET, 0x0064);

    	// Digital receiver configuration, sub-register 0x26 is a 16-bit tuning register. The value here needs
		// to change depending on the preamble length expected by the receiver. The values required are given
		// in Table 32.  Please take care not to write other values to this register as doing so may cause
		// the DW1000 to malfunction
    	// Expected Receive Preamble Length in Symbols | Value to program to Sub-Register 0x27:26 â€“ DRX_TUNE4H
    	// --------------------------------------------+------------------------------------------------------
    	// 64                                          | 0x0010
    	// 128 or greater                              | 0x0028    <---------
    	dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE4H_OFFSET, 0x0028);
    }
    else if (dw1000_preambleLength == DW1000_TX_PREAMBLE_LEN_64)
    {
        // Digital receiver configuration, sub-register 0x06 is a 16-bit tuning register. The value
        // here needs to change depending on use case. The values needed are given in Table 30 below. Please take
        // care not to write other values to this register as doing so may cause the DW1000 to malfunction.
    	// Use case                                                                  | Value to program to Sub-Register 0x27:06 â€“ DRX_TUNE1b
    	// --------------------------------------------------------------------------+------------------------------------------------------
    	// Preamble lengths > 1024 symbols, for 110 kbps operation                   | 0x0064
    	// Preamble lengths 128 to 1024 symbols, for 850 kbps and 6.8 Mbps operation | 0x0020
    	// Preamble length = 64 symbols, for 6.8 Mbps operation                      | 0x0010    <---------
    	dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE1b_OFFSET, 0x0010);

    	// Digital receiver configuration, sub-register 0x26 is a 16-bit tuning register. The value here needs
		// to change depending on the preamble length expected by the receiver. The values required are given
		// in Table 32.  Please take care not to write other values to this register as doing so may cause
		// the DW1000 to malfunction
    	// Expected Receive Preamble Length in Symbols | Value to program to Sub-Register 0x27:26 â€“ DRX_TUNE4H
    	// --------------------------------------------+------------------------------------------------------
    	// 64                                          | 0x0010    <---------
    	// 128 or greater                              | 0x0028
    	dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE4H_OFFSET, 0x0010);
    }
    else
    {
        // Digital receiver configuration, sub-register 0x06 is a 16-bit tuning register. The value
        // here needs to change depending on use case. The values needed are given in Table 30 below. Please take
        // care not to write other values to this register as doing so may cause the DW1000 to malfunction.
    	// Use case                                                                  | Value to program to Sub-Register 0x27:06 â€“ DRX_TUNE1b
    	// --------------------------------------------------------------------------+------------------------------------------------------
    	// Preamble lengths > 1024 symbols, for 110 kbps operation                   | 0x0064
    	// Preamble lengths 128 to 1024 symbols, for 850 kbps and 6.8 Mbps operation | 0x0020    <---------
    	// Preamble length = 64 symbols, for 6.8 Mbps operation                      | 0x0010
    	dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE1b_OFFSET, 0x0020);

    	// Digital receiver configuration, sub-register 0x26 is a 16-bit tuning register. The value here needs
    	// to change depending on the preamble length expected by the receiver. The values required are given
    	// in Table 32.  Please take care not to write other values to this register as doing so may cause
    	// the DW1000 to malfunction
    	// Expected Receive Preamble Length in Symbols | Value to program to Sub-Register 0x27:26 â€“ DRX_TUNE4H
    	// --------------------------------------------+------------------------------------------------------
    	// 64                                          | 0x0010
    	// 128 or greater                              | 0x0028    <---------
    	dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_TUNE4H_OFFSET, 0x0028);
    }

	// Digital receiver configuration, sub-register 0x08 is a tuning register. The value here
	// needs to change depending on a number of parameters. The values needed are given in Table 31 below.
	// Please take care not to write other values to this register as doing so may cause the DW1000 to malfunction.
    if (dw1000_pacSize == DW1000_PAC_SIZE_8) dw1000_writeUInt32(DW1000_DRX_CONF, DW1000_DRX_TUNE2_OFFSET, dw1000_digital_bb_config[dw1000_pulseFrequency - DW1000_TX_PULSE_FREQ_16MHZ][0]);
    else if (dw1000_pacSize == DW1000_PAC_SIZE_16) dw1000_writeUInt32(DW1000_DRX_CONF, DW1000_DRX_TUNE2_OFFSET, dw1000_digital_bb_config[dw1000_pulseFrequency - DW1000_TX_PULSE_FREQ_16MHZ][1]);
    else if (dw1000_pacSize == DW1000_PAC_SIZE_32) dw1000_writeUInt32(DW1000_DRX_CONF, DW1000_DRX_TUNE2_OFFSET, dw1000_digital_bb_config[dw1000_pulseFrequency - DW1000_TX_PULSE_FREQ_16MHZ][2]);
    else if (dw1000_pacSize == DW1000_PAC_SIZE_64) dw1000_writeUInt32(DW1000_DRX_CONF, DW1000_DRX_TUNE2_OFFSET, dw1000_digital_bb_config[dw1000_pulseFrequency - DW1000_TX_PULSE_FREQ_16MHZ][3]);

    // Digital receiver configuration, sub-register 0x20 is used to set the 16-bit SFD detection
    // timeout counter period, in units of preamble symbols.   The SFD detection timeout starts running as soon as
	// preamble is detected. If the SFD sequence is not detected before the timeout period expires then the
	// timeout will act to abort the reception currently in progress, and set RXSFDTO event status bit in Register
	// file: 0x0F â€“ System Event Status Register. SFD timeout events are also counted in Sub-Register 0x2F:10 â€“ SFD
	// Timeout Error Counter, assuming that counting is enabled by the EVC_EN bit in Sub-Register 0x2F:00 â€“ Event
	// Counter Control.
    dw1000_writeUInt16(DW1000_DRX_CONF, DW1000_DRX_SFDTOC_OFFSET, timeoutOfSFD);

    // AGC configuration and control, sub-register 0x04 is a 16-bit tuning register for the AGC.
    // The value here needs to change depending on the PRF. The values needed depending on the RXPRF
    // configuration are given in Table 22 below. Please take care not to write other values to this register as doing
    // so may cause the DW1000 to malfunction.
    // Tabelle 22
	if (dw1000_pulseFrequency == DW1000_TX_PULSE_FREQ_64MHZ) dw1000_writeUInt16(DW1000_AGC_CTRL, DW1000_AGC_TUNE1_OFFSET, 0x889B);
	else dw1000_writeUInt16(DW1000_AGC_CTRL, DW1000_AGC_TUNE1_OFFSET, 0x8870);

	// AGC configuration and control, sub-register 0x0C is a 32-bit tuning register for the AGC.
	// The default value of this register needs to be reconfigured for optimum operation of the AGC. Please ensure
	// to program it to the value given in Table 23 below. Please take care not to write other values to this register
	// as doing so may cause the DW1000 to malfunction.
	// Tabelle 23
	dw1000_writeUInt32(DW1000_AGC_CTRL, DW1000_AGC_TUNE2_OFFSET, 0X2502A907);

	// AGC configuration and control, sub-register 0x12 is a 16-bit tuning register for the AGC.
	// The default value of this register needs to be reconfigured for optimum operation of the AGC. Please ensure
	// to program it to the value given in Table 24 below.  Please take care not to write other values to this register
	// as doing so may cause the DW1000 to malfunction.
	// Tabelle 24
	dw1000_writeUInt16(DW1000_AGC_CTRL, DW1000_AGC_TUNE3_OFFSET, 0x0035);

	// Lege RX und TX Kanal fest
	dw1000_chan_ctrl &= DW1000_CHAN_CTRL_REMOVE_CHANNELS;
	dw1000_chan_ctrl |= (dw1000_channel);		// TX
	dw1000_chan_ctrl |= (dw1000_channel << 4);	// RX

	// Lege die Pulsfrequenz fest
	dw1000_chan_ctrl &= DW1000_CHAN_CTRL_REMOVE_PRF;
	dw1000_chan_ctrl |= (((unsigned long int)dw1000_pulseFrequency) << 18);

	// Lege den RX und TX Preambelcode fest
	dw1000_chan_ctrl &= DW1000_CHAN_CTRL_REMOVE_PCODE;
	dw1000_chan_ctrl |= ((((unsigned long int)dw1000_preambleCode) & 0x1f) << 22);
	dw1000_chan_ctrl |= ((((unsigned long int)dw1000_preambleCode) & 0x1f) << 27);

    // Konfiguriere die TX Preambelgröße, die TX Pulsfrequenz, das TX Ranging Bit und die Datenrate
    dw1000_tx_fctrl &= DW1000_TX_FCTRL_RESET_TXBR;
    dw1000_tx_fctrl &= DW1000_TX_FCTRL_RESET_TXPRF;
    dw1000_tx_fctrl &= DW1000_TX_FCTRL_RESET_TXPE;
    dw1000_tx_fctrl &= DW1000_TX_FCTRL_RESET_TXPSR;
    dw1000_tx_fctrl |= DW1000_TX_FCTRL_TR;
    dw1000_tx_fctrl |= ((((unsigned long int)dw1000_dataRate) << DW1000_TX_FCTRL_SHIFT_TXBR) & DW1000_TX_FCTRL_MASK_TXBR);
    dw1000_tx_fctrl |= ((((unsigned long int)dw1000_pulseFrequency) << DW1000_TX_FCTRL_SHIFT_TXPRF) & DW1000_TX_FCTRL_MASK_TXPRF);
    dw1000_tx_fctrl |= ((((unsigned long int)dw1000_preambleLength) << DW1000_TX_FCTRL_SHIFT_PLEN) & DW1000_TX_FCTRL_MASK_PLEN);

    // Lege die Systemkonfiguration fest
    dw1000_sys_cfg = DW1000_SYS_CFG_HIRQ_POL;
    dw1000_sys_cfg = (dw1000_smartPower) ? dw1000_sys_cfg & ~DW1000_SYS_CFG_DIS_STXP : dw1000_sys_cfg | DW1000_SYS_CFG_DIS_STXP;
    dw1000_sys_cfg = (dw1000_doublebuffered) ? dw1000_sys_cfg & ~DW1000_SYS_CFG_DIS_DRXB : dw1000_sys_cfg | DW1000_SYS_CFG_DIS_DRXB;
    dw1000_sys_cfg = (dw1000_receiverAutoReenable) ? dw1000_sys_cfg | DW1000_SYS_CFG_RXAUTR : dw1000_sys_cfg & ~DW1000_SYS_CFG_RXAUTR;
    dw1000_sys_cfg = (dw1000_extendedFrameLength) ? dw1000_sys_cfg | DW1000_SYS_CFG_LONGFRAME : dw1000_sys_cfg & ~DW1000_SYS_CFG_LONGFRAME;
	if (dw1000_frameCheck) dw1000_sys_cfg &= ~DW1000_SYS_CFG_DIS_FCE;
	else dw1000_sys_cfg |= DW1000_SYS_CFG_DIS_FCE;

    // Lege die Systemkonfiguration fest (Framefilterung)
    															dw1000_sys_cfg &= DW1000_SYS_CFG_NOFRAMEFILTER;
	if (dw1000_framefilter) 									dw1000_sys_cfg |= DW1000_SYS_CFG_FFEN;
	if (dw1000_framefilter & DW1000_FRAMEFILTER_COORDINATOR)	dw1000_sys_cfg |= DW1000_SYS_CFG_FFBC;
	if (dw1000_framefilter & DW1000_FRAMEFILTER_ALLOW_BEACON)	dw1000_sys_cfg |= DW1000_SYS_CFG_FFAB;
	if (dw1000_framefilter & DW1000_FRAMEFILTER_ALLOW_DATA)		dw1000_sys_cfg |= DW1000_SYS_CFG_FFAD;
	if (dw1000_framefilter & DW1000_FRAMEFILTER_ALLOW_ACK)		dw1000_sys_cfg |= DW1000_SYS_CFG_FFAA;
	if (dw1000_framefilter & DW1000_FRAMEFILTER_ALLOW_MAC)		dw1000_sys_cfg |= DW1000_SYS_CFG_FFAM;
	if (dw1000_framefilter & DW1000_FRAMEFILTER_ALLOW_RESERVED)	dw1000_sys_cfg |= DW1000_SYS_CFG_FFAR;
	if (dw1000_framefilter & DW1000_FRAMEFILTER_ALLOW_TYPE4)	dw1000_sys_cfg |= DW1000_SYS_CFG_FFA4;
	if (dw1000_framefilter & DW1000_FRAMEFILTER_ALLOW_TYPE5)	dw1000_sys_cfg |= DW1000_SYS_CFG_FFA5;

	// Schreibe die lokale Konfiguration auf das Modul
	dw1000_writeUInt32(DW1000_TX_FCTRL, 0, dw1000_tx_fctrl);
	dw1000_writeUInt32(DW1000_SYS_CFG, 0, dw1000_sys_cfg);
	dw1000_writeUInt32(DW1000_CHAN_CTRL, 0, dw1000_chan_ctrl);

	// Lege die Antennendelaywerte fest
	dw1000_writeUInt16(DW1000_LDE_IF, DW1000_LDE_RXANTD_OFFSET, (unsigned short)dw1000_antennadelay_rx) ;
	dw1000_writeUInt16(DW1000_TX_ANTD, 0, (unsigned short)dw1000_antennadelay_tx) ;

	// Aktiviere alle Takte
	//dw1000_setClock(DW1000_PLL_ALL_CLOCK);

	// Deaktiviere AGC
	//dw1000_writeUInt16(DW1000_AGC_CTRL, DW1000_AGC_TUNE1_OFFSET, 0);
	//dw1000_writeUInt32(DW1000_AGC_CTRL, DW1000_AGC_TUNE2_OFFSET, 0);
	//dw1000_writeUInt16(DW1000_AGC_CTRL, DW1000_AGC_TUNE3_OFFSET, 0);

	// Device is initializing
	dw1000_isinitializeing = 0;


}



//! Liest die Chip-ID, bestehend aus Revision, Version, Model und einem konstanten 0xdeca0130 Wert
/*  \returns Gerätekennung [0xdeca (16-31); MODEL (8-15); VER (4-7); REV (0-3)] */
DW1000_OPTIMIZE unsigned int dw1000_getDeviceId(void) { return dw1000_readUInt32(DW1000_DEV_ID, 0); }


//! Aktiviert oder deaktiviert die Smart Power Funktionalität (nur bei 6.8MBit)
/*! Smart TX power control applies at the 6.8 Mbps data rate. When sending short data frames at this rate (and providing that the frame transmission rate is < 1 frame per millisecond) it is possible to increase the transmit power and still remain within regulatory power limits which are typically specified as average power per millisecond.
 *  \param val aktiv:1; inaktiv:0 */
DW1000_OPTIMIZE void dw1000_setSmartPower(unsigned char val) { if (dw1000_isinitializeing) dw1000_smartPower = val; }


//! Gibt aus, ob die Smart Power Funktionalität aktiviert oder deaktiviert ist
/*! Smart TX power control applies at the 6.8 Mbps data rate. When sending short data frames at this rate (and providing that the frame transmission rate is < 1 frame per millisecond) it is possible to increase the transmit power and still remain within regulatory power limits which are typically specified as average power per millisecond.
 *  \returns 0 wenn Smart Power deaktiviert ist; ungleich 0 wenn Smart Power aktiv ist */
DW1000_OPTIMIZE unsigned char dw1000_getSmartPower(void) { return dw1000_smartPower; }


//! Legt die Flagmaske des Framefilters fest
/*! Frame filtering is a feature of the DW1000 IC that can parse the received data of a frame that complies with the MAC encoding standard, identifying the frame type and its destination address fields, match these against the ICâ€™s own address information, and only accept frames that pass the filtering rules.<br><br>
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
DW1000_OPTIMIZE void dw1000_setFrameFilter(unsigned char val) { if (dw1000_isinitializeing) dw1000_framefilter = val; }


//! Gibt das aktuelle Flagmaske des Framefilters aus
/*! Frame filtering is a feature of the DW1000 IC that can parse the received data of a frame that complies with the MAC encoding standard, identifying the frame type and its destination address fields, match these against the ICâ€™s own address information, and only accept frames that pass the filtering rules.<br><br>
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
DW1000_OPTIMIZE unsigned char dw1000_getFrameFilter(void) { return dw1000_framefilter; }


//! Aktiviert oder deaktiviert das Double Buffering
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  This DW1000 has a pair of receive buffers offering the capability to receive into one of the pair while the host system is reading previously received data from the other buffer of the pair. This is useful in a TDOA RTLS anchor node where it is desired to have the receiver on as much as possible to avoid missing any tag blink messages.
 *  \param val 0: deaktivieren; 1: aktivieren */
DW1000_OPTIMIZE void dw1000_setDoubleBuffering(unsigned char val) { if (dw1000_isinitializeing) dw1000_doublebuffered = val; }


//! Gibt aus, ob das Double Buffering aktiviert oder deaktiviert ist
/*! This DW1000 has a pair of receive buffers offering the capability to receive into one of the pair while the host system is reading previously received data from the other buffer of the pair. This is useful in a TDOA RTLS anchor node where it is desired to have the receiver on as much as possible to avoid missing any tag blink messages.
 *  \returns 0: deaktiviert; 1: aktiviert */
DW1000_OPTIMIZE unsigned char dw1000_getDoubleBuffering(unsigned char val) { return dw1000_doublebuffered; }


//! Aktiviert oder deaktiviert die automatische Aktivierung des Empfängers
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  \param val 0: deaktivieren; 1: aktivieren */
DW1000_OPTIMIZE void dw1000_setReceiverAutoReenable(unsigned char val) { if (dw1000_isinitializeing) dw1000_receiverAutoReenable = val; }


//! Gibt aus, ob die automatische Aktivierung des Empfängers aktiviert oder deaktiviert ist
/*! \returns 0: deaktiviert; 1: aktiviert */
DW1000_OPTIMIZE unsigned char dw1000_getReceiyverAutoReenable(void) { return dw1000_receiverAutoReenable; }


//! Aktiviert oder deaktiviert das automatische Empfangen direkt nach dem Senden
/*! \param val 0: deaktivieren; 1: aktivieren */
DW1000_OPTIMIZE void dw1000_setReceiveAfterTX(unsigned char val) { dw1000_receiveAfterTX = val; }


//! Gibt aus, ob die nach dem Senden automatisch empfangen werden soll
/*! \returns 0: deaktiviert; 1: aktiviert */
DW1000_OPTIMIZE unsigned char dw1000_getReceiveAfterTX(void) { return dw1000_receiveAfterTX; }


//! Aktiviere oder deaktiviere die erweiterte Framegröße (1023 statt 127 Bytes)
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  \param val 0: deaktivieren; 1: aktivieren */
DW1000_OPTIMIZE void dw1000_setExtendedFrameLength(unsigned char val) { if (dw1000_isinitializeing) dw1000_extendedFrameLength = val; }


//! Gibt aus, ob die erweiterte Framegröße (1023 statt 127 Bytes) aktiviert oder deaktiviert ist
/*! \returns 0: deaktiviert; 1: aktiviert */
DW1000_OPTIMIZE unsigned char dw1000_getExtendedFrameLength(void) { return dw1000_extendedFrameLength; }


//! Legt die Datenrate fest
/*! Diese Einstellung kann nur im Konfigurationsmodus festgelegt werden<br><br>
 *  Mögliche Werte sind:
 *		<ul>
 *			<li>DW1000_TRX_RATE_110KBPS: 110kb/sek
 *			<li>DW1000_TRX_RATE_850KBPS: 850kb/sek
 *			<li>DW1000_TRX_RATE_6800KBPS: 6.8Mb/sek
 *		</ul>
 *  \param rate Einzustellende Datenrate */
DW1000_OPTIMIZE void dw1000_setDataRate(unsigned char rate) { if (dw1000_isinitializeing) dw1000_dataRate = rate; }


//! Gibt die aktuelle Datenrate aus
/*! Mögliche Werte sind:
 *		<ul>
 *			<li>DW1000_TRX_RATE_110KBPS: 110kb/sek
 *			<li>DW1000_TRX_RATE_850KBPS: 850kb/sek
 *			<li>DW1000_TRX_RATE_6800KBPS: 6.8Mb/sek
 *		</ul>
 *  \returns Aktuelle Datenrate */
DW1000_OPTIMIZE unsigned char dw1000_getDataRate(void) { return dw1000_dataRate; }


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
 *  timestamp accuracy) (see DWM1000 User Manual, section 9.3).
 *  \param freq Einzustellende Pulsfrequenz */
DW1000_OPTIMIZE void dw1000_setPulseFrequency(unsigned char freq) { if (dw1000_isinitializeing) dw1000_pulseFrequency = freq; }

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
DW1000_OPTIMIZE unsigned char dw1000_getPulseFrequency(void) { return dw1000_pulseFrequency; }


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
DW1000_OPTIMIZE void dw1000_setPreambleLength(unsigned char prealen) { if (dw1000_isinitializeing) dw1000_preambleLength = prealen; }


//! Gibt die aktuelle Preamblelänge des Moduls aus
/*! Mögliche Werte sind: (C++)
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
DW1000_OPTIMIZE unsigned char dw1000_getPreambleLength(void) { return dw1000_preambleLength; }


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
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_109
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_11
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_12
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_17
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_18
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_19
 *		<li>DW1000_PREAMBLE_CODE_64MHZ_20
 *	</ul>
 *	\param prealen Einzustellende Preambelkodierung (Achtung! Sollte die Kodierung inkompatibel zu den restlichen Einstellungen sein, wird sie auf einen Standardwert gesetzt) */
DW1000_OPTIMIZE void dw1000_setPreambleCode(unsigned char preacode) { if (dw1000_isinitializeing) dw1000_preambleCode = preacode; }


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
DW1000_OPTIMIZE unsigned char dw1000_getPreambleCode(void) { return dw1000_preambleCode; }


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
 *	\Returns Aktueller LDO-Parameter */
DW1000_OPTIMIZE unsigned char dw1000_getLdoParam(void) { return dw1000_ldoparameter; }


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
DW1000_OPTIMIZE void dw1000_setChannel(unsigned char newchannel) { if (dw1000_isinitializeing) dw1000_channel = newchannel; }


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
DW1000_OPTIMIZE unsigned char dw1000_getChannel(void) { return dw1000_channel; }


//! Legt die Verzögerung zwischen Funkchip und Antenne fest
/*! \param delay Antennenverzögerung (in Systemtakten - ca. 15ps) */
void dw1000_setAntennaDelay(unsigned short rxdelay, unsigned short txdelay)
{
	if (dw1000_antennadelay_rx != rxdelay || dw1000_antennadelay_tx != txdelay)
	{
		// Lege die neue Antennenverzögerung fest
		dw1000_antennadelay_rx = rxdelay;
		dw1000_antennadelay_tx = txdelay;

		// Lege die Antennendelaywerte fest
		if (!dw1000_isinitializeing)
		{
			dw1000_writeUInt16(DW1000_LDE_IF, DW1000_LDE_RXANTD_OFFSET,(unsigned short)rxdelay) ;
			dw1000_writeUInt16(DW1000_TX_ANTD, 0, (unsigned short)txdelay) ;
		}
	}
}


//! Gibt die aktuelle Sendeleistung aus
/*! \returns Sendeleistung in dBm */
DW1000_OPTIMIZE float dw1000_getTxPower()
{
	// Lese die aktuelle TX-Leistung aus
	unsigned char txpowerbits = (unsigned char)(dw1000_readUInt32(DW1000_TX_POWER, 0) & 0b00000000000000000000000011111111);
	float output = 0;
	switch(txpowerbits & 0b11100000)
	{
		case 0b00000000: output = 18; break;
		case 0b00100000: output = 15; break;
		case 0b01000000: output = 12; break;
		case 0b01100000: output =  9; break;
		case 0b10000000: output =  6; break;
		case 0b10100000: output =  3; break;
		case 0b11000000: output =  0; break;
		case 0b11100000: output = -1; break;
	}
	return (output > 0) ? output + ((txpowerbits & 0b00011111) * 0.5f) : 0;
}


//! Legt die Sendeleistung fest
/*! \param power Sendeleistung in dBm */
DW1000_OPTIMIZE float dw1000_setTxPower(float power)
{
	// Stelle sicher, dass der Sendeleitstungs-Wert korrekt ist
	if (power < 0) power = 0;
	else if (power > 33.5) power = 33.5;

	// Berechne die COARSE-Flags
	unsigned int power_coarse = (unsigned int)(power / 3);
	if (power_coarse > 6) power_coarse = 6;
	unsigned char power_coarse_flags = (unsigned char)((0b00000110 - power_coarse) << 5);

	// Berechne die FINE-Flags
	power_coarse_flags |= (unsigned char)((power - (power_coarse * 3)) * 2);

    // Schreibe den Power-Wert in das DW1000-Register
    dw1000_writeUInt32(DW1000_TX_POWER, 0, power_coarse_flags | ((unsigned int)power_coarse_flags << 8) | ((unsigned int)power_coarse_flags << 16) | ((unsigned int)power_coarse_flags << 24));

    // Gebe die TX-Leistung aus
    return dw1000_getTxPower();
}

//! Lädt die LDO-Kalibrierungsdaten, wenn notwendig
DW1000_OPTIMIZE void dw1000_loadLDOCalibrationData()
{
	// Lade die LDO-Kalibrierungsdaten, wenn notwendig
	if (((dw1000_readOTPUInt8(DW1000_LDOTUNE_ADDRESS)) & 0xff) != 0)
	{
		// Force LDE
		dw1000_writeUInt8(DW1000_OTP_IF, DW1000_OTP_SF, ((dw1000_ldoparameter & DW1000_OTP_SF_SEL_MASK) | DW1000_OTP_SF_LDO_KICK | DW1000_OTP_SF_OPS_KICK) & DW1000_OTP_SF_MASK);
	}
}


//! Liest den XTAL Trimmungswert
DW1000_OPTIMIZE void dw1000_loadXTalTrimming()
{
	unsigned char xtrim = dw1000_readOTPUInt8(DW1000_XTRIM_ADDRESS) & 0x1f;
	if (xtrim == 0) xtrim = dw1000_clockoffset;
	else dw1000_clockoffset = xtrim;
	unsigned char xtalt = 0b01100000 | (xtrim & DW1000_FS_XTALT_MASK);
	dw1000_writeUInt8(DW1000_FS_CTRL, DW1000_FS_XTALT_OFFSET, xtalt);
}


//! Lädt den leading edge Code in den Geräteram
DW1000_OPTIMIZE void dw1000_loadLeadingEdgeAlgorithm()
{
	// Lade den LDE-Code und warte, bis das Schreiben vollständig ist
	dw1000_writeUInt16(DW1000_OTP_IF, DW1000_OTP_CTRL, DW1000_OTP_CTRL_LDELOAD); // set load LDE kick bit
	while((dw1000_readUInt16(DW1000_OTP_IF, DW1000_OTP_CTRL) & DW1000_OTP_CTRL_LDELOAD)) { asm(""); }
}

unsigned char dw1000_getClockOffset()
{
	return dw1000_clockoffset;
}

unsigned char dw1000_setClockOffset(unsigned char offset)
{
	if (offset > DW1000_FS_XTALT_MASK) offset = DW1000_FS_XTALT_MASK;
	dw1000_clockoffset = offset;
	if (!dw1000_isinitializeing) dw1000_loadXTalTrimming();
	return dw1000_clockoffset;
}

