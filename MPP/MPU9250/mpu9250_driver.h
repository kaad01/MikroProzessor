#ifndef MPU9250_HEADER
#define MPU9250_HEADER

//=========================================================================
// Anschlussbelegung
//=========================================================================
//	INT		PB11
// 	SCL		PPB6	SCL1
//	SDA		PPB7 	SDA1
//=========================================================================

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
#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
//#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"



//=========================================================================
// board_lib
//=========================================================================
#include "interrupts.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "mpu9250_def.h"


/* MANUELL ZU ÄNDERN **********************************/

// Über diese Definition lässt sich die Floatingpoint Genauigkeit bei Berechnungen auf Basis der Messdaten einstellen.
#define MPU9250_FLOAT double
//#define MPU9250_FLOAT float

// Die Nummer der SPI, die zur Kommunikation mit dem Modul verwendet wird
#define MPU9250_SPI_NUMBER		3

// Die Pin- und Portnummern, die als Schnittstelle zum Modul verwendet werden
#define MPU9250_PIN_CS			15
#define MPU9250_PIN_CLK			10
#define MPU9250_PIN_MISO		11
#define MPU9250_PIN_MOSI		5
#define MPU9250_PIN_IRQ			4
#define MPU9250_PORT_CS			A
#define MPU9250_PORT_CLK		C
#define MPU9250_PORT_MISO		C
#define MPU9250_PORT_MOSI		B
#define MPU9250_PORT_IRQ		C

// Der Interrupt Kanal, der die Meldungen des Moduls abfängt
#define MPU9250_IRQ_EXTICHANNEL	EXTI4_IRQn

// Aktiviert alle Takte, die für MPU9250 notwendig sind
#define MPU9250_PERIPHEAL_ENABLE \
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); \
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); \
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); \
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE); \
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE)

// Mögliche Parameter für die Samplerate und Tiefpassfilterung des Gyroskops
typedef enum
{
	MPU9250_GYRO_32KHZ_FILTER_8000HZ			= 0b01000000,
	MPU9250_GYRO_32KHZ_FILTER_3600HZ			= 0b10000000,
	MPU9250_GYRO_8KHZ_FILTER_250HZ				= 0b00000000,
	MPU9250_GYRO_1KHZ_FILTER_184HZ				= 0b00000001,
	MPU9250_GYRO_1KHZ_FILTER_92HZ				= 0b00000010,
	MPU9250_GYRO_1KHZ_FILTER_41HZ				= 0b00000011,
	MPU9250_GYRO_1KHZ_FILTER_20HZ				= 0b00000100,
	MPU9250_GYRO_1KHZ_FILTER_10HZ				= 0b00000101,
	MPU9250_GYRO_1KHZ_FILTER_5HZ				= 0b00000110,
	MPU9250_GYRO_8KHZ_FILTER_3600HZ				= 0b00000111
} mpu9250_gyro_rate_e;

// Mögliche Parameter für die Einstellung der Auflösung des Gyroskops
typedef enum
{
	MPU9250_GYRO_250_DPS      					= 0b00000000,
	MPU9250_GYRO_500_DPS      					= 0b00001000,
	MPU9250_GYRO_1000_DPS     					= 0b00010000,
	MPU9250_GYRO_2000_DPS     					= 0b00011000
} mpu9250_gyro_sensitivity_e;

// Mögliche Parameter für die Samplerate und Filterung des Beschleunigungssensors
typedef enum
{
	MPU9250_ACCEL_1KHZ_FILTER_460HZ				= 0b00000000,
	MPU9250_ACCEL_4KHZ_FILTER_1300HZ			= 0b10000000,
	MPU9250_ACCEL_1KHZ_FILTER_184HZ				= 0b00000001,
	MPU9250_ACCEL_1KHZ_FILTER_92HZ				= 0b00000010,
	MPU9250_ACCEL_1KHZ_FILTER_41HZ				= 0b00000011,
	MPU9250_ACCEL_1KHZ_FILTER_20HZ				= 0b00000100,
	MPU9250_ACCEL_1KHZ_FILTER_10HZ				= 0b00000101,
	MPU9250_ACCEL_1KHZ_FILTER_5HZ				= 0b00000110
} mpu9250_accel_rate_e;

// Mögliche Parameter für die Einstellung der Auflösung des Beschleunigungssensors
typedef enum
{
	MPU9250_ACCEL_2G      						= 0b00000000,
	MPU9250_ACCEL_4G      						= 0b00001000,
	MPU9250_ACCEL_8G      						= 0b00010000,
	MPU9250_ACCEL_16G     						= 0b00011000
} mpu9250_accel_sensitivity_e;








// Diese Struktur bündelt drei Messwerte (Triple)
typedef struct { MPU9250_FLOAT x; MPU9250_FLOAT y; MPU9250_FLOAT z; } mpu9250_triple_t;

// Diese Struktur bündelt vier Messwerte (Quaternion)
typedef struct { MPU9250_FLOAT q0; MPU9250_FLOAT q1; MPU9250_FLOAT q2; MPU9250_FLOAT q3; } mpu9250_quaternion_t;

void delay_ms(unsigned long interval);
void delay_us(unsigned long interval);

// *******************************************************************************************
// Auslesen der Sensoren
// *******************************************************************************************

// Initialisiert die MPU
unsigned char mpu9250_init();

void mpu9250_uninit();

// Startet die Messungen
unsigned char mpu9250_start();

// Beendet die Messungen
unsigned char mpu9250_stop();

// Initialisiert das Gyroskop
unsigned char mpu9250_enable_gyroscope(
	mpu9250_gyro_rate_e rate, 					/* Erfassungsrate */
	mpu9250_gyro_sensitivity_e sensitivity 		/* Empfindlichkeit */
);

// Deakiviere das Gyroskop
unsigned char mpu9250_disable_gyroscope();

// Richtet den Beschleunigungssensor ein
unsigned char mpu9250_enable_accelerometer(
	mpu9250_accel_rate_e rate, 					/* Erfassungsrate */
	mpu9250_accel_sensitivity_e sensitivity  	/* Empfindlichkeit */
);

// Deakiviere den Beschleunigungssensor
unsigned char mpu9250_disable_accelerometer();

// Diese Methode aktiviert den Temperatursensor
unsigned char mpu9250_enable_temperaturesensor();

// Diese Methode deaktiviert den Temperatursensor
unsigned char mpu9250_disable_temperaturesensor();

// Diese Methode aktiviert den FIFO beim Auslesen der Messwerte
unsigned char mpu9250_enable_fifo();

// Diese Methode deaktiviert den FIFO beim Auslesen der Messwerte
unsigned char mpu9250_disable_fifo();

// Diese Methode aktiviert den Digitalkompass des Moduls
unsigned char mpu9250_enable_magnetometer();

// Diese Methode deaktiviert den Digitalkompass des Moduls
unsigned char mpu9250_disable_magnetometer();




unsigned char mpu9250_measuregyrooffsetandnoise(mpu9250_triple_t* offset, mpu9250_triple_t* noise);







// *******************************************************************************************
// Auslesen der Sensoren
// *******************************************************************************************

// Liest den FIFO aus
unsigned char mpu9250_readFifo(unsigned char* hasmore);

// Liest den Beschleunigungssensor aus
void mpu9250_read_ACC(mpu9250_triple_t* accel);

// Liest das Gyroskop aus
void mpu9250_read_GYR(mpu9250_triple_t* gyro);

// Liest den Magnetfeldsensor aus
void mpu9250_read_MAG(mpu9250_triple_t* compass);

// Liest den Beschleunigungssensor und das Gyroskop aus
void mpu9250_read_ACC_GYR(mpu9250_triple_t* accel, mpu9250_triple_t* gyro);

// Liest den Beschleunigungssensor und den Magnetfeldsensor aus
void mpu9250_read_ACC_MAG(mpu9250_triple_t* accel, mpu9250_triple_t* compass);

// Liest das Gyroskop und den Magnetfeldsensor aus
void mpu9250_read_GYR_MAG(mpu9250_triple_t* gyro, mpu9250_triple_t* compass);

// Liest den Beschleunigungssensor, das Gyroskop und den Magnetfeldsensor aus
void mpu9250_read_ACC_GYR_MAG(mpu9250_triple_t* accel, mpu9250_triple_t* gyro, mpu9250_triple_t* compass);


// *******************************************************************************************
// Sensorfustion
// *******************************************************************************************

// Fusioniert die gegebenenen 6 Sensormesswerte
mpu9250_quaternion_t mpu9250_fuse6DOF(mpu9250_quaternion_t oldquaternion, unsigned int frequency, MPU9250_FLOAT correctionvalue, mpu9250_triple_t* acceldata, mpu9250_triple_t* gyrodata);

// Fusioniert die gegebenenen 9 Sensormesswerte
mpu9250_quaternion_t mpu9250_fuse9DOF(mpu9250_quaternion_t oldquaternion, unsigned int frequency, MPU9250_FLOAT correctionvalue, mpu9250_triple_t* acceldata, mpu9250_triple_t* gyrodata, mpu9250_triple_t* magdata);


// *******************************************************************************************
// Operationen auf Quaternionen
// *******************************************************************************************

// Erstellt eine neue Quaternion
mpu9250_quaternion_t mpu9250_createQuaternionNumber();

// Normalisiert die gegebene Quaternion a und gibt das Ergebnis aus
mpu9250_quaternion_t mpu9250_normalizeQuaternionNumber(mpu9250_quaternion_t a);

// Konjugiert die gegebene Quaternion a und gibt das Ergebnis aus
mpu9250_quaternion_t mpu9250_conjugateQuaternionNumber(mpu9250_quaternion_t a);

// Invertiert die gegebene Quaternion a und gibt das Ergebnis aus
mpu9250_quaternion_t mpu9250_invertQuaternionNumber(mpu9250_quaternion_t a);

// Normalisiert die gegebene Quaternion a und gibt das Ergebnis aus (schnelle Variante)
mpu9250_quaternion_t mpu9250_normalizeQuaternionNumberFast(mpu9250_quaternion_t a);

// Addiert die Quaternionen a und b und gibt das Ergebnis aus
mpu9250_quaternion_t mpu9250_addQuaternionNumbers(mpu9250_quaternion_t a, mpu9250_quaternion_t b);

// Subtrahiert die Quaternion b von a und gibt das Ergebnis aus
mpu9250_quaternion_t mpu9250_subQuaternionNumbers(mpu9250_quaternion_t a, mpu9250_quaternion_t b);

// Multipliziert die Quaternion a und b und gibt das Ergebnis aus
mpu9250_quaternion_t mpu9250_multiplyQuaternionNumbers(mpu9250_quaternion_t a, mpu9250_quaternion_t b);



// *******************************************************************************************
// Umwandlung von Sensordaten
// *******************************************************************************************

// Projeziert den gegebenen Sensorwert (in Weltkoordinaten) anhand der Drehquaternion in Sensorkoordinaten
mpu9250_triple_t mpu9250_convertWorldToSensor(mpu9250_triple_t worlddata, mpu9250_quaternion_t quaternion);

// Projeziert den gegebenen Sensorwert (in Sensorkoordinaten) anhand der Drehquaternion in Weltkoordinaten
mpu9250_triple_t mpu9250_convertSensorToWorld(mpu9250_triple_t sensordata, mpu9250_quaternion_t quaternion);

// Projeziert den gegebenen Sensorwert (in Weltkoordinaten) anhand der Drehquaternion in Sensorkoordinaten
mpu9250_triple_t mpu9250_convertWorldToSensorRaw(mpu9250_triple_t worlddata, mpu9250_quaternion_t quaternion, mpu9250_quaternion_t inversequaternion);

// Projeziert den gegebenen Sensorwert (in Sensorkoordinaten) anhand der Drehquaternion in Weltkoordinaten
mpu9250_triple_t mpu9250_convertSensorToWorldRaw(mpu9250_triple_t sensordata, mpu9250_quaternion_t quaternion, mpu9250_quaternion_t inversequaternion);






// Gibt den Interrupt Status aus
unsigned char mpu9250_getInterruptStatus();


#endif
