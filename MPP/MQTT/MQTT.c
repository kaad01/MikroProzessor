#include "MQTT.h"
#include "MQTTClient.h"


//=========================================================================
	unsigned char	Broker_Name[] 		= "diskstation.fritz.box";
	//unsigned char	Broker_Name[] 		= "test.mosquitto.org";
	unsigned long 	Broker_IP 			= 0L;
	//unsigned short 	Broker_Port 		= 9001; // Websocket
	unsigned short 	Broker_Port 		= 1883;
	//unsigned char 	Broker_User[30] 	= "test";
	//unsigned char 	Broker_Password[30] = "test";

//=========================================================================

	unsigned char 	mqtt_run = 0;
	unsigned char 	abbruch = 0;

//	unsigned char	buf_msg[200];
//	int 			buf_msg_len = sizeof(buf_msg);

	unsigned char	buf_out[200];
	int 			buf_out_len = sizeof(buf_out);

	unsigned char	buf_in[200];
	int 			buf_in_len = sizeof(buf_in);

	int tx_mqtt_msg 	= 0;	// Flag ob mqtt nachricht zu senden ist
	int tx_mqtt_bytes 	= 0;	// Anzahl zu sendender Bytes in der mqtt Nachricht
	int rx_mqtt_msg 	= 0;	// Flag ob mqtt Nachricht zu empfangen ist
	int rx_mqtt_bytes 	= 0;	// Anzahl der empfangenen Bytes in der mqtt Nachricht

//=========================================================================

	Network network;
	Client client;

	unsigned long MilliTimer;

//=========================================================================




//#########################################################################
//########## Beispiel mit direkter Socket Nutzung und Paho MQTTClient API
//#########################################################################

//=========================================================================
void mqtt_client(void)
//=========================================================================
{
	// schaltet select aufruf im systickhandler aus
	mqtt_run = 1;


	//######## Verbindung zum Broker aufbauen

	// bindet CC3100 Schnittstelle ein
	NewNetwork(&network);
	// Socket öffnen
	ConnectNetwork(&network, (char*)Broker_Name, Broker_Port);
	// Client anlegen
	MQTTClient(&client, &network, 1000,(unsigned char *) buf_out, buf_out_len, (unsigned char *) buf_in, buf_in_len);
	// MQTT Struct für die Verbindung anlegen
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	// Struct füllen
	data.clientID.cstring = "hwpx";
	data.keepAliveInterval = 20;
	data.cleansession = 1;
	// beim MQTT Broker anmelden
	MQTTConnect(&client,&data);

	//######## Subscribe

	MQTTSubscribe(&client, "hwp1/led", 0, mqtt_client_topic_handler);
	MQTTSubscribe(&client, "hwp1/beeper", 0, mqtt_client_topic_handler);
	MQTTSubscribe(&client, "hwp1/message", 0, mqtt_client_topic_handler);

	//######## Publish

	MQTTMessage msg;
	msg.id = 0;
	msg.dup = 0;
	msg.qos = 0;
	msg.retained = 0;
	msg.payload = "hallo hier hwpx bin online!";
	msg.payloadlen = 15;
	MQTTPublish(&client,"hwp1/message", &msg);


	//######## Warteschleife

	MQTTYield(&client,60000);

	//######## Broker abmelden

	MQTTDisconnect(&client);

	// schaltet select aufruf im systickhandler ein
	mqtt_run = 0;

}

//=========================================================================
void mqtt_client_topic_handler(MessageData* msg)
//=========================================================================
{
	char daten[50] = {0};
	char topic[50] = {0};

	memcpy(daten, (char*) msg->message->payload, msg->message->payloadlen);
	memcpy(topic, (char*) msg->topicName->lenstring.data, msg->topicName->lenstring.len);

	if (msg->message->payloadlen > 0)
	{
		usart2_printf("Topic=%s   Daten=%s\r\n", topic, daten);
	}
	else
	{
		usart2_printf("Topic=%s\r\n", topic);
	}
}



//=========================================================================
void mqtt_sensor_aktor_beispiel(void)
//=========================================================================
{
	char topic[30] = {0};
	char message[200] = {0};

	// Verbindung zum mqtt Broker aufbauen
	mqtt_open_connection();

	// topics subscribe und Handler anmelden
	MQTTSubscribe(&client, "hwpx/led", 0, mqtt_topic_handler_led);
	MQTTSubscribe(&client, "hwpx/beeper", 0, mqtt_topic_handler_beeper);
	MQTTSubscribe(&client, "hwpx/message", 0, mqtt_topic_handler_message);
	abbruch = 0;
	while(!abbruch)
	{
		// BME280 Sensordaten lesen und übertragen
		BME280_read_sensor();
		sprintf(topic,"hwpx");
		sprintf(message,"Luftdruck=%7.2f hPa \r\nHoehe=%7.2f m \r\nTemperatur=%5.2f Grad Celsius\r\nFeuchtigkeit=%5.2f Prozent\r\n",	BME280.druck, BME280.hoehe, BME280.temperatur, BME280.feuchtigkeit);
		mqtt_publish( topic, message);

		// aktiv auf Nachrichten zu angemeldeten topics warten
		// bei Nachrichteneingang wird automatisch angemeldeter
		// topic handler gestartet
		MQTTYield(&client,1000);
	}

	// bei abbruch der while schleife wird Verbindung beendet
	mqtt_close_connection();
}



void mqtt_open_connection(void)
{
	// schaltet select aufruf im systickhandler aus
	mqtt_run = 1;

	// bindet CC3100 Schnittstelle ein
	NewNetwork(&network);

	// Socket öffnen
	ConnectNetwork(&network, (char*)Broker_Name, Broker_Port);

	// Client anlegen
	MQTTClient(&client, &network, 1000,(unsigned char *) buf_out, buf_out_len, (unsigned char *) buf_in, buf_in_len);

	// MQTT Struct für die Verbindung anlegen
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

	// Struct füllen
	data.clientID.cstring = "hwpx";
	data.keepAliveInterval = 20;
	data.cleansession = 1;

	// beim MQTT Broker anmelden
	MQTTConnect(&client,&data);
}



void mqtt_close_connection(void)
{
	// beendet die Verbindung zum Broker
	MQTTDisconnect(&client);
	// schaltet select aufruf im systickhandler ein
	mqtt_run = 0;
}



void mqtt_publish(char* topic, char* message)
{
	// Übergebene message und topic verarbeiten und an broker schicken
	MQTTMessage msg;
	msg.id = 0;
	msg.dup = 0;
	msg.qos = 0;
	msg.retained = 0;
	msg.payload = message;
	msg.payloadlen = strlen(message);
	MQTTPublish(&client,topic, &msg);
}



//=========================================================================
void mqtt_topic_handler_led(MessageData* msg)
//=========================================================================
{
	char daten[200] = {0};
	char topic[50] = {0};

	memcpy(daten, (char*) msg->message->payload, msg->message->payloadlen);
	memcpy(topic, (char*) msg->topicName->lenstring.data, msg->topicName->lenstring.len);

	if (msg->message->payloadlen > 0)
	{
		if (!strcmp("hwpx/led",topic))
		{
			if (!strcmp("on",daten)){LED_GR_ON;}
			if (!strcmp("off",daten)){LED_GR_OFF;}
		}
	}
	else
	{
		usart2_printf("Topic=%s\r\n", topic);
	}
}



//=========================================================================
void mqtt_topic_handler_beeper(MessageData* msg)
//=========================================================================
{
	char daten[200] = {0};
	char topic[50] = {0};

	memcpy(daten, (char*) msg->message->payload, msg->message->payloadlen);
	memcpy(topic, (char*) msg->topicName->lenstring.data, msg->topicName->lenstring.len);

	if (msg->message->payloadlen > 0)
	{
		if (!strcmp("hwpx/beeper",topic))
		{
			if (!strcmp("kurz",daten)){beep(1000,500,0);}
			if (!strcmp("lang",daten)){beep(3000,1500,0);}
		}
	}
	else
	{
		usart2_printf("Topic=%s\r\n", topic);
	}
}



//=========================================================================
void mqtt_topic_handler_message(MessageData* msg)
//=========================================================================
{
	char daten[200] = {0};
	char topic[50] = {0};

	memcpy(daten, (char*) msg->message->payload, msg->message->payloadlen);
	memcpy(topic, (char*) msg->topicName->lenstring.data, msg->topicName->lenstring.len);

	if (msg->message->payloadlen > 0)
	{
		if (!strcmp("hwpx/message",topic))
		{
			if (!strcmp("shutdown",daten))
			{
				set_autostart_in(0,0,1,30);
			}
			if (!strcmp("stop",daten))
			{
				abbruch = 1;
			}
		}
	}
	else
	{
		usart2_printf("Topic=%s\r\n", topic);
	}
}





//#########################################################################
//########## Beispiel mit CC3100_select() Socket Nutzung
//#########################################################################

void mqtt_connect(void)
	{
		// Struct anlegen und füllen
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

		// Struct füllen
		data.clientID.cstring = "hwp01"; // max 23 Zeichen
		data.keepAliveInterval = 20;	// Timeout in sek
		data.cleansession = 1;			// nichts aufheben

		// buffer aus struct füllen
		tx_mqtt_bytes = MQTTSerialize_connect(buf_out, buf_out_len, &data);

		gethostbyname((signed char*) &Broker_Name,
						(unsigned char) strlen((char*)Broker_Name),
						&Broker_IP,
						SL_AF_INET);

		//if (Broker_IP == 0) {Broker_IP = 0xC0A8BC15 ;}

		handle_MQTT = CC3100_openSocketul(Broker_IP, Broker_Port, TCP_Client, mqtt_read_callback, mqtt_write_callback, 0);

		if (handle_MQTT == 99){ return;}

		memset(tx_buf[handle_MQTT], 0, sizeof(2048));
		memset(rx_buf[handle_MQTT], 0, sizeof(2048));

		rx_mqtt_msg = 0;
		tx_mqtt_msg = 1;

		while (tx_mqtt_msg == 1) {;} // warten bis Anmeldungsdaten gesendet wurden
		while (rx_mqtt_msg == 0) {;} // warten bis Broker auf Anmeldung geantwortet hat

	}




void mqtt_pub_bme280_sensor(void)
{
	unsigned char	pub_data[50] = {0};
	int		pub_len = 0;

	if (handle_MQTT != 99)
	{

		BME280_read_sensor();

		MQTTString pub_topic = MQTTString_initializer;

		while (tx_mqtt_msg ) {;}	// warten ob noch eine Nachricht im Sendezustand ist

		// Druckwert in das mqtt protokoll konvertieren
		sprintf((char*)pub_data, "%7.2f",BME280.druck);
		pub_len = (int)strlen((char*)pub_data);
		pub_topic.cstring = "hwp1/druck";
		tx_mqtt_bytes = MQTTSerialize_publish(buf_out, buf_out_len, 0, 1, 0, 0, pub_topic, pub_data, pub_len);


		tx_mqtt_msg = 1;				//  Flag für mqtt_write_callback() Funktion setzen das Daten zum Senden bereitstehen
		while (tx_mqtt_msg == 1) {;}	// warten das Daten durch mqtt_write_callback() gesendet wurden
		while (rx_mqtt_msg == 0) {;}	// warten das Daten durch mqtt_read_callback() bereitgestellt wurden


		// Temperaturwert in das mqtt protokoll konvertieren
		sprintf((char*)pub_data, "%5.2f", BME280.temperatur);
		pub_len = strlen((char*)pub_data);
		pub_topic.cstring = "hwp1/temperatur";
		tx_mqtt_bytes = MQTTSerialize_publish(buf_out, buf_out_len, 0, 1, 0, 0, pub_topic, pub_data, pub_len);


		tx_mqtt_msg = 1;				//  Flag für mqtt_write_callback() Funktion setzen das Daten zum Senden bereitstehen
		while (tx_mqtt_msg == 1) {;}	// warten das Daten durch mqtt_write_callback() gesendet wurden
		while (rx_mqtt_msg == 0) {;}	// warten das Daten durch mqtt_read_callback() bereitgestellt wurden


		// Feuchtigkeitswert in das mqtt protokoll konvertieren
		sprintf((char*)pub_data, "%5.2f", BME280.feuchtigkeit);
		pub_len = strlen((char*)pub_data);
		pub_topic.cstring = "hwp1/feuchtigkeit";
		tx_mqtt_bytes = MQTTSerialize_publish(buf_out, buf_out_len, 0, 1, 0, 0, pub_topic, pub_data, pub_len);


		tx_mqtt_msg = 1;				//  Flag für mqtt_write_callback() Funktion setzen das Daten zum Senden bereitstehen
		while (tx_mqtt_msg == 1) {;}	// warten das Daten durch mqtt_write_callback() gesendet wurden
		while (rx_mqtt_msg == 0) {;}	// warten das Daten durch mqtt_read_callback() bereitgestellt wurden
	}

}




//=========================================================================
void mqtt_pub_rtc(void)
//=========================================================================
{
	if (handle_MQTT != 99)
	{
		unsigned char	pub_data[50];
		int		pub_len = 0;

		while ( tx_mqtt_msg ){;}

		RTC_TimeTypeDef RTC_Time_Aktuell; 	// 	Zeit
		RTC_GetTime(RTC_Format_BIN, &RTC_Time_Aktuell);

		sprintf((char*)pub_data, "%.2d:%.2d:%.2d Uhr",
				RTC_Time_Aktuell.RTC_Hours,
				RTC_Time_Aktuell.RTC_Minutes,
				RTC_Time_Aktuell.RTC_Seconds);

		pub_len = strlen((char*)pub_data);

		MQTTString pub_topic = MQTTString_initializer;
		pub_topic.cstring = "hwp1/message";

		tx_mqtt_bytes = MQTTSerialize_publish(buf_out, buf_out_len, 0, 1, 0, 0, pub_topic, pub_data, pub_len);

		tx_mqtt_msg = 1;
		while (tx_mqtt_msg == 1) {;}
		while (rx_mqtt_msg == 0) {;}
	}
}


//=========================================================================
void mqtt_pub(char* topic, char* msg)
//=========================================================================
{

	if ((handle_MQTT != 99) & (tx_mqtt_msg == 0))
	{
		MQTTString pub_topic = MQTTString_initializer;

		pub_topic.cstring = topic;

		tx_mqtt_bytes = MQTTSerialize_publish(buf_out, buf_out_len, 0, 1, 0, 0, pub_topic, (unsigned char*)msg, strlen(msg));

		tx_mqtt_msg = 1;
		while (tx_mqtt_msg == 1) {;}
		while (rx_mqtt_msg == 0) {;}
	}
}

//=========================================================================
void mqtt_sub_topic(void)
//=========================================================================
{
	if (handle_MQTT != 99)
	{
		MQTTString topics[4] = {MQTTString_initializer};

		topics[0].cstring = "hwp1/message";
		topics[1].cstring = "hwp1/led";
		topics[2].cstring = "hwp1/beeper";
		topics[3].cstring = "hwp1/taster";

		int reqQoS[4] = {0};

		tx_mqtt_bytes = MQTTSerialize_subscribe(
										buf_out, 		// puffer
										buf_out_len, 	// pufferlänge
										0, 				// dup
										0, 				// msg id
										4, 				// Einträge array
										topics, 	// sub_topic array
										reqQoS
										);				// topic qos

		tx_mqtt_msg = 1;
		while (tx_mqtt_msg == 1) {;}
		while (rx_mqtt_msg == 0) {;}
	}
}

//=========================================================================
void mqtt_sub_topic1(void)
//=========================================================================
{
	if (handle_MQTT != 99)
	{
		MQTTString topic[1] = {MQTTString_initializer};

		topic[0].cstring = "hwp/test";

		int reqQoS[1] = {0};

		tx_mqtt_bytes = MQTTSerialize_subscribe(
										buf_out, 		// puffer
										buf_out_len, 	// pufferlänge
										0, 				// dup
										0, 				// msg id
										1, 				// Einträge array
										topic, 	// sub_topic array
										reqQoS
										);				// topic qos

		tx_mqtt_msg = 1;
		while (tx_mqtt_msg == 1) {;}
		while (rx_mqtt_msg == 0) {;}
	}
}


//=========================================================================
void mqtt_read_callback( char* rx, uint16_t rx_len, sockaddr* from, uint16_t socket_Position)
//=========================================================================
{
	rx_mqtt_bytes = 0;

	if(rx_len !=0)		// wenn Daten da
	{
		memcpy(buf_in,rx,rx_len);	// Daten in ausgabepuffer laden
		rx_mqtt_bytes = rx_len;		// Anzahl der Bytes merken
		rx_mqtt_msg = 1; 			// Nachricht ist da
		// message erhalten dann auswerten
		if ((buf_in[0] & 0xF0) == 0x30 )
		{
			rd_pub_message();
		}
	}
}




//=========================================================================
long mqtt_write_callback(	char* tx, uint16_t tx_len, uint16_t socket_Position)
//=========================================================================
{
	tx_len = 0;

	if ( tx_mqtt_msg == 1)						// Wenn Nachricht vorliegt senden
	{
		memcpy(tx , buf_out, tx_mqtt_bytes);	// Nachricht eintragen
		tx_len = tx_mqtt_bytes;					// Länge eintragen
		tx_mqtt_msg = 0;						// gesendet
	}
	return tx_len; 	// Länge der zu sendenden Daten zurückgeben
}




//=========================================================================
void rd_pub_message(void)
//=========================================================================
{
	int i = 0;
	int multi = 1;
	int rx_rem_len = 0;
	int rx_topic_len = 0;
	char topic_name[50] = {0};
	char topic_msg[50] = {0};

	usart2_printf("\r\nMessage Type:     0x%x%2\r\n",buf_in[i]);

	i++;
	do
	{
		rx_rem_len += (buf_in[i] & 127) * multi;
		multi *= 128;
		i++;
	}
	while ((buf_in[i] & 128) != 0);

	usart2_printf("Remaining Length: %d\r\n",rx_rem_len);

	rx_topic_len = buf_in[i];
	rx_topic_len <<= 8; //TODO
	i++;
	rx_topic_len += buf_in[i];

	usart2_printf("Topic Length:     %d\r\n",rx_topic_len);

	i++;
	memcpy(topic_name,&buf_in[i],rx_topic_len);

	usart2_printf("Topic Name:       %s\r\n",topic_name);

	i += rx_topic_len;
	memcpy(topic_msg,&buf_in[i],(rx_rem_len - rx_topic_len -2));

	usart2_printf("Publish Message:  %s\r\n",topic_msg);
}




//#########################################################################
//########## CC3100 Socket Interface
//#########################################################################


//=========================================================================
void NewNetwork(Network* n)
//=========================================================================
{
	n->my_socket = 0;
	n->mqttread = cc3100_read;
	n->mqttwrite = cc3100_write;
	n->disconnect = cc3100_disconnect;
}


//=========================================================================
int ConnectNetwork(Network* n, char* addr, int port)
//=========================================================================
{
	SlSockAddrIn_t sAddr;
	int addrSize;
	int retVal;
	long unsigned int ipAddress = 0;

	sl_NetAppDnsGetHostByName((signed char*)addr, strlen(addr), &ipAddress, AF_INET); // TODO IP wird nicht aufgelöst

	sAddr.sin_family = AF_INET;
	sAddr.sin_port = sl_Htons((unsigned short)port);
	sAddr.sin_addr.s_addr = sl_Htonl(ipAddress);

	addrSize = sizeof(SlSockAddrIn_t);

	n->my_socket = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
	if( n->my_socket < 0 )
		{
			// error
			return -1;
		}

	retVal = sl_Connect(n->my_socket, ( SlSockAddr_t *)&sAddr, addrSize);
	if( retVal < 0 )
		{
			// error
			sl_Close(n->my_socket);
			return retVal;
		}
	return retVal;
}


//=========================================================================
int cc3100_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
//=========================================================================
{
	SlTimeval_t timeVal;
	SlFdSet_t fdset;
	int rc = 0;
	int recvLen = 0;

	SL_FD_ZERO(&fdset);
	//SL_FD_SET(0,&fdset);
	SL_FD_SET(n->my_socket, &fdset);

	timeVal.tv_sec = 0;
	timeVal.tv_usec = timeout_ms * 1000;
	if (sl_Select(n->my_socket + 1, &fdset, NULL, NULL, &timeVal) == 1) {
		do {
			rc = sl_Recv(n->my_socket, buffer + recvLen, len - recvLen, 0);
			recvLen += rc;
			if (rc <= 0) {sl_Close(n->my_socket); return 0;} // todo
		} while(recvLen < len);
	}
	return recvLen;
}


//=========================================================================
int cc3100_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
//=========================================================================
{
	SlTimeval_t timeVal;
	SlFdSet_t fdset;
	int rc = 0;
	int readySock;

	SL_FD_ZERO(&fdset);
	SL_FD_SET(n->my_socket, &fdset);

	timeVal.tv_sec = 0;
	timeVal.tv_usec = timeout_ms * 1000;
	do {
		readySock = sl_Select(n->my_socket + 1, NULL, &fdset, NULL, &timeVal);
	} while(readySock != 1);
	rc = sl_Send(n->my_socket, buffer, len, 0);
	return rc;
}



//=========================================================================
void cc3100_disconnect(Network* n)
//=========================================================================
{
	sl_Close(n->my_socket);
}





//#########################################################################
//########## Timeout Hilffunktionen
//#########################################################################


//=========================================================================
void InitTimer(Timer* timer)
//=========================================================================
{
	timer->end_time = 0;
}


//=========================================================================
void MQTT_SysTickHandler(void)
//=========================================================================
{
	MilliTimer = MilliTimer + 1;
}


//=========================================================================
char expired(Timer* timer)
//=========================================================================
{
	long left = timer->end_time - MilliTimer;
	return (left < 0);
}


//=========================================================================
int left_ms(Timer* timer)
//=========================================================================
{
	long left = timer->end_time - MilliTimer;
	return (left < 0) ? 0 : left;
}


//=========================================================================
void countdown_ms(Timer* timer, unsigned int timeout)
//=========================================================================
{
	timer->end_time = MilliTimer + timeout;
}


//=========================================================================
void countdown(Timer* timer, unsigned int timeout)
//=========================================================================
{
	timer->end_time = MilliTimer + (timeout * 1000);
}




