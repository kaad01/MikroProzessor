#include "mpu9250_driver.h"
#include "mpu9250_def.h"
#include "mpu9250_serial.h"
#include "global.h"


// Nähert die inverse Wurzel aus x
O3 INLINE MPU9250_FLOAT mpu9250_invSqrt(MPU9250_FLOAT x)
{
//	MPU9250_FLOAT halfx = (MPU9250_FLOAT)0.5 * x;
//	MPU9250_FLOAT y = x;
//	long i = *(long*)&y;
//	i = (MPU9250_FLOAT)0x5f3759d - (i>>1);
//	y = *(MPU9250_FLOAT*)&i;
//	y = y * ((MPU9250_FLOAT)1.5 - (halfx * y * y));
//	return y;
	return 1/sqrt(x);
}



// *******************************************************************************************
// Sensorfustion
// *******************************************************************************************

// Fusioniert die gegebenenen 6 Sensormesswerte
O3 INLINE mpu9250_quaternion_t mpu9250_fuse6DOF(mpu9250_quaternion_t oldquaternion, unsigned int frequency, MPU9250_FLOAT correctionvalue, mpu9250_triple_t* acceldata, mpu9250_triple_t* gyrodata)
{
	// Bereite das Lesen der Daten vor
	MPU9250_FLOAT
		gx = 0, gy = 0, gz = 0,
		recipNorm = 1,
		q0 = oldquaternion.q0,
		q1 = oldquaternion.q1,
		q2 = oldquaternion.q2,
		q3 = oldquaternion.q3;

	// Lese die Gyroskopdaten ein
	if (gyrodata)
	{
		gx = gyrodata->x;
		gy = gyrodata->y;
		gz = gyrodata->z;
	}

	// Rate of change of quaternion from gyroscope
	MPU9250_FLOAT qDot1 = (MPU9250_FLOAT)0.5 * (-q1 * gx - q2 * gy - q3 * gz);
	MPU9250_FLOAT qDot2 = (MPU9250_FLOAT)0.5 * (q0 * gx + q2 * gz - q3 * gy);
	MPU9250_FLOAT qDot3 = (MPU9250_FLOAT)0.5 * (q0 * gy - q1 * gz + q3 * gx);
	MPU9250_FLOAT qDot4 = (MPU9250_FLOAT)0.5 * (q0 * gz + q1 * gy - q2 * gx);

	// Prüfe, ob Beschleunigungswerte vorliegen
	if (acceldata)
	{
		MPU9250_FLOAT
			ax = acceldata->x,
			ay = acceldata->y,
			az = acceldata->z;

		// Normalise accelerometer measurement
		recipNorm = mpu9250_invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		MPU9250_FLOAT _2q0 = (MPU9250_FLOAT)2.0 * q0;
		MPU9250_FLOAT _2q1 = (MPU9250_FLOAT)2.0 * q1;
		MPU9250_FLOAT _2q2 = (MPU9250_FLOAT)2.0 * q2;
		MPU9250_FLOAT _2q3 = (MPU9250_FLOAT)2.0 * q3;
		MPU9250_FLOAT _4q0 = (MPU9250_FLOAT)4.0 * q0;
		MPU9250_FLOAT _4q1 = (MPU9250_FLOAT)4.0 * q1;
		MPU9250_FLOAT _4q2 = (MPU9250_FLOAT)4.0 * q2;
		MPU9250_FLOAT _8q1 = (MPU9250_FLOAT)8.0 * q1;
		MPU9250_FLOAT _8q2 = (MPU9250_FLOAT)8.0 * q2;
		MPU9250_FLOAT q0q0 = q0 * q0;
		MPU9250_FLOAT q1q1 = q1 * q1;
		MPU9250_FLOAT q2q2 = q2 * q2;
		MPU9250_FLOAT q3q3 = q3 * q3;

		// Gradient decent algorithm corrective step
		MPU9250_FLOAT s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		MPU9250_FLOAT s1 = _4q1 * q3q3 - _2q3 * ax + (MPU9250_FLOAT)4.0 * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		MPU9250_FLOAT s2 = (MPU9250_FLOAT)4.0 * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		MPU9250_FLOAT s3 = (MPU9250_FLOAT)4.0 * q1q1 * q3 - _2q1 * ax + (MPU9250_FLOAT)4.0 * q2q2 * q3 - _2q2 * ay;

		// normalise step magnitude
		recipNorm = mpu9250_invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);

		// Apply feedback step
		qDot1 -= correctionvalue * s0 * recipNorm;
		qDot2 -= correctionvalue * s1 * recipNorm;
		qDot3 -= correctionvalue * s2 * recipNorm;
		qDot4 -= correctionvalue * s3 * recipNorm;
	}

	// Integrate rate of change of quaternion to yield quaternion
	MPU9250_FLOAT duration = 1 / (MPU9250_FLOAT)frequency;
	q0 += qDot1 * duration;
	q1 += qDot2 * duration;
	q2 += qDot3 * duration;
	q3 += qDot4 * duration;

	// Quaternion normalisieren
	recipNorm = mpu9250_invSqrt(q0*q0+q1*q1+q2*q2+q3*q3);

	// Erstelle die Ausgabequaternion
	mpu9250_quaternion_t output;
	output.q0 = q0 * recipNorm;
	output.q1 = q1 * recipNorm;
	output.q2 = q2 * recipNorm;
	output.q3 = q3 * recipNorm;
	return output;
}

// Fusioniert die gegebenenen 9 Sensormesswerte
O3 INLINE mpu9250_quaternion_t mpu9250_fuse9DOF(mpu9250_quaternion_t oldquaternion, unsigned int frequency, MPU9250_FLOAT correctionvalue, mpu9250_triple_t* acceldata, mpu9250_triple_t* gyrodata, mpu9250_triple_t* magdata)
{
	mpu9250_quaternion_t output = mpu9250_createQuaternionNumber();
	return output;
}



// *******************************************************************************************
// Operationen auf Quaternionen
// *******************************************************************************************


// Erstellt eine neue Quaternion q=(0,0,0,1)
O3 INLINE mpu9250_quaternion_t mpu9250_createQuaternionNumber()
{
	volatile mpu9250_quaternion_t output;
	output.q0 = 1;
	output.q1 = 0;
	output.q2 = 0;
	output.q3 = 0;
	return output;
}

// Normalisiert die gegebene Quaternion a und gibt das Ergebnis aus
O3 INLINE mpu9250_quaternion_t mpu9250_normalizeQuaternionNumber(mpu9250_quaternion_t a)
{
	volatile double length = sqrt(a.q0*a.q0 + a.q1*a.q1 + a.q2*a.q2 + a.q3*a.q3);
	if (length != 0)
	{
		volatile mpu9250_quaternion_t output;
		output.q0 = a.q0 / length;
		output.q1 = a.q1 / length;
		output.q2 = a.q2 / length;
		output.q3 = a.q3 / length;
		return output;
	}
	return a;
}

// Normalisiert die gegebene Quaternion a und gibt das Ergebnis aus
O3 INLINE mpu9250_quaternion_t mpu9250_normalizeQuaternionNumberFast(mpu9250_quaternion_t a)
{
	volatile double recipNorm = mpu9250_invSqrt(a.q0*a.q0 + a.q1*a.q1 + a.q2*a.q2 + a.q3*a.q3);
	if (recipNorm != 1)
	{
		volatile mpu9250_quaternion_t output;
		output.q0 = a.q0 * recipNorm;
		output.q1 = a.q1 * recipNorm;
		output.q2 = a.q2 * recipNorm;
		output.q3 = a.q3 * recipNorm;
		return output;
	}
	return a;
}

// Konjugiert die gegebene Quaternion a und gibt das Ergebnis aus
O3 INLINE mpu9250_quaternion_t mpu9250_conjugateQuaternionNumber(mpu9250_quaternion_t a)
{
	volatile mpu9250_quaternion_t output;
	output.q0 = a.q0;
	output.q1 = -a.q1;
	output.q2 = -a.q2;
	output.q3 = -a.q3;
	return output;
}

// Invertiert die gegebene Quaternion a und gibt das Ergebnis aus
O3 INLINE mpu9250_quaternion_t mpu9250_invertQuaternionNumber(mpu9250_quaternion_t a)
{
	return mpu9250_normalizeQuaternionNumberFast(mpu9250_conjugateQuaternionNumber(mpu9250_normalizeQuaternionNumberFast(a)));
}

// Addiert die Quaternionen a und b und gibt das Ergebnis aus
O3 INLINE mpu9250_quaternion_t mpu9250_addQuaternionNumbers(mpu9250_quaternion_t a, mpu9250_quaternion_t b)
{
	volatile mpu9250_quaternion_t output;
	output.q0 = a.q0 + b.q0;
	output.q1 = a.q1 + b.q1;
	output.q2 = a.q2 + b.q2;
	output.q3 = a.q3 + b.q3;
	return output;
}

// Subtrahiert die Quaternion b von a und gibt das Ergebnis aus
O3 INLINE mpu9250_quaternion_t mpu9250_subQuaternionNumbers(mpu9250_quaternion_t a, mpu9250_quaternion_t b)
{
	volatile mpu9250_quaternion_t output;
	output.q0 = a.q0 - b.q0;
	output.q1 = a.q1 - b.q1;
	output.q2 = a.q2 - b.q2;
	output.q3 = a.q3 - b.q3;
	return output;
}

// Multipliziert die Quaternion a und b und gibt das Ergebnis aus
O3 INLINE mpu9250_quaternion_t mpu9250_multiplyQuaternionNumbers(mpu9250_quaternion_t a, mpu9250_quaternion_t b)
{
	volatile mpu9250_quaternion_t output;
	output.q0 = a.q0*b.q0 - a.q1*b.q1 - a.q2*b.q2 - a.q3*b.q3;
	output.q1 = a.q0*b.q1 + a.q1*b.q0 + a.q2*b.q3 - a.q3*b.q2;
	output.q2 = a.q0*b.q2 - a.q1*b.q3 + a.q2*b.q0 + a.q3*b.q1;
	output.q3 = a.q0*b.q3 + a.q1*b.q2 - a.q2*b.q1 + a.q3*b.q0;
	return output;
}


// *******************************************************************************************
// Umwandlung von Sensordaten
// *******************************************************************************************

// Projeziert den gegebenen Sensorwert (in Weltkoordinaten) anhand der Drehquaternion in Sensorkoordinaten
O3 INLINE mpu9250_triple_t mpu9250_convertWorldToSensor(mpu9250_triple_t worlddata, mpu9250_quaternion_t quaternion)
{
	quaternion = mpu9250_invertQuaternionNumber(quaternion);
	return mpu9250_convertWorldToSensorRaw(worlddata, quaternion, mpu9250_conjugateQuaternionNumber(quaternion));
}

// Projeziert den gegebenen Sensorwert (in Sensorkoordinaten) anhand der Drehquaternion in Weltkoordinaten
O3 INLINE mpu9250_triple_t mpu9250_convertSensorToWorld(mpu9250_triple_t sensordata, mpu9250_quaternion_t quaternion)
{
	quaternion = mpu9250_invertQuaternionNumber(quaternion);
	return mpu9250_convertSensorToWorldRaw(sensordata, quaternion, mpu9250_conjugateQuaternionNumber(quaternion));
}

// Projeziert den gegebenen Sensorwert (in Weltkoordinaten) anhand der Drehquaternion in Sensorkoordinaten
O3 INLINE mpu9250_triple_t mpu9250_convertWorldToSensorRaw(mpu9250_triple_t worlddata, mpu9250_quaternion_t quaternion, mpu9250_quaternion_t inversequaternion)
{
	volatile mpu9250_quaternion_t sensorquat;
	volatile mpu9250_quaternion_t projectedquat;
	volatile mpu9250_triple_t output;
	sensorquat.q0 = 0;
	sensorquat.q1 = worlddata.x;
	sensorquat.q2 = worlddata.y;
	sensorquat.q3 = worlddata.z;
	projectedquat = mpu9250_multiplyQuaternionNumbers(inversequaternion, mpu9250_multiplyQuaternionNumbers(sensorquat, quaternion));
	output.x = projectedquat.q1;
	output.y = projectedquat.q2;
	output.z = projectedquat.q3;
	return output;
}

// Projeziert den gegebenen Sensorwert (in Sensorkoordinaten) anhand der Drehquaternion in Weltkoordinaten
O3 INLINE mpu9250_triple_t mpu9250_convertSensorToWorldRaw(mpu9250_triple_t sensordata, mpu9250_quaternion_t quaternion, mpu9250_quaternion_t inversequaternion)
{
	volatile mpu9250_quaternion_t sensorquat;
	volatile mpu9250_quaternion_t projectedquat;
	volatile mpu9250_triple_t output;
	sensorquat.q0 = 0;
	sensorquat.q1 = sensordata.x;
	sensorquat.q2 = sensordata.y;
	sensorquat.q3 = sensordata.z;
	projectedquat = mpu9250_multiplyQuaternionNumbers(quaternion, mpu9250_multiplyQuaternionNumbers(sensorquat, inversequaternion));
	output.x = projectedquat.q1;
	output.y = projectedquat.q2;
	output.z = projectedquat.q3;
	return output;
}
