#ifndef RTC_H_
#define RTC_H_

//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rng.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_syscfg.h"



//=========================================================================
// board_lib
//=========================================================================
#include "beeper.h"
#include "led.h"
#include "power.h"
#include "usart.h"
#include "SDCARD.h"



//=========================================================================
// standard_lib
//===========================================================================
#include "stdio.h"
#include "string.h"
#include <stdint.h>
#include <stdbool.h>
#include "time.h"



//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================

//=========================================================================
//========== Macros
//=========================================================================
#define ABSTAND_1970_2000 (946684800);

//=========================================================================
//========== Variablen
//=========================================================================
typedef struct {
	uint16_t year;	/* 1..4095 */
	uint8_t month; 	/* 1..12 */
	uint8_t mday; 	/* 1..31 */
	uint8_t wday; 	/* 0..6, Sunday = 0*/
	uint8_t hour; 	/* 0..23 */
	uint8_t min; 	/* 0..59 */
	uint8_t sec; 	/* 0..59 */
	uint8_t dst; 	/* 0 Winter, !=0 Summer */
} RTC_t;

typedef enum { TIMEBASE_1970 = 0,TIMEBASE_1980 = 1,TIMEBASE_2000= 2, TIMEBASE_1900=3} TimeBase;


extern RTC_TimeTypeDef ntp_sync_startTime;
extern RTC_TimeTypeDef ntp_sync_stop_Time;

extern RTC_DateTypeDef ntp_sync_startDate;
extern RTC_DateTypeDef ntp_sync_stop_Date;

//=========================================================================
//========== Funktionen
//=========================================================================


uint8_t start_RTC(void);
void init_RTC(void);
void RTC_IRQ_Handler(void);


void show_RTC_Time(void);
void show_RTC_Alarm(void);


void set_RTC_from_NTPsec(uint64_t sec, int8_t Zeitzone);


_Bool set_RTC_Alarm_in(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek, void (*callback)(void));
void set_autostart_in(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek);
void set_autostart_zeit(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek);
void start_RTC_Alarm(void);
void set_autostart_FSF(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek);

uint32_t rtc_getSek(TimeBase tb);
uint32_t convert_RTC_struct_to_sek(const RTC_TimeTypeDef *t, RTC_DateTypeDef *d, TimeBase tb);
int convert_RTC_sek_to_struct(uint32_t sec, RTC_TimeTypeDef *t, RTC_DateTypeDef *d, TimeBase tb);


uint32_t Zufallszahl(void);

#endif
