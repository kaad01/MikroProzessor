#ifndef __BME280_H__
#define __BME280_H__



//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_syscfg.h"


//=========================================================================
// board_lib
//=========================================================================
#include "i2c.h"
#include "usart.h"
#include "bme280_driver.h"
#include "CC3100_Board.h"
#include "SDCARD.h"


//=========================================================================
// standard_lib
//=========================================================================
#include "stdio.h"
#include "string.h"
#include "math.h"

//=========================================================================
// Macros
//=========================================================================
#define MASK_DATA1	0xFF
#define MASK_DATA2	0x80
#define MASK_DATA3	0x7F
#define	I2C_BUFFER_LEN 8
#define SPI_BUFFER_LEN 5


//=========================================================================
// Variablen
//=========================================================================
typedef struct BME280
{
	float temperatur;
	float druck;
	float feuchtigkeit;
	float hoehe;

}sensor;

extern sensor BME280;


//=========================================================================
// Funktionen
//=========================================================================

void BME280_read_sensor(void);
void BME280_print_sensorwerte(void);
void BME280_store_sensorwerte(void);

void BME280_I2C2_init(void);
s8 BME280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 BME280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
void BME280_delay_msek(unsigned long msek);


#endif
