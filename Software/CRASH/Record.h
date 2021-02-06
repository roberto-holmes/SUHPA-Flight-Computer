#pragma once

#include <SdFat.h>
#include <stdint.h>
#include <usb_serial.h>

struct DataRecord
{
public:
	DataRecord () {}
	~DataRecord () {}

	uint32_t time;

	// User inputs
	float vertVal;
	float horizVal;
	int vertTrim;
	int horizTrim;

	// Radio info
	float ping;
	float packetLoss;

	// Sensors
	float gravity[3];
	float linearAcceleration[3];
	float acceleration[3];
	float gyro[3];
	float mag[3];
	float orientation[3];

	float altitude;
	float temperature;

	float batteryPercent;

	void printData ()
	{
		//Serial.print (time);
		//Serial.print (",");

		Serial.print (vertVal);
		Serial.print (" ");
		Serial.print (horizVal);
		Serial.print (" ");

		Serial.print (vertTrim);
		Serial.print (" ");
		Serial.print (horizTrim);
		Serial.print (" ");

		Serial.print (ping);
		Serial.print (" ");
		Serial.print (packetLoss);
		Serial.print (" ");

		Serial.print (gravity[0]);
		Serial.print (" ");
		Serial.print (gravity[1]);
		Serial.print (" ");
		Serial.print (gravity[2]);
		Serial.print (" ");
		Serial.print (linearAcceleration[0]);
		Serial.print (" ");
		Serial.print (linearAcceleration[1]);
		Serial.print (" ");
		Serial.print (linearAcceleration[2]);
		Serial.print (" ");
		Serial.print (acceleration[0]);
		Serial.print (" ");
		Serial.print (acceleration[1]);
		Serial.print (" ");
		Serial.print (acceleration[2]);
		Serial.print (" ");
		Serial.print (gyro[0]);
		Serial.print (" ");
		Serial.print (gyro[1]);
		Serial.print (" ");
		Serial.print (gyro[2]);
		Serial.print (" ");
		Serial.print (mag[0]);
		Serial.print (" ");
		Serial.print (mag[1]);
		Serial.print (" ");
		Serial.print (mag[2]);
		Serial.print (" ");
		Serial.print (orientation[0]);
		Serial.print (" ");
		Serial.print (orientation[1]);
		Serial.print (" ");
		Serial.print (orientation[2]);
		Serial.print (" ");

		Serial.print (altitude);
		Serial.print (" ");
		Serial.print (temperature);
		Serial.print (" ");

		Serial.print (batteryPercent);

		Serial.println ();
	}

	void saveHeader (ExFile* sdFile)
	{
		sdFile->print ("time");
		sdFile->print (",");

		sdFile->print ("vertVal");
		sdFile->print (",");
		sdFile->print ("horizVal");
		sdFile->print (",");

		sdFile->print ("vertTrim");
		sdFile->print (",");
		sdFile->print ("horizTrim");
		sdFile->print (",");

		sdFile->print ("ping");
		sdFile->print (",");
		sdFile->print ("packetLoss");
		sdFile->print (",");

		sdFile->print ("gravityX");
		sdFile->print (",");
		sdFile->print ("gravityY");
		sdFile->print (",");
		sdFile->print ("gravityZ");
		sdFile->print (",");
		sdFile->print ("linearAccelerationX");
		sdFile->print (",");
		sdFile->print ("linearAccelerationY");
		sdFile->print (",");
		sdFile->print ("linearAccelerationZ");
		sdFile->print (",");
		sdFile->print ("accelerationX");
		sdFile->print (",");
		sdFile->print ("accelerationY");
		sdFile->print (",");
		sdFile->print ("accelerationZ");
		sdFile->print (",");
		sdFile->print ("gyroX");
		sdFile->print (",");
		sdFile->print ("gyroY");
		sdFile->print (",");
		sdFile->print ("gyroZ");
		sdFile->print (",");
		sdFile->print ("magX");
		sdFile->print (",");
		sdFile->print ("magY");
		sdFile->print (",");
		sdFile->print ("magZ");
		sdFile->print (",");
		sdFile->print ("orientationX");
		sdFile->print (",");
		sdFile->print ("orientationY");
		sdFile->print (",");
		sdFile->print ("rientationZ");
		sdFile->print (",");

		sdFile->print ("altitude");
		sdFile->print (",");
		sdFile->print ("temperature");
		sdFile->print (",");

		sdFile->print ("batteryPercent");

		sdFile->println ();
	}

	void saveData (ExFile* sdFile)
	{
		sdFile->print (time);
		sdFile->print (",");

		sdFile->print (vertVal);
		sdFile->print (",");
		sdFile->print (horizVal);
		sdFile->print (",");

		sdFile->print (vertTrim);
		sdFile->print (",");
		sdFile->print (horizTrim);
		sdFile->print (",");

		sdFile->print (ping);
		sdFile->print (",");
		sdFile->print (packetLoss);
		sdFile->print (",");

		sdFile->print (gravity[0]);
		sdFile->print (",");
		sdFile->print (gravity[1]);
		sdFile->print (",");
		sdFile->print (gravity[2]);
		sdFile->print (",");
		sdFile->print (linearAcceleration[0]);
		sdFile->print (",");
		sdFile->print (linearAcceleration[1]);
		sdFile->print (",");
		sdFile->print (linearAcceleration[2]);
		sdFile->print (",");
		sdFile->print (acceleration[0]);
		sdFile->print (",");
		sdFile->print (acceleration[1]);
		sdFile->print (",");
		sdFile->print (acceleration[2]);
		sdFile->print (",");
		sdFile->print (gyro[0]);
		sdFile->print (",");
		sdFile->print (gyro[1]);
		sdFile->print (",");
		sdFile->print (gyro[2]);
		sdFile->print (",");
		sdFile->print (mag[0]);
		sdFile->print (",");
		sdFile->print (mag[1]);
		sdFile->print (",");
		sdFile->print (mag[2]);
		sdFile->print (",");
		sdFile->print (orientation[0]);
		sdFile->print (",");
		sdFile->print (orientation[1]);
		sdFile->print (",");
		sdFile->print (orientation[2]);
		sdFile->print (",");

		sdFile->print (altitude);
		sdFile->print (",");
		sdFile->print (temperature);
		sdFile->print (",");

		sdFile->print (batteryPercent);

		sdFile->println ();
	}
};