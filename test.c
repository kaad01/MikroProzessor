#include "test.h"



//=========================================================================
void rtc_aktualisieren(void)
//=========================================================================
{
	get_NTP_Time("");
	wait_uSek(1000000);
	CC3100_set_time();
}



//=========================================================================
void cc3100_flash_formatieren(void)
//=========================================================================
{
	set_autostart_FSF(0,0,0,5);
	Stop_CC3100_select = 1;
	CC3100_format_serial_flash();
	set_POWER_OFF();
	while(1){;}
}


//=========================================================================
void html_datein_laden(void)
//=========================================================================
{
	// so kann man den inhalt der sf flash datein auslesen
	//CC3100_sf_file_read( (unsigned char *)"www/index.html");
	//CC3100_sf_file_read( (unsigned char *)"sys/f43_sys_servicepack.ucf");
	//CC3100_sf_file_read( (unsigned char *)"/sys/f43_sys_servicepack.ucf");

	// vom ftp server die datein auf die sd karte laden
	ftp_connect("mpp.imp.fu-berlin.de", "ftp.mpp", "Lapa8heu");
	ftp_get_sd_file("/hwp01","index.html"," ","index.html");
	ftp_get_sd_file("/hwp01","hwp.html"," ","hwp.html");
	ftp_get_sd_file("/hwp01","mqtt.html"," ","mqtt.html");
	ftp_get_sd_file("/hwp01","status.txt"," ","status.txt");
	ftp_close();

	// die vorhandenen datein auf dem sf flash löschen
	// notwendig da sf filesystem begrenzt in der funktionalität
	CC3100_sf_file_erase( (unsigned char *)"www/index.html");
	CC3100_sf_file_erase( (unsigned char *)"www/hwp.html");
	CC3100_sf_file_erase( (unsigned char *)"www/mqtt.html");
	CC3100_sf_file_erase( (unsigned char *)"www/status.txt");

	// jetzt die datein von der sd karte auf den sf flash kopieren
	// danach verfügt der integrierte webserver des cc3100
	// sofort über die neuen seiten
	sd_card_copy_file_to_cc3100("0:index.html", "www/index.html");
	sd_card_copy_file_to_cc3100("0:hwp.html", "www/hwp.html");
	sd_card_copy_file_to_cc3100("0:mqtt.html", "www/mqtt.html");
	sd_card_copy_file_to_cc3100("0:status.txt", "www/status.txt");
}



//=========================================================================
void uwb_config_datei_laden(void)
//=========================================================================
{	// vom FTP Server wird die Konfigurationsdatei fuer den UWB Transceiver neu geladen
	ftp_connect("mpp.imp.fu-berlin.de", "ftp.mpp", "Lapa8heu");
	ftp_get_sd_file("/hwp01","uwbconfig.dat"," ","uwbconfig.dat");
	ftp_close();
	set_autostart_in(0,0,0,2);
}



//=========================================================================
void power_off(void)
//=========================================================================
{	// Ausgabe von 3 Piepsignalen
	// die RTC wird programmiert um die Stromversorgung in 30Sekunden
	// wieder einzuschalten und dann wird die Stromversorgung ausgeschaltet
	set_autostart_in(0,0,0,30);
}



//=========================================================================
void bme280_lesen(void)
//=========================================================================
{
	BME280_read_sensor();
	BME280_print_sensorwerte();
}



//=========================================================================
void sd_karte_schreiben(void)
//=========================================================================
{	// hier erfolgt die Ausgabe der Verzeichnisstruktur
	// und danach wird eine Datei angelegt und in die Datei geschieben
	// abschließend wird die Datei gelesen und über die serielle Schnittstelle ausgegeben
	char sd_file_name[30] = {0};
	char sd_file_data[200] = {0};
	char d[1000] = {0};
	sd_card_print_dir(d);

	sprintf(sd_file_name,"abcd.txt");
	sprintf(sd_file_data,"Beispieltext\r\n");

	sd_card_file_write(sd_file_name, sd_file_data, strlen(sd_file_data));

	sd_card_print_file(sd_file_name);
}



//=========================================================================
void paho_mqtt_client(void)
//=========================================================================
{	// MQTT Paho Client Nutzung
	// ....
	usart2_send("\r\nStart MQTT Paho Client\r\n");
	mqtt_client();
	usart2_send("\r\nStop MQTT Paho Client\r\n");
}



//=========================================================================
void select_mqtt_connect_sub(void)
//=========================================================================
{	// Verbindung zum MQTT Broker auf Basis der Nutzung von Select
	// Aufbau der Verbindung zum Broker und
	// Subscribe von Topics
	mqtt_connect();
	mqtt_sub_topic();
}



//=========================================================================
void select_mqtt_pub(void)
//=========================================================================
{	// publish der Sensordaten und RTC Zeit
	// ueber schon bestehende verbindung zum Broker
	// erfolgt laengere zeit kein publish wird der
	// Broker die Verbindung schliessen
	mqtt_pub_bme280_sensor();
}







//=========================================================================
void beep_ton(void)
//=========================================================================
{
	beep(4000,200,200);
}



//=========================================================================
void transfer_sd_karte_auf_ftp(void)
//=========================================================================
{
	ftp_connect("mpp.imp.fu-berlin.de", "ftp.mpp", "Lapa8heu");
	ftp_put_sd_file("","index.html","/mpp/hwp10","index.html");
	ftp_close();
}



//=========================================================================
void uwb_deepsleep(void)
//=========================================================================
{
	dw1000_idle();
	dw1000_entersleep();
}



//=========================================================================
void uwb_sniffmode(void)
//=========================================================================
{
	dw1000_wakefromsleep();
	dw1000_enableSniffMode();
	dw1000_startReceive();
}



//=========================================================================
void uwb_rxmode(void)
//=========================================================================
{
	dw1000_wakefromsleep();
	dw1000_idle();
	dw1000_disableSniffMode();
	dw1000_startReceive();
}



//=========================================================================
void mcu_sleep_mode(void)
//=========================================================================
{
	// Wechsel in den Sleep Mode mit einem der beiden Befehle:
	// __WFI() - wait for Interrupt oder
	// __WFE() - wait for Event
	// Systick Interrupte werde ausgeschaltet
	SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
	__WFI();
	SysTick->CTRL  |= SysTick_CTRL_TICKINT_Msk;
}



//=========================================================================
void mcu_stop_mode(void)
//=========================================================================
{
	SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
	PWR_FlashPowerDownCmd ( ENABLE );

	PWR_EnterSTOPMode ( PWR_Regulator_LowPower , PWR_STOPEntry_WFI );

	PWR_FlashPowerDownCmd ( DISABLE );
	SysTick->CTRL  |= SysTick_CTRL_TICKINT_Msk;

	// Bei Rückkehr durch Interrupt muß Clocksystem initialisiert werden
	// wird schon in den ISR das Clocksystem benoetigt
	// muss es schon da neu initialisiert werden
	SystemInit();
}




//=========================================================================
void mcu_standby_mode(void)
//=========================================================================
{
	usart2_send("mcu_standby_mode\r\n");

	// Strukt anlegen
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// EXTI-Line Konfiguration für WakeUp
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// NIVC Konfiguration für WakeUp
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RTC_WakeUpCmd(DISABLE);

	// Konfiguration der Clock
    // RTC_WakeUpClock_RTCCLK_Div2;    (122,070usek...4sek) Zeitbasis: 61,035us
    // RTC_WakeUpClock_RTCCLK_Div4;    (244,140usek...8sek)	Zeitbasis: 122,070us
    // RTC_WakeUpClock_RTCCLK_Div8;    (488,281usek...16sek)Zeitbasis: 244,140us
    // RTC_WakeUpClock_RTCCLK_Div16;   (976,562usek...32sek)Zeitbasis: 488,281us
    // RTC_WakeUpClock_CK_SPRE_16bits; (1sek...65535sek)	Zeitbasis: 1s
    // RTC_WakeUpClock_CK_SPRE_17bits: (1sek...131070sek)	Zeitbasis: 1s
	RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);

	// RTC_WakeUpCouter mit einem Wert zwischen 0x0000...0xFFFF setzen
	// Wert des WakeUpCounter aus Zeitintervall/Zeitbasis berechnen
	// WakeUpCounterwert für intervall von 2s bei RTC_WakeUpClock_RTCCLK_Div2
	// ergibt sich aus 1s/63,035s = 31728
	RTC_SetWakeUpCounter(65000);

	// Clear PWR Wakeup WUF Flag
	PWR_ClearFlag(PWR_CSR_WUF);
	PWR_WakeUpPinCmd(ENABLE);

    // Clear RTC Wakeup WUTF Flag
	RTC_ClearITPendingBit(RTC_IT_WUT);
	RTC_ClearFlag(RTC_FLAG_WUTF);

	// Freigeben
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	RTC_ITConfig(RTC_IT_WUT, ENABLE);	// Bit 14
	RTC_AlarmCmd(RTC_CR_WUTE, ENABLE); 	// Bit 10

	RTC_WakeUpCmd(ENABLE);

    usart2_send("StandBy Mode Start\r\n");
	SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
    wait_uSek_CC3100(10000);

    PWR_EnterSTANDBYMode();

	while(1){;}
}



//=========================================================================
void wlan_on(void)
//=========================================================================
{
	CC3100_init();
	Stop_CC3100_select = 0;
	CC3100_fw_check();
	CC3100_set_in_STA_Mode(0);
}



//=========================================================================
void wlan_off(void)
//=========================================================================
{
	sl_Stop(0xFF);
	wait_uSek(1000000);
	Stop_CC3100_select = 0;
}







//=========================================================================
void auto_wakeup(void)
//=========================================================================
{
	// Strukt anlegen
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// EXTI-Line Konfiguration für WakeUp
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// NIVC Konfiguration für WakeUp
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RTC_WakeUpCmd(DISABLE);

	// Konfiguration der Clock
    // RTC_WakeUpClock_RTCCLK_Div2;    (122,070usek...4sek) Zeitbasis: 61,035us
    // RTC_WakeUpClock_RTCCLK_Div4;    (244,140usek...8sek)	Zeitbasis: 122,070us
    // RTC_WakeUpClock_RTCCLK_Div8;    (488,281usek...16sek)Zeitbasis: 244,140us
    // RTC_WakeUpClock_RTCCLK_Div16;   (976,562usek...32sek)Zeitbasis: 488,281us
    // RTC_WakeUpClock_CK_SPRE_16bits; (1sek...65535sek)	Zeitbasis: 1s
    // RTC_WakeUpClock_CK_SPRE_17bits: (1sek...131070sek)	Zeitbasis: 1s
	RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div4);

	// RTC_WakeUpCouter mit einem Wert zwischen 0x0000...0xFFFF setzen
	// Wert des WakeUpCounter aus Zeitintervall/Zeitbasis berechnen
	// WakeUpCounterwert für intervall von 2s bei RTC_WakeUpClock_RTCCLK_Div2
	// ergibt sich aus 1s/63,035s = 31728
	RTC_SetWakeUpCounter(16000);

	// Clear PWR Wakeup WUF Flag
	PWR_ClearFlag(PWR_CSR_WUF);
	PWR_WakeUpPinCmd(ENABLE);

    // Clear RTC Wakeup WUTF Flag
	RTC_ClearITPendingBit(RTC_IT_WUT);
	RTC_ClearFlag(RTC_FLAG_WUTF);

	// Freigeben
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	RTC_OutputConfig(RTC_Output_WakeUp, RTC_OutputPolarity_Low);
	RTC_OutputTypeConfig(RTC_OutputType_OpenDrain);

	RTC_ITConfig(RTC_IT_WUT, ENABLE);	// Bit 14
	RTC_AlarmCmd(RTC_CR_WUTE, ENABLE); 	// Bit 10

	RTC_WakeUpCmd(ENABLE);

	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
	while(1){;}
}



//=========================================================================
void periodic_stop_mode_mit_wakeup_irq(void)
//=========================================================================
{
	// Strukt anlegen
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// EXTI-Line Konfiguration für WakeUp
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// NIVC Konfiguration für WakeUp
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RTC_WakeUpCmd(DISABLE);

		// Konfiguration der Clock
		// RTC_WakeUpClock_RTCCLK_Div2;    (122,070usek...4sek) Zeitbasis: 61,035us
		// RTC_WakeUpClock_RTCCLK_Div4;    (244,140usek...8sek)	Zeitbasis: 122,070us
		// RTC_WakeUpClock_RTCCLK_Div8;    (488,281usek...16sek)Zeitbasis: 244,140us
		// RTC_WakeUpClock_RTCCLK_Div16;   (976,562usek...32sek)Zeitbasis: 488,281us
		// RTC_WakeUpClock_CK_SPRE_16bits; (1sek...65535sek)	Zeitbasis: 1s
		// RTC_WakeUpClock_CK_SPRE_17bits: (1sek...131070sek)	Zeitbasis: 1s
		RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div4);

		// RTC_WakeUpCouter mit einem Wert zwischen 0x0000...0xFFFF setzen
		// Wert des WakeUpCounter aus Zeitintervall/Zeitbasis berechnen
		// WakeUpCounterwert für intervall von 2s bei RTC_WakeUpClock_RTCCLK_Div2
		// ergibt sich aus 1s/63,035s = 31728
		//RTC_SetWakeUpCounter(16000);
		RTC_SetWakeUpCounter(1638);

		RTC_ClearITPendingBit(RTC_IT_WUT);
		RTC_ClearFlag(RTC_FLAG_WUTF);
		RTC_ITConfig(RTC_IT_WUT, ENABLE);

		DW1000_init();
		uwb_deepsleep();

	RTC_WakeUpCmd(ENABLE);

	while (1)
	{
		RTC_ClearITPendingBit(RTC_IT_WUT);
		RTC_ClearFlag(RTC_FLAG_WUTF);

		SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk; // SysTick-IRQ Off

		//PWR_FlashPowerDownCmd ( ENABLE );
		PWR_EnterSTOPMode ( PWR_Regulator_LowPower , PWR_STOPEntry_WFI );
		//PWR_FlashPowerDownCmd ( DISABLE );

		SysTick->CTRL  |= SysTick_CTRL_TICKINT_Msk; // SysTick-IRQ On

		//###### tuwas #####
		PA1_on;
		wait_uSek(10000);
//		uwb_send = 1;
//		dw1000_wakefromsleep();
//		dw1000_initializeTransmit();
//		dw1000_writeByteArrayToTxBuffer((unsigned char*)"hallo",5);
//		dw1000_startTransmit(0);
//		while(uwb_send !=0) {;}
//		uwb_deepsleep();
		PA1_off;
		//##################
	}
}




//=========================================================================
void periodic_power_onoff_mit_wakeup(void)
//=========================================================================
{
	init_indikator();
		PA1_on;
	SystemInit();		// Basisinitialisierung des Mikrocontrollers
		PA1_off;
	start_RTC();		// RTC freigeben
		PA1_on;

		timer_init();
		timer_start();

	init_POWER_ON();
	usart2_init();
	init_I2C2();
	mpu9250_init();		// MPU aus
	mpu9250_uninit();	// 28,000ms
	sd_card_init();
	init_LED();
	init_TASTER1();
	init_BEEPER();
	DW1000_init();		// 3,600mSek
	//uwb_deepsleep();	// ToDo !!!
		PA1_on;

	InitSysTick();		// SysTick Timer aktivieren mit 10ms Tick
		PA1_off;

	while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == Bit_SET) {;}

	// Strukt anlegen
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// EXTI-Line Konfiguration für WakeUp
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// NIVC Konfiguration für WakeUp
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RTC_WakeUpCmd(DISABLE);
	PWR_BackupAccessCmd(ENABLE);

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
		PWR_BackupRegulatorCmd(ENABLE);
		while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET) { ; }

		// Konfiguration der Clock
		// RTC_WakeUpClock_RTCCLK_Div2;    (122,070usek...4sek) Zeitbasis: 61,035us
		// RTC_WakeUpClock_RTCCLK_Div4;    (244,140usek...8sek)	Zeitbasis: 122,070us
		// RTC_WakeUpClock_RTCCLK_Div8;    (488,281usek...16sek)Zeitbasis: 244,140us
		// RTC_WakeUpClock_RTCCLK_Div16;   (976,562usek...32sek)Zeitbasis: 488,281us
		// RTC_WakeUpClock_CK_SPRE_16bits; (1sek...65535sek)	Zeitbasis: 1s
		// RTC_WakeUpClock_CK_SPRE_17bits: (1sek...131070sek)	Zeitbasis: 1s
		RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div4);

		// RTC_WakeUpCouter mit einem Wert zwischen 0x0000...0xFFFF setzen
		// Wert des WakeUpCounter aus Zeitintervall/Zeitbasis berechnen
		// Bsp: WakeUpCounterwert für intervall von 2s bei RTC_WakeUpClock_RTCCLK_Div2
		// 		ergibt sich aus 1s/63,035s = 31728
		RTC_SetWakeUpCounter(32000); //#######Achtung Zeit mus groesser als tuwas sein!!!

		PWR_ClearFlag(PWR_CSR_WUF);
		RTC_ClearFlag(RTC_FLAG_WUTF);
		RTC_ClearITPendingBit(RTC_IT_WUT);

		// PC13 aktivieren für automatisches power on
		RTC_OutputConfig(RTC_Output_WakeUp, RTC_OutputPolarity_Low);
		RTC_OutputTypeConfig(RTC_OutputType_OpenDrain);

		RTC_ITConfig(RTC_IT_WUT, ENABLE);
		RTC_AlarmCmd(RTC_CR_WUTE, ENABLE);

	PWR_BackupAccessCmd(DISABLE);
	RTC_WakeUpCmd(ENABLE);

	PA1_on;

	//###### tuwas ##### 3.17mSek
	uwb_send = 1;
	//dw1000_wakefromsleep();
	dw1000_initializeTransmit();
	dw1000_writeByteArrayToTxBuffer((unsigned char*)"hallo",5);
	dw1000_startTransmit(0);
	while(uwb_send !=0) {;}
	uwb_deepsleep();
	timer_stop("mcu run time");

	// Kill Bit auf 0
	// 500ms nach Power on wird Spannungsregler ausgeschaltet
	set_POWER_OFF();

	wait_uSek(1000);

	PA1_off;

	// niedrigsten Stromverbrauch wählen und auf
	// Power off warten
	SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
	PWR_FlashPowerDownCmd ( ENABLE );
	PWR_EnterSTOPMode ( PWR_Regulator_LowPower , PWR_STOPEntry_WFI );
	PWR_FlashPowerDownCmd ( DISABLE );
	SysTick->CTRL  |= SysTick_CTRL_TICKINT_Msk;

	while(1){;}
}



//=========================================================================
void periodic_wakeup_onoff(void)
//=========================================================================
{
	RTC_WakeUpCmd(DISABLE);
	PWR_BackupAccessCmd(ENABLE);

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
		PWR_BackupRegulatorCmd(ENABLE);
		while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET) { ; }

		RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div4);
		RTC_SetWakeUpCounter(16000);

		PWR_ClearFlag(PWR_CSR_WUF);
		RTC_ClearFlag(RTC_FLAG_WUTF);
		RTC_ClearITPendingBit(RTC_IT_WUT);

		RTC_OutputConfig(RTC_Output_WakeUp, RTC_OutputPolarity_Low);
		RTC_OutputTypeConfig(RTC_OutputType_OpenDrain);

		RTC_ITConfig(RTC_IT_WUT, ENABLE);
		RTC_AlarmCmd(RTC_CR_WUTE, ENABLE);

	PWR_BackupAccessCmd(DISABLE);
	RTC_WakeUpCmd(ENABLE);

	// hier etwas tun
	usart2_send("power on\r\n>action...");
	wait_uSek(1000000);
	usart2_send("OK<\r\n power off");
	wait_uSek_CC3100(1000);

	set_POWER_OFF();
	while(1){;}
}



//=========================================================================
void wait_uSek(unsigned long us)
//=========================================================================
{
	// wartet uSekunden gilt aber nur bei 168MHz Taktfrequenz
	us *= 21;
	while(us--) { __NOP(); }
}



//=========================================================================
void wait_mSek(unsigned long ms)
//=========================================================================
{
	// wartet uSekunden gilt aber nur bei 168MHz Taktfrequenz
	ms *= 21000;
	while(ms--) { __NOP(); }
}



//=========================================================================
void timer_init(void)
//=========================================================================
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 840 - 1;
	TIM_TimeBaseStructure.TIM_Period = 0xffff - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM6, ENABLE);
}



//=========================================================================
void timer_start(void)
//=========================================================================
{
	TIM_SetCounter(TIM6, 0);
}



//=========================================================================
void timer_stop(char* chars)
//=========================================================================
{	char out[100] = {0};
	unsigned int time_run = TIM_GetCounter(TIM6);
	float a = time_run / 100.0;
	sprintf(out,"%s => %7.3f msek\r\n ",chars, a);
	usart2_send(out);
}



//=========================================================================
void init_indikator(void)
//=========================================================================
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;//GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//GPIO_OType_PP;
	GPIO_InitStructure.GPIO_OType =  GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}



//=========================================================================
void wwdg_watchdog(void)
//=========================================================================
{
    char usart2_tx_buffer[50] = {0};
	unsigned char value_watchdog_counter = 0x7f;
    unsigned char window_value = 0x50;
    unsigned char window_value_refresh = 0x30;
    unsigned char cnt_i = 0;
    unsigned char cnt_j = 0;

    sprintf(usart2_tx_buffer,"\r\nNeustart\r\n");
    usart2_send(usart2_tx_buffer);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

    // Interrupcontroller konfigurieren
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);


    WWDG_SetPrescaler(WWDG_Prescaler_8);
    WWDG_SetWindowValue(window_value);
    WWDG_Enable(value_watchdog_counter);

        WWDG_ClearFlag();
        WWDG_EnableIT();

    cnt_i = (unsigned char) (value_watchdog_counter + 1);

    while(1){

        cnt_j = (unsigned char) ((WWDG->CR) & 0x7F) ;

        if (cnt_j  < cnt_i ) {

            sprintf(usart2_tx_buffer,"i = %u\r\n",cnt_j);
            usart2_send(usart2_tx_buffer);

            cnt_i = cnt_j;

            if (cnt_i == window_value_refresh ) {

                WWDG_SetCounter(value_watchdog_counter);

                sprintf(usart2_tx_buffer,"####### neu geladen\r\n");
                usart2_send(usart2_tx_buffer);

                cnt_i = (unsigned char) (value_watchdog_counter + 1);
            }
        }
    }
}
