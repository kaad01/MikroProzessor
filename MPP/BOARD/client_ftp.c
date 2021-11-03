#include "client_ftp.h"

//=========================================================================
//==== variablen
//=========================================================================

#define f_size(fp) ((fp)->fsize)

// Zugangsdaten
char 	ftp_user[20] = {0};
char 	ftp_password[20] ={0};
char 	sdcard_file_name[20];

// Status Server Verbindung
char 	ftp_command_login = 0;

// Command Socket
int	 	ftp_return_code = 0;
char	ftp_send_command[80];
char 	ftp_return_string[80];

// Data Socket
int 	ftp_bytes_to_send = 0;
int 	ftp_bytes_to_receive = 0;
int 	bytes_to_write = 0;
int 	aaa = 0;
unsigned int 	bytes_written = 0;
char 	ftp_data_send = 0;
char 	ftp_data_received = 0;
char 	ftp_data_buffer[2048];



//=========================================================================
//==== ftp_connect
//=========================================================================
int ftp_connect(char* server,
				char* user,
				char* passw)
{
	unsigned long ip_ftp_server = 0L;

	strcpy(ftp_user,user);
	strcpy(ftp_password,passw);

	gethostbyname( (signed char*)server,
				(unsigned char) strlen(server),
				&ip_ftp_server,
				SL_AF_INET);

	handle_FTP_Command = CC3100_openSocketul(ip_ftp_server,
									21,
									TCP_Client,
									ftp_read_command_callback,
									ftp_write_command_callback,
									0);
	if (handle_FTP_Command >= 16) { handle_FTP_Command = handle_FTP_Command -16;}

	memset(tx_buf[handle_FTP_Command], 0, sizeof(2048));
	memset(rx_buf[handle_FTP_Command], 0, sizeof(2048));

	while (ftp_command_login == 0){;}

	return 1;
}




int ftp_connect_data(char* adr)
{
	int	 i,ip1,ip2,ip3,ip4,port,port1,port2;
	char delimiter[] = "(,)";
	char *ptr;

	ptr = strtok(adr, delimiter);
	i = 0;
	while(ptr != NULL)
	{
		if (i==1) {ip1= atoi(ptr);}
		if (i==2) {ip2= atoi(ptr);}
		if (i==3) {ip3= atoi(ptr);}
		if (i==4) {ip4= atoi(ptr);}
		if (i==5) {port1= atoi(ptr);}
		if (i==6) {port2= atoi(ptr);}
		i++;
		ptr = strtok(NULL, delimiter);
	}

	port = (port1 <<8 ) + port2 ;

	handle_FTP_Data = CC3100_openSocket	(
					(unsigned char) ip1,
					(unsigned char) ip2,
					(unsigned char) ip3,
					(unsigned char) ip4,
					(uint16_t) port,
					TCP_Client,
					ftp_read_data_callback,
					ftp_write_data_callback,
					0);

	if (handle_FTP_Data >= 16) { handle_FTP_Data = handle_FTP_Data -16;}
	memset(tx_buf[handle_FTP_Data], 0, sizeof(2048));
	memset(rx_buf[handle_FTP_Data], 0, sizeof(2048));

	return 1;
}




//=========================================================================
//==== ftp_close
//=========================================================================
int ftp_close(void)
{
    if(handle_FTP_Data != 99) { ftp_close_data(); }
    if(handle_FTP_Command != 99)
    {
    	ftp_command_login  = 0;
        CC3100_closeSocket(handle_FTP_Command);
        handle_FTP_Command = 99;
    }

    return 1 ;
}

int ftp_close_data(void){
    if(handle_FTP_Data != 99)
    {
        CC3100_closeSocket(handle_FTP_Data);
        handle_FTP_Data = 99;
    }
    return 1 ;
}




//=========================================================================
//==== ftp_put_file
//=========================================================================
int ftp_put_sd_file(char* local_path,
					char* local_file_name,
					char* remote_path,
					char* remote_file_name)
{

	FIL 	file_objekt;
    FRESULT res_code;

    unsigned int 	bytes_to_read = 0;
    unsigned int 	bytes_readwrite = 0;
    unsigned int 	offset = 0;
    unsigned int 	btr = 0;

	char f_name[30];

	if(handle_FTP_Command != 99)
	{
		if (strlen(remote_path) != 0)
		{
			sprintf(ftp_send_command,"CWD %s\r\n",remote_path);
			while (ftp_return_code != 250) { ; }
		}

		sprintf(ftp_send_command,"TYPE I\r\n");
		while (ftp_return_code != 200) { ; }

		sprintf(ftp_send_command,"PASV\r\n");
		while (ftp_return_code != 227) { ; }

		ftp_connect_data(ftp_return_string);

		sprintf(ftp_send_command,"STOR %s\r\n",remote_file_name);
		while (ftp_return_code != 150) { ; }

		sprintf(f_name,"0:");
		strcat(f_name,local_file_name);

		res_code = f_open(&file_objekt, local_file_name, FA_READ);

		if(res_code == FR_OK)
		{
			bytes_to_read = f_size(&file_objekt);
			bytes_readwrite = 1;
			ftp_bytes_to_send = bytes_to_read;

			ftp_data_send = 0;
			offset = 0;
			while (ftp_bytes_to_send > 0 || bytes_readwrite > 0)
			{
				if((ftp_data_send == 0 ) && (bytes_readwrite > 0))
				{
					if ( ftp_bytes_to_send >= sizeof(ftp_data_buffer) )
					{
						btr = sizeof(ftp_data_buffer);
					}
					else
					{
						btr = ftp_bytes_to_send;
					}
					memset(ftp_data_buffer,0x00,sizeof(ftp_data_buffer));
					res_code = f_read(&file_objekt, &ftp_data_buffer, btr, &bytes_readwrite);

					aaa = bytes_readwrite;

					ftp_bytes_to_send = ftp_bytes_to_send - bytes_readwrite;

					offset = offset + bytes_readwrite;
					res_code = f_lseek(&file_objekt,offset);

					ftp_data_send = 1;
				}
			}
		}
		res_code = f_close(&file_objekt);

		while( ftp_data_send != 0){ ; }

		ftp_close_data();

		return 1;
	}
	else
	{
		return -1;
	}
}



//=========================================================================
//==== ftp_get_file
//=========================================================================
int ftp_get_sd_file(char* remote_path,
					char* remote_file_name,
					char* local_path,
					char* local_file_name)
{
	//char bbb[100];

    FRESULT res_code;
	FIL 	file_objekt;

	char delimiter1[] = " ";
	char *ptr1;

	if(handle_FTP_Command != 99)
	{
		if (strlen(remote_path) != 0)
		{
			sprintf(ftp_send_command,"CWD %s\r\n",remote_path);
			while (ftp_return_code != 250) { ; }
		}

//		if (strlen(local_path) != 0 )
//		{
//			f_mkdir (const XCHAR*);
//			f_opendir (DIR*, const XCHAR*);
//			f_chdir (const XCHAR*);
//		}

		sprintf(ftp_send_command,"TYPE I\r\n");
		while (ftp_return_code != 200) { ; }

		sprintf(ftp_send_command,"PASV\r\n");
		while (ftp_return_code != 227) { ; }

		ftp_connect_data(ftp_return_string);

		sprintf(ftp_send_command,"SIZE %s\r\n",remote_file_name);
		while (ftp_return_code != 213) { ; }
		//213 Bytes  - Anzahl Bytes in der Datei aus Antwort ermitteln
		ptr1 = strtok(ftp_return_string, delimiter1);
		ptr1 = strtok(NULL, delimiter1);	//213
		ftp_bytes_to_receive = atoi(ptr1);	//Bytes

		res_code = f_open(&file_objekt,local_file_name,  FA_WRITE | FA_CREATE_ALWAYS);
		if (res_code < 0 )
		{
			//todo
		}

		sprintf(ftp_send_command,"RETR %s\r\n",remote_file_name);
		while (ftp_return_code != 150) { ; }

		while ((ftp_bytes_to_receive > 0) || (ftp_data_received == 1))
		{
			if (ftp_data_received == 1)
			{
				res_code = f_lseek(&file_objekt, f_size(&file_objekt));
				res_code = f_write(&file_objekt,ftp_data_buffer,bytes_to_write,&bytes_written);

				memset(ftp_data_buffer,0x00,sizeof(ftp_data_buffer));
				ftp_data_received = 0;
				if (ftp_bytes_to_receive == 0) {ftp_bytes_to_receive = -1;}
			}
		}

		res_code = f_close(&file_objekt);

		ftp_close_data();

		while (ftp_return_code != 226) { ; }

		sprintf(ftp_send_command,"TYPE A\r\n");
		while (ftp_return_code != 200) { ; }

		return 1;
	}
	else
	{
		return -1;
	}
}



//=========================================================================
//==== ftp_read_command_callback
//=========================================================================
void ftp_read_command_callback(	char* rx,
						uint16_t rx_len,
						sockaddr* from,
						uint16_t socket_Position)
{
	char ftp_return_code_string[3];
	memset(ftp_return_string,0x00,sizeof(ftp_return_string));
	ftp_return_code = 0;

	if (rx_len > 0)
	{
		memcpy(ftp_return_code_string,rx,3);

		// FTP Return Code
		ftp_return_code = atoi(ftp_return_code_string);

		usart2_send("\r\n=>");
		usart2_send(ftp_return_code_string);
		usart2_send("\r\n");

		if (ftp_command_login  == 0)
		{
			// 220	Service ready for new user.
			if(ftp_return_code == 220) { sprintf(ftp_send_command,"USER %s\r\n",ftp_user); }

			// 331 	User name okay, need password.
			if(ftp_return_code == 331) { sprintf(ftp_send_command,"PASS %s\r\n",ftp_password); }

			// 230	User logged in, proceed. Logged out if appropriate.
			if(ftp_return_code == 230) { sprintf(ftp_send_command,"TYPE A\r\n"); }

			// 200	Command okay.
			if(ftp_return_code == 200) { ftp_command_login = 1; }
		}
		else
		{
			// 227	Entering Passive Mode (h1,h2,h3,h4,p1,p2).
			if(ftp_return_code == 227) { memcpy(ftp_return_string,rx,sizeof(ftp_return_string)); }

			// 213	File status. (Code Leerzeichen Länge)
			if(ftp_return_code == 213) {
				memcpy(ftp_return_string,rx,sizeof(ftp_return_string)
						);
			//usart2_send(ftp_return_string);
			}
		}
	}
}




//=========================================================================
//==== ftp_write_command_callback
//=========================================================================
long ftp_write_command_callback(	char* tx,
						uint16_t tx_len,
						uint16_t socket_Position)
{
	tx_len = strlen(ftp_send_command);

	if (tx_len > 0)
	{
		memcpy(tx,ftp_send_command,tx_len);

		usart2_send("<=");
		usart2_send(ftp_send_command);
	}
	memset(ftp_send_command,0x00,sizeof(ftp_send_command));
	return tx_len;
}




//=========================================================================
//==== ftp_read_data_callback
//=========================================================================
void ftp_read_data_callback( 	char* rx,
						uint16_t rx_len,
						sockaddr* from,
						uint16_t socket_Position)
{
	bytes_to_write = 0;
	if (rx_len > 0)
	{
		ftp_data_received = 1;
		//usart2_send("=>Daten:\r\n");
		//usart2_send(rx);
		//usart2_send("\r\n");
		bytes_to_write = rx_len;
		ftp_bytes_to_receive = ftp_bytes_to_receive - rx_len;
		memcpy(ftp_data_buffer,rx,rx_len);
	}

}




//=========================================================================
//==== ftp_write_data_callback
//=========================================================================
long ftp_write_data_callback(	char* tx,
								uint16_t tx_len,
								uint16_t socket_Position)
{
	tx_len = 0;
	if (ftp_data_send == 1)
	{
		tx_len = aaa;
		//tx_len = strlen(ftp_data_buffer);
		memcpy(tx,ftp_data_buffer,tx_len);
		ftp_data_send = 0;
	}
	return tx_len;
}


