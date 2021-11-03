#include "dw1000.h"
#include "dw1000_user.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
#include "global.h"
#include "usart.h"

//! Kann deaktiviert werden, um ohne DMA zu kommunizieren
#define DW1000_DMA (1)

//! Chipselect-Leitung auf LOW ziehen (muss an das System angepasst werden)
#define DW1000_CS_LOW GPIOB->BSRRH = GPIO_Pin_1

//! Chipselect-Leitung auf HIGH ziehen (muss an das System angepasst werden)
#define DW1000_CS_HIGH GPIOB->BSRRL = GPIO_Pin_1

// Initialisierungsstrukturen für die GPIO Interrupts
NVIC_InitTypeDef dw1000_nvic_structure;				//!< Enthält alle notwendigen Angaben zur Einrichtung eines Pins
EXTI_InitTypeDef dw1000_exti_structure;				//!< Enthält alle notwendigen Angaben zur Einrichtung eines Interrupts
SPI_InitTypeDef dw1000_spi_structure;				//!< Enthält alle notwendigen Angaben zur Einrichtung der SPI-Schnittstelle

unsigned char dw1000_isresetpinoutput = 0;			//!< Ist 1, wenn die Reset-Leitung auf Ausgang geschaltet ist


void disableIRQ()
{
	__disable_irq();
}

void enableIRQ()
{
	__enable_irq();
}

void delay__ms(unsigned long interval)
{
	interval *= 21;
	interval = interval * 1000;
    while(interval--) { __NOP(); }
}

//! Wartet die angegebene Zeit an Millisekunden (gilt aber nur bei 168MHz Taktfrequenz)
DW1000_OPTIMIZE DW1000_INLINE void dw1000_delay_mSek(unsigned short ms) { delay__ms(ms); }

//! Diese Methode gibt die Anzahl der Millisekunden aus, die seit Systemstart vergangen sind
DW1000_OPTIMIZE DW1000_INLINE unsigned long int dw1000_getTickCounter() { return systickcounter; }


//! Methode zur Initialisierung der Hardwareschnittstellen zum DW1000 Chip
/*!	Vor der Initialisierung des Chips, wird diese Funktion zur Initialisierung der Hardware aufgerufen.<br>
 *  Sie muss implementiert werden, damit alle Leitungen und Bussysteme für die spätere Verwendung bereit sind */
void dw1000_initializehardware()
{
	// Initialisierungsstruktur für GPIO
	GPIO_InitTypeDef GPIO_InitStructure;

	// Clocksystem einschalten
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,  ENABLE);		// A6 (MISO), A7 (MOSI)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,  ENABLE);		// B1 (CS), B3 (CLK), B4 (WAKE), B5 (RST)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,  ENABLE);		// C1 (IRQ)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,   ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// Stop the SPI clock
	SPI_I2S_DeInit(SPI1);

	// Initialisiere Chipselect (PB1)
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_1);

	// Initialisiere CLK (PB3)
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);

	// Initialisiere MOSI (PA7)
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	// Initialisiere MISO (PA6)
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);

	// Initialisiere IRQ (PC1)
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Initialisiere Wakeup (PB4)
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_4);

	// Initialisiere RST (PB15)
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_15);

	//Initialisiere SPI1
	dw1000_spi_structure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	dw1000_spi_structure.SPI_Mode = SPI_Mode_Master;
	dw1000_spi_structure.SPI_DataSize = SPI_DataSize_8b;
	dw1000_spi_structure.SPI_CPOL = SPI_CPOL_Low;
	dw1000_spi_structure.SPI_CPHA = SPI_CPHA_1Edge;
	dw1000_spi_structure.SPI_NSS = SPI_NSS_Soft;
	dw1000_spi_structure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	dw1000_spi_structure.SPI_FirstBit = SPI_FirstBit_MSB;
	dw1000_spi_structure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &dw1000_spi_structure);

	// Initialisiere die externen Interrupts
	dw1000_exti_structure.EXTI_Line = EXTI_Line1;
	dw1000_exti_structure.EXTI_LineCmd = DISABLE;
	dw1000_exti_structure.EXTI_Mode = EXTI_Mode_Interrupt;
	dw1000_exti_structure.EXTI_Trigger = EXTI_Trigger_Rising;

	// Initialisiere und aktiviere den Interrupt für die IRQ Leitung
	dw1000_nvic_structure.NVIC_IRQChannel = EXTI1_IRQn;
	dw1000_nvic_structure.NVIC_IRQChannelPreemptionPriority = 0x15;
	dw1000_nvic_structure.NVIC_IRQChannelSubPriority = 0x00;
	dw1000_nvic_structure.NVIC_IRQChannelCmd = DISABLE;

	// Aktiviere die Interrupts für die IRQ Leitung
	EXTI_Init(&dw1000_exti_structure);
	NVIC_Init(&dw1000_nvic_structure);

	// IRQ verwendet Interrupt auf Line 1
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource1);

	// Deaktiviere die Interrupts für den Moment
	dw1000_disableInterrupts();

	#ifdef DW1000_DMA

		// Deinitialisiere die DMA
		DMA_DeInit(DMA2_Stream3); //SPI1_TX_DMA_STREAM
		DMA_DeInit(DMA2_Stream2); //SPI1_RX_DMA_STREAM

		// Aktiviere die Clock für die DMA 2
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

		DMA_InitTypeDef DMA_InitStructure;

		// Structure auf default setzen
		DMA_StructInit(&DMA_InitStructure);

		// Lege die Grundinitialisierung fest
		DMA_InitStructure.DMA_BufferSize = 0;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI1->DR));
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;

		// Configure Tx DMA
		DMA_InitStructure.DMA_Channel = DMA_Channel_3;
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;
		DMA_Init(DMA2_Stream3, &DMA_InitStructure);

		// Configure Rx DMA
		DMA_InitStructure.DMA_Channel = DMA_Channel_3;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;
		DMA_Init(DMA2_Stream2, &DMA_InitStructure);

		// Fordere RX und TX an
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);

	#endif

	SPI_Cmd(SPI1, ENABLE);

	// Deaktiviere die Interrupts für den Moment
	dw1000_enableInterrupts();
}


//! Methode zur Aktivierung der Interrupt-Leitung für Statusänderungen des DW1000-Chips
/*!	Es ist mitunter notwendig, die Interrupte zeitweise zu deaktivieren, um Überläufe zu vermeiden.<br>
 *  Diese Methode muss implementiert werden, um eine Funktionalität zur Deaktivierung des Statusinterrupts zu gewährleisten */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_enableInterrupts()
{
	dw1000_exti_structure.EXTI_LineCmd = ENABLE;
	dw1000_nvic_structure.NVIC_IRQChannelCmd = ENABLE;
	EXTI_Init(&dw1000_exti_structure);
	NVIC_Init(&dw1000_nvic_structure);
	enableIRQ();
}


//! Methode zur Deaktivierung der Interrupt-Leitung für Statusänderungen des DW1000-Chips
/*!	Es ist mitunter notwendig, die Interrupte zeitweise zu deaktivieren, um Überläufe zu vermeiden.<br>
 * Diese Methode muss implementiert werden, um eine Funktionalität zur Deaktivierung des Statusinterrupts zu gewährleisten */
DW1000_OPTIMIZE DW1000_INLINE void dw1000_disableInterrupts()
{
	disableIRQ();
	dw1000_exti_structure.EXTI_LineCmd = DISABLE;
	dw1000_nvic_structure.NVIC_IRQChannelCmd = DISABLE;
	EXTI_Init(&dw1000_exti_structure);
	NVIC_Init(&dw1000_nvic_structure);
}


//! Diese Methode schaltet die Reset-Leitung auf Ausgang und legt einen HIGH-Pegel an.
void dw1000_setResetPin()
{
	if (!dw1000_isresetpinoutput)
	{
		// Initialisierungsstruktur für GPIO
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		dw1000_isresetpinoutput = 1;
	}
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
}


//! Diese Methode schaltet die Reset-Leitung auf Ausgang und legt einen LOW-Pegel an.
void dw1000_resetResetPin()
{
	if (!dw1000_isresetpinoutput)
	{
		// Initialisierungsstruktur für GPIO
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		dw1000_isresetpinoutput = 1;
	}
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}


//! Diese Methode schaltet die Reset-Leitung auf Eingang und liest ihren Wert
unsigned char dw1000_readResetPin()
{
	if (dw1000_isresetpinoutput)
	{
		// Initialisierungsstruktur für GPIO
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		dw1000_isresetpinoutput = 0;
	}
	return (GPIOB->IDR & GPIO_Pin_15) != 0;
}


//! Diese Methode legt einen High-Pegel auf den Wake-Pin
void dw1000_setWakePin() { GPIO_SetBits(GPIOB, GPIO_Pin_4); }


//! Diese Methode legt einen Low-Pegel auf den Wake-Pin
void dw1000_resetWakePin() { GPIO_ResetBits(GPIOB, GPIO_Pin_4); }


//! Diese Methode liest den Pegel, der aktuell am Wake-Pin angelegt wird
unsigned char dw1000_readWakePin() { return ((GPIOB->ODR & GPIO_Pin_4) != 0); }


//! Methode zur Reduzierung der SPI-Taktung auf maximal 3MHz
DW1000_OPTIMIZE DW1000_INLINE void dw1000_reduceClock(void)
{
	if (dw1000_spi_structure.SPI_BaudRatePrescaler != SPI_BaudRatePrescaler_32)
	{
		dw1000_spi_structure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
		SPI_Init(SPI1, &dw1000_spi_structure);
	}
}


//! Methode zur Anhebung der SPI-Taktung auf maximal 20MHz
DW1000_OPTIMIZE DW1000_INLINE void dw1000_raiseClock(void)
{
	if (dw1000_spi_structure.SPI_BaudRatePrescaler != SPI_BaudRatePrescaler_4)
	{
		dw1000_spi_structure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
		SPI_Init(SPI1, &dw1000_spi_structure);
	}
}


//! Diese Methode liest und schreibt die SPI
/*! \param content Der Inhalt dieses Puffers wird auf die SPI geschrieben. Die Antwort wird ebenfalls in diesen Puffer gelesen.
 *  \param contentlength Anzahl der Symbole, die auf die SPI-Schnittstelle geschrieben, bzw. von ihr gelesen werden sollen */
DW1000_OPTIMIZE DW1000_INLINE unsigned short dw1000_readWriteSpi(unsigned char* content, unsigned short contentlength)
{
	// Prüfe, ob die Eingabe plausibel ist
	if (content && contentlength)
	{
		// Deaktiviere alle Interrupts (__disable_irq()), wenn notwendig
		disableIRQ();

		// SPI Transfer durch Chipselect-PIN (low) einleiten
		DW1000_CS_LOW;

#ifdef DW1000_DMA

		// Gebe die Paketlänge an (Verschachtelt, damit nur eine Berechnung notwendig ist)
		DMA2_Stream3->NDTR = contentlength;
		DMA2_Stream2->NDTR = contentlength;

		// Lege die Adresse des Puffers fest, der geschrieben werden soll
		DMA2_Stream3->M0AR = (uint32_t)content;
		DMA2_Stream2->M0AR = (uint32_t)content;

		// Aktiviere den DMA Transfer
		DMA2_Stream2->CR |= (uint32_t)DMA_SxCR_EN;
		DMA2_Stream3->CR |= (uint32_t)DMA_SxCR_EN;

		// Warte zunächst auf Beendigung des Schreib- dann auf die des Lesevorgangs
		while ((DMA2->LISR & 0b00001010010000000000000000000000) == 0) asm("");	// FCIF3, TEIF3 und FEIF3 (Sowohl Bestätigung als auch Fehler)
		while ((DMA2->LISR & 0b00000000001010010000000000000000) == 0) asm("");	// FCIF2, TEIF2 und FEIF2 (Sowohl Bestätigung als auch Fehler)

		// Setze alle Flags zurück, die Komplettierung des DMA Transfers signalisieren (Durch schreiben einer 1 in LIFCR)
		DMA2->LIFCR = 0b00001010011010010000000000000000;	// FCIF3, TEIF3, FEIF3, FCIF2, TEIF2 und FEIF2 (Sowohl Bestätigung als auch Fehler)

#else

		// Zeigerposition im aktuellen SPI Transfer
		unsigned char transferpos = 0;

		// Übermittle den Header Byte für Byte im Polling-Modus
		// TODO: Verwende DMA
		for (transferpos=0; transferpos<contentlength; transferpos++)
		{
			SPI1->DR = content[transferpos];
			while( !(SPI1->SR & SPI_I2S_FLAG_TXE)) asm("");
			while( !(SPI1->SR & SPI_I2S_FLAG_RXNE)) asm("");
			content[transferpos] = SPI1->DR;
		}

#endif

		// SPI Transfer durch Chipselect-PIN (high) abschließen
		DW1000_CS_HIGH;

		// Reaktiviere die Interrupts (__enable_irq())
		enableIRQ();

		// Gebe die Anzahl der geschriebenen Symbole aus
		return contentlength;

	}
	return 0;
}

void dw1000_log(char* str)
{
	printf(str);
}


