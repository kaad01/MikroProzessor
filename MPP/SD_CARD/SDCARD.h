#ifndef __SDCARD_H__
#define __SDCARD_H__



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
#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
//#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
//#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
//#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"


//=========================================================================
// standard_lib
//=========================================================================
#include "stdio.h"
#include "string.h"



//=========================================================================
// board_lib
//=========================================================================
#include "usart.h"
#include "ff.h"
#include "diskio.h"
#include "fs.h"
#include "CC3100.h"




//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================

//========== Macros



//========== Variablen



//========== Funktionen

void sd_card_init(void);
void sd_card_SysTick_Handler(void);
int sd_card_copy_file_to_cc3100(char* sd_file_name, char* sf_file_name);

int sd_card_file_write (char* file_name, char* file_data, int bytes_to_write );
int sd_card_file_read (char* file_name, char* file_data, int bytes_to_read );

int sd_card_print_dir (char* sd_path);
int sd_card_print_file (char* sd_file_name);



#endif //__SDCARD_H__
