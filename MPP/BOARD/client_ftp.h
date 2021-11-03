#ifndef __FTP_CLIENT_H__
#define __FTP_CLIENT_H__


//=========================================================================
// cmsis_lib
//=========================================================================


//=========================================================================
// board_lib
//=========================================================================


//=========================================================================
// mpp_lib
//=========================================================================
#include "simplelink.h"
#include "CC3100.h"
#include "SDCARD.h"
#include "ff.h"
#include "diskio.h"


//=========================================================================
// standard_lib
//=========================================================================
#include "stdlib.h"
#include "string.h"


//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================

//========== Macros

//========== Variablen

//========== Funktionen

int ftp_connect(char* server,
				char* user,
				char* passw);

int ftp_connect_data(char* adr);

int ftp_close(void);

int ftp_close_data(void);

int ftp_put_sd_file(char* local_path,
					char* local_file_name,
					char* remote_path,
					char* remote_file_name);

int ftp_get_sd_file(char* remote_path,
					char*remote_file_name,
					char* local_path,
					char* local_file_name);

void ftp_read_command_callback(char* rx,
					uint16_t rx_len,
					sockaddr* from,
					uint16_t socket_Position);

long ftp_write_command_callback(char* tx,
					uint16_t tx_len,
					uint16_t socket_Position);

void ftp_read_data_callback(char* rx,
					uint16_t rx_len,
					sockaddr* from,
					uint16_t socket_Position);

long ftp_write_data_callback(char* tx,
					uint16_t tx_len,
					uint16_t socket_Position);

// FTP Commands
//=============
//
//   http://www.nsftools.com/tips/RawFTP.htm



// FTP Return Codes
//=================
// 110	Restart marker reply. In this case, the text is exact and not left to the particular implementation; it must read: MARK yyyy = mmmm (yyyy is user-process data stream marker, and mmmm server's equivalent marker. Note the spaces between markers and =).
// 120	Service ready in nnn minutes.
// 125	Data connection already open; transfer starting.
// 150	File status okay; about to open data connection.
// 200	Command okay.
// 202 	Command not implemented, superfluous at this site.
// 211	System status, or system help reply.
// 212	Directory status.
// 213	File status.
// 214	Help message. On how to use the server or the meaning of a particular non-standard command. This reply is useful only to the human user.
// 215	NAME system type. Where NAME is an official system name from the list in the Assigned Numbers document.
// 220	Service ready for new user.
// 221	Service closing control connection.
// 225	Data connection open; no transfer in progress.
// 226	Closing data connection. Requested file action successful (for example: file transfer, file abort).
// 227	Entering Passive Mode (h1,h2,h3,h4,p1,p2).
// 230	User logged in, proceed. Logged out if appropriate.
// 250	Requested file action okay, completed.
// 257	PATHNAME created.
// 331 	User name okay, need password.
// 332	Need account for login.
// 350 	Requested file action pending further information.
// 421	Service not available, closing control connection. This may be a reply to any command if the service knows it must shut down.
// 425	Cannot open data connection.
// 426	Connection closed; transfer aborted.
// 450	Requested file action not taken.
// 451	Requested action aborted. Local error in processing.
// 452	Requested action not taken. Insufficient storage space in system. File unavailable (For example, file busy).
// 500	Syntax error, command unrecognized. This may include errors such as command line too long.
// 501	Syntax error in parameters or arguments.
// 502	Command not implemented.
// 503	Bad sequence of commands.
// 504	Command not implemented for that parameter.
// 530	Not logged in.
// 532 	Need account for storing files.
// 550	Requested action not taken. File unavailable (For example, file not found, no access).
// 551	Requested action aborted. Page type unknown.
// 552	Requested file action aborted. Exceeded storage allocation (for current directory or dataset).
// 553	Requested action not taken. File name not allowed.

#endif // __FTP_CLIENT_H__
