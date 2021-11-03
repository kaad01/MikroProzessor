//=========================================================================
// Anschlussbelegung des CC3100 WLAN Transceivers
//=========================================================================
//	CC3100-PIN		STM32-Portleitung
//	Hibernate		PB14
// 	IRQ				PC0
// 	SPI2-CS			PB9
// 	SPI2-CLK		PB13
// 	SPI2-MISO		PC2
// 	SPI2-MOSI		PC3
//
//	USART-RXD		PC6
//	USART-TXD		PC7
//=========================================================================


//=========================================================================
// board_lib
//=========================================================================
#include "simplelink.h"
#include "CC3100_Board.h"


//=========================================================================
// cc3100 serial flash Formatierungssequenzen
//=========================================================================
const char sf_512KB_format_command[24]	= {0x00, 0x17, 0xac, 0x28, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
const char sf_1MB_format_command[24] 	= {0x00, 0x17, 0x2d, 0x28, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
const char sf_2MB_format_command[24] 	= {0x00, 0x17, 0x2e, 0x28, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
const char sf_4MB_format_command[24] 	= {0x00, 0x17, 0x30, 0x28, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
const char sf_8MB_format_command[24] 	= {0x00, 0x17, 0x34, 0x28, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
const char sf_16MB_format_command[24] 	= {0x00, 0x17, 0x3c, 0x28, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

const char disconnect_command[4] = {0x00, 0x03, 0x26, 0x26};

int 	ack_cc3100_resceived = 0;


//=========================================================================
// spi interface
//=========================================================================

#define ASSERT_CS()         GPIO_ResetBits(GPIOB, GPIO_Pin_9);
#define DEASSERT_CS()       GPIO_SetBits(GPIOB, GPIO_Pin_9);

unsigned char cc3100_rx_puffer[2048] = {0};
unsigned char cc3100_tx_puffer[2048] = {0};



//=========================================================================
int spi_Close(Fd_t fd)
//=========================================================================
{
	// TODO
    CC3100_InterruptDisable();
    return NONOS_RET_OK; // 0
}



//=========================================================================
Fd_t spi_Open(char *ifName, unsigned long flags)
//=========================================================================
{
	CC3100_InterruptEnable();
	return NONOS_RET_OK;
}


//=========================================================================
int spi_Write(Fd_t fd, unsigned char *pBuff, int len)
//=========================================================================
{
    int len_to_return = len;
    int i = 0;
    unsigned char rxd = 0;
    ASSERT_CS();
    while(len)
    {
        SPI2->DR = pBuff[i];
        while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
        while( !(SPI2->SR & SPI_I2S_FLAG_RXNE) );
        rxd = (unsigned char) (SPI2->DR);
        rxd = rxd +1; //ToDo  sinnfrei
        i++;
        len--;
    }
    DEASSERT_CS();
    return len_to_return;
}



//=========================================================================
int spi_Read(Fd_t fd, unsigned char *pBuff, int len)
//=========================================================================
{
	memset(pBuff,0,len);
    int i = 0;
    ASSERT_CS();
    for(i=0; i< len; i++)
    {
        SPI2->DR = 0xFF;
        while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
        while( !(SPI2->SR & SPI_I2S_FLAG_RXNE) );
        pBuff[i] = (unsigned char) (SPI2->DR);
    }
    DEASSERT_CS();
    return len;
}


// usigned short len TODO
//=========================================================================
  int spi_dma_Write (Fd_t fd, unsigned char *pBuff, int len)
//=========================================================================
{
	// Lege die Länge fest
	DMA_MemoryTargetConfig(DMA1_Stream3, (uint32_t)cc3100_rx_puffer, DMA_Memory_0);
	DMA_SetCurrDataCounter(DMA1_Stream3, (unsigned short)len);
	DMA_MemoryTargetConfig(DMA1_Stream4, (uint32_t)pBuff, DMA_Memory_0);
	DMA_SetCurrDataCounter(DMA1_Stream4, (unsigned short)len);

	// SPI Transfer durch Chipselect-PIN (low) einleiten
	ASSERT_CS();

	// Aktiviere den DMA Transfer
	DMA_Cmd(DMA1_Stream3, ENABLE);
	DMA_Cmd(DMA1_Stream4, ENABLE);

	// Warte zunächst auf Beendigung des Schreib- dann auf die des Lesevorgangs
	while (DMA_GetFlagStatus(DMA1_Stream3, DMA_FLAG_TCIF3)==RESET) asm("");
	while (DMA_GetFlagStatus(DMA1_Stream4, DMA_FLAG_TCIF4)==RESET) asm("");

	// SPI Transfer durch Chipselect-PIN (high) abschließen
	DEASSERT_CS();

	// Setze alle Flags zurück, die Komplettierung des DMA Transfers signalisieren (Durch schreiben einer 1 in LIFCR)
	DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
	DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);

	// Gebe die Anzahl der gelesenen Zeichen aus
	return len;
}
//	DMA_Cmd(DMA1_Stream3, DISABLE);	// TXD
//	DMA_Cmd(DMA1_Stream4, DISABLE);	// RXD
//
//	// TXD Stream
//	DMA_MemoryTargetConfig(DMA1_Stream4,(uint32_t) pBuff, DMA_Memory_0);
//	DMA_SetCurrDataCounter(DMA1_Stream4, (uint16_t) len);
//
//	// Memory
//
//	// RXD Stream
//	DMA_MemoryTargetConfig(DMA1_Stream3, (uint32_t) &cc3100_rx_puffer, DMA_Memory_0); // &
//	DMA_SetCurrDataCounter(DMA1_Stream3, (uint16_t) len);
//
//    ASSERT_CS();
//
//    DMA_Cmd(DMA1_Stream3, ENABLE);
//	DMA_Cmd(DMA1_Stream4, ENABLE);
//
//	SPI_Cmd(SPI2, ENABLE);
//
//    while (DMA_GetFlagStatus(DMA1_Stream4, DMA_FLAG_TCIF4)==RESET);
//    while (DMA_GetFlagStatus(DMA1_Stream3, DMA_FLAG_TCIF3)==RESET);
//
//    DEASSERT_CS();
//
//    DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
//    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
//
//	DMA_Cmd(DMA1_Stream4, DISABLE);	// TXD
//	DMA_Cmd(DMA1_Stream3, DISABLE);	// RXD
//
//	SPI_Cmd(SPI1, DISABLE);
//
//    return len;
//}


//=========================================================================
int spi_dma_Read (Fd_t fd, unsigned char *pBuff, int len)
//=========================================================================
{
	// Lege die Länge fest
	DMA_MemoryTargetConfig(DMA1_Stream3, (uint32_t)pBuff, DMA_Memory_0);
	DMA_SetCurrDataCounter(DMA1_Stream3, (unsigned short)len);
	DMA_MemoryTargetConfig(DMA1_Stream4, (uint32_t)pBuff, DMA_Memory_0);
	DMA_SetCurrDataCounter(DMA1_Stream4, (unsigned short)len);

	// SPI Transfer durch Chipselect-PIN (low) einleiten
	ASSERT_CS();

	// Aktiviere den DMA Transfer
	DMA_Cmd(DMA1_Stream3, ENABLE);
	DMA_Cmd(DMA1_Stream4, ENABLE);

	// Warte zunächst auf Beendigung des Schreib- dann auf die des Lesevorgangs
	while (DMA_GetFlagStatus(DMA1_Stream3, DMA_FLAG_TCIF3)==RESET) asm("");
	while (DMA_GetFlagStatus(DMA1_Stream4, DMA_FLAG_TCIF4)==RESET) asm("");

	// SPI Transfer durch Chipselect-PIN (high) abschließen
	DEASSERT_CS();

	// Setze alle Flags zurück, die Komplettierung des DMA Transfers signalisieren (Durch schreiben einer 1 in LIFCR)
	DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
	DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);

	// Gebe die Anzahl der gelesenen Zeichen aus
	return len;
}

//	memset(pBuff, 0x00, len);
//	//memset(cc3100_rx_puffer, 0x00, sizeof(cc3100_rx_puffer));
//	memset(cc3100_tx_puffer, 0xFF, sizeof(cc3100_tx_puffer));
//
//	DMA_Cmd(DMA1_Stream3, DISABLE);	// TXD
//	DMA_Cmd(DMA1_Stream4, DISABLE);	// RXD
//
//	// TXD Stream
//	DMA_MemoryTargetConfig(DMA1_Stream4,(uint32_t) &cc3100_tx_puffer, DMA_Memory_0);
//	DMA_SetCurrDataCounter(DMA1_Stream4, (uint16_t) len);
//
////	DMA_StructInit(&DMA_InitStruct);
////
////	// TXD - DMA1 Channel_0 Stream_4	hier den puffer mit 0xFF durch ein Byte ersetzen
////	DMA_InitStruct.DMA_Channel 				= DMA_Channel_0;
////	DMA_InitStruct.DMA_PeripheralBaseAddr 	= (uint32_t) & (SPI2->DR);
////	DMA_InitStruct.DMA_Memory0BaseAddr 		= (uint32_t) &cc3100_tx_puffer;		// TODO
////	DMA_InitStruct.DMA_DIR 					= DMA_DIR_MemoryToPeripheral;
////	DMA_InitStruct.DMA_BufferSize 			= (uint16_t) len;					// TODO
////	DMA_InitStruct.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
////	DMA_InitStruct.DMA_MemoryInc 			= DMA_MemoryInc_Disable;			//!!!TODO
////	DMA_InitStruct.DMA_PeripheralDataSize 	= DMA_PeripheralDataSize_Byte;
////	DMA_InitStruct.DMA_MemoryDataSize 		= DMA_MemoryDataSize_Byte;
////	DMA_InitStruct.DMA_Mode 				= DMA_Mode_Normal;
////	DMA_InitStruct.DMA_Priority 			= DMA_Priority_High;
////	DMA_InitStruct.DMA_FIFOMode 			= DMA_FIFOMode_Disable;
////	DMA_InitStruct.DMA_FIFOThreshold 		= DMA_FIFOThreshold_HalfFull;
////	DMA_InitStruct.DMA_MemoryBurst 			= DMA_MemoryBurst_Single;
////	DMA_InitStruct.DMA_PeripheralBurst 		= DMA_PeripheralBurst_Single;
////	DMA_Init(DMA1_Stream4, &DMA_InitStruct);
//
//	// RXD Stream
//	DMA_MemoryTargetConfig(DMA1_Stream3, (uint32_t) pBuff, DMA_Memory_0);
//	DMA_SetCurrDataCounter(DMA1_Stream3, (uint16_t) len);
//
////	DMA_StructInit(&DMA_InitStruct);
////
////	// RXD - DMA1 Channel_0 Stream_3
////	DMA_InitStruct.DMA_Channel 				= DMA_Channel_0;
////	DMA_InitStruct.DMA_PeripheralBaseAddr 	= (uint32_t) & (SPI2->DR);
////	DMA_InitStruct.DMA_Memory0BaseAddr 		= (uint32_t) pBuff; 				// TODO immer neu setzen
////	DMA_InitStruct.DMA_DIR 					= DMA_DIR_PeripheralToMemory;
////	DMA_InitStruct.DMA_BufferSize 			= (uint16_t) len;					// TODO immer neu setzen
////	DMA_InitStruct.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
////	DMA_InitStruct.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
////	DMA_InitStruct.DMA_PeripheralDataSize 	= DMA_PeripheralDataSize_Byte;
////	DMA_InitStruct.DMA_MemoryDataSize 		= DMA_MemoryDataSize_Byte;
////	DMA_InitStruct.DMA_Mode 				= DMA_Mode_Normal;
////	DMA_InitStruct.DMA_Priority 			= DMA_Priority_High;
////	DMA_InitStruct.DMA_FIFOMode 			= DMA_FIFOMode_Disable;
////	DMA_InitStruct.DMA_FIFOThreshold 		= DMA_FIFOThreshold_HalfFull;
////	DMA_InitStruct.DMA_MemoryBurst 			= DMA_MemoryBurst_Single;
////	DMA_InitStruct.DMA_PeripheralBurst 		= DMA_PeripheralBurst_Single;
////	DMA_Init(DMA1_Stream3, &DMA_InitStruct);
//
//	ASSERT_CS();
//
//    DMA_Cmd(DMA1_Stream3, ENABLE);
//	DMA_Cmd(DMA1_Stream4, ENABLE);
//
//	SPI_Cmd(SPI2, ENABLE);
//
//    while (DMA_GetFlagStatus(DMA1_Stream4, DMA_FLAG_TCIF4)==RESET);
//    while (DMA_GetFlagStatus(DMA1_Stream3, DMA_FLAG_TCIF3)==RESET);
//
//    DEASSERT_CS();
//
//    DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
//    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
//
//	DMA_Cmd(DMA1_Stream4, DISABLE);	// TXD
//	DMA_Cmd(DMA1_Stream3, DISABLE);	// RXD
//
//	SPI_Cmd(SPI1, DISABLE);
//
//    return len;
//}



//=========================================================================
void DMA1_Stream4_IRQHandler(void)
//=========================================================================
{
	// Stream RX/TX ausschalten
	//DMA_Cmd(DMA1_Stream3, DISABLE);
	DMA_Cmd(DMA1_Stream4, DISABLE);
	// DMA Interrupt Handler aufrufen
	DMA_Int_Handler();
	//DMA Streamx transfer complete flag zurücksetzen
	DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
	DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
}


//=========================================================================
void DMA_Int_Handler(void)
//=========================================================================
{
	unsigned long ucTxFinished, ucRxFinished;
	//unsigned short data_to_recv;

	//DMA Streamx transfer complete flag abfragen
	ucTxFinished = DMA_GetFlagStatus(DMA1_Stream4, DMA_FLAG_TCIF4);
	ucRxFinished = DMA_GetFlagStatus(DMA1_Stream3, DMA_FLAG_TCIF3);

	if (ucTxFinished && ucRxFinished)
	{
		DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
		DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
		SPI_I2S_ClearFlag(SPI2, SPI_IT_TXE | SPI_IT_RXNE);
	}
}




//=========================================================================
// cc3100 interface
//=========================================================================

P_EVENT_HANDLER        pIrqEventHandler = 0;

_u8 IntIsMasked;



//=========================================================================
int registerInterruptHandler(P_EVENT_HANDLER InterruptHdl , void* pValue)
//=========================================================================
{
    pIrqEventHandler = InterruptHdl;

    return 0;
}



//=========================================================================
void Delay(unsigned long interval)
//=========================================================================
{
	interval *= 12;
	interval = interval * 1000;
    while(interval--) { __NOP(); }
}



//=========================================================================
void wait_uSek_CC3100(unsigned long us)
//=========================================================================
{
	// wartet uSekunden gilt aber nur bei 168MHz Taktfrequenz
	us *= 12;
	while(us--) { __NOP(); }
}



//=========================================================================
void CC3100_init()
//=========================================================================
{
    //=========================================================================
    // Clocksystem einschalten
    //=========================================================================
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    // CLK - PB13
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);

    // MOSI - PC3
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);

    // MISO - PC2
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_SPI2);

    //=========================================================================
    // CC3100 SPI2 Setup
    //=========================================================================
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(SPI2, &SPI_InitStructure);

    // Wegen DMA nicht verwenden!
    SPI_Cmd(SPI2, ENABLE);

    //=========================================================================
    // CC3100 GPIO Setup
    //=========================================================================

    // CS - PB9
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_9);

    //=========================================================================
    // CC3100 HIB 0-hibernate mode 1-aktiv mode
    //=========================================================================
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);

    //=========================================================================
    // CC3100 IRQ Setup
    //=========================================================================

    // WLAN_IRQ
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);

    EXTI_InitTypeDef EXTI_SPIGPIO_InitStructure;
    EXTI_SPIGPIO_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_SPIGPIO_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_SPIGPIO_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_SPIGPIO_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_SPIGPIO_InitStructure);

    NVIC_InitTypeDef NVIC_InitSPIGPIOStructure;
    NVIC_InitSPIGPIOStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitSPIGPIOStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitSPIGPIOStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitSPIGPIOStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitSPIGPIOStructure);


    //=========================================================================

    // Structure auf default setzen
    DMA_InitTypeDef DMA_InitStruct;

    // RXD - DMA1 Channel 0 Stream3
    DMA_StructInit(&DMA_InitStruct);

    // RXD - DMA1 Channel_0 Stream_3
    DMA_InitStruct.DMA_Channel               = DMA_Channel_0;
    DMA_InitStruct.DMA_PeripheralBaseAddr    = (uint32_t)&(SPI2->DR);
    DMA_InitStruct.DMA_Memory0BaseAddr       = 0;                    // TODO immer neu setzen
    DMA_InitStruct.DMA_DIR                   = DMA_DIR_PeripheralToMemory;
    DMA_InitStruct.DMA_BufferSize            = 0;                    // TODO immer neu setzen
    DMA_InitStruct.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize        = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode                  = DMA_Mode_Normal;      // DMA_Mode_Circular wenn man dauernd senden will;
    DMA_InitStruct.DMA_Priority              = DMA_Priority_Medium;
    DMA_InitStruct.DMA_FIFOMode              = DMA_FIFOMode_Disable;
    DMA_InitStruct.DMA_FIFOThreshold         = DMA_FIFOThreshold_HalfFull;
    DMA_InitStruct.DMA_MemoryBurst           = DMA_MemoryBurst_Single;
    DMA_InitStruct.DMA_PeripheralBurst       = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream3, &DMA_InitStruct);

    // Structure auf default setzen
    DMA_StructInit(&DMA_InitStruct);

    // TXD - DMA1 Channel_0 Stream_4
    DMA_InitStruct.DMA_Channel               = DMA_Channel_0;
    DMA_InitStruct.DMA_PeripheralBaseAddr    = (uint32_t)&(SPI2->DR);
    DMA_InitStruct.DMA_Memory0BaseAddr       = 0;                    // TODO
    DMA_InitStruct.DMA_DIR                   = DMA_DIR_MemoryToPeripheral;
    DMA_InitStruct.DMA_BufferSize            = 0;                    // TODO
    DMA_InitStruct.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize        = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode                  = DMA_Mode_Normal;      // DMA_Mode_Circular wenn man dauernd senden will;
    DMA_InitStruct.DMA_Priority              = DMA_Priority_Medium;
    DMA_InitStruct.DMA_FIFOMode              = DMA_FIFOMode_Disable;
    DMA_InitStruct.DMA_FIFOThreshold         = DMA_FIFOThreshold_HalfFull;
    DMA_InitStruct.DMA_MemoryBurst           = DMA_MemoryBurst_Single;
    DMA_InitStruct.DMA_PeripheralBurst       = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream4, &DMA_InitStruct);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

    DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
}



//=========================================================================
void CC3100_enable()
//=========================================================================
{
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
	//Delay(1520);
}



//=========================================================================
void CC3100_disable()
//=========================================================================
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_14);
	Delay(15);
}



//=========================================================================
void CC3100_InterruptEnable()
//=========================================================================
{
// TODO so
	NVIC_EnableIRQ(EXTI0_IRQn);
// 	oder so
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource1);
//
//	EXTI_ClearITPendingBit(EXTI_Line10);
//
//	EXTI_InitTypeDef EXTI_SPIGPIO_InitStructure;
//	EXTI_SPIGPIO_InitStructure.EXTI_Line = EXTI_Line10;
//	EXTI_SPIGPIO_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_SPIGPIO_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_SPIGPIO_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//	EXTI_Init(&EXTI_SPIGPIO_InitStructure);
//
//	NVIC_InitTypeDef NVIC_InitSPIGPIOStructure;
//	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_InitSPIGPIOStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //EXTI15_10_IRQn;
//	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
//	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelSubPriority = 0x01;
//	NVIC_Init(&NVIC_InitSPIGPIOStructure);
}



//=========================================================================
void CC3100_InterruptDisable()
//=========================================================================
{
// TODO
	NVIC_DisableIRQ(EXTI0_IRQn);
// 	oder so
//	EXTI_ClearITPendingBit(EXTI_Line10);
//
//	EXTI_InitTypeDef EXTI_SPIGPIO_InitStructure;
//	EXTI_SPIGPIO_InitStructure.EXTI_Line = EXTI_Line10;
//	EXTI_SPIGPIO_InitStructure.EXTI_LineCmd = DISABLE;
//	EXTI_SPIGPIO_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_SPIGPIO_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//	EXTI_Init(&EXTI_SPIGPIO_InitStructure);
//
//	NVIC_InitTypeDef NVIC_InitSPIGPIOStructure;
//	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelCmd = DISABLE;
//	NVIC_InitSPIGPIOStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //EXTI15_10_IRQn;
//	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
//	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelSubPriority = 0x01;
//	NVIC_Init(&NVIC_InitSPIGPIOStructure);

//	//SYSCFG_DeInit();
}



//=========================================================================
void WLAN_intHandler()
//=========================================================================
{
    	if(pIrqEventHandler)
        {
    		//uart_send("=>WLAN IRQ\r\n");
            pIrqEventHandler(0);
        }
}



//=========================================================================
void UART1_intHandler()
//=========================================================================
{
	//unsigned long intStatus;
	//intStatus =UARTIntStatus(UART1_BASE,0);
	//UARTIntClear(UART1_BASE,intStatus);
}



//=========================================================================
void MaskIntHdlr()
//=========================================================================
{
	IntIsMasked = TRUE;
}



//=========================================================================
void UnMaskIntHdlr()
//=========================================================================
{
	IntIsMasked = FALSE;
}


//=========================================================================
// Funktionen zur Formatierung des CC3100 serial flash
//=========================================================================

//=========================================================================
static void cc3100_uart6_init(void)
//=========================================================================
{
	// Struct Anlegen
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// Periperie Clocksystem Einschalten
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	// GPIO Initialisieren
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7; //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Alternativ Funktion Zuweisen

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	// USART als Alternativ Funktion Initialisieren
	USART_InitStructure.USART_BaudRate = 921600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode =  USART_Mode_Tx | USART_Mode_Rx ; //
	USART_Init(USART6, &USART_InitStructure);

	// USART Einschalten
	USART_Cmd(USART6, ENABLE);

	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);

	NVIC_EnableIRQ(USART6_IRQn);
}




//=========================================================================
static void cc3100_nHIB_0(void)
//=========================================================================
{
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

   	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, GPIO_Pin_14);
	GPIO_ResetBits(GPIOC, GPIO_Pin_6);
}




//=========================================================================
static void cc3100_nHIB_1(void)
//=========================================================================
{
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
}





//=========================================================================
int CC3100_format_serial_flash(void)
//=========================================================================
{
	int i = 0;

	usart2_send("\r\n======Formatierung=Serial=Flash======\r\n");

	ack_cc3100_resceived = 0;

	usart2_send("1. Bootloader starten ... ");

	cc3100_nHIB_0();
    cc3100_uart6_init();
	wait_uSek_CC3100(100000);
	cc3100_nHIB_1();

	while (ack_cc3100_resceived == 0) {;}
	ack_cc3100_resceived = 0;

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);

	usart2_send("2. Serial Flash formatieren ... ");

	for(i = 0; i < sizeof(sf_1MB_format_command); i++)
	{
		USART_SendData(USART6, (unsigned short int)sf_1MB_format_command[i]);
		while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET){}
	}

	while (ack_cc3100_resceived == 0){;}
	ack_cc3100_resceived = 0;

	usart2_send("3. Bootloader stoppen ... ");

	for(i = 0; i < sizeof(disconnect_command); i++)
	{
		USART_SendData(USART6, (unsigned short int)disconnect_command[i]);
		while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET){}
	}

	while (ack_cc3100_resceived == 0){;}

	usart2_send("=> Formatierung - OK\r\n");

	return 0;
}




//=========================================================================
void CC3100_uart6_receive_IRQ(void)
//=========================================================================
{
	static char ack_puffer[2] = {0xff,0xff};
    char zeichen;

    if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
   	{
    	zeichen = (char)USART_ReceiveData(USART6);
    	ack_puffer[0] = ack_puffer[1];
    	ack_puffer[1] = zeichen;
    	// ack besteht aus den zwei Byte 0x00 0xcc
    	if ((ack_puffer[0] == 0x00) && (ack_puffer[1] == 0xCC))
    	{
    		ack_cc3100_resceived = 1;
    	   	ack_puffer[0] = 0xFF;
        	ack_puffer[1] = 0xFF;
        	usart2_send("OK\r\n");
    	}
   	}
}

