#include "SDCARD.h"


//=========================================================================
// Variablen
//=========================================================================
FATFS 	sd_fat;					// Work area (file system object) for logical drives
FIL 	sd_fileobjekt;			// File objects
TCHAR  	sd_filebuffer[2048];	//File copy buffer
FRESULT sd_resultcode;			//FatFs Function common result code
FILINFO sd_fileinfo;
FRESULT f_readdir (	DIR* dp,      	// [IN] Directory object
					FILINFO* fno  	// [OUT] File information structure
				  );

char	largeFileName[_MAX_LFN];
char 	path[20] = {0};


//=========================================================================
// Funktionen
//=========================================================================

//=========================================================================
void sd_card_init(void)
//=========================================================================
{
	sd_fileinfo.lfname = largeFileName;
	sd_fileinfo.lfsize = (_MAX_LFN + 1) * 2;
    sd_resultcode = f_mount(&sd_fat, "0:", 0);
}



//=========================================================================
void sd_card_SysTick_Handler(void)
//=========================================================================
{
	disk_timerproc();
}



//=========================================================================
int sd_card_copy_file_to_cc3100(char* sd_file_name, char* sf_file_name)
//=========================================================================
{
	int 			sd_bytes_to_read = 0;
	unsigned int 	sd_bytes_rd = 0;
	unsigned int 	sd_bytes_rdwr = 0;
	int				offset = 0;

	_i32			fileHandle 	= -1;
	_u32			Token = 0;
	_i32			sf_resultcode;

	Stop_CC3100_select = true;

	usart2_send("\r\nCopy SD->SF..");

	sd_resultcode = f_open(&sd_fileobjekt,sd_file_name,  FA_OPEN_EXISTING | FA_READ);
	sd_bytes_to_read = f_size(&sd_fileobjekt);
	// TODO filegröße richtig anlegen
	sf_resultcode = sl_FsOpen(	(_u8 *)sf_file_name,
								FS_MODE_OPEN_CREATE(sd_bytes_to_read, 0 ),
								&Token,
								&fileHandle);

	if ((sd_resultcode == FR_OK) && (sf_resultcode == 0))
	{
		offset = 0;
		while (1)
		{
			memset(sd_filebuffer,0x00,sizeof(sd_filebuffer));

			if ( sd_bytes_to_read >= sizeof(sd_filebuffer) ) { sd_bytes_rd = sizeof(sd_filebuffer); }
			else { sd_bytes_rd = sd_bytes_to_read; }

			sd_resultcode = f_read(&sd_fileobjekt, &sd_filebuffer, sd_bytes_rd, &sd_bytes_rdwr);
			// Todo Ende richtig abfangen
			// usart2_send(sd_filebuffer);

			sf_resultcode = sl_FsWrite(fileHandle,offset,(_u8 *)sd_filebuffer, sd_bytes_rdwr);

			sd_bytes_to_read = sd_bytes_to_read - sd_bytes_rdwr;

			if (sd_bytes_to_read <=0) { break; }

			offset = offset + sd_bytes_rdwr;
			sd_resultcode = f_lseek(&sd_fileobjekt,offset);
		}
	}
	sd_resultcode = f_close(&sd_fileobjekt);
	sf_resultcode = sl_FsClose(fileHandle, 0, 0, 0);
	usart2_send("OK!\r\n");
	Stop_CC3100_select = false;

	return 1;
}



//=========================================================================
int sd_card_file_write (char* file_name, char* file_data, int bytes_to_write )
//=========================================================================
{
	unsigned int sd_bytes_wr = 0;

	sd_resultcode = f_open(&sd_fileobjekt, file_name,  FA_WRITE);

	if (sd_resultcode != FR_NOT_READY)
	{

		if (sd_resultcode == FR_NO_FILE)
		{
			sd_resultcode = f_open(&sd_fileobjekt, file_name,  FA_WRITE | FA_CREATE_ALWAYS);
		}

		if (sd_resultcode == FR_OK)
		{
			sd_resultcode = f_lseek(&sd_fileobjekt, f_size(&sd_fileobjekt));
			sd_resultcode = f_write(&sd_fileobjekt, file_data, bytes_to_write, &sd_bytes_wr);
		}

	}
	else
	{
		usart2_printf("SD Card nicht gesteckt\r\n");
	}

	sd_resultcode = f_close(&sd_fileobjekt);
	return sd_resultcode;
}



//=========================================================================
int sd_card_file_read (char* file_name, char* file_data, int bytes_to_read )
//=========================================================================
{
	unsigned int sd_bytes_rd = 0;

	sd_resultcode = f_open(&sd_fileobjekt, file_name,  FA_READ);

	if (sd_resultcode == FR_OK)
	{
		sd_resultcode = f_read(&sd_fileobjekt, &file_data, bytes_to_read , &sd_bytes_rd);
		sd_resultcode = f_close(&sd_fileobjekt);
	}

	return sd_resultcode;
}



//=========================================================================
int sd_card_print_dir (char* sd_path)
//=========================================================================
{
	FRESULT	res;
	FILINFO file_info;
	DIR 	dir;

	int 	i;
	char 	*fn;   /* This function assumes non-Unicode configuration */

	static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */

	file_info.lfname = lfn;
	file_info.lfsize = sizeof lfn;

	res = f_opendir(&dir, sd_path);

	if (res == FR_OK)
	{
		i = strlen(path);
		while(1)
		{
			res = f_readdir(&dir, &file_info);

			if (res != FR_OK || file_info.fname[0] == 0) { break; }

			if (file_info.fname[0] == '.') { continue; }

			fn = *file_info.lfname ? file_info.lfname : file_info.fname;

			if (file_info.fattrib & AM_DIR)
			{
				sprintf(&path[i], "/%s", fn);
				res = sd_card_print_dir(path);
				path[i] = 0;
				if (res != FR_OK) { break; }
			}
			else
			{
				usart2_printf("%s/%s\r\n", path, fn);
			}
		}
		f_closedir(&dir);
	}
	return res;
}



//=========================================================================
int sd_card_print_file (char* sd_file_name)
//=========================================================================
{
	int sd_bytes_to_read = 0;
	unsigned int sd_bytes_rd = 0;
	unsigned int sd_bytes_rdwr = 0;
	int	offset=0;

	// Datei öffnen
	sd_resultcode = f_open(&sd_fileobjekt, sd_file_name,  FA_OPEN_EXISTING | FA_READ);

	// wenn Datei geöffnet dann lesen
	if (sd_resultcode == FR_OK)
	{
		// Größe der Datei abfragen
		sd_bytes_to_read = f_size(&sd_fileobjekt);
		offset = 0;

		// Datei lesen bis zum Ende
		while (1)
		{
			memset(sd_filebuffer,0x00,sizeof(sd_filebuffer));

			if ( sd_bytes_to_read >= sizeof(sd_filebuffer) )
			{
				sd_bytes_rd = sizeof(sd_filebuffer);
			}
			else
			{
				sd_bytes_rd = sd_bytes_to_read;
			}

			sd_resultcode = f_read(&sd_fileobjekt, &sd_filebuffer, sd_bytes_rd, &sd_bytes_rdwr);

			usart2_send(sd_filebuffer);

			sd_bytes_to_read = sd_bytes_to_read - sd_bytes_rdwr;

			if (sd_bytes_to_read <=0) {break;}

			offset = offset + sd_bytes_rdwr;

			sd_resultcode = f_lseek(&sd_fileobjekt,offset);
		}
	}

	sd_resultcode = f_close(&sd_fileobjekt);
	return sd_resultcode;
}
