#include "dw1000_driver.h"
#include <string.h>
#include "dw1000_user.h"

unsigned char dw1000_tempbuffer[4096 + 128] = {0};	//!< Lese- und Schreibpuffer



//! Liest eine vorzeichenlose 32Bit Zahl aus dem angegebenen Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \returns Gelesener Registerinhalt */
DW1000_OPTIMIZE DW1000_INLINE unsigned int dw1000_readUInt32(unsigned char address, unsigned short offset)
{
	volatile unsigned int buffer = 0;
	dw1000_read(address, 4, (unsigned char*)&buffer, offset);
	return buffer;
}


//! Liest eine vorzeichenlose 16Bit Zahl aus dem angegebenen Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \returns Gelesener Registerinhalt */
DW1000_OPTIMIZE DW1000_INLINE unsigned short dw1000_readUInt16(unsigned char address, unsigned short offset)
{
	unsigned short buffer = 0;
	dw1000_read(address, 2, (unsigned char*)&buffer, offset);
	return buffer;
}


//! Liest eine vorzeichenlose 8Bit Zahl aus dem angegebenen Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \returns Gelesener Registerinhalt */
DW1000_OPTIMIZE DW1000_INLINE unsigned char dw1000_readUInt8(unsigned char address, unsigned short offset)
{
	volatile unsigned char buffer = 0;
	dw1000_read(address, 1, (unsigned char*)&buffer, offset);
	return buffer;
}


//! Schreibt einen 32Bit Wert in das angegebene Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \param value Zu schreibender Wert */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_writeUInt32(unsigned char address, unsigned short offset, unsigned int value) { dw1000_write(address, 4, (unsigned char*)&value, offset); }


//! Schreibt einen 16Bit Wert in das angegebene Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \param value Zu schreibender Wert */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_writeUInt16(unsigned char address, unsigned short offset, unsigned short value) { dw1000_write(address, 2, (unsigned char*)&value, offset); }


//! Schreibt einen 8Bit Wert in das angegebene Register
/*! \param address Registeradresse
 *  \param offset Registeroffset (siehe Doku)
 *  \param value Zu schreibender Wert */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_writeUInt8(unsigned char address, unsigned short offset, unsigned char value) { dw1000_write(address, 1, (unsigned char*)&value, offset); }


//! Liest einen 8Bit-Wert aus dem OTP-Speicher
/*! \param address Registeradresse
 *  \returns Gelesener Registerinhalt */
unsigned char dw1000_readOTPUInt8(unsigned short address)
{
	unsigned char result;

	// Schreibe in das Adressregister
	dw1000_writeUInt16(DW1000_OTP_IF, DW1000_OTP_ADDR, address);

	// Assert OTP Read (self clearing)
	dw1000_writeUInt8(DW1000_OTP_IF, DW1000_OTP_CTRL, 0x03);	// Force OTP read, Lese angegebene Adresse
	dw1000_writeUInt8(DW1000_OTP_IF, DW1000_OTP_CTRL, 0x00);	// Force OTP read

	// Lese den Registerinhalt
	result = dw1000_readUInt8(DW1000_OTP_IF, DW1000_OTP_RDAT);

	// Gebe das Resultat aus
	return result;
}


//! Liest einen 16Bit-Wert aus dem OTP-Speicher
/*! \param address Registeradresse
 *  \returns Gelesener Registerinhalt */
DW1000_OPTIMIZE unsigned short dw1000_readOTPUInt16(unsigned short address)
{
	unsigned short result;

	// Schreibe in das Adressregister
	dw1000_writeUInt16(DW1000_OTP_IF, DW1000_OTP_ADDR, address);

	// Assert OTP Read (self clearing)
	dw1000_writeUInt8(DW1000_OTP_IF, DW1000_OTP_CTRL, 0x03);	// Force OTP read, Lese angegebene Adresse
	dw1000_writeUInt8(DW1000_OTP_IF, DW1000_OTP_CTRL, 0x00);	// Force OTP read

	// Lese den Registerinhalt
	result = dw1000_readUInt16(DW1000_OTP_IF, DW1000_OTP_RDAT);

	// Gebe das Resultat aus
	return result;
}


//! Liest einen 32Bit-Wert aus dem OTP-Speicher
/*! \param address Registeradresse
 *  \returns Gelesener Registerinhalt */
DW1000_OPTIMIZE unsigned int dw1000_readOTPUInt32(unsigned short address)
{
	unsigned int result;

	// Schreibe in das Adressregister
	dw1000_writeUInt16(DW1000_OTP_IF, DW1000_OTP_ADDR, address);

	// Assert OTP Read (self clearing)
	dw1000_writeUInt8(DW1000_OTP_IF, DW1000_OTP_CTRL, 0x03);	// Force OTP read, Lese angegebene Adresse
	dw1000_writeUInt8(DW1000_OTP_IF, DW1000_OTP_CTRL, 0x00);	// Force OTP read

	// Lese den Registerinhalt
	result = dw1000_readUInt32(DW1000_OTP_IF, DW1000_OTP_RDAT);

	// Gebe das Resultat aus
	return result;
}


//! Liest die angegebene Anzahl von Zeichen an der angegebenen Adresse und schreibt sie in den übergebenen Buffer
/*! \param address Registeradresse
 *  \param length Anzahl der zu lesenden Zeichen
 *  \param buffer Puffer, in den gelesen werden soll
 *  \param offset Registeroffset (siehe Doku.)
 *  \returns Anzahl der transferrierten Zeichen (inkl. Header) */
DW1000_OPTIMIZE DW1000_INLINE int dw1000_read(unsigned char address, unsigned short length, unsigned char* buffer, unsigned short offset)
{
	// Initialisiere den Datenheader
	volatile unsigned char header[3];

	// Initialisiere die Headerlänge
	volatile unsigned char headerlength = 1;

	// Prüfe, ob die angegebenen Parameter gültig sind
	if (address > 0x3F) return -1;
    if (offset > 0x7FFF) return -1;                     // index is limited to 15-bits.
    if ((offset + length)> 0x7FFF) return -1;           // sub-addressible area is limited to 15-bits.

	// Prüfe, ob ein Datenoffset erwünscht ist
	if (offset)
	{
		// Lese mit Subadresse (Offset)
		header[0] = DW1000_READ_SUB | address;
		if (offset < 128)
		{
			header[1] = (unsigned char)offset;
			headerlength = 2;
		}
		else
		{
			header[1] = (unsigned char)offset;
			header[2] = (unsigned char)(offset >> 7);
			headerlength = 3;
		}
	}
	else
	{
		// Definiere den Datenheader
		header[0] = DW1000_READ | address;
	}

	// Copy the header and buffer to the DMA
	memcpy((void*)dw1000_tempbuffer, (void*)header, (size_t)headerlength);

	// Lese die SPI und schreibe den Puffer in das Ausgabearray
	unsigned short returnvalue = dw1000_readWriteSpi(dw1000_tempbuffer, length + headerlength);
	memcpy((void*)buffer, (void*)&dw1000_tempbuffer[headerlength], (size_t)length);
	return returnvalue;
}


//! Schreibt die angegebene Anzahl von Zeichen aus dem übergebenen Buffer an die angegebene Adresse
/*! \param address Registeradresse
 *  \param length Anzahl der zu schreibenden Daten
 *  \param buffer Zu schreibende Daten
 *  \param offset Registeroffset (siehe Doku)
 *  \returns Anzahl der geschriebenen Zeichen (inkl. Header) */
DW1000_OPTIMIZE DW1000_INLINE int dw1000_write(unsigned char address, unsigned short length, unsigned char* buffer, unsigned short offset)
{
	// Initialisiere den Datenheader
	volatile unsigned char header[3];

	// Initialisiere die Headerlänge
	volatile unsigned char headerlength = 1;

	// Prüfe, ob ein Datenoffset erwünscht ist
	if (offset)
	{
		// Lese mit Subadresse (Offset)
		header[0] = DW1000_WRITE_SUB | address;
		if (offset < 128)
		{
			header[1] = (unsigned char)offset;
			headerlength = 2;
		}
		else
		{
			header[1] = (unsigned char)(DW1000_EXTENDEDINDEX | offset);
			header[2] = (unsigned char)(offset >> 7);
			headerlength = 3;
		}
	}
	else
	{
		// Definiere den Datenheader
		header[0] = DW1000_WRITE | address;
	}

	// Copy the header and buffer to the DMA
	memcpy(dw1000_tempbuffer, (unsigned char*)header, headerlength);
	memcpy(&(dw1000_tempbuffer[headerlength]), buffer, length);

	// Schreibe den Inhalt auf die SPI
	return dw1000_readWriteSpi(dw1000_tempbuffer, headerlength + length);
}


//! Schreibt die angegebene Anzahl von Zeichen aus dem übergebenen Buffern an die angegebene Adresse
/*! Die Methode sendet den Datenblock buffer1+buffer2 mit der Länge length1+length2 an die gegebene Registeradresse
 *  \param address Registeradresse
 *  \param length1 Länge des ersten Buffers
 *  \param buffer1 Erster Buffer
 *  \param length2 Länge des zweiten Buffers
 *  \param buffer2 Zweiter Buffer
 *  \returns Anzahl der geschriebenen Zeichen (inkl. Header) */
DW1000_OPTIMIZE DW1000_INLINE int dw1000_writeTwoBuffers(unsigned char address, unsigned short length1, unsigned char* buffer1, unsigned short length2, unsigned char* buffer2)
{
	// Initialisiere den Datenheader
	unsigned char header[3];

	// Initialisiere die Headerlänge
	unsigned char headerlength = 1;

	// Definiere den Datenheader
	header[0] = DW1000_WRITE | address;

	// Copy the header and buffer to the DMA
	memcpy(dw1000_tempbuffer, header, headerlength);
	memcpy(&dw1000_tempbuffer[headerlength], buffer1, length1);
	memcpy(&dw1000_tempbuffer[headerlength + length1], buffer2, length2);

	// Schreibe den Inhalt auf die SPI
	return dw1000_readWriteSpi(dw1000_tempbuffer, headerlength + length1 + length2);
}



//! Liest den Akkumulatorspeicher aus
/*! Achtung! Das erste Symbol wird ignoriert - Dokumentation lesen
 *  \param buffer Buffer, in den der Speicherinhalt geschrieben werden soll
 *  \param length Anzahl der gelesenen Zeichen (Achtung! das erste wird stets ignoriert)
 *  \returns Anzahl der gelesenen Zeichen */
DW1000_OPTIMIZE unsigned short dw1000_getAccMem(unsigned char* buffer, unsigned short length)
{
	if (buffer && length)
	{
		// Bereite den gewünschten Registerinhalt vor
		volatile unsigned char pmscctrl0_0;
		volatile unsigned char pmscctrl0_1;

		// Lese die aktuellen Taktungseinstellungen
		pmscctrl0_0 = dw1000_readUInt8(DW1000_PMSC, 0);
		pmscctrl0_1 = dw1000_readUInt8(DW1000_PMSC, 1);

		// Aktiviere das Lesen
		pmscctrl0_0 = 0x48 | (pmscctrl0_0 & 0xb3);
		pmscctrl0_1 = 0x80 | pmscctrl0_1;

		// Schreibe die geänderten Taktungseinstellungen
		dw1000_writeUInt8(DW1000_PMSC, 0, pmscctrl0_0);
		dw1000_writeUInt8(DW1000_PMSC, 1, pmscctrl0_1);

		// Lese den Akkumulatorspeicher ein
		dw1000_read(DW1000_ACC_MEM, length, buffer, 0);

		// Deaktiviere das Lesen
		pmscctrl0_0 = pmscctrl0_0 & 0xb3;
		pmscctrl0_1 = pmscctrl0_1 & 0x7f;

		// Schreibe die geänderten Taktungseinstellungen
		dw1000_writeUInt8(DW1000_PMSC, 0, pmscctrl0_0);
		dw1000_writeUInt8(DW1000_PMSC, 1, pmscctrl0_1);

		return length;
	}
	return 0;
}


//! Schreibt das angegebene Byte Array in den TX Buffer
/*! \param array Array, das zu übermitteln ist
 *  \param len Anzahl der Symbole aus dem Array, die zu übermitteln sind */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_writeByteArrayToTxBuffer(unsigned char* array, int len)
{
	if (len >= 0 && array)
	{
		// Beim automatischen Framecheck werden 2 Byte CRC-16 an den Frame gehängt
		if(dw1000_frameCheck) len+=2;

		// Prüfe, ob die Länge korrekt ist
		if(len > DW1000_MAXEXTENDEDPACKAGESIZE) return;
		if(len > DW1000_MAXPACKAGESIZE && !dw1000_extendedFrameLength) return;

		// Aktualisiere das TX Konfigurationsregister
		dw1000_tx_fctrl &= DW1000_TX_FCTRL_RESET_TFLEN;
		dw1000_tx_fctrl |= len;

		// Schreibe die Länge des TX Puffers in das TX Array
		dw1000_writeUInt16(DW1000_TX_FCTRL, 0, dw1000_tx_fctrl & 0xffff);

		// Schreibe das Datenarray in den Puffer
		if (dw1000_frameCheck) dw1000_write(DW1000_TX_BUFFER, len-2, array, 0);
		else dw1000_write(DW1000_TX_BUFFER, len, array, 0);
	}
}


//! Schreibt das angegebene Byte Array sowie Headerinformationen in den TX Buffer
/*! \param header Bytearray mit Headerinformationen
 *  \param headerlength Länge des Headers
 *  \param array Array, das zu übermitteln ist
 *  \param len Anzahl der Symbole aus dem Array, die zu übermitteln sind */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_writeByteArrayAndHeaderToTxBuffer(unsigned char* header, int headerlength, unsigned char* array, int arraylength)
{
	int len = headerlength + arraylength;
	if (len >= 0 && array)
	{
		// Beim automatischen Framecheck werden 2 BYte CRC-16 an den Frame gehängt
		if(dw1000_frameCheck) len+=2;

		// Prüfe, ob die Länge korrekt ist
		if(len > DW1000_MAXEXTENDEDPACKAGESIZE) return;
		if(len > DW1000_MAXPACKAGESIZE && !dw1000_extendedFrameLength) return;

		// Schreibe den TX Buffer
		dw1000_writeTwoBuffers(DW1000_TX_BUFFER, headerlength, header, arraylength, array);

		// Aktualisiere das TX Konfigurationsregister
		dw1000_tx_fctrl &= DW1000_TX_FCTRL_RESET_TFLEN;
		dw1000_tx_fctrl |= len;

		// Schreibe die Länge des TX Puffers in das TX Array
		dw1000_writeUInt16(DW1000_TX_FCTRL, 0, dw1000_tx_fctrl & 0xffff);
	}
}

//! Schreibt die gegebene Zeichenkette in den TX Buffer
/*! \param array Null-terminierte Zeichenkette, die kopiert werden soll */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_writeStringToTxBuffer(unsigned char* array) { dw1000_writeByteArrayToTxBuffer(array, strlen((char*)array) + 1); }


//! Schreibt die gegebene Zeichenkette sowie Headerinformationen in den TX Buffer
/*! \param header Bytearray mit Headerinformationen
 *  \param headerlength Länge des Headers
 *  \param array Null-terminierte Zeichenkette, die kopiert werden soll */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_writeStringAndHeaderToTxBuffer(unsigned char* header, int headerlength, unsigned char* array) { dw1000_writeByteArrayAndHeaderToTxBuffer(header, headerlength, array, strlen((char*)array) + 1); }


//! Liest den RX Buffer in das gegebene Byte Array
/*! \param string Array, in das der RX-Puffer geschrieben werden soll
 *  \param len Anzahl der Zeichen, die gelesen werden sollen */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_readRxBufferToByteArray(unsigned char* array, int len) {	if (len > 0) dw1000_read(DW1000_RX_BUFFER, len, array, 0); }


//! Gibt die Anzahl der Zeichen im RX Buffer oder TX Buffer (abhängig vom Modus) aus
/*! \returns Anzahl der Zeichen im RX-Buffer */
DW1000_OPTIMIZE DW1000_INLINE unsigned short dw1000_getRxTxBufferLen()
{
	unsigned short len = 0;

	// Prüfe, in welchem Modus sich das Gerät befindet
	if(dw1000_devicemode == DW1000_TX_MODE) len = (unsigned short)(dw1000_tx_fctrl & DW1000_TX_FCTRL_MASK_TFLEN);
	else
	{
		dw1000_read(DW1000_RX_FINFO, 2, (unsigned char*)&len, 0);
		len &= 0x03FF;
	}

	// Beim Frame Check werden 2 Byte CRC-16 angehängt. Ziehe diese von der Länge ab
	if(dw1000_frameCheck && len > 2) return len-2;

	// Gebe die Länge aus
	return len;
}
