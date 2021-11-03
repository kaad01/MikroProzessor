#ifndef TASTER_H
#define TASTER_H


//=========================================================================
// Anschlussbelegung
//=========================================================================
//	Taster1 (On/Off) - PC8 	liefert 1 wenn nicht gedrückt
//	Taster2 (Boot)	 - PC5	liefert 0 wenn nicht gedrückt
//=========================================================================


//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"


//=========================================================================
// board_lib
//=========================================================================
#include "beeper.h"
#include "init.h"
#include "led.h"
#include "usart.h"
#include "main.h"
#include "power.h"
#include "rtc.h"
#include "dw1000.h"

//=========================================================================
// standard_lib
//=========================================================================
#include "string.h"



//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================

//========== Macros


//========== Variablen


//========== Funktionen


void init_TASTER1(void);
void init_TASTER2(void);

void init_Taster1_IRQ8(void);
void init_Taster2_IRQ5(void);

void TASTER1_IRQ(void);
void TASTER2_IRQ(void);


#endif // TASTER_H
