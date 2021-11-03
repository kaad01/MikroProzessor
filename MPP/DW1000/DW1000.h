#ifndef __DW1000_H_
#define __DW1000_H_


//#########################################################################
//########## cmsis_lib
//#########################################################################
//#include "stm32f4xx.h"
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


//#########################################################################
//########## mpp_lib
//#########################################################################
//#include "ADXL363.h"
//#include "BME280.h"
////=========================================================================
//#include "beeper.h"
//#include "flash.h"
//#include "global.h"
//#include "i2c.h"
//#include "init.h"
//#include "interrupts.h"
//#include "led.h"
//#include "power.h"
//#include "rtc.h"
//#include "taster.h"
#include "usart.h"
////=========================================================================
//#include "dw1000_def.h"
#include "dw1000_base.h"
#include "dw1000_user.h"
#include "dw1000_ranging.h"
//#include "DW1000.h"
#include "MPU9250.h"

//#########################################################################
//########## standard_lib
//#########################################################################
#include "stdio.h"
#include "string.h"
//#include "math.h"



//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================



//=========================================================================
//========== Macros
//=========================================================================



//=========================================================================
//========== Variablen
//=========================================================================
// Variable fuer das Timeout-Handling
extern unsigned long systickcounter;
extern int dw1000_timeout_ms;
// Globale Variablen fuer die Entfernung von den vier Ankerpunkten
extern float r1;
extern float r2;
extern float r3;
extern float r4;
extern unsigned int knoten_id;
extern char uwb_send;
extern char temp[100];

//=========================================================================
//========== Funktionen
//=========================================================================


// DW1000 Initialisierung und in den RX Mode setzen
void DW1000_init(void);

// Eventhandler für abgeschlossene Sendeprozesse
void HandleTransmission(	unsigned long long timestamp,
							float temperature,
							unsigned char error);

// Eventhandler für abgeschlossene Entfernungsmessungen
void HandleRangeReceived(	unsigned short remoteinstance,
							float range);

// Eventhandler für eingehende Textmitteilungen
void HandleTextMessageReceived(	unsigned short sender,
								unsigned char* content,
								unsigned long long int rxtimestamp,
								double rxpower,
								float rxtemperature);

// Eventhandler für eingehende Bytearraymitteilungen
void HandleByteMessageReceived(	unsigned short sender,
								unsigned char* content,
								int contentlength,
								unsigned long long int rxtimestamp,
								double rxpower,
								float rxtemperature);

void distanz(unsigned int id);
void position(void);
void lokalisieren(void);

#endif // DW1000
