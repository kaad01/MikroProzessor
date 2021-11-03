#ifndef __UART_H__
#define __UART_H__




//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_usart.h"



//=========================================================================
// board_lib
//=========================================================================



//=========================================================================
// standard_lib
//=========================================================================
#include "stdio.h"
#include "string.h"
#include "global.h"
#include "stdarg.h"
#include "stdlib.h"


//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================

//========== Macros

#define USART2_TX_BUFFERSIZE 1024
#define USART2_RX_BUFFERSIZE 1024

#define uart_send usart2_send

//========== Variablen
extern char uart2_RX_zeichen;

//========== Funktionen

void usart2_init(void);
void USART2_IRQ(void);

void usart2_printf(const char* format, ...);
void usart2_printf_unblocking(const char* format, ...);

void usart2_send(char* chars);
void usart2_send_unblocking(char* chars);


#endif // __UART_H__
