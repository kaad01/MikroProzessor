#ifndef __BEEPER_H__
#define __BEEPER_H__



//=========================================================================
// Anschlussbelegung
//=========================================================================
//	BEEPER	PB8		TIM10C1 oder ...
//=========================================================================
//	Timer10 und Timer7 werden eingesetzt
//
//void TIM7_IRQHandler(void) wird benötigt


//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_tim.h"


//=========================================================================
// board_lib
//=========================================================================


//=========================================================================
// standard_lib
//=========================================================================


//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================

//========== Macros
#define alarm_ton_len 	(20)
#define TON_SEQUENZ 	(1)
#define TON_EINZELN		(0)

//========== Variablen
extern short unsigned int alarm_ton[2][alarm_ton_len];
extern int ton_ausgabe;
//========== Funktionen

void init_BEEPER(void);

void timer_tonfrequenz(unsigned short int frequenz_in_Hz);
void timer_tondauer( unsigned short int time_in_ms);

void beep( 	unsigned short int ton_frequenz,
			unsigned short int ton_dauer,
			unsigned short int ton_pause );

void beeper_sirene(void);

void BEEPER_IRQHandler(void);

#endif // __BEEPER_H__
