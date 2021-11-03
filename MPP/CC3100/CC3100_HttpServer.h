#ifndef __CC3100_HttpServer_H__
#define __CC3100_HttpServer__


//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
//#include "misc.h"
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
//#include "stm32f4xx_exti.h"
//#include "stm32f4xx_flash.h"
//#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
//#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
//#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
//#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
//#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"


//=========================================================================
// standard_lib
//=========================================================================



//=========================================================================
// board_lib
//=========================================================================
#include "usart.h"

#include "simplelink.h"
#include "protocol.h"
#include "netapp.h"
#include "CC3100.h"
#include "CC3100_EventHandler.h"
#include "led.h"


//=========================================================================
// Funktions_Deklarationen
//=========================================================================

void start_HttpServer(void);

void Http_Get_Event (	SlHttpServerEvent_t *pHttpEvent,
        				SlHttpServerResponse_t *pHttpResponse);

void Http_Post_Event (	SlHttpServerEvent_t *pHttpEvent,
        				SlHttpServerResponse_t *pHttpResponse);

int initializeAppVariables(void);
int set_authentication_check (_u8 enable);
int get_auth_name (_u8 *auth_name);
int get_auth_password (_u8 *auth_password);
int get_auth_realm (_u8 *auth_realm);
int get_device_urn (_u8 *device_urn);
int get_domain_name (_u8 *domain_name);
int set_port_number(_u16 num);

#endif // __CC3100_HttpServer__
