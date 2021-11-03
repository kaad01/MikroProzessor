#include "BME280.h"


sensor BME280;


//=========================================================================
void BME280_read_sensor(void)
//=========================================================================
{
	u8 wait_time = BME280_INIT_VALUE;

//	s32 v_data_uncomp_tem_s32 = BME280_INIT_VALUE;
//	s32 v_data_uncomp_pres_s32 = BME280_INIT_VALUE;
//	s32 v_data_uncomp_hum_s32 = BME280_INIT_VALUE;

	s32 v_actual_temp_s32 = BME280_INIT_VALUE;
	u32 v_actual_press_u32 = BME280_INIT_VALUE;
	u32 v_actual_humity_u32 = BME280_INIT_VALUE;

	s32 com_rslt = ERROR;

	struct bme280_t bme280;

	bme280.bus_read = BME280_I2C_bus_read;
	bme280.bus_write = BME280_I2C_bus_write;
	bme280.delay_msec = BME280_delay_msek;
	bme280.dev_addr = BME280_I2C_ADDRESS2;

	BME280_I2C2_init();

	bme280_init(&bme280);

	com_rslt += bme280_set_power_mode(BME280_NORMAL_MODE);

	com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_2X);
	com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_16X);
	com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_2X);
	com_rslt += bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);

	bme280_compute_wait_time(&wait_time);
	BME280_delay_msek(wait_time);

	com_rslt += bme280_read_pressure_temperature_humidity(
				&v_actual_press_u32,
				&v_actual_temp_s32,
				&v_actual_humity_u32);

	com_rslt += bme280_set_power_mode(BME280_SLEEP_MODE);

	BME280.temperatur = (v_actual_temp_s32 /  500.0) + 24;
	BME280.druck = v_actual_press_u32 / 100.0;
	BME280.feuchtigkeit = v_actual_humity_u32 / 1024.0;

	double x = BME280.druck/1013.25;
	double y = 1/5.255;
	double z = (float)(pow(x,y));
	BME280.hoehe = (float)((288.15/0.0065)*(1.0 - z));
}


void BME280_print_sensorwerte(void)
{
	char outbuffer[200];
	sprintf(outbuffer," ... Luftdruck=%7.2f hPa \r\n ... Hoehe=%7.2f m \r\n ... Temperatur=%5.2f Grad Celsius\r\n ... Feuchtigkeit=%5.2f Prozent\r\n",BME280.druck, BME280.hoehe, BME280.temperatur, BME280.feuchtigkeit);
	usart2_send(outbuffer);
}


void BME280_store_sensorwerte(void)
{
	char outbuffer[100];

	char dateiname[] = "loggdatei.txt";

	RTC_TimeTypeDef RTC_Time_Aktuell  ;
	RTC_DateTypeDef RTC_Date_Aktuell ;

	RTC_GetDate(RTC_Format_BIN, &RTC_Date_Aktuell);
	RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);

	sprintf(outbuffer, "%.2d%.2d%.2d,%.2d%.2d%.2d,%7.2f,%5.2f,%5.2f\r\n",
			RTC_Date_Aktuell.RTC_Year,
			RTC_Date_Aktuell.RTC_Month,
			RTC_Date_Aktuell.RTC_Date,
			RTC_Time_Aktuell.RTC_Hours,
			RTC_Time_Aktuell.RTC_Minutes,
			RTC_Time_Aktuell.RTC_Seconds,
			BME280.druck,
			BME280.temperatur,
			BME280.feuchtigkeit);
	usart2_send(outbuffer);
	sd_card_file_write((char*)dateiname, (char*)outbuffer,strlen(outbuffer));
}


void BME280_I2C2_init(void)
{
	//=== benötigte Clocksystem einschalten

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	//=== I2C3 Init

	// Zurücksetzen der I2C3 Schnittstelle und Default Werte laden
	I2C_DeInit(I2C2);

	GPIO_InitTypeDef GPIO_InitStructure;

	// SCL - PA8 Alternativ Funktion PB10
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_I2C2);

	// SDA - PC9 Alternativ Funktion PB11
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_I2C2);

	//	I2C Konfiguration
	I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_ClockSpeed = 400000;	// 400kHz
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C2, &I2C_InitStructure);

	// Enable I2C2
	I2C_Cmd(I2C2, ENABLE);
}


//=========================================================================
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
//=========================================================================
{
	if (I2C_start(I2C2, dev_addr, I2C_Direction_Transmitter )) {return -1; }
	if (I2C_write(I2C2, reg_addr)) {return -1; }
	if (I2C_restart(I2C2, dev_addr, I2C_Direction_Receiver )) return 1;
	while ((cnt--) > 1)
	{
		*reg_data = I2C_read_ack(I2C2 );
		reg_data++;
	}
	*reg_data = I2C_read_nack(I2C2 );
	I2C_stop(I2C2 );
	return 0;
}


//=========================================================================
s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt)
//=========================================================================
{
	if (I2C_start(I2C2, dev_addr, I2C_Direction_Transmitter )) {return -1; }
	if (I2C_write(I2C2, reg_addr)) {return -1; }
	while (cnt--)
	{
		I2C_write(I2C2, *reg_data++);
	}
	I2C_stop(I2C2 );
	return 0;
}



//=========================================================================
void BME280_delay_msek(unsigned long msek)
//=========================================================================
{	u32 msec = 0;
	msek *= 21;
	msec = msek * 1000;
	while(msec--) { __NOP(); }
}
