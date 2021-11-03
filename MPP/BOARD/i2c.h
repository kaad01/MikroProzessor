#ifndef __I2C_H__
#define __I2C_H__

//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"



//=========================================================================
// board_lib
//=========================================================================
#include "usart.h"
#include "CC3100_Board.h"



//=========================================================================
// MPP_lib
//=========================================================================


//=========================================================================
// standard_lib
//=========================================================================
#include "stdio.h"
#include "string.h"




//=============================================================================
// init I2C2 - SCL2 an PB10 und SDA2 an PB11
void init_I2C2(void);

//=============================================================================
//
int I2C_write_buf(	unsigned char slave_addr,
					unsigned char reg_addr,
					unsigned char length,
					unsigned char const *data);


//=============================================================================
int I2C_read_buf(	unsigned char slave_addr,
					unsigned char reg_addr,
					unsigned char length,
					unsigned char *data);
//

//=============================================================================
//	sendet Start Condition + Slave Adresse und RW-Bit
int I2C_start(I2C_TypeDef* I2Cx,
				uint8_t addr,
				uint8_t rdwr);


//=============================================================================
//	Stop Condition
void I2C_stop(I2C_TypeDef* I2Cx);


//=============================================================================
//	sendet ein Byte an den I2C-Slave
int I2C_write(I2C_TypeDef* I2Cx,
				uint8_t data);


//=============================================================================
//	empfängt ein Byte vom I2C-Slave und generiert ein ACK
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);


//=============================================================================
//	empfängt ein Byte vom I2C-Slave und generiert kein ACK
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);


//=============================================================================
//
int I2C_restart(I2C_TypeDef* I2Cx,
				uint8_t addr,
				uint8_t rdwr);


//=============================================================================
//
int I2C_check_dev(I2C_TypeDef* I2Cx,
					uint8_t addr);


//=============================================================================
// gibt eine Fehlermeldung aus
int I2C_timeout(char *msg);



#endif // __I2C_H__
