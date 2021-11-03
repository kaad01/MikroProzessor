#include "mpu9250_driver.h"
#include "mpu9250_def.h"
#include "mpu9250_serial.h"
#include "global.h"



// Hier findet die Zusammenstellung der modulspezifischen Parameter statt,
// die sich aus den Port/Pin/Spi-Angaben in der mpu9250.h ergeben
// ----------------------------------------------------------------------------------------------
#define MPU9250_SPI 					CONCAT2(SPI, MPU9250_SPI_NUMBER)
#define MPU9250_SPI_AF					CONCAT2(GPIO_AF_SPI, MPU9250_SPI_NUMBER)
#define MPU9250_SPI_CS_GPIOPORT			CONCAT2(GPIO, MPU9250_PORT_CS)
#define MPU9250_SPI_CS_GPIOPIN			CONCAT2(GPIO_Pin_, MPU9250_PIN_CS)
#define MPU9250_SPI_CLK_GPIOPORT		CONCAT2(GPIO, MPU9250_PORT_CLK)
#define MPU9250_SPI_CLK_GPIOPIN			CONCAT2(GPIO_Pin_, MPU9250_PIN_CLK)
#define MPU9250_SPI_CLK_GPIOPINSOURCE	CONCAT2(GPIO_PinSource, MPU9250_PIN_CLK)
#define MPU9250_SPI_MISO_GPIOPORT		CONCAT2(GPIO, MPU9250_PORT_MISO)
#define MPU9250_SPI_MISO_GPIOPIN		CONCAT2(GPIO_Pin_, MPU9250_PIN_MISO)
#define MPU9250_SPI_MISO_GPIOPINSOURCE	CONCAT2(GPIO_PinSource, MPU9250_PIN_MISO)
#define MPU9250_SPI_MOSI_GPIOPORT		CONCAT2(GPIO, MPU9250_PORT_MOSI)
#define MPU9250_SPI_MOSI_GPIOPIN		CONCAT2(GPIO_Pin_, MPU9250_PIN_MOSI)
#define MPU9250_SPI_MOSI_GPIOPINSOURCE	CONCAT2(GPIO_PinSource, MPU9250_PIN_MOSI)
#define MPU9250_IRQ_GPIOPORT			CONCAT2(GPIO, MPU9250_PORT_IRQ)
#define MPU9250_IRQ_GPIOPIN				CONCAT2(GPIO_Pin_, MPU9250_PIN_IRQ)
#define MPU9250_IRQ_EXTIPORT			CONCAT2(EXTI_PortSourceGPIO, MPU9250_PORT_IRQ)
#define MPU9250_IRQ_EXTIPIN				CONCAT2(EXTI_PinSource, MPU9250_PIN_IRQ)
#define MPU9250_IRQ_EXTILINE			CONCAT2(EXTI_Line, MPU9250_PIN_IRQ)
#define MPU9250_CS_LOW 					MPU9250_SPI_CS_GPIOPORT->BSRRH = MPU9250_SPI_CS_GPIOPIN;
#define MPU9250_CS_HIGH 				MPU9250_SPI_CS_GPIOPORT->BSRRL = MPU9250_SPI_CS_GPIOPIN;


unsigned char
	mpu9250_isfastspi = 0;

// Lege die SPI-Teiler für Konfiguration und Lesen der Daten fest
const uint16_t
	mpu9250_prescaler_config = SPI_BaudRatePrescaler_32,
	mpu9250_prescaler_run = SPI_BaudRatePrescaler_8;


//
int mpu9250_initserial()
{
	// Initialisierungsstruktur für GPIO
	GPIO_InitTypeDef GPIO_InitStructure;

	// Initialisierungsstruktur für die SPI
	SPI_InitTypeDef SPI_InitStructure;

	// Initialisierungsstrukturen für die GPIO Interrupts
	//NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	// Clocksystem einschalten
	MPU9250_PERIPHEAL_ENABLE;

	// Stop the SPI clock
	SPI_I2S_DeInit(MPU9250_SPI);

	// Initialisiere Chipselect
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_SPI_CS_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(MPU9250_SPI_CS_GPIOPORT, &GPIO_InitStructure);
	GPIO_SetBits(MPU9250_SPI_CS_GPIOPORT, MPU9250_SPI_CS_GPIOPIN);

	// Initialisiere CLK
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_SPI_CLK_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(MPU9250_SPI_CLK_GPIOPORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(MPU9250_SPI_CLK_GPIOPORT, MPU9250_SPI_CLK_GPIOPINSOURCE, MPU9250_SPI_AF);

	// Initialisiere MOSI
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_SPI_MOSI_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(MPU9250_SPI_MOSI_GPIOPORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(MPU9250_SPI_MOSI_GPIOPORT, MPU9250_SPI_MOSI_GPIOPINSOURCE, MPU9250_SPI_AF);

	// Initialisiere MISO
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_SPI_MISO_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(MPU9250_SPI_MISO_GPIOPORT, &GPIO_InitStructure);
	GPIO_PinAFConfig(MPU9250_SPI_MISO_GPIOPORT, MPU9250_SPI_MISO_GPIOPINSOURCE, MPU9250_SPI_AF);

	// Initialisiere IRQ
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_IRQ_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(MPU9250_IRQ_GPIOPORT, &GPIO_InitStructure);

	//Initialisiere SPI
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(MPU9250_SPI, &SPI_InitStructure);

	// IRQ verwendet Interrupt auf Line 1
	SYSCFG_EXTILineConfig(MPU9250_IRQ_EXTIPORT, MPU9250_IRQ_EXTIPIN);

	// Initialisiere die externen Interrupts
	EXTI_InitStructure.EXTI_Line = MPU9250_IRQ_EXTILINE;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);

	// Initialisiere und aktiviere den Interrupt für die IRQ Leitung
//	NVIC_InitStructure.NVIC_IRQChannel = MPU9250_IRQ_EXTICHANNEL;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	// Aktiviere die SPI
	SPI_Cmd(MPU9250_SPI, ENABLE);

	// Aktiviere die Interrupts für die IRQ Leitung
	// NVIC_Init(&NVIC_InitStructure);

	// Abgeschlossen
	return 1;
}

int mpu9250_deinitserial()
{
	// Initialisierungsstruktur für GPIO
	GPIO_InitTypeDef GPIO_InitStructure;

	// Stop the SPI clock
	SPI_I2S_DeInit(MPU9250_SPI);

	// Initialisiere Chipselect
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_SPI_CS_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(MPU9250_SPI_CS_GPIOPORT, &GPIO_InitStructure);

	// Initialisiere CLK
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_SPI_CLK_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(MPU9250_SPI_CLK_GPIOPORT, &GPIO_InitStructure);

	// Initialisiere MOSI
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_SPI_MOSI_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(MPU9250_SPI_MOSI_GPIOPORT, &GPIO_InitStructure);

	// Initialisiere MISO
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_SPI_MISO_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(MPU9250_SPI_MISO_GPIOPORT, &GPIO_InitStructure);

	// Initialisiere IRQ
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = MPU9250_IRQ_GPIOPIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(MPU9250_IRQ_GPIOPORT, &GPIO_InitStructure);

	// Aktiviere die SPI
	SPI_Cmd(MPU9250_SPI, DISABLE);

	// Abgeschlossen
	return 1;
}

O3 INLINE void mpu9250_slowSPI()
{
	if (mpu9250_isfastspi)
	{
		SPI_InitTypeDef SPI_InitStructure;
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
		SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
		SPI_InitStructure.SPI_BaudRatePrescaler = mpu9250_prescaler_config;
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
		SPI_InitStructure.SPI_CRCPolynomial = 7;
		SPI_Init(MPU9250_SPI, &SPI_InitStructure);
		mpu9250_isfastspi = 0;
	}
}

O3 INLINE void mpu9250_fastSPI()
{
	if (!mpu9250_isfastspi/* && mpu9250_usefifo*/)
	{
		SPI_InitTypeDef SPI_InitStructure;
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
		SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
		SPI_InitStructure.SPI_BaudRatePrescaler = mpu9250_prescaler_run;
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
		SPI_InitStructure.SPI_CRCPolynomial = 7;
		SPI_Init(MPU9250_SPI, &SPI_InitStructure);
		mpu9250_isfastspi = 1;
	}
}

// Schreibe die gegebenen Daten auf die I2C Schnittstelle
O3 INLINE int mpu9250_send(unsigned char subaddress, unsigned char* data, unsigned int datalength)
{
	// Temporäre Variable
	volatile unsigned char temp = 0;
	temp = temp;

	// Zeigerposition im aktuellen SPI Transfer
	volatile unsigned char transferpos = 0;

	// SPI Transfer durch Chipselect-PIN (low) einleiten
	MPU9250_CS_LOW;

	// Übermittle den Header Byte für Byte im Polling-Modus
	MPU9250_SPI->DR = subaddress;
	while( !(MPU9250_SPI->SR & SPI_I2S_FLAG_TXE)) asm("");
	while( !(MPU9250_SPI->SR & SPI_I2S_FLAG_RXNE)) asm("");
	temp = MPU9250_SPI->DR;

	// Lese die Daten Byte für Byte im Polling Modus
	for (transferpos=0; transferpos<datalength; transferpos++)
	{
		MPU9250_SPI->DR = data[transferpos];
		while( !(MPU9250_SPI->SR & SPI_I2S_FLAG_TXE)) asm("");
		while( !(MPU9250_SPI->SR & SPI_I2S_FLAG_RXNE)) asm("");
		temp = MPU9250_SPI->DR;
	}

	// SPI Transfer durch Chipselect-PIN (high) abschließen
	MPU9250_CS_HIGH;

	// Warte 50us
	delay_us(50);

	// Gebe die Datenlänge zurück
	return datalength;
}

O3 INLINE int mpu9250_sendUInt8(unsigned char subaddress, unsigned char value)
{
	return mpu9250_send(subaddress, &value, 1);
}

// Empfängt die angegebene Anzahl von Symbolen
O3 INLINE int mpu9250_receive(unsigned char subaddress, unsigned char* data, unsigned int datalength)
{
	// Temporäre Variable
	volatile unsigned char temp = 0;
	temp = temp;

	// Zeigerposition im aktuellen SPI Transfer
	volatile unsigned char transferpos = 0;

	// SPI Transfer durch Chipselect-PIN (low) einleiten
	MPU9250_CS_LOW;

	// Übermittle den Header Byte für Byte im Polling-Modus
	MPU9250_SPI->DR = (subaddress | 0b10000000);
	while(!(MPU9250_SPI->SR & SPI_I2S_FLAG_TXE)) __asm__("");
	while(!(MPU9250_SPI->SR & SPI_I2S_FLAG_RXNE)) __asm__("");
	temp = MPU9250_SPI->DR;

	// Lese die Daten Byte für Byte im Polling Modus
	// TODO: Verwende DMA
	for (transferpos=0; transferpos<datalength; transferpos++)
	{
		MPU9250_SPI->DR = 0;
		while(!(MPU9250_SPI->SR & SPI_I2S_FLAG_TXE)) __asm__("");
		while(!(MPU9250_SPI->SR & SPI_I2S_FLAG_RXNE)) __asm__("");
		data[transferpos] = MPU9250_SPI->DR;
	}

	// SPI Transfer durch Chipselect-PIN (high) abschließen
	MPU9250_CS_HIGH;

	// Gebe die Datenlänge zurück
	return datalength;
}

O3 INLINE unsigned char mpu9250_receiveUInt8(unsigned char subaddress)
{
	unsigned char output = 0;
	mpu9250_receive(subaddress, &output, 1);
	return output;
}
