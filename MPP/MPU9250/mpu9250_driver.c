#include "mpu9250_driver.h"
#include "mpu9250_serial.h"
#include "interrupts.h"
#include "global.h"



// Vorgefertigte Registerinhalte
unsigned char
	mpu9250_register_config = 0,
	mpu9250_register_gyroconfig = 0,
	mpu9250_register_accelconfig = 0,
	mpu9250_register_accelconfig2 = 0,
	mpu9250_register_fifo_en = 0,
	mpu9250_register_powermanagement1 = 0,
	mpu9250_register_powermanagement2 = 0,
	mpu9250_register_userctrl = 0;

unsigned char
	mpu9250_rxbuffer[32] = {0};		// Temporärer Puffer für das Lesen von Daten

MPU9250_FLOAT
	mpu9250_acceldivisor_gplsb = 1,		// g per LSB for Accellerometer
	mpu9250_acceldivisor_mpsplsb = 1,	// m/s² per LSB for accellerometer
	mpu9250_gyrodivisor_rps = 1,		// Gyroeinstellung (RAD pro Sekunde)
	mpu9250_gyrodivisor_dps = 1,		// Gyroeinstellung (Grad pro Sekunde)
	mpu9250_gyrooffset_x_rps = 0,		// X-Offset des Gyroskops
	mpu9250_gyrooffset_y_rps = 0,		// Y-Offset des Gyroskops
	mpu9250_gyrooffset_z_rps = 0;		// Z-Offset des Gyroskops

unsigned long int
	mpu9250_lastmagrefresh = 0;

// Präpariere Quaternionsberechnung
MPU9250_FLOAT
	mpu9250_ax, mpu9250_ay, mpu9250_az,
	mpu9250_gx, mpu9250_gy, mpu9250_gz,
	mpu9250_mx, mpu9250_my, mpu9250_mz,
	mpu9250_temperature;



unsigned char
	mpu9250_usefifo = 0,
	mpu9250_usegyroscope = 0,
	mpu9250_useaccelerometer = 0,
	mpu9250_usetemperaturesensor = 0,
	mpu9250_usemagnetometer = 0,
	mpu9250_isrunning = 0,
	mpu9250_isinitialized = 0;







// Liest die I2C Gerätekennung vom Modul (erwarteter Wert: 0x71)
unsigned char mpu9250_getI2CId() { return mpu9250_receiveUInt8(MPU9250_REGISTER_WHO_AM_I); }

// Gibt den Interrupt Status aus
unsigned char mpu9250_getInterruptStatus() { return mpu9250_receiveUInt8(MPU9250_REGISTER_INT_STATUS); }

// Gibt die Anzahl an Zeilen aus, die im FIFO stehen
unsigned short mpu9250_getFifoCount();

// Setzt den FIFO zurück
void mpu9250_resetFifo();






void delay_ms(unsigned long interval)
{
	interval *= 21;
	interval = interval * 1000;
    while(interval--) { __NOP(); }
}


void delay_us(unsigned long us)
{
	// wartet uSekunden gilt aber nur bei 168MHz Taktfrequenz
	us *= 21;
	while(us--) { __NOP(); }
}





// Commits the configuration
O3 INLINE void mpu9250_commit()
{
	if (mpu9250_isrunning)
	{
		// Slow down the spi
		mpu9250_slowSPI();

		// Deaktiviere alle Sensoren
		mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_2, 0b11111111);

		// Schreibe den Power
		if (mpu9250_usetemperaturesensor) mpu9250_register_powermanagement1 &= ~MPU9250_REGISTER_PWR_MGMT_1_TEMP_DIS;
		else mpu9250_register_powermanagement1 |= MPU9250_REGISTER_PWR_MGMT_1_TEMP_DIS;

		// Lege die Taktung fest
		if (mpu9250_usegyroscope) mpu9250_register_powermanagement1 &= 0b11111110;
		else mpu9250_register_powermanagement1 |= 0b00000001;

		// Schreibe die Konfigurationsregister
		mpu9250_sendUInt8(MPU9250_REGISTER_GYRO_CONFIG, mpu9250_register_gyroconfig);
		mpu9250_sendUInt8(MPU9250_REGISTER_ACCEL_CONFIG, mpu9250_register_accelconfig);
		mpu9250_sendUInt8(MPU9250_REGISTER_ACCEL_CONFIG2, mpu9250_register_accelconfig2);
		mpu9250_sendUInt8(MPU9250_REGISTER_CONFIG, mpu9250_register_config);
		mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_1, mpu9250_register_powermanagement1);
		mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_2, mpu9250_register_powermanagement2);
		mpu9250_sendUInt8(MPU9250_REGISTER_INT_ENABLE, MPU9250_REGISTER_INT_ENABLE_DATA_RDY_EN);

		// Lese den Magnetfeldsensor aus
		if (mpu9250_usemagnetometer)
		{
			int loop;
			for (loop = 0; loop < 10; loop++)
			{
				mpu9250_getInterruptStatus();

				// SPI-Geschwindigkeit beträgt 400kHz, Multimaster aktiviert, Stop beim Wechsel des Slaves
				mpu9250_sendUInt8(MPU9250_REGISTER_I2C_MST_CTRL, MPU9250_REGISTER_I2C_MST_CTRL_MULT_MST_EN | MPU9250_REGISTER_I2C_MST_CTRL_I2C_MST_P_NSR | 13);

				// Schutzzeit
				delay_ms(50); // TODO 50

				// Setze den Magnetfeldsensor zurück
				mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, mpu9250_register_userctrl | MPU9250_REGISTER_USER_CTRL_I2C_MST_RST);

				// Schutzzeit
				delay_ms(50); // TODO 50

				// Reset AK8963
				mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_ADDR, MPU9250_AK8963_I2C_ADDR | MPU9250_AK8963_READ);
				mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_REG, MPU9250_AK8963_WIA);
				mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_CTRL, 0b11000000);

				// Prüfe, ob die Kennung stimmt
				if (mpu9250_receiveUInt8(MPU9250_REGISTER_I2C_SLV4_DI) == 72) break;
			}

			// Zurücksetzen
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_ADDR, MPU9250_AK8963_I2C_ADDR);
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_REG, MPU9250_AK8963_CNTL2);
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_DO, 0x01);
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_CTRL, 0b11000000);
			delay_ms(20);

			// 16Bit continuous
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_REG, MPU9250_AK8963_CNTL1);
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_DO, 0x16);
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV4_CTRL, 0b11000000);
			delay_ms(20);

			// Verwende kontinuierliche Messung mit 16Bit Breite
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV0_ADDR, MPU9250_AK8963_READ | MPU9250_AK8963_I2C_ADDR);
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV0_REG, MPU9250_AK8963_HXL);	// Choose device
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV0_CTRL, 0b10000000 | 7);		// Absenden
		}
		else
		{
			// Aktiviere den I2C-Mastermode
			mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, MPU9250_REGISTER_USER_CTRL_I2C_MST_EN);

			// SPI-Geschwindigkeit beträgt 400kHz
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_MST_CTRL, MPU9250_REGISTER_I2C_MST_CTRL_MULT_MST_EN | 13);

			// Lege 0x0C als I2C Adresse für das Magnetometer fest
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV0_ADDR, MPU9250_AK8963_I2C_ADDR);

			// Schalte den Sensor ab
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV0_REG, MPU9250_AK8963_CNTL1);	// Choose device
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV0_DO, 0x12); 					// Disabled
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV0_CTRL, 0x81);				// Absenden

			// Deaktiviere die SPI
			mpu9250_sendUInt8(MPU9250_REGISTER_I2C_MST_CTRL, 0);	// Power down

			// Deaktiviere den I2C-Mastermode
			mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, 0);
		}

		// Wenn FIFO verwendet wird, setze ihn zurück. Ansonsten deaktiviere ihn
		if (mpu9250_usefifo)
		{
			// Setze den FIFO zurück
			mpu9250_resetFifo();
		}
		else
		{
			// Deaktiviere den FIFO
			mpu9250_sendUInt8(MPU9250_REGISTER_FIFO_EN, 0b00000000);
		}

		// Fasten the spi
		mpu9250_fastSPI();
	}
}

// Initialisiert die MPU
unsigned char mpu9250_init()
{
	// Zurücksetzen
	mpu9250_stop();
	mpu9250_isinitialized = 0;

	// Initialisiere die serielle Schnittstelle
	mpu9250_initserial();

	// Kreiiere eine Hilfsvariable für das SETUP
	//unsigned char setupcontent[18] = {0};

	// Setze alle Register zurück
	mpu9250_register_config = 0;
	mpu9250_register_gyroconfig = 0;
	mpu9250_register_accelconfig = 0;
	mpu9250_register_accelconfig2 = 0;
	mpu9250_register_fifo_en = 0;
	mpu9250_register_powermanagement1 = 0;
	mpu9250_register_powermanagement2 = 0b11111111;
	mpu9250_register_userctrl = 0;			// SPI

	// Lege die Standardkonfiguration fest
	mpu9250_register_config |= 0b01000000;										// When set to ‘1’, when the fifo is full, additional writes will not be written to fifo
	mpu9250_register_config &= MPU9250_REGISTER_CONFIG_EXT_SYNC_SET_MASK;		// FSYNC disabled

	// Auto selects the best available clock source – PLL if ready, else use the Internal oscillato
	mpu9250_register_powermanagement1 = 1;

	// I2C deaktivieren
	mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, MPU9250_REGISTER_USER_CTRL_I2C_IF_DIS);

	// Setze alle Register zurück
	if (mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_1, 0b10000000))
	{
		// Warte 50ms
		delay_ms(50);

		// Wache das Modul wieder auf
		if (mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_1, 0))
		{
			// I2C deaktivieren
			mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL,
					mpu9250_register_userctrl |
					MPU9250_REGISTER_USER_CTRL_SIG_COND_RST |
					MPU9250_REGISTER_USER_CTRL_I2C_FIFO_RST |
					MPU9250_REGISTER_USER_CTRL_I2C_MST_RST);

			// Prüfe die Gerätekennung
			if (mpu9250_getI2CId() == 0x71)
			{
				mpu9250_isinitialized = 1;
				mpu9250_commit();
				return 1;
			}
		}
	}

	// Initialisierung fehlgeschlagen
	return 0;
}

void mpu9250_uninit()
{
	mpu9250_slowSPI();
	mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_2, 0b11111111);
	mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_1, 0b01000000);
	mpu9250_deinitserial();
}

// Startet die Messungen
unsigned char mpu9250_start()
{
	// Prüfe, ob das Modul gestartet werden kann
	if (mpu9250_isinitialized && !mpu9250_isrunning)
	{
		// Das Modul wird gestartet
		mpu9250_isrunning = 1;

		// Takte die SPI herunter
		mpu9250_slowSPI();

		// Setze alle Register zurück
		mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_1, 0b10000000);

		// Schutzzeit
		delay_ms(20);

		// Deaktiviere alle Sensoren
		mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_2, 0b11111111);

		// Divides the internal sample rate (see register CONFIG) to generate the sample rate that controls sensor data output rate, FIFO sample rate.  NOTE: This register is only effective when Fchoice = 2’b11 (fchoice_b register bits are 2’b00), and (0 < dlpf_cfg < 7), such that the average filter’s output is selected (see chart below)
		// Data should be sampled at or above sample rate; SMPLRT_DIV is only used for1kHz internal sampling
		mpu9250_sendUInt8(MPU9250_REGISTER_SMPLRT_DIV, 0b00000000);

		// Interrupt status is cleared if any read operation is performed
		mpu9250_sendUInt8(MPU9250_REGISTER_INT_PIN_CFG, 0/*MPU9250_REGISTER_INT_PIN_CFGINT_ANYRD_2CLEAR*/);

		// Schreibe die restlichen Änderungen
		mpu9250_commit();

		// Beschleunige die SPI
		mpu9250_fastSPI();

		// Erfolgreich abgeschlossen
		return 1;
	}
	return 0;
}

// Beendet die Messungen
unsigned char mpu9250_stop()
{
	// Prüfe, ob das Modul gestartet werden kann
	if (mpu9250_isinitialized && mpu9250_isrunning)
	{
		// Das Modul wurde gestoppt
		mpu9250_isrunning = 0;

		// Takte die SPI herunter
		mpu9250_slowSPI();

		// Deaktiviere alle Sensoren
		mpu9250_sendUInt8(MPU9250_REGISTER_PWR_MGMT_2, 0b11111111);
	}
	return 0;
}

// Initialisiert das Gyroskop
unsigned char mpu9250_enable_gyroscope(mpu9250_gyro_rate_e rate, mpu9250_gyro_sensitivity_e sensitivity)
{
	// Ist das Modul nicht korrekt initialisiert, dann breche ab
	if (!mpu9250_isinitialized) return 0;

	// Gyroskop wird verwendet
	mpu9250_usegyroscope = 1;

	// Lege die Einstellung des digitalen Tiefpassfilters fest
	mpu9250_register_config = (mpu9250_register_config & MPU9250_REGISTER_CONFIG_DLPF_CFG_MASK) | (rate & ~MPU9250_REGISTER_CONFIG_DLPF_CFG_MASK);

	// Lege den FCHOISE-Wert fest
	switch(rate)
	{
		case MPU9250_GYRO_32KHZ_FILTER_8000HZ: mpu9250_register_gyroconfig = (mpu9250_register_gyroconfig & MPU9250_REGISTER_GYRO_CONFIG_GYRO_FCOISE_B_MASK) | 0b00000011; break;
		case MPU9250_GYRO_32KHZ_FILTER_3600HZ: mpu9250_register_gyroconfig = (mpu9250_register_gyroconfig & MPU9250_REGISTER_GYRO_CONFIG_GYRO_FCOISE_B_MASK) | 0b00000010; break;
		default: mpu9250_register_gyroconfig = (mpu9250_register_gyroconfig & MPU9250_REGISTER_GYRO_CONFIG_GYRO_FCOISE_B_MASK) | 0b00000000; break;
	}

	// Lege die Empfindlichkeit (Grad pro Sekunde) fest
	switch(sensitivity)
	{
		case MPU9250_GYRO_250_DPS: mpu9250_gyrodivisor_dps = 131; break;
		case MPU9250_GYRO_500_DPS: mpu9250_gyrodivisor_dps = 65.5; break;
		case MPU9250_GYRO_1000_DPS: mpu9250_gyrodivisor_dps = 32.8; break;
		case MPU9250_GYRO_2000_DPS: mpu9250_gyrodivisor_dps = 16.4; break;
	}

	// Lege die Radians pro Sekunde fest
	mpu9250_gyrodivisor_rps = mpu9250_gyrodivisor_dps * MPU9250_DEGTORAD;

	// Lege den Skalierungsfaktor fest
	mpu9250_register_gyroconfig = (mpu9250_register_gyroconfig & MPU9250_REGISTER_GYRO_CONFIG_GYRO_FS_SEL_MASK) | (sensitivity & ~MPU9250_REGISTER_GYRO_CONFIG_GYRO_FS_SEL_MASK);

	// Bereite das Speichern in den FIFO vor
	mpu9250_register_fifo_en |= MPU9250_REGISTER_FIFO_EN_GYRO;

	// Aktiviere das Gyroskop
	mpu9250_register_powermanagement2 &= 0b11111000;

	// Schreibe die Änderung
	mpu9250_commit();

	// Erfolgreich abgeschlossen
	return 1;
}

// Deakiviere das Gyroskop
unsigned char mpu9250_disable_gyroscope()
{
	if (mpu9250_usegyroscope)
	{
		// Gyroskop wird nicht länger verwendet
		mpu9250_usegyroscope = 0;

		// Bereite das Speichern in den FIFO vor
		mpu9250_register_fifo_en &= ~MPU9250_REGISTER_FIFO_EN_GYRO;

		// Deaktiviere das Gyroskop
		mpu9250_register_powermanagement2 |= 0b00000111;

		// Schreibe die Änderung
		mpu9250_commit();

		// Fertig
		return 1;
	}

	// Keine Änderung
	return 0;
}


// Richtet den Beschleunigungssensor ein
unsigned char mpu9250_enable_accelerometer(mpu9250_accel_rate_e rate, mpu9250_accel_sensitivity_e sensitivity)
{
	// Ist das Modul nicht korrekt initialisiert, dann breche ab
	if (!mpu9250_isinitialized) return 0;

	// Beschleunigungssensor wird verwendet
	mpu9250_useaccelerometer = 1;

	// Lege den FCHOISE-Wert fest
	switch(rate)
	{
		case MPU9250_ACCEL_4KHZ_FILTER_1300HZ:
			mpu9250_register_accelconfig2 = mpu9250_register_accelconfig2 & MPU9250_REGISTER_ACCEL_CONFIG2_ADLPFCFG_MASK;
			mpu9250_register_accelconfig2 = mpu9250_register_accelconfig2 | MPU9250_REGISTER_ACCEL_CONFIG2_FCHOISE_B;
			break;
		default:
			mpu9250_register_accelconfig2 = (mpu9250_register_accelconfig2 & MPU9250_REGISTER_ACCEL_CONFIG2_ADLPFCFG_MASK) | (rate & ~MPU9250_REGISTER_ACCEL_CONFIG2_ADLPFCFG_MASK);
			mpu9250_register_accelconfig2 = mpu9250_register_accelconfig2 & ~MPU9250_REGISTER_ACCEL_CONFIG2_FCHOISE_B;
			break;
	}

	// Lege die Empfindlichkeit in g/LSB fest
	switch(sensitivity)
	{
		case MPU9250_ACCEL_2G:
			mpu9250_acceldivisor_gplsb = 2 * 8192;
			break;
		case MPU9250_ACCEL_4G:
			mpu9250_acceldivisor_gplsb = 2 * 4096;
			break;
		case MPU9250_ACCEL_8G:
			mpu9250_acceldivisor_gplsb = 2 * 2048;
			break;
		default:
			mpu9250_acceldivisor_gplsb = 2 * 1024;
			break;
	}

	// Berechne die Empfindlichkeit in m/s² pro LSB
	mpu9250_acceldivisor_mpsplsb = (MPU9250_FLOAT)9.81 / mpu9250_acceldivisor_gplsb;

	// Setze die Skalierung fest
	mpu9250_register_accelconfig = (mpu9250_register_accelconfig & MPU9250_REGISTER_ACCEL_CONFIG_ACCEL_FS_SEL_MASK) | (sensitivity & ~MPU9250_REGISTER_ACCEL_CONFIG_ACCEL_FS_SEL_MASK);

	// Bereite das Speichern in den FIFO vor
	mpu9250_register_fifo_en |= MPU9250_REGISTER_FIFO_EN_ACCEL;

	// Deaktiviere den Beschleunigungssensor
	mpu9250_register_powermanagement2 &= 0b11000111;

	// Schreibe die Änderung
	mpu9250_commit();

	// Erfolgreich abgeschlossen
	return 1;
}

// Deakiviere den Beschleunigungssensor
unsigned char mpu9250_disable_accelerometer()
{
	if (mpu9250_useaccelerometer)
	{
		// Beschleunigungssensor wird nicht länger verwendet
		mpu9250_useaccelerometer = 0;

		// Bereite das Speichern in den FIFO vor
		mpu9250_register_fifo_en &= ~MPU9250_REGISTER_FIFO_EN_ACCEL;

		// Deaktiviere den Beschleunigungssensor
		mpu9250_register_powermanagement2 |= 0b00111000;

		// Schreibe die Änderung
		mpu9250_commit();

		// Fertig
		return 1;
	}

	// Keine Änderung
	return 0;
}

// Diese Methode aktiviert den Temperatursensor
unsigned char mpu9250_enable_temperaturesensor()
{
	if (!mpu9250_usetemperaturesensor)
	{
		// Temperatursensor wird verwendet
		mpu9250_usetemperaturesensor = 1;

		// Bereite das Speichern in den FIFO vor
		mpu9250_register_fifo_en |= MPU9250_REGISTER_FIFO_EN_TEMP;

		// Aktiviere die Temperaturmessung
		mpu9250_register_powermanagement1 &= ~MPU9250_REGISTER_PWR_MGMT_1_TEMP_DIS;

		// Schreibe die Konfiguration
		mpu9250_commit();

		// Fertig
		return 1;
	}
	return 0;
}

// Diese Methode deaktiviert den Temperatursensor
unsigned char mpu9250_disable_temperaturesensor()
{
	if (mpu9250_usetemperaturesensor)
	{
		// Temperatursensor wird nicht länger verwendet
		mpu9250_usetemperaturesensor = 0;

		// Bereite das Speichern in den FIFO vor
		mpu9250_register_fifo_en &= ~MPU9250_REGISTER_FIFO_EN_TEMP;

		// Deaktiviere die Temperaturmessung
		mpu9250_register_powermanagement1 |= ~MPU9250_REGISTER_PWR_MGMT_1_TEMP_DIS;

		// Schreibe die Konfiguration
		mpu9250_commit();

		// Fertig
		return 1;
	}
	return 0;
}

// Diese Methode aktiviert den Digitalkompass des Moduls
unsigned char mpu9250_enable_magnetometer()
{
	// Ist das Modul nicht korrekt initialisiert, dann breche ab
	if (!mpu9250_isinitialized) return 0;

	// Der Magnetfeldsensor wird verwendet
	mpu9250_usemagnetometer = 1;

	// Deaktiviere den I2C-Master
	mpu9250_register_userctrl |= MPU9250_REGISTER_USER_CTRL_I2C_MST_EN;

	// Schreibe die Änderungen
	mpu9250_commit();

	// Abgeschlossen
	return 1;
}

// Diese Methode deaktiviert den Digitalkompass des Moduls
unsigned char mpu9250_disable_magnetometer()
{
	if (mpu9250_usemagnetometer)
	{
		// Der Kompass wird nicht länger verwendet
		mpu9250_usemagnetometer = 0;

		// Deaktiviere den I2C-Master
		mpu9250_register_userctrl &= (~MPU9250_REGISTER_USER_CTRL_I2C_MST_EN);

		// Schreibe die Änderungen
		mpu9250_commit();

		// Abgeschlossen
		return 1;
	}

	// Keine Aktion
	return 0;
}

// Diese Methode muss aufgerufen werden, wenn der FIFO verwendet werden soll
unsigned char mpu9250_enable_fifo()
{
	// Ist das Modul nicht korrekt initialisiert, dann breche ab
	if (!mpu9250_isinitialized) return 0;

	// Prüfe, ob sich etwas geändert hat
	if (!mpu9250_usefifo)
	{
		// Der FIFO wird verwendet
		mpu9250_usefifo = 1;

		// Aktiviere den FIFO
		mpu9250_register_userctrl |= MPU9250_REGISTER_USER_CTRL_FIFO_EN;

		// Schreibe alle Einstellungen
		mpu9250_commit();

		// Erledigt
		return 1;
	}

	// Keine Änderungen
	return 1;
}

// Diese Methode deaktiviert den FIFO beim Auslesen der Messwerte
unsigned char mpu9250_disable_fifo()
{
	// Ist das Modul nicht korrekt initialisiert, dann breche ab
	if (!mpu9250_isinitialized) return 0;

	// Prüfe, ob sich etwas geändert hat
	if (mpu9250_usefifo)
	{
		// Der FIFO wird verwendet
		mpu9250_usefifo = 0;

		// Deaktiviere den FIFO
		mpu9250_register_userctrl &= (~MPU9250_REGISTER_USER_CTRL_FIFO_EN);

		// Schreibe alle Einstellungen
		mpu9250_commit();

		// Erledigt
		return 1;
	}

	// Keine Änderungen
	return 1;
}

// Gibt die Anzahl an Zeilen aus, die im FIFO stehen
unsigned short mpu9250_getFifoCount()
{
	unsigned char tmp[2];
	mpu9250_receive(MPU9250_REGISTER_FIFO_COUNT_H, tmp, 2);
	return (((unsigned short)tmp[0]) << 8) | tmp[1];
}




// Setzt den FIFO zurück
void mpu9250_resetFifo()
{
	printf("RESET FIFO\r\n");

	// Setze die Baudrate auf 48MHz/32 - 1.5MHz
	mpu9250_slowSPI();

	// Deaktiviere den FIFO
	mpu9250_sendUInt8(MPU9250_REGISTER_FIFO_EN, 0);

	// Deaktiviere den FIFO und setze ihn zurück
	//mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, (mpu9250_register_userctrl & (~MPU9250_REGISTER_USER_CTRL_FIFO_EN)) | MPU9250_REGISTER_USER_CTRL_I2C_FIFO_RST);
	//mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, mpu9250_register_userctrl);

	mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, mpu9250_register_userctrl & (~MPU9250_REGISTER_USER_CTRL_FIFO_EN));
	mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, (mpu9250_register_userctrl & (~MPU9250_REGISTER_USER_CTRL_FIFO_EN)) | MPU9250_REGISTER_USER_CTRL_I2C_FIFO_RST);
	mpu9250_sendUInt8(MPU9250_REGISTER_USER_CTRL, mpu9250_register_userctrl);

	// Schutzzeit
	delay_ms(20);

	// Lege fest, welche Sensoren in den FIFO schreiben sollen
	mpu9250_sendUInt8(MPU9250_REGISTER_FIFO_EN, mpu9250_register_fifo_en);

	// Setze die Baudrate zurück
	mpu9250_fastSPI();
}





// *******************************************************************************************
// Auslesen der Sensoren
// *******************************************************************************************

// Liest den FIFO aus
INLINE O3 unsigned char mpu9250_readFifo(unsigned char* hasmore)
{
	if (mpu9250_usefifo)
	{
		// Prüfe zunächst, welche Werte im Fifo stehen
		volatile unsigned int length = 0;
		if (mpu9250_register_fifo_en & MPU9250_REGISTER_FIFO_EN_TEMP) length += 2;
		if (mpu9250_register_fifo_en & MPU9250_REGISTER_FIFO_EN_GYRO) length += 6;
		if (mpu9250_register_fifo_en & MPU9250_REGISTER_FIFO_EN_ACCEL) length += 6;

		// Prüfe, ob genügend Werte im Buffer liegen
		mpu9250_receive(MPU9250_REGISTER_FIFO_COUNT_H, (unsigned char*)mpu9250_rxbuffer, 2);
		volatile unsigned int availablelength = (((unsigned short)mpu9250_rxbuffer[0]) << 8) | mpu9250_rxbuffer[1];

		// Prüfe, ob ein Pufferüberlauf anliegt
		if (availablelength >= 512)
		{
			// Setze den FIFO zurück
			mpu9250_resetFifo();
			if (hasmore) *hasmore = 0;
			return 0;
		}

		// Prüfe, ob genügend Daten im FIFO liegen
		else if (availablelength >= length)
		{
			// Lese den Puffer aus
			mpu9250_receive(MPU9250_REGISTER_FIFO_R_W, (unsigned char*)mpu9250_rxbuffer, length);

			// Bereite den Ausgabeindex vor
			//volatile unsigned char valueindex = 0;
			volatile unsigned char readvalueindex = 0;

			// Prüfe, ob der Beschleunigungssensor gelesen werden soll
			if (mpu9250_register_fifo_en & MPU9250_REGISTER_FIFO_EN_ACCEL)
			{
				// Lese die Messdaten ein
				mpu9250_ax = (short)((mpu9250_rxbuffer[readvalueindex] << 8) | mpu9250_rxbuffer[readvalueindex + 1]) * mpu9250_acceldivisor_mpsplsb; readvalueindex += 2;
				mpu9250_ay = (short)((mpu9250_rxbuffer[readvalueindex] << 8) | mpu9250_rxbuffer[readvalueindex + 1]) * mpu9250_acceldivisor_mpsplsb; readvalueindex += 2;
				mpu9250_az = (short)((mpu9250_rxbuffer[readvalueindex] << 8) | mpu9250_rxbuffer[readvalueindex + 1]) * mpu9250_acceldivisor_mpsplsb; readvalueindex += 2;
			}

			// Prüfe, ob die Temperatur gelesen werden soll
			if (mpu9250_register_fifo_en & MPU9250_REGISTER_FIFO_EN_TEMP)
			{
				// Lese die Messdaten aus
				mpu9250_temperature = (short)((mpu9250_rxbuffer[readvalueindex] << 8) | mpu9250_rxbuffer[readvalueindex + 1]) / 340.0 + 36.53; readvalueindex += 2;
			}

			// Prüfe, ob der Beschleunigungssensor gelesen werden soll
			if (mpu9250_register_fifo_en & MPU9250_REGISTER_FIFO_EN_GYRO)
			{
				// Lese die Messdaten ein
				mpu9250_gx = (short)((mpu9250_rxbuffer[readvalueindex] << 8) | mpu9250_rxbuffer[readvalueindex + 1]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_x_rps; readvalueindex += 2;
				mpu9250_gy = (short)((mpu9250_rxbuffer[readvalueindex] << 8) | mpu9250_rxbuffer[readvalueindex + 1]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_y_rps; readvalueindex += 2;
				mpu9250_gz = (short)((mpu9250_rxbuffer[readvalueindex] << 8) | mpu9250_rxbuffer[readvalueindex + 1]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_z_rps; readvalueindex += 2;
			}

			if (hasmore) *hasmore = (availablelength > length);
			return 1;
		}
	}
	// Nicht genügend daten
	if (hasmore) *hasmore = 0;
	return 0;
}

// Liest den Beschleunigungssensor aus
O3 INLINE void mpu9250_read_ACC(mpu9250_triple_t* accel)
{
	if (mpu9250_usefifo)
	{
		accel->x = mpu9250_ax;
		accel->y = mpu9250_ay;
		accel->z = mpu9250_az;
	}
	else
	{
		// Lese die Messwerte direkt von den Sensoren
		mpu9250_receive(MPU9250_REGISTER_ACCEL_XOUT_H, mpu9250_rxbuffer, 6);
		accel->x = (short)((mpu9250_rxbuffer[0] << 8) | mpu9250_rxbuffer[1]) * mpu9250_acceldivisor_mpsplsb;
		accel->y = (short)((mpu9250_rxbuffer[2] << 8) | mpu9250_rxbuffer[3]) * mpu9250_acceldivisor_mpsplsb;
		accel->z = (short)((mpu9250_rxbuffer[4] << 8) | mpu9250_rxbuffer[5]) * mpu9250_acceldivisor_mpsplsb;
	}
}

// Liest das Gyroskop aus
O3 INLINE void mpu9250_read_GYR(mpu9250_triple_t* gyro)
{
	if (mpu9250_usefifo)
	{
		gyro->x = mpu9250_gx;
		gyro->y = mpu9250_gy;
		gyro->z = mpu9250_gz;
	}
	else
	{
		// Lese die Messwerte direkt von den Sensoren
		mpu9250_receive(MPU9250_REGISTER_GYRO_XOUT_H, mpu9250_rxbuffer, 6);
		gyro->x = (short)((mpu9250_rxbuffer[0] << 8) | mpu9250_rxbuffer[1]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_x_rps;
		gyro->y = (short)((mpu9250_rxbuffer[2] << 8) | mpu9250_rxbuffer[3]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_y_rps;
		gyro->z = (short)((mpu9250_rxbuffer[4] << 8) | mpu9250_rxbuffer[5]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_z_rps;
	}
}

// Liest den Magnetfeldsensor aus
O3 INLINE void mpu9250_read_MAG(mpu9250_triple_t* compass)
{
	// Prüfe, ob der 100Hz-Takt eingehalten wird
	unsigned long int systick = systickcounter;
	if (systick < mpu9250_lastmagrefresh) mpu9250_lastmagrefresh = systick;
	else if (systick - mpu9250_lastmagrefresh >= 10)
	{
		// Speichere den neuen Zeitpunkt
		mpu9250_lastmagrefresh = systick;

		// Lese alle Register aus
		mpu9250_receive(MPU9250_REGISTER_EXT_SENS_DATA_00, mpu9250_rxbuffer, 6);
		// Verwende kontinuierliche Messung mit 16Bit Breite
		mpu9250_sendUInt8(MPU9250_REGISTER_I2C_SLV0_CTRL, 0b10000000 | 7);		// Absenden



		mpu9250_mx = (short)((mpu9250_rxbuffer[1] << 8) | mpu9250_rxbuffer[0]);
		mpu9250_my = (short)((mpu9250_rxbuffer[3] << 8) | mpu9250_rxbuffer[2]);
		mpu9250_mz = (short)((mpu9250_rxbuffer[5] << 8) | mpu9250_rxbuffer[4]);
	}

	// Lese die Messdaten ein
	compass->x = mpu9250_mx;
	compass->y = mpu9250_my;
	compass->z = mpu9250_mz;
}

// Liest den Beschleunigungssensor und das Gyroskop aus
O3 INLINE void mpu9250_read_ACC_GYR(mpu9250_triple_t* accel, mpu9250_triple_t* gyro)
{
	if (mpu9250_usefifo)
	{
		// Gebe die zuletzt gemessenen Werte aus
		accel->x = mpu9250_ax;
		accel->y = mpu9250_ay;
		accel->z = mpu9250_az;
		gyro->x = mpu9250_gx;
		gyro->y = mpu9250_gy;
		gyro->z = mpu9250_gz;
	}
	else
	{
		// Lese die Messwerte direkt von den Sensoren
		mpu9250_receive(MPU9250_REGISTER_ACCEL_XOUT_H, mpu9250_rxbuffer, 14);
		accel->x = (short)((mpu9250_rxbuffer[0] << 8) | mpu9250_rxbuffer[1]) * mpu9250_acceldivisor_mpsplsb;
		accel->y = (short)((mpu9250_rxbuffer[2] << 8) | mpu9250_rxbuffer[3]) * mpu9250_acceldivisor_mpsplsb;
		accel->z = (short)((mpu9250_rxbuffer[4] << 8) | mpu9250_rxbuffer[5]) * mpu9250_acceldivisor_mpsplsb;
		gyro->x = (short)((mpu9250_rxbuffer[8] << 8) | mpu9250_rxbuffer[9]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_x_rps;
		gyro->y = (short)((mpu9250_rxbuffer[10] << 8) | mpu9250_rxbuffer[11]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_y_rps;
		gyro->z = (short)((mpu9250_rxbuffer[12] << 8) | mpu9250_rxbuffer[13]) / mpu9250_gyrodivisor_rps - mpu9250_gyrooffset_z_rps;
	}
}

// Liest den Beschleunigungssensor und den Magnetfeldsensor aus
O3 INLINE void mpu9250_read_ACC_MAG(mpu9250_triple_t* accel, mpu9250_triple_t* compass)
{
	if (mpu9250_usefifo)
	{
		accel->x = mpu9250_ax;
		accel->y = mpu9250_ay;
		accel->z = mpu9250_az;
		mpu9250_read_MAG(compass);
	}
	else
	{
		mpu9250_read_MAG(compass);
		mpu9250_read_ACC(accel);
	}
}

// Liest das Gyroskop und den Magnetfeldsensor aus
O3 INLINE void mpu9250_read_GYR_MAG(mpu9250_triple_t* gyro, mpu9250_triple_t* compass)
{
	if (mpu9250_usefifo)
	{
		gyro->x = mpu9250_gx;
		gyro->y = mpu9250_gy;
		gyro->z = mpu9250_gz;
		mpu9250_read_MAG(compass);
	}
	else
	{
		mpu9250_read_MAG(compass);
		mpu9250_read_GYR(gyro);
	}
}

// Liest den Beschleunigungssensor, das Gyroskop und den Magnetfeldsensor aus
O3 INLINE void mpu9250_read_ACC_GYR_MAG(mpu9250_triple_t* accel, mpu9250_triple_t* gyro, mpu9250_triple_t* compass)
{
	if (mpu9250_usefifo)
	{
		accel->x = mpu9250_ax;
		accel->y = mpu9250_ay;
		accel->z = mpu9250_az;
		gyro->x = mpu9250_gx;
		gyro->y = mpu9250_gy;
		gyro->z = mpu9250_gz;
		mpu9250_read_MAG(compass);
	}
	else
	{
		mpu9250_read_ACC_GYR(accel, gyro);
		mpu9250_read_MAG(compass);
	}
}





unsigned char mpu9250_measuregyrooffsetandnoise(mpu9250_triple_t* offset, mpu9250_triple_t* noise)
{
return 0;
}
