#ifndef __test_h__
#define __test_h__
//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
//#include "stm32f4xx_adc.h"
//#include "stm32f4xx_can.h"
//#include "stm32f4xx_crc.h"
//#include "stm32f4xx_cryp_aes.h"
//#include "stm32f4xx_cryp_des.h"
//#include "stm32f4xx_cryp_tdes.h"
//#include "stm32f4xx_cryp.h"
//#include "stm32f4xx_dac.h"
//#include "stm32f4xx_dbgmcu.h"
//#include "stm32f4xx_dcmi.h"
//#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
//#include "stm32f4xx_flash.h"
//#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
//#include "stm32f4xx_i2c.h"
#include "stm32f4xx_iwdg.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
#include "stm32f4xx_wwdg.h"
//#########################################################################
//########## mpp_lib
//#########################################################################
#include "BME280.h"
//=========================================================================
#include "beeper.h"
#include "client_ftp.h"
#include "client_ntp.h"
//#include "client_wetter.h"
//#include "client_email.h"
#include "global.h"
#include "i2c.h"
//#include "init.h"
#include "interrupts.h"
//#include "led.h"
#include "power.h"
#include "rtc.h"
#include "taster.h"
#include "usart.h"
//=========================================================================
#include "simplelink.h"
#include "netapp.h"
#include "CC3100.h"
#include "CC3100_Board.h"
//=========================================================================
#include "dw1000_driver.h"
#include "dw1000_ranging.h"
#include "DW1000.h"
//=========================================================================
#include "MQTT.h"
//=========================================================================
#include "MPU9250.h"
//=========================================================================
#include "SDCARD.h"
//=========================================================================
//#include "CoOS.h"

#include "mpu9250_driver.h"


//#########################################################################
//########## standard_lib
//#########################################################################
#include "stdio.h"
#include "string.h"
#include "math.h"

//#########################################################################
//########## Eigene Funktionen, Macros und Variablen
//#########################################################################

//=========================================================================
// Macros
//=========================================================================

#define PA1_on 		(GPIO_SetBits(GPIOA, GPIO_Pin_1))
#define PA1_off		(GPIO_ResetBits(GPIOA, GPIO_Pin_1))
//=========================================================================
// Variablen
//=========================================================================


//=========================================================================
// Funktionen
//=========================================================================
void rtc_aktualisieren(void);
void html_datein_laden(void);
void uwb_config_datei_laden(void);
void paho_mqtt_client(void);
void select_mqtt_connect_sub(void);
void select_mqtt_pub(void);
void sd_karte_schreiben(void);
void transfer_sd_karte_auf_ftp(void);
void bme280_lesen(void);
void power_off(void);
void beep_ton(void);
void cc3100_flash_formatieren(void);

void uwb_deepsleep(void);
void uwb_sniffmode(void);
void uwb_rxmode(void);

void mcu_sleep_mode(void);
void mcu_stop_mode(void);
void mcu_standby_mode(void);

void wlan_on(void);
void wlan_off(void);

void periodic_stop_mode_mit_wakeup_irq(void);
void periodic_power_onoff_mit_wakeup(void);
void periodic_wakeup_onoff(void);

void wait_uSek(unsigned long us);
void wait_mSek(unsigned long ms);
void timer_init(void);
void timer_start(void);
void timer_stop(char* chars);
void init_indikator(void);
void auto_wakeup(void);
void wwdg_watchdog(void);

//=========================================================================
#endif
