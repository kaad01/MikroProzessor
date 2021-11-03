#include "DW1000.h"

// Variable fuer das Timeout-Handling
unsigned long systickcounter = 0;
int dw1000_timeout_ms = 0;

// Globale Variablen fuer die Entfernung von den vier Ankerpunkten
float r1=0;
float r2=0;
float r3=0;
float r4=0;

unsigned int knoten_id = 0;
char uwb_send = 0;
char temp[100] = {0};

// DW1000 Initialisierung und Eventhandler anmelden
void DW1000_init(void){
	uwbranging_initialize();
	uwbranging_registerByteArrayMessageHandler(HandleByteMessageReceived);
	uwbranging_registerTextMessageHandler(HandleTextMessageReceived);
	uwbranging_registerRangeHandler(HandleRangeReceived);
}


// Eventhandler für abgeschlossene Sendeprozesse
void HandleTransmission(unsigned long long timestamp, float temperature, unsigned char error)
{
	usart2_send("Paket erfolgreich gesendet\r\n");
	uwb_send = 0;
}


// Eventhandler für abgeschlossene Entfernungsmessungen
void HandleRangeReceived(unsigned short remoteinstance, float range)
{

	if ( remoteinstance == knoten_id)
	{
		if (range > 0) {r1 = range;}  else {r1  = 0;}
	}
	else
	{
		switch (remoteinstance)
		{
			case 100:  if (range > 0) {r1 = range;}  else {r1  = 0;} break;
			case 101:  if (range > 0) {r2 = range;}  else {r2  = 0;} break;
			case 102:  if (range > 0) {r3 = range;}  else {r3  = 0;} break;
			case 103:  if (range > 0) {r4 = range;}  else {r4  = 0;} break;
		}
	}
	ismeasuring = 0;
}


// Eventhandler für eingehende Textmitteilungen
void HandleTextMessageReceived(unsigned short sender, unsigned char* content, unsigned long long int rxtimestamp, double rxpower, float rxtemperature)
{
	usart2_send("Textmitteilung: ");
	usart2_send((char*)content); usart2_send("\r\n");
}


// Eventhandler für eingehende Bytearraymitteilungen
void HandleByteMessageReceived(unsigned short sender, unsigned char* content, int contentlength, unsigned long long int rxtimestamp, double rxpower, float rxtemperature)
{
	char tmp[64];
	sprintf(tmp, "Bytearray-Paket mit %d Bytes empfangen \r\n", contentlength);
	usart2_send(tmp);
}



//=========================================================================
// ermittelt einmalig die Entfernung zu dem übergebenen Ankerknoten
void distanz(unsigned int id)
//=========================================================================
{
	knoten_id = id;

	dw1000_wakefromsleep();

	uwbranging_tick();

	ismeasuring = 1;

	while (!uwbranging_startRanging(id))
		{
			uwbranging_tick();
			ismeasuring = 1;
		}
		while(ismeasuring)
		{
			uwbranging_tick();
		}

	dw1000_delay_mSek(4);

	dw1000_entersleep();

	sprintf(temp,"Distanz zu Knoten %d ist %6.2f m\r\n",knoten_id, r1);
	usart2_send(temp);

	knoten_id = 0;
}



//=========================================================================
// ermittelt einmalig die Entfernung zu den vier Ankerknoten 100...103
void position(void)
//=========================================================================
{
	int16_t i = 0;

	dw1000_wakefromsleep();

	for (i=100; i<=103; i++)
	{
		ismeasuring = 1;
		while (!uwbranging_startRanging(i))
		{
			uwbranging_tick();
			ismeasuring = 1;
		}
		while(ismeasuring)
		{
			uwbranging_tick();
		}
		dw1000_delay_mSek(4);
	}

	dw1000_entersleep();

	sprintf(temp,"%6.2f;%6.2f;%6.2f;%6.2f\r\n",r1,r2,r3,r4);
	usart2_send(temp);

	r1=r2=r3=r4=-1;
}



//=========================================================================
void lokalisieren(void)
//=========================================================================
{
	// Quaternion
	mpu9250_quaternion_t quaternion;
	// Beschleunigung, Rotation und Magnetfeldrichtung
	mpu9250_triple_t acceleration;
	mpu9250_triple_t magnetometer;
	mpu9250_triple_t rotation;

	int i = 1;
	unsigned char hasmore = 1;
	unsigned long int interval = systickcounter;

	if (mpu9250_init())
	{
		// Initialisiere das Gyroskop
		// Parameter 1: Abtastrate (mpu9250.h)
		// Parameter 2: Auflösung (mpu9250.h)
		mpu9250_enable_gyroscope(MPU9250_GYRO_1KHZ_FILTER_184HZ, MPU9250_GYRO_250_DPS);
		// Initialisiere den Beschleunigungssensor
		// Parameter 1: Abtastrate (mpu9250.h)
		// Parameter 2: Auflösung (mpu9250.h)
		mpu9250_enable_accelerometer( MPU9250_ACCEL_1KHZ_FILTER_184HZ, MPU9250_ACCEL_2G );
		// Initialisiere den Magnetfeldsensor
		mpu9250_enable_magnetometer();
		quaternion =  mpu9250_createQuaternionNumber();
		// Festlegen, ob der FIFO verwendet werden soll
		mpu9250_enable_fifo();
		// Beginne mit den Messungen
		mpu9250_start();
	}

	mqtt_connect();
	//dw1000_wakefromsleep();

	while(1)
	{
		//====================Entfernung Messen
		for (i=100; i<=103; i++)
		{
			ismeasuring = 1;
			while (!uwbranging_startRanging(i))
			{
				uwbranging_tick();
				ismeasuring = 1;
			}
			while(ismeasuring)
			{
				uwbranging_tick();
			}
			dw1000_delay_mSek(4);
		}
		//====================Sensor lesen
		hasmore = 1;
		while(hasmore)		// FIFO auslesen
		{
			if (mpu9250_readFifo(&hasmore))
			{
				mpu9250_read_ACC_GYR_MAG(&acceleration, &rotation, &magnetometer);
				quaternion = mpu9250_fuse6DOF(quaternion, 1000, 0.2, &acceleration, &rotation);
			}
			else
			{
				break;
			}
		}
		//====================zyklische Ausgabe an Broker
		if (systickcounter > (interval + 200)) // alle 200ms Werte an MQTT Broker schicken
		{
			interval = systickcounter;
			sprintf(temp,"%f;%f;%f;%f;%f;%f;%f;%f\r\n",quaternion.q0, quaternion.q1, quaternion.q2, quaternion.q3,r1,r2,r3,r4);
			usart2_send(temp);
			sprintf(temp,"%f;%f;%f;%f;%f;%f;%f;%f",quaternion.q0, quaternion.q1, quaternion.q2, quaternion.q3,r1,r2,r3,r4);
			mqtt_pub("hwp1/q",temp);
		}
		//====================
	}
}



