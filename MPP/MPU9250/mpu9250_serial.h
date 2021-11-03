#ifndef MPU9250_SERIAL
#define MPU9250_SERIAL

//
int mpu9250_initserial();

int mpu9250_deinitserial();

void mpu9250_slowSPI();

void mpu9250_fastSPI();

// Schreibe die gegebenen Daten auf die SPI-Schnittstelle
int mpu9250_send(unsigned char subaddress, unsigned char* data, unsigned int datalength);

// Schreibt einen 8Bit-Wert ohne Vorzeichen an die SPI-Schnittstelle
int mpu9250_sendUInt8(unsigned char subaddress, unsigned char value);

// Empfängt die angegebene Anzahl von Symbolen
int mpu9250_receive(unsigned char subaddress, unsigned char* data, unsigned int datalength);

// Empfängt einen 8Bit-Wert ohne Vorzeichen von der SPI-Schnittstelle
unsigned char mpu9250_receiveUInt8(unsigned char subaddress);

#endif
