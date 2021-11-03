#include "usart.h"

char usart2_tx_buffer[USART2_TX_BUFFERSIZE];
char usart2_rx_buffer[USART2_RX_BUFFERSIZE];
char uart2_RX_zeichen = 0;
unsigned char usart2_running = 0;


// Sendet
void usart2_printf(const char* format, ...)
{
	if (format)
	{
		// Warte, bis der letzte Transfer abgeschlossen wurde
		while (usart2_running && DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_TCIF6) == RESET) asm("");
		DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);

		// Erstelle die Ausgabezeichenkette
		va_list arglist;
		va_start(arglist, format);
		int length = vsnprintf((char *)usart2_tx_buffer, USART2_TX_BUFFERSIZE, (char *)format, arglist);
		va_end(arglist);

		// Prüfe, ob die erstellte Zeichenkette in den TX-Puffer passte
		if (length <= USART2_TX_BUFFERSIZE)
		{
			// Die USART ist aktiv
			usart2_running = 1;

			// Gebe die Paketlänge an (Verschachtelt, damit nur eine Berechnung notwendig ist)
			DMA_SetCurrDataCounter(DMA1_Stream6, (unsigned short)length);

			// Aktiviere den DMA Transfer
			DMA_Cmd(DMA1_Stream6, ENABLE);
		}
	}
}


// Sendet eine Zeichenkette an die serielle Schnittstelle
void usart2_send(char* chars)
{
	if (chars)
	{
		int length = strlen(chars);
		if (length <= USART2_TX_BUFFERSIZE)
		{
			// Warte, bis der letzte Transfer abgeschlossen wurde
			while (usart2_running && DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_TCIF6) == RESET) asm("");
			DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);

			// Die USART ist aktiv
			usart2_running = 1;

			// Kopiere die Zeichenkette in den TX-Puffer
			strcpy(usart2_tx_buffer, chars);

			// Gebe die Paketlänge an (Verschachtelt, damit nur eine Berechnung notwendig ist)
			DMA_SetCurrDataCounter(DMA1_Stream6, (unsigned short)length);

			// Aktiviere den DMA Transfer
			DMA_Cmd(DMA1_Stream6, ENABLE);
		}
	}
}


// Sendet
void usart2_printf_unblocking(const char* format, ...)
{
	if (format && (!usart2_running || DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_TCIF6) != RESET))
	{
		// Lösche die Flags
		DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);

		// Erstelle die Ausgabezeichenkette
		va_list arglist;
		va_start(arglist, format);
		int length = vsnprintf((char *)usart2_tx_buffer, USART2_TX_BUFFERSIZE, (char *)format, arglist);
		va_end(arglist);

		// Prüfe, ob die erstellte Zeichenkette in den TX-Puffer passte
		if (length <= USART2_TX_BUFFERSIZE)
		{
			// Die USART ist aktiv
			usart2_running = 1;

			// Gebe die Paketlänge an (Verschachtelt, damit nur eine Berechnung notwendig ist)
			DMA_SetCurrDataCounter(DMA1_Stream6, (unsigned short)length);

			// Aktiviere den DMA Transfer
			DMA_Cmd(DMA1_Stream6, ENABLE);
		}
	}
}


// Sendet eine Zeichenkette an die serielle Schnittstelle
void usart2_send_unblocking(char* chars)
{
	if (chars)
	{
		int length = strlen(chars);
		if (length <= USART2_TX_BUFFERSIZE && (!usart2_running || DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_TCIF6) != RESET))
		{
			// Lösche die Flags
			DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);

			// Die USART ist aktiv
			usart2_running = 1;

			// Kopiere die Zeichenkette in den TX-Puffer
			strcpy(usart2_tx_buffer, chars);

			// Gebe die Paketlänge an (Verschachtelt, damit nur eine Berechnung notwendig ist)
			DMA_SetCurrDataCounter(DMA1_Stream6, (unsigned short)length);

			// Aktiviere den DMA Transfer
			DMA_Cmd(DMA1_Stream6, ENABLE);
		}
	}
}


void usart2_init(void)
{
	// Initialisierungsstrukturen vorbereiten
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	// Clocksystem aktivieren
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	// GPIOA Configuration:  USART2 TX on PA2 RX on PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Alternative Funktion festlegen
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// USART Initialisieren
	USART_InitStructure.USART_BaudRate = 921600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStructure);

	// Deinitialisiere die DMA
	DMA_DeInit(DMA1_Stream6); //USART2_TX_DMA_STREAM

	// Structure auf default setzen
	DMA_StructInit(&DMA_InitStructure);

	// Deaktiviere den DMA Transfer
	DMA_Cmd(DMA1_Stream6, DISABLE);

	//// Lösche das DMA-Flag
	DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);

	// Lege die Grundinitialisierung fest
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(USART2->DR));
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)usart2_tx_buffer;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);

	// Fordere RX und TX an
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	NVIC_EnableIRQ(USART2_IRQn);

	// USART Deaktivieren
	USART_Cmd(USART2, ENABLE);
}



void USART2_IRQ(void)
{
	char zeichen;
	static int j = 0;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
		{
			zeichen = (char)USART_ReceiveData(USART2);
 			if (zeichen=='\r')	// Ende Zeichenketten Eingabe
				{
					usart2_rx_buffer[j] = 0x00 ;
					sprintf(usart2_tx_buffer, "Zeichenkette=%s Länge=%d\r\n", usart2_rx_buffer, j);
					usart2_send(usart2_tx_buffer);
					memset(usart2_rx_buffer,0x00,20);
					j=0;
				}
			else
				{
					usart2_rx_buffer[j] = zeichen;
					j++;
					if (j >= 30) { j = 0; }
				}
		}
}
