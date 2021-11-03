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


#ifndef _BOARD_H
#define _BOARD_H


//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
//#include "stm32f4xx_adc.h"
//#include "stm32f4xx_can.h"
//#include "stm32f4xx_crc.h"
//#include "stm32f4xx_cryp_aes.h"
//#include "stm32f4xx_cryp_des.h"
//#include "stm32f4xx_cryp_tdes.h"
//#include "stm32f4xx_cryp.h"
//#include "stm32f4xx_dac.h"
//#include "stm32f4xx_dbgmcu.h"
//#include "stm32f4xx_dcmi.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
//#include "stm32f4xx_flash.h"
//#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
//#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"



//=========================================================================
// standard_lib
//=========================================================================
#include "string.h"



//=========================================================================
// board_lib
//=========================================================================
#include "usart.h"



//=========================================================================
// spi interface
//=========================================================================

typedef unsigned int Fd_t;

extern unsigned char cc3100_rx_puffer[2048];
extern unsigned char cc3100_tx_puffer[2048];

Fd_t spi_Open(char *ifName, unsigned long flags);
int spi_Close(Fd_t fd);
int spi_Read(Fd_t fd, unsigned char *pBuff, int len);
int spi_Write(Fd_t fd, unsigned char *pBuff, int len);

int spi_dma_Write (Fd_t fd, unsigned char *pBuff, int len);
int spi_dma_Read (Fd_t fd, unsigned char *pBuff, int len);
void DMA1_Stream4_IRQHandler(void);
void DMA_Int_Handler(void);




//=========================================================================
// cc3100 interface
//=========================================================================

void Delay(unsigned long interval);
void wait_uSek_CC3100(unsigned long us);

void CC3100_init();
void CC3100_enable();
void CC3100_disable();

typedef void (*P_EVENT_HANDLER)(void* pValue);
int registerInterruptHandler(P_EVENT_HANDLER InterruptHdl , void* pValue);

void CC3100_InterruptEnable();
void CC3100_InterruptDisable();

void WLAN_intHandler();
void UART1_intHandler();

void MaskIntHdlr();
void UnMaskIntHdlr();





//=========================================================================
// cc3100 serial flash formatieren
//=========================================================================

extern int 	ack_cc3100_resceived;

int CC3100_format_serial_flash(void);
void CC3100_uart6_receive_IRQ(void);

#endif
