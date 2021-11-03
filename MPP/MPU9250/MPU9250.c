#include "MPU9250.h"

// Aktuelle Quaternion
mpu9250_quaternion_t quaternion;

// Aktuell gemessene Beschleunigung, Rotation und Magnetfeldrichtung
mpu9250_triple_t acceleration;
mpu9250_triple_t magnetometer;
mpu9250_triple_t rotation;

void mpu9250_rd_sensor(void)
{
	int t = 0;
	char quat[100] ={0};

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
	while(1)
	{
		unsigned char hasmore = 1;
		while(hasmore)
		{
			if (mpu9250_readFifo(&hasmore))
			{	//uwbranging_tick();
				//uwbranging_startRanging(1);
				mpu9250_read_ACC_GYR_MAG(&acceleration, &rotation, &magnetometer);
				quaternion = mpu9250_fuse6DOF(quaternion, 1000, 0.2, &acceleration, &rotation);
				//quaternion = mpu9250_fuse6DOF(quaternion, 1000, 0.01, &acceleration, &rotation);
			}
			else
			{
				break;
			}

		}
		t++;
		if (t>=1000)
		{
			t = 0;
			// ToDo uart2_RX_zeichen = 0x00;
			sprintf(quat,"%f;   %f;  %f;  %f\r\n",quaternion.q0, quaternion.q1, quaternion.q2, quaternion.q3);
			usart2_send(quat);
			//mqtt_pub("hwp1/qu",quat);
		}

	}

}


void reads(void){

	char quat[100] ={0};
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

		wait_mSek(2);
		mpu9250_start();

		while(1)
			{
				wait_mSek(500);
				mpu9250_read_ACC_GYR_MAG(&acceleration, &rotation, &magnetometer);
				sprintf(quat,"%f;   %f;  %f;  \r\n",acceleration.x, acceleration.y, acceleration.z);
				usart2_send(quat);
				sprintf(quat,"%f;   %f;  %f;  \r\n",rotation.x, rotation.y, rotation.z);
				usart2_send(quat);
				sprintf(quat,"%f;   %f;  %f;  \r\n",magnetometer.x, magnetometer.y, magnetometer.z);
				usart2_send(quat);
				usart2_send("===============================================\r\n");
			}
	}
}
