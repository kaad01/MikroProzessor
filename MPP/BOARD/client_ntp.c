/*
 * Copyright (C) 2014 Markus Hoffmann <mackone@inf.fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. This LICENSE in it's most up to date version can be
 * found under: https://www.gnu.org/licenses/lgpl-2.1.html
 */


#include "client_ntp.h"


// Funktionen zum ermitteln der Zeit über einen Zeitserver

//=== Globale Variablen =======================================================
uint32_t last_NTP_Sync;
char NTP_Server_Name[32];
char NTP_Servers[10][30];
_Bool retryNTP = false;
_Bool ntp_Zeit_aktualisiert = false;
ntp_packet_t ntp_pkt;


//=== Lokale Funktionen =======================================================
void retrieve_NTP_time_Callback(void);
void init_ntp_packet(void);

void ntp2stream(ntp_packet_t* p, char * tx);
void stream2ntp(ntp_packet_t* p, char * rx);

_Bool resync_Needed(void);
long ntp_writeCallback(char* tx, uint16_t tx_len, uint16_t socket_Position);

//=============================================================================
//=== Globale Funktionen (Schnittstelle) ======================================
//=============================================================================

void ntp_read_callback(	char* rx,
						uint16_t rx_len,
						sockaddr* from,
						uint16_t socket_Position)
{	//TODO index socket_Position
	RTC_GetTime(RTC_Format_BIN, &ntp_sync_stop_Time);
	RTC_GetDate(RTC_Format_BIN, &ntp_sync_stop_Date);

	stream2ntp(&ntp_pkt, rx);

	set_RTC_from_NTPsec(ntp_pkt.ts_tx, 1);

	handle_NTP = CC3100_closeSocket(handle_NTP);
	memset(&rx_daten, 0, sizeof(rx_daten));	//TODO

	S3_WaitForAnswer = false;

	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

	ntp_Zeit_aktualisiert = true;
}
//*****************************************************************************
//
//! get_NTP_Time
//!
//!  @param  	String - Name des Servers
//!				ntp.inode.at 				195.58.160.5 	Vienna
//!				ntp3.proserve.nl 			212.204.198.85 	Amsterdam
//!				ntp1.madavi.de 				194.50.97.12 	Stuttgart, Germany
//!				ntp.spadhausen.com 			109.168.118.249 Milano , Italy
//!				ntp3.tcpd.net 				23.23.128.218 	London, UK
//!				dmz0.la-archdiocese.net 	209.151.225.100 Los Angeles, CA
//!				Optimussupreme.64bitVPS.com 216.128.88.62 	Brooklyn, New York
//!				ntp.mazzanet.id.au 			203.206.205.83 	Regional Victoria, Australia
//!				a.ntp.br
//!				ntps1-0.cs.tu-berlin.de
//!
//!  @return 	Zeit in Milisekunden vom 1.1.1900 aus gesehen
//!
//!  @brief  	Nicht blockierende Funktion die die Zeit von einem InternetServer
//! 			abruft.
//
//*****************************************************************************
unsigned long get_NTP_Time(char* SNTP_Server_Name) {
	unsigned long NTP_Server = 0L;
	uint16_t port_NTP = 123;
	static uint8_t index;
	signed char result = 0;

	init_NTPServer();
	if (index++ > 9) { index = 0;}
	index =0;

	gethostbyname( (_i8*) NTP_Servers[index],
					strlen(NTP_Servers[index]),
					&NTP_Server,
					SL_AF_INET);
	if (handle_NTP != 99)
	{
		handle_NTP = CC3100_closeSocket(handle_NTP);
	}

	// ul steht für unsigned long ip als zahl aus gethost zurück....
	handle_NTP = CC3100_openSocketul(NTP_Server, port_NTP, UDP_ClientServer, ntp_read_callback, ntp_write_callback, 0);
	S3_WaitForAnswer = false;
	memset(tx_buf[handle_NTP], 0, sizeof(2048));
	memset(rx_buf[handle_NTP], 0, sizeof(2048));

	usart2_printf("=> Abfrage NTP Zeitserver: %s \r\n", NTP_Servers[index]);

	init_ntp_packet();
	ntp2stream(&ntp_pkt, tx_buf[handle_NTP]);

	RTC_GetTime(RTC_Format_BIN, &ntp_sync_startTime);
	RTC_GetDate(RTC_Format_BIN, &ntp_sync_startDate);

	result = sendto(handle_NTP, &tx_buf[handle_NTP], 48, 0, &sockets[handle_NTP], sizeof(S3_NTP));

	if (result >= 0)
	{
		S3_WaitForAnswer = true;
		//set_RTC_Alarm_in(0, 0, 0, 20, &retrieve_NTP_time_Callback);	//TODO
		//start_RTC_Alarm();											//TODO
	}

	return handle_NTP;
}

long ntp_write_callback(char* tx, uint16_t tx_len, uint16_t socket_Position) {
				// TX Buffer füllen
	return 0; 	// Länge der zu sendenden Daten zurückgeben
}

void init_NTPServer(void) {
	strcpy(NTP_Servers[0], "ntps1-0.cs.tu-berlin.de");
	strcpy(NTP_Servers[1], "ntp0.fau.de");
	strcpy(NTP_Servers[2], "time.windows.com");
	strcpy(NTP_Servers[3], "time.nist.gov");
	strcpy(NTP_Servers[4], "ntp.spadhausen.com");
	strcpy(NTP_Servers[5], "ntp3.tcpd.net");
	strcpy(NTP_Servers[6], "dmz0.la-archdiocese.net");
	strcpy(NTP_Servers[7], "Optimussupreme.64bitVPS.com");
	strcpy(NTP_Servers[8], "ntp.mazzanet.id.au");
	strcpy(NTP_Servers[9], "a.ntp.br");
}

//=== LOKALE FUNKTIONEN =======================================================
void init_ntp_packet(void) {
	ntp_pkt.li = 3;
	ntp_pkt.version = 3; 		//	Client
	ntp_pkt.Mode = 1; 			//	Mode 				3: Client, 4: Server
	ntp_pkt.pr_Stratum = 0; 	//	Peer Clock Stratum	16: unsynchronized
	ntp_pkt.pr_pollInt = 10; 	//	Polling Interval	6-10
	ntp_pkt.pr_clkPrec = 250; 	//	Clock Precision		250 0,015625 sec
	ntp_pkt.rt_Delay = 0; 		//	Root Delay			0
	ntp_pkt.rt_disper = 66192; 	//	Dispersion 			66192 = 1,01 sec
}

void ntp2stream(ntp_packet_t* p, char * tx)
{
	memcpy(tx, p, 4);
	uint32_stream((unsigned char*) tx, 4, &p->rt_Delay);
	uint32_stream((unsigned char*) tx, 8, &p->rt_disper);
}


void stream2ntp(ntp_packet_t* p, char * rx)
{
	memcpy(p, rx, 4);
	stream_uint32((unsigned char *) rx, 4, &p->rt_Delay);
	stream_uint32((unsigned char *) rx, 8, &p->rt_disper);
	stream_uint32((unsigned char *) rx, 12, &p->ref_ID);
	stream_uint64(rx, 16, &p->ts_ref);
	stream_uint64(rx, 24, &p->ts_org);
	stream_uint64(rx, 32, &p->ts_rx);
	stream_uint64(rx, 40, &p->ts_tx);
	last_NTP_Sync = p->ts_tx >> 32;
}


void uint32_stream(unsigned char * to, uint16_t offset, uint32_t* from)
{
	*(to + offset) = (*from & 0xFF000000) >> 24;
	*(to + offset + 1) = (*from & 0xFF0000) >> 16;
	*(to + offset + 2) = (*from & 0xFF00) >> 8;
	*(to + offset + 3) = (*from & 0xFF);
}


void stream_uint32(unsigned char * from, uint16_t offset, uint32_t* to)
{
	unsigned char temp[4] = { 0, 0, 0, 0 };
	temp[0] = (*(from + offset + 3));
	temp[1] = (*(from + offset + 2));
	temp[2] = (*(from + offset + 1));
	temp[3] = (*(from + offset));

	*to = *((uint32_t*) temp);
}


void stream_uint64(char * from, uint16_t offset, uint64_t* to)
{
	static unsigned char temp[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	temp[0] = (*(from + offset + 7));
	temp[1] = (*(from + offset + 6));
	temp[2] = (*(from + offset + 5));
	temp[3] = (*(from + offset + 4));
	temp[4] = (*(from + offset + 3));
	temp[5] = (*(from + offset + 2));
	temp[6] = (*(from + offset + 1));
	temp[7] = (*(from + offset));

	*to = *((uint64_t*) temp);
}


void retrieve_NTP_time_Callback(void)
{
	uart_send("NTP Timeout\r\n");
	if (S3_WaitForAnswer == true)
	{
		retryNTP = true;
	}
}


_Bool resync_Needed(void)
{
	uint32_t time = 0;
	time = rtc_getSek(TIMEBASE_1900);

	if (time > (last_NTP_Sync + 604800))
	{
		return true;
	}
	else
	{
		return false;
	}
}



