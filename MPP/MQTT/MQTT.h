#ifndef MQTT_H
#define MQTT_H

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
//#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
//#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
//#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
//#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"


//=========================================================================
// board_lib
//=========================================================================
#include "simplelink.h"

#include "CC3100.h"

#include "MQTTPacket.h"

#include "rtc.h"
#include "usart.h"
#include "beeper.h"


//=========================================================================
// standard_lib
//=========================================================================
#include "string.h"



//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================



//=========================================================================
//========== Macros
//=========================================================================



//=========================================================================
//========== Variablen
//=========================================================================

extern 	unsigned char 	mqtt_run;

typedef struct Timer Timer;

struct Timer
{
	unsigned long systick_period;
	unsigned long end_time;
};

typedef struct Network Network;

struct Network
{
	int my_socket;
	int (*mqttread) (Network*, unsigned char*, int, int);
	int (*mqttwrite) (Network*, unsigned char*, int, int);
	void (*disconnect) (Network*);
};

enum QoS
{
	QOS0,
	QOS1,
	QOS2
};

typedef struct MQTTMessage MQTTMessage;

typedef struct MessageData MessageData;

struct MQTTMessage
{
    enum QoS qos;
    char retained;
    char dup;
    unsigned short id;
    void *payload;
    size_t payloadlen;
};

struct MessageData
{
    MQTTMessage* message;
    MQTTString* topicName;
};






//#########################################################################
//########## Beispiel mit Socket direkt Nutzung
//#########################################################################
// allgemeines Beispiel
void mqtt_client(void);
void mqtt_client_topic_handler(MessageData* msg);

// Beispiel mit Sensor und Aktor Einbindung
void mqtt_sensor_aktor_beispiel(void);
// nutzt folgende Funktionsaufrufe
void mqtt_open_connection(void);
void mqtt_close_connection(void);
void mqtt_publish(char* topic, char* message);
void mqtt_topic_handler_led(MessageData* msg);
void mqtt_topic_handler_beeper(MessageData* msg);
void mqtt_topic_handler_message(MessageData* msg);





//#########################################################################
//########## Beispiel mit Socket Select Nutzung
//#########################################################################
void mqtt_connect(void);
void mqtt_sub_topic(void);	// ein array von topics
void mqtt_sub_topic1(void);	// nur ein topic
void mqtt_pub_bme280_sensor(void);
void mqtt_pub(char* topic, char* msg);

void mqtt_read_callback(char* rx,
						uint16_t rx_len,
						sockaddr* from,
						uint16_t socket_Position);

long mqtt_write_callback(char* tx,
						uint16_t tx_len,
						uint16_t socket_Position);

void rd_pub_message(void);




//#########################################################################
//########## CC3100 Socket Interface
//#########################################################################
void NewNetwork(Network* n);
int ConnectNetwork(Network* n, char* addr, int port);
int cc3100_read(Network* n, unsigned char* buffer, int len, int timeout_ms);
int cc3100_write(Network* n, unsigned char* buffer, int len, int timeout_ms);
void cc3100_disconnect(Network* n);




//#########################################################################
//########## Timeout Hilffunktionen
//#########################################################################
void InitTimer(Timer* timer);
void MQTT_SysTickHandler(void);
char expired(Timer* timer);
int left_ms(Timer* timer);
void countdown_ms(Timer* timer, unsigned int timeout);
void countdown(Timer* timer, unsigned int timeout);


#endif
