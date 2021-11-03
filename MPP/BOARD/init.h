#ifndef INIT_H
#define INIT_H



//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
//#include "misc.h"
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
//#include "stm32f4xx_exti.h"
//#include "stm32f4xx_flash.h"
//#include "stm32f4xx_fsmc.h"
//#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
//#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
//#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
//#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
//#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"


//=========================================================================
// board_lib
//=========================================================================
#include "beeper.h"
//#include "client_ftp.h"
//#include "client_ntp.h"
//#include "global.h"
#include "i2c.h"
#include "init.h"
#include "interrupts.h"
#include "led.h"
#include "power.h"
//#include "rtc.h"
#include "taster.h"
#include "usart.h"
#include "test.h"


//=========================================================================
// MPP_lib
//=========================================================================
#include "CC3100.h"
#include "DW1000.h"
#include "SDCARD.h"
#include "dw1000.h"
#include "MPU9250.h"



//=========================================================================
// standard_lib
//=========================================================================
#include "stdio.h"
#include "string.h"



//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================

//========== Macros


//========== Variablen


//========== Funktionen

void init_board(void);

void InitSysTick(void);

void StartBootLoader(unsigned int BootLoaderStatus);


#endif
