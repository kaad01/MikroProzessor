#ifndef __POWER_H__
#define __POWER_H__



//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"



//=========================================================================
// board_lib
//=========================================================================
#include "usart.h"



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

// ON/OFF Logik LTC2950
void init_POWER_ON(void);
void set_POWER_OFF(void);

// Spannungsmessung CR2032 RTC Batterie
void read_VBAT_RTC(void);




#endif // __POWER_H__
