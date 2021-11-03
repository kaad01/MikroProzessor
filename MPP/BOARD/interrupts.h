#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__



//=========================================================================
// Funktionen der CMSIS zum Zugriff auf den NVIC
//=========================================================================
//	NVIC_EnableIRQ(IRQn_Type IRQn);		// ausgewählte IRQ
//	NVIC_DisableIRQ(IRQn_Type IRQn);	// ausgewählte IRQ
//
//	NVIC_SetPendingIRQ(IRQn_Type IRQn);
//	NVIC_ClearPendingIRQ(IRQn_Type IRQn);
//	uint32_t NVIC_GetPendingIRQ(IRQn_Type IRQn);
//
//	void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority);
//	uint32_t NVIC_GetPriority(SysTick_IRQn);
//
//	__enable_irq(); 	// alle IRQ
//	__disable_irq();	// alle IRQ




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
//#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
//#include "stm32f4xx_flash.h"
//#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
//#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
//#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
#include "stm32f4xx_wwdg.h"




//=========================================================================
// board_lib
//=========================================================================
//#include "beeper.h"
//#include "client_ftp.h"
//#include "client_ntp.h"
//#include "global.h"
//#include "i2c.h"
//#include "init.h"
//#include "interrupts.h"
#include "led.h"
//#include "power.h"
//#include "rtc.h"
#include "taster.h"
#include "usart.h"



//=========================================================================
// MPP_lib
//=========================================================================

// CC3100
#include "simplelink.h"
#include "CC3100_Board.h"
#include "nonos.h"
#include "CC3100.h"
#include "CC3100_EventHandler.h"
#include "MQTT.h"

// client
#include "client_ftp.h"
#include "client_ntp.h"

// BME280
#include "BME280.h"

// SDCARD
#include "SDCARD.h"

// DW1000
#include "global.h"
#include "dw1000_driver.h"
#include "DW1000.h"

// CoOS
#include "arch.h"




//=========================================================================
// standard_lib
//=========================================================================
#include "stdio.h"
#include "string.h"




//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================

//========== Macros

#define USART2_TX_BUFFERSIZE 1024
#define USART2_RX_BUFFERSIZE 1024

#define uart_send usart2_send

//========== Variablen
extern int32_t timer;
extern int32_t Tas1;
extern int32_t Tas2;
extern int16_t T1;
extern int16_t T2;
extern char tasten[50];

//========== Funktionen




#endif // __INTERRUPTS_H__
