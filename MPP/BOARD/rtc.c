#include "rtc.h"


// Anlegen der Structs für aktuelle Daten
RTC_TimeTypeDef RTC_Time_Aktuell; 	// 	Zeit
RTC_DateTypeDef RTC_Date_Aktuell; 	// 	Datum
RTC_AlarmTypeDef RTC_Alarm_Aktuell; //	Alarm

// Anlegen der Structs zur Initialisierung
RTC_TimeTypeDef RTC_Time_Struct; 	// 	Zeit
RTC_DateTypeDef RTC_Date_Struct; 	// 	Datum
RTC_AlarmTypeDef RTC_Alarm_Struct; 	//	Alarm
RTC_InitTypeDef RTC_Init_Struct; 	//	Zeitformat und Vorteiler

//
NVIC_InitTypeDef NVIC_InitStruct; 	//
EXTI_InitTypeDef EXTI_InitStruct; 	//

//
RTC_InitTypeDef RTC_InitStructure;
RTC_TimeTypeDef RTC_TimeStructure;

uint32_t AsynchPrediv = 0, SynchPrediv = 0;

#define FIRSTYEAR   2000		// start year
#define FIRSTDAY    1			// 1 = Monday
static const uint8_t DaysInMonth[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

RTC_TimeTypeDef ntp_sync_startTime;
RTC_TimeTypeDef ntp_sync_stop_Time;
RTC_DateTypeDef ntp_sync_startDate;
RTC_DateTypeDef ntp_sync_stop_Date;


static bool isDST(const RTC_TimeTypeDef *t, const RTC_DateTypeDef *d);
static _Bool isLeapYear(uint16_t year);
static bool adjustDST(RTC_TimeTypeDef *t, RTC_DateTypeDef *d);

char outbuffer[100] = {0};

void (*RTC_Alarm_CallBack[10])(void) = {NULL};


/*******************************************************************************
 * Function Name  : set_RTC_from_NTPsec
 * Description    : Setzt anhand die RTC Zeit (RTC_Time_Aktuell und RTC_Date_Aktuell)
 * 					anhand der Sekundenanzahl seit dem 1.1.1970 00:00:00
 * Input          : uint64_t - Sekunden(32Bit)/Subsekunden(32Bit)
 * Output         : None
 * Return         : None
 *******************************************************************************/
void set_RTC_from_NTPsec(uint64_t sec, int8_t Zeitzone) {
	RTC_TimeTypeDef t;
	RTC_DateTypeDef d;
	uint32_t pureSec = (sec >> 32);
	uint32_t start = 0;
	uint32_t stop = 0;
	uint32_t diff = 0;
	char data[100] = { 0 };
	char* wochentag[] = {"Mo","Di","Mi","Do","Fr","Sa","So"};

	start = convert_RTC_struct_to_sek(&ntp_sync_startTime, &ntp_sync_startDate, TIMEBASE_2000);
	stop = convert_RTC_struct_to_sek(&ntp_sync_stop_Time, &ntp_sync_stop_Date, 	TIMEBASE_2000);
	diff = (stop - start) / 2;

	pureSec += diff;
	convert_RTC_sek_to_struct(pureSec, &t, &d, TIMEBASE_1900);
	adjustDST(&t, &d);

	RTC_Date_Aktuell.RTC_Year = d.RTC_Year;
	RTC_Date_Aktuell.RTC_Month = d.RTC_Month;
	RTC_Date_Aktuell.RTC_Date = d.RTC_Date;
	RTC_Date_Aktuell.RTC_WeekDay = d.RTC_WeekDay;

	RTC_Time_Aktuell.RTC_Hours = t.RTC_Hours + Zeitzone;
	RTC_Time_Aktuell.RTC_Minutes = t.RTC_Minutes;
	RTC_Time_Aktuell.RTC_Seconds = t.RTC_Seconds;

	RTC_SetDate(RTC_Format_BIN, &RTC_Date_Aktuell);
	RTC_SetTime(RTC_Format_BIN, &RTC_Time_Aktuell);

	start_RTC();


	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);
	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);

	sprintf(data, "\r\nRTC-NTP Aktualisierung: %.2d:%.2d:%.2d Uhr,",
			RTC_Time_Aktuell.RTC_Hours,
			RTC_Time_Aktuell.RTC_Minutes,
			RTC_Time_Aktuell.RTC_Seconds);
	usart2_send(data);

	sprintf(data, "%s., d. %.2d.%.2d.%.2d \r\n",
			wochentag[RTC_Date_Aktuell.RTC_WeekDay-1],
			RTC_Date_Aktuell.RTC_Date,
			RTC_Date_Aktuell.RTC_Month,
			RTC_Date_Aktuell.RTC_Year + 2000);
	usart2_send(data);
}


//=========================================================================
// wandelt Sekunden in das RTC Format
//=========================================================================
int convert_RTC_sek_to_struct(uint32_t sec, RTC_TimeTypeDef *t,
		RTC_DateTypeDef *d, TimeBase tb) {
	uint16_t day;
	uint16_t year;
	uint16_t dayofyear;
//	uint8_t leap400;
	uint8_t month;
	uint16_t erstesJahr;

// Sekunden berechnen
	t->RTC_Seconds = sec % 60;
// Minuten berechenen
	sec /= 60;
	t->RTC_Minutes = sec % 60;
// Stunden berechenen
	sec /= 60;
	t->RTC_Hours = sec % 24;
// Tage berechnen
	day = (uint16_t)(sec / 24);
// Wochentag berechnen Firstday =6
	d->RTC_WeekDay = (day + FIRSTDAY) % 7; // weekday
// Jahr berechnen von 0...99
	switch (tb) {
	case 0:
		erstesJahr = 1970;
		break;
	case 1:
		erstesJahr = 1980;
		break;
	case 2:
		erstesJahr = 2000;
		break;
	case 3:
		erstesJahr = 1900;
		break;
	}
// Schaltjahr
	year = erstesJahr; // %100 vorher 0..99
//	leap400 = 4 - ((erstesJahr - 1) / 100 & 3); // 4, 3, 2, 1

	for (;;) {
		dayofyear = 365;
		if ((year % 4) == 0) {
			dayofyear = 366; // leap year
			if (year % 100 == 0) { // 100 year exception
				if (year % 400 == 0) { // 400 year exception
				} else {
					dayofyear = 365;
				}
			}
		}
		if (day < dayofyear) {
			break;
		}
		day -= dayofyear;
		year++; // 00..136 / 99..235
	}
// Jahr eintragen
	d->RTC_Year = year - 2000; // + century
	if (year < 2000)
		return -1;

	if (dayofyear & 1 && day > 58) { // no leap year and after 28.2.
		day++; // skip 29.2.
	}

	for (month = 1; day >= DaysInMonth[month - 1]; month++) {
		day -= DaysInMonth[month - 1];
	}

// Monat und Tag eintragen
	d->RTC_Month = month; // 1..12
	d->RTC_Date = day + 1; // 1..31
	return 0;
}

//=========================================================================
// wandelt RTC Format in Sekunden nach dem 1.1.1970/1980/2000, 00:00:00 Uhr
//=========================================================================

uint32_t convert_RTC_struct_to_sek(const RTC_TimeTypeDef *t, RTC_DateTypeDef *d,
		TimeBase tb) {
	uint8_t i;
	uint32_t result = 0;
	uint16_t idx, year;
	//86400Sek = ein Tag

	year = d->RTC_Year + 2000;

	/* Calculate days since 1.1.0000 */
	result = (uint32_t) year * 365;
	if (d->RTC_Year >= 1) {
		result += (year + 3) / 4;
		result -= (year - 1) / 100;
		result += (year - 1) / 400;
	}

	/* Start with tb a.d. */
	switch (tb) {
	case 0:
		result -= 719528UL; // Abstand 1.1.0000 bis 1.1.1970 = 719528 Tage
		break;
	case 1:
		result -= 723180UL; // Abstand 1.1.0000 bis 1.1.1980 = 723180 Tage
		break;
	case 2:
		result -= 730485UL; // Abstand 1.1.0000 bis 1.1.2000
		break;
	case 3:
		result -= 693961UL; // Abstand 1.1.0000 bis 1.1.1900
		break;
	}

	/* Make month an array index */
	if (d->RTC_Month == 0)
		d->RTC_Month = 1;
	idx = d->RTC_Month - 1;

	/* Loop thru each month, adding the days */
	for (i = 0; i < idx; i++) {
		result += DaysInMonth[i];
	}

	/* Leap year? adjust February */
	if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) {
		;
	} else {
		if (d->RTC_Month > 1) {
			result--;
		}
	}

	/* Add remaining days */
	if (d->RTC_Date == 0)
		d->RTC_Date = 1;
	result += d->RTC_Date;

	/* Convert to seconds, add all the other stuff */
	result = (result - 1) * 86400L + (uint32_t) t->RTC_Hours * 3600
			+ (uint32_t) t->RTC_Minutes * 60 + t->RTC_Seconds;

	return result;
}



/*******************************************************************************
 * Function Name  : rtc_getSek
 * Description    : gibt die aktuelle Zeit in Sekunden zurück
 * Input          : None
 * Output         : None
 * Return         : Sekunden seit TimeBase
 *******************************************************************************/
uint32_t rtc_getSek(TimeBase tb) {

	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);
	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);

	return convert_RTC_struct_to_sek(&RTC_Time_Aktuell, &RTC_Date_Aktuell,
			tb);

}


uint8_t start_RTC(void) {
	uint8_t state ;

	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x32F2)
	{
		// RTC war noch nicht initialisiert ...
		init_RTC();
		state = 1;
	}
	else
	{	// RCC_GetFlagStatus(Flag) check die folgende Flags
		// RCC_FLAG_HSIRDY: 	HSI oscillator clock ready
		// RCC_FLAG_HSERDY: 	HSE oscillator clock ready
		// RCC_FLAG_PLLRDY: 	main PLL clock ready
		// RCC_FLAG_PLLI2SRDY: 	PLLI2S clock ready
		// RCC_FLAG_LSERDY: 	LSE oscillator clock ready
		// RCC_FLAG_LSIRDY: 	LSI oscillator clock ready
		// RCC_FLAG_BORRST: 	POR/PDR or BOR reset
		// RCC_FLAG_PINRST: 	Pin reset
		// RCC_FLAG_PORRST: 	POR/PDR reset
		// RCC_FLAG_SFTRST: 	Software reset
		// RCC_FLAG_IWDGRST: 	Independent Watchdog reset
		// RCC_FLAG_WWDGRST: 	Window Watchdog reset
		// RCC_FLAG_LPWRRST: 	Low Power reset

		// wenn die RTC schon initialisiert war ...

		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
			{
			//usart2_send("Behandlung des Power On Reset\r\n");
			// Behandlung des Power On Reset
			}
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
			{
			//usart2_send("Behandlung des Externen Resets\r\n");
			// Behandlung des Externen Resets
			}


		if (PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)
		{
			//	Wake Up flag
			// This flag indicates that a wakeup event
			// was received from the WKUP pin or from the
			// RTC alarm (Alarm A or Alarm B),
			// RTC Tamper event, RTC TimeStamp event or RTC Wakeup.
			// An additional wakeup event is detected if the WKUP pin is enabled
			// (by setting the EWUP bit) when the WKUP pin level is already high

			// ToDo
			//usart2_send("PWR_FLAG_WU\r\n");

		}
//
//
		if (PWR_GetFlagStatus(PWR_FLAG_SB) != RESET)
		{
			// StandBy flag gesetzt wenn Rückkehr aus dem StandBy mode

			// ToDO
			//usart2_send("PWR_FLAG_SB\r\n");
		}
//
//		if (PWR_GetFlagStatus(PWR_FLAG_BRR) != RESET)
//		{
//			// PWR_FLAG_BRR: Backup regulator ready flag. This bit is not reset
//			// when the device wakes up from Standby mode or by a system reset
//			// or power reset.
//		}
//
//		if (PWR_GetFlagStatus(PWR_FLAG_PVDO) != RESET)
//		{
//			// PWR_FLAG_PVDO: PVD Output. This flag is valid only if PVD is enabled
//			// by the PWR_PVDCmd() function. The PVD is stopped by Standby mode
//			// For this reason, this bit is equal to 0 after Standby or reset
//			// until the PVDE bit is set.
//		}
//
//		if (PWR_GetFlagStatus(PWR_FLAG_VOSRDY) != RESET)
//		{
//			// PWR_FLAG_VOSRDY: This flag indicates that the Regulator voltage
//			// scaling output selection is ready.
//		}


		// PWR Clock einschalten
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		// Zugriff auf RTC erlauben
		PWR_BackupAccessCmd(ENABLE);

		// Warten auf die RTC APB registers Synchronisation
		RTC_WaitForSynchro();

		state = 0;
	}

	// Autostart erfolgt über RTC Alarm A mit PC13
	// oder ON/OFF Taster manuell betätigt

	// RTC Alarm A&B sperren
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	RTC_ITConfig(RTC_IT_ALRA, DISABLE);
	RTC_ITConfig(RTC_IT_ALRB, DISABLE);

	// Alarm Flags
	RTC_ClearFlag(RTC_FLAG_ALRAF);
	RTC_ClearFlag(RTC_FLAG_ALRBF);
	EXTI_ClearITPendingBit(EXTI_Line17);

	// Timestamp und Tamper
	RTC_ClearITPendingBit(RTC_IT_TS);
	RTC_ClearITPendingBit(RTC_IT_TAMP1);
	EXTI_ClearITPendingBit(EXTI_Line21);

	// WakeUp
	RTC_ClearITPendingBit(RTC_IT_WUT);
	EXTI_ClearITPendingBit(EXTI_Line22);

	// PC13 Output
	RTC_OutputConfig(RTC_Output_Disable, RTC_OutputPolarity_High);
	RTC_OutputTypeConfig(RTC_OutputType_OpenDrain);

	return state;
}

void init_RTC(void) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);

// Enable LSE OSC
	RCC_LSEConfig(RCC_LSE_ON);

// Warten bis LSE bereit
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
		;
	}

// Auswahl der RTC Clock Source
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

// RTC Clock freigeben
	RCC_RTCCLKCmd(ENABLE);

// RTC APB Register Synchronisation
	RTC_WaitForSynchro();

// Init RTC Data Register and RTC Prescaler
	RTC_InitTypeDef RTC_Init_Struct;
	RTC_Init_Struct.RTC_AsynchPrediv = 0x7F;
	RTC_Init_Struct.RTC_SynchPrediv = 0xFF;
	RTC_Init_Struct.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_Init_Struct);

// Alarm, Datum und Zeit initial setzen
// Datum: Sonnabend, den 01.01.2000 Zeit: 00:00:00 Uhr
// 1. Tag de Monats 00:00:05 Uhr

// Alarm initial setzen und sperren
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours = 0x00;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes = 0x00;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds = 0x00;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDay = 0x00;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	RTC_Alarm_Struct.RTC_AlarmMask = RTC_AlarmMask_None;
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_Alarm_Struct);
// RTC Alarm A Interrupt freigeben
	RTC_ITConfig(RTC_IT_ALRA, DISABLE);
// Alarm freigeben
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
// Flag zurücksetzen
	RTC_ClearFlag(RTC_FLAG_ALRAF);

// Datum initial auf Sonnabend den 01.01.2000 setzen
	RTC_Date_Struct.RTC_Year = 0x00;
	RTC_Date_Struct.RTC_Month = RTC_Month_January;
	RTC_Date_Struct.RTC_Date = 0x01;
	RTC_Date_Struct.RTC_WeekDay = RTC_Weekday_Saturday;
	RTC_SetDate(RTC_Format_BCD, &RTC_Date_Struct);

// Zeit initial auf 00:00:00 Uhr setzen
	RTC_Time_Struct.RTC_H12 = RTC_H12_AM;
	RTC_Time_Struct.RTC_Hours = 0x00;
	RTC_Time_Struct.RTC_Minutes = 0x00;
	RTC_Time_Struct.RTC_Seconds = 0x00;
	RTC_SetTime(RTC_Format_BCD, &RTC_Time_Struct);

// Indikator für die RTC konfiguration setzen
	RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
}


void autostart(void)
{
		//set_RTC_Alarm_in(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek, void (*callback
		set_RTC_Alarm_in(0, 0, 0, 30, NULL);
		// Alarm, Datum und Zeit initial setzen
		// Datum: Sonnabend, den 01.01.2000 Zeit: 00:00:00 Uhr
		// 1. Tag de Monats 00:00:05 Uhr

		// Alarm initial setzen und sperren
		RTC_Alarm_Struct.RTC_AlarmTime.RTC_H12		= RTC_H12_AM;
		RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours	= 0x00;
		RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes	= 0x00;
		RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds	= 0x05;
		RTC_Alarm_Struct.RTC_AlarmDateWeekDay		= 0x01;
		RTC_Alarm_Struct.RTC_AlarmDateWeekDaySel	= RTC_AlarmDateWeekDaySel_Date;
		RTC_Alarm_Struct.RTC_AlarmMask				= RTC_AlarmMask_DateWeekDay;
		RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_Alarm_Struct);
		// RTC Alarm A Interrupt freigeben
		RTC_ITConfig(RTC_IT_ALRA, DISABLE);
		// Alarm freigeben
		RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
		// Flag zurücksetzen
		RTC_ClearFlag(RTC_FLAG_ALRAF);

		// aktiviert für den ALARM A PC13 als Open Drain
		// bei Alarm Low-Pegel
		// zum Einschalten der Versorgungsspannung
		RTC_OutputConfig(RTC_Output_AlarmA, RTC_OutputPolarity_Low);
		RTC_OutputTypeConfig(RTC_OutputType_OpenDrain);

		// Indikator für die RTC konfiguration setzen
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x32F2);
}

void show_RTC_Time(void) {
	char data[50] = { 0 };
	char* wochentag[] = {"Mo","Di","Mi","Do","Fr","Sa","So"};

	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);
	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);

	sprintf(data, "\r\nRTC-Zeit:  %.2d:%.2d:%.2d Uhr, ",
			RTC_Time_Aktuell.RTC_Hours,
			RTC_Time_Aktuell.RTC_Minutes,
			RTC_Time_Aktuell.RTC_Seconds);
	usart2_send(data);

	sprintf(data, "%s., d. %.2d.%.2d.%.2d \r\n",
			wochentag[RTC_Date_Aktuell.RTC_WeekDay-1],
			RTC_Date_Aktuell.RTC_Date,
			RTC_Date_Aktuell.RTC_Month,
			RTC_Date_Aktuell.RTC_Year + 2000);
	usart2_send(data);
}

void show_RTC_Alarm(void) {
	char data[50] = { 0 };
	char* wochentag[] = {"Mo","Di","Mi","Do","Fr","Sa","So"};
	RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_Alarm_Aktuell);

	sprintf((char*) data, "RTC-Alarm: %1.2d:%1.2d:%1.2d Uhr, ",
			RTC_Alarm_Aktuell.RTC_AlarmTime.RTC_Hours,
			RTC_Alarm_Aktuell.RTC_AlarmTime.RTC_Minutes,
			RTC_Alarm_Aktuell.RTC_AlarmTime.RTC_Seconds);
	uart_send(data);

	// Tag des Monats
	if ( RTC_Alarm_Aktuell.RTC_AlarmDateWeekDaySel == RTC_AlarmDateWeekDaySel_Date )
	{
		sprintf(data, "am %1.2d. Tag des Monats\r\n", RTC_Alarm_Aktuell.RTC_AlarmDateWeekDay);
		usart2_send(data);
	}

	// Wochentag
	if ( RTC_Alarm_Aktuell.RTC_AlarmDateWeekDaySel == RTC_AlarmDateWeekDaySel_WeekDay )
	{
		sprintf(data, " am %s.\r\n", wochentag[RTC_Alarm_Aktuell.RTC_AlarmDateWeekDay]);
		usart2_send(data);
	}
}

_Bool Zeit_ueberlauf_Korektur(RTC_AlarmTypeDef* r) {
	uint8_t MonatsTageAnzahl = 0;
	_Bool Alarm_is_next_Month = false;
	if (r->RTC_AlarmTime.RTC_Seconds >= 60) {
		r->RTC_AlarmTime.RTC_Minutes += r->RTC_AlarmTime.RTC_Seconds / 60;
		r->RTC_AlarmTime.RTC_Seconds = r->RTC_AlarmTime.RTC_Seconds % 60;
	}
	if (r->RTC_AlarmTime.RTC_Minutes >= 60) {
		r->RTC_AlarmTime.RTC_Hours += r->RTC_AlarmTime.RTC_Minutes / 60;
		r->RTC_AlarmTime.RTC_Minutes = r->RTC_AlarmTime.RTC_Minutes % 60;
	}
	if (r->RTC_AlarmTime.RTC_Hours >= 24) {
		r->RTC_AlarmDateWeekDay += r->RTC_AlarmTime.RTC_Hours / 24;
		r->RTC_AlarmTime.RTC_Hours = r->RTC_AlarmTime.RTC_Hours % 24;
	}

	MonatsTageAnzahl = DaysInMonth[RTC_Date_Aktuell.RTC_Month - 1];

	if (isLeapYear(RTC_Date_Aktuell.RTC_Year) == false
			&& RTC_Date_Aktuell.RTC_Month == 2)
		MonatsTageAnzahl--;
	if (r->RTC_AlarmDateWeekDay >= MonatsTageAnzahl) {
		r->RTC_AlarmDateWeekDay = r->RTC_AlarmDateWeekDay - MonatsTageAnzahl;
		Alarm_is_next_Month = true;
	}
	if (RTC_Date_Aktuell.RTC_Date
			< r->RTC_AlarmDateWeekDay&& Alarm_is_next_Month == true)
		return false;
	return true;
}


void set_autostart_FSF(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek)
{
	_Bool setzen_moeglich = false;	//TODO
	char alarmOutput[128];

	//=== Alarm vor dem stellen ausschalten =========================
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	//=== aktuelle Zeit auslesen ====================================
	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);
	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);

	//=== Alarm Struct füllen =======================================
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours = RTC_Time_Aktuell.RTC_Hours + Std;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes = RTC_Time_Aktuell.RTC_Minutes + Min;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds = RTC_Time_Aktuell.RTC_Seconds + Sek;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDay = RTC_Date_Aktuell.RTC_Date + Tagen;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;

	//=== Alarm Maske setzen ========================================
	RTC_Alarm_Struct.RTC_AlarmMask = RTC_AlarmMask_None;

	//=== Überläufe der Sek, Min, Std und Tage korrigieren ==========
	setzen_moeglich = Zeit_ueberlauf_Korektur(&RTC_Alarm_Struct);

	sprintf(alarmOutput, "\r\nAutostart nach Formatierung erfolgt um %1.2d:%1.2d:%1.2d Uhr\r\n",
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours,
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes,
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds);
	uart_send(alarmOutput);

	//=== Wenn Alarmzeit einstellbar dann einstellen ================
	if (setzen_moeglich == true)
		{
		// Init RTC Alarm A register
		RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_Alarm_Struct);
		// RTC Alarm A Interrupt freigeben
		RTC_ITConfig(RTC_IT_ALRA, DISABLE);
		// Alarm freigeben
		RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
		// Flag zurücksetzen
		RTC_ClearFlag(RTC_FLAG_ALRAF);
		// aktiviert für den ALARM A PC13 als Open Drain
		// bei Alarm Low-Pegel
		// zum Einschalten der Versorgungsspannung
		RTC_OutputConfig(RTC_Output_AlarmA, RTC_OutputPolarity_Low);
		RTC_OutputTypeConfig(RTC_OutputType_OpenDrain);

		RTC_ITConfig(RTC_IT_ALRA, ENABLE);
		// Alarm freigeben
		RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

		}

}


void set_autostart_in(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek)
{
	_Bool setzen_moeglich = false;	//TODO
	char alarmOutput[128];

	//=== Alarm vor dem stellen ausschalten =========================
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	//=== aktuelle Zeit auslesen ====================================
	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);
	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);

	//=== Alarm Struct füllen =======================================
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours = RTC_Time_Aktuell.RTC_Hours + Std;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes = RTC_Time_Aktuell.RTC_Minutes + Min;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds = RTC_Time_Aktuell.RTC_Seconds + Sek;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDay = RTC_Date_Aktuell.RTC_Date + Tagen;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;

	//=== Alarm Maske setzen ========================================
	RTC_Alarm_Struct.RTC_AlarmMask = RTC_AlarmMask_None;

	//=== Überläufe der Sek, Min, Std und Tage korrigieren ==========
	setzen_moeglich = Zeit_ueberlauf_Korektur(&RTC_Alarm_Struct);

	sprintf(alarmOutput, "Autostart erfolgt um %1.2d:%1.2d:%1.2d Uhr, am %1.2d. Tag des Monats\r\n",
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours,
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes,
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds,
			RTC_Alarm_Struct.RTC_AlarmDateWeekDay);
	uart_send(alarmOutput);

	//=== Wenn Alarmzeit einstellbar dann einstellen ================
	if (setzen_moeglich == true)
		{
		// Init RTC Alarm A register
		RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_Alarm_Struct);
		// RTC Alarm A Interrupt freigeben
		RTC_ITConfig(RTC_IT_ALRA, DISABLE);
		// Alarm freigeben
		RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
		// Flag zurücksetzen
		RTC_ClearFlag(RTC_FLAG_ALRAF);
		// aktiviert für den ALARM A PC13 als Open Drain
		// bei Alarm Low-Pegel
		// zum Einschalten der Versorgungsspannung
		RTC_OutputConfig(RTC_Output_AlarmA, RTC_OutputPolarity_Low);
		RTC_OutputTypeConfig(RTC_OutputType_OpenDrain);

		RTC_ITConfig(RTC_IT_ALRA, ENABLE);
		// Alarm freigeben
		RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

		beep(4000,200,200);
		beep(4000,200,200);
		show_RTC_Alarm();			// Ausgabe eingestellte Alarmzeit
		beep(4000,200,200);

		set_POWER_OFF();

		}

}


void set_autostart_zeit(uint8_t Wochentag, uint8_t Std, uint8_t Min, uint8_t Sek)
{

	uint32_t alarm_maske = RTC_AlarmMask_All;	//  ((uint32_t)0x80808080)

	//=== Alarm vor dem stellen ausschalten =========================
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	//=== aktuelle Zeit auslesen ====================================
	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);
	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);

	if (Wochentag != 0) { alarm_maske &= ~RTC_AlarmMask_DateWeekDay; }	//  ((uint32_t)0x80000000)
	if (Std != 0) { alarm_maske &= ~RTC_AlarmMask_Hours; }			//  ((uint32_t)0x00800000)
	if (Min != 0) { alarm_maske &= ~RTC_AlarmMask_Minutes; }      	//  ((uint32_t)0x00008000)
	if (Sek != 0) { alarm_maske &= ~RTC_AlarmMask_Seconds; }    	//  ((uint32_t)0x00000080)

	// löst jede Sekunde aus
	if (Wochentag == 0 && Std == 0 && Min == 0 && Sek==0 ) { alarm_maske = RTC_AlarmMask_All; }


	//=== Alarm Struct füllen =======================================
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours = Std;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes = Min;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds = Sek;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDay = Wochentag;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_WeekDay; // Wochentag 1-7
	//=== Alarm Maske setzen ========================================
	RTC_Alarm_Struct.RTC_AlarmMask = alarm_maske;

	usart2_printf("RTC Alarm gestellt auf %d:%d:%d Uhr am %d. Tag des Monats\r\n",
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours,
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes,
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds,
			RTC_Alarm_Struct.RTC_AlarmDateWeekDay);

	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_Alarm_Struct);
	RTC_ITConfig(RTC_IT_ALRA, DISABLE);
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	RTC_ClearFlag(RTC_FLAG_ALRAF);
	RTC_OutputConfig(RTC_Output_AlarmA, RTC_OutputPolarity_Low);
	RTC_OutputTypeConfig(RTC_OutputType_OpenDrain);

	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

	beep(4000,200,200);
	beep(4000,200,200);
	beep(4000,200,200);
	set_POWER_OFF();
}


_Bool set_RTC_Alarm_in(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek,
		void (*callback)(void)) {

	_Bool setzen_moeglich = false;	//TODO
	char alarmOutput[128];

	//=== Alarm vor dem stellen ausschalten =========================
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	//=== aktuelle Zeit auslesen ====================================
	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);
	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);

	//=== Alarm Struct füllen =======================================
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours = RTC_Time_Aktuell.RTC_Hours + Std;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes = RTC_Time_Aktuell.RTC_Minutes
			+ Min;
	RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds = RTC_Time_Aktuell.RTC_Seconds
			+ Sek;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDay = RTC_Date_Aktuell.RTC_Date + Tagen;
	RTC_Alarm_Struct.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
	//=== Alarm Maske setzen ========================================
	RTC_Alarm_Struct.RTC_AlarmMask = RTC_AlarmMask_None;
	//=== Überläufe der Sek, Min, Std und Tage korrigieren ==========
	setzen_moeglich = Zeit_ueberlauf_Korektur(&RTC_Alarm_Struct);

	sprintf(alarmOutput, "RTC Alarm gestellt auf %d:%d:%d Uhr am %d. Tag des Monats\r\n",
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Hours,
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Minutes,
			RTC_Alarm_Struct.RTC_AlarmTime.RTC_Seconds,
			RTC_Alarm_Struct.RTC_AlarmDateWeekDay);
	uart_send(alarmOutput);

	//=== Wenn Alarmzeit einstellbar dann einstellen ================
	if (setzen_moeglich == true) {
		// Init RTC Alarm A register
		RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_Alarm_Struct);
		if (callback != NULL) {
		RTC_Alarm_CallBack[0] = callback;
		}
		return true;
	}
	return false;
}

void start_RTC_Alarm(void) {
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

//=== RTC Alarm A Interruptkonfiguration =====================
// EXTI-Line Konfiguration
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStruct.EXTI_Line = EXTI_Line17;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);

// NIVC Konfiguration
	NVIC_InitStruct.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
	RTC_ClearFlag(RTC_FLAG_ALRAF);
}


void RTC_IRQ_Handler(void) {
	if (RTC_GetITStatus(RTC_IT_ALRA) != RESET) {
		uart_send("\r\nALARM ALRA\r\n");
		if (RTC_Alarm_CallBack[0] != NULL)
			RTC_Alarm_CallBack[0]();
//		wait_uSek(3000000);
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		EXTI_ClearITPendingBit(EXTI_Line17);
	}
}

//=============================================================================
//=== lokale Hilfsfunktionen ==================================================
//=============================================================================

/*******************************************************************************
 * Function Name  : adjustDST
 * Description    : adjusts time to DST if needed
 * Input          : non DST time-struct, must be fully populated including weekday
 * Output         : time-stuct gets modified
 * Return         : false: no DST ("winter"), true: in DST ("summer")
 *  DST according to German standard
 *  Based on code from Peter Dannegger found in the mikrocontroller.net forum.
 *******************************************************************************/
static bool adjustDST(RTC_TimeTypeDef *t, RTC_DateTypeDef *d) {
	uint8_t hour, day, wday, month; // locals for faster access

	hour = t->RTC_Hours;
	day = d->RTC_Date;
	wday = d->RTC_WeekDay;
	month = d->RTC_Month;

	if (isDST(t, d)) {
		hour++; // add one hour
		if (hour == 24) { // next day
			hour = 0;
			wday++; // next weekday
			if (wday == 7) {
				wday = 0;
			}
			if (day == DaysInMonth[month - 1]) { // next month
				day = 0;
				month++;
			}
			day++;
		}
		d->RTC_Month = month;
		t->RTC_Hours = hour;
		d->RTC_Date = day;
		d->RTC_WeekDay = wday;
		return true;
	} else {
		return false;
	}
}

/*******************************************************************************
 * Function Name  : isDST
 * Description    : checks if given time is in Daylight Saving time-span.
 * Input          : time-struct, must be fully populated including weekday
 * Output         : none
 * Return         : false: no DST ("winter"), true: in DST ("summer")
 *  DST according to German standard
 *  Based on code from Peter Dannegger found in the microcontroller.net forum.
 *******************************************************************************/
static bool isDST(const RTC_TimeTypeDef *t, const RTC_DateTypeDef *d) {
	uint8_t wday, month; // locals for faster access

	month = d->RTC_Month;

	if (month < 3 || month > 10) { // month 1, 2, 11, 12
		return false; // -> Winter
	}

	wday = d->RTC_WeekDay;

	if (d->RTC_Date - wday >= 25 && (wday || t->RTC_Hours >= 2)) { // after last Sunday 2:00
		if (month == 10) { // October -> Winter
			return false;
		}
	} else { // before last Sunday 2:00
		if (month == 3) { // March -> Winter
			return false;
		}
	}

	return true;
}

/*******************************************************************************
 * Function Name  : isLeapYear
 * Description    : checks if given year is a leapyear.
 * Input          : uitn16_t year (f.E. 1989)
 * Output         : None
 * Return         : _Bool true = is leapyear, false is no leapyear
 *  Decides if a given year is a leapyear or not
 *******************************************************************************/
static _Bool isLeapYear(uint16_t year) {
	if (year % 400 == 0) {
		return true;
	} else if (year % 100 == 0) {
		return false;
	} else if (year % 4 == 0) {
		return true;
	}
	return false;
}




uint32_t Zufallszahl(void)
{
	uint32_t zahl=0;

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);

	while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET);
	zahl = RNG_GetRandomNumber();

	RNG_Cmd(DISABLE);
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, DISABLE);

	return(zahl);
}
