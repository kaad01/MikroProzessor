#include "fat_time.h"

DWORD get_fattime (void)
{
	DWORD time_stamp;
	char wochentag[] = {"Mo","Di","Mi","Do","Fr","Sa","So"};
	char data[30] = { 0 };

	RTC_TimeTypeDef Zeit  ;
	RTC_DateTypeDef Datum ;

	RTC_GetTime(RTC_Format_BIN, &Zeit);
	RTC_GetDate(RTC_Format_BIN, &Datum);

	sprintf(data, "\r\n%.2d:%.2d:%.2d Uhr ",
			Zeit.RTC_Hours,
			Zeit.RTC_Minutes,
			Zeit.RTC_Seconds);
	usart2_send(data);

	sprintf(data, "am %s. ,d. %.2d.%.2d.%.2d \r\n",
			wochentag[Datum.RTC_WeekDay-1],
			Datum.RTC_Date, Datum.RTC_Month,
			Datum.RTC_Year + 2000);
	usart2_send(data);

	time_stamp =  (((DWORD)Datum.RTC_Year + 20) << 25)
				| (((DWORD)Datum.RTC_Month) << 21)
				| (((DWORD)Datum.RTC_Date) << 16)
				| ((WORD)(Zeit.RTC_Hours) << 11)
				| ((WORD)(Zeit.RTC_Minutes) << 5)
				| ((WORD)(Zeit.RTC_Seconds) >> 1);

	return time_stamp;
}

