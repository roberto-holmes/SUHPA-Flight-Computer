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

	float pressure;
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

		Serial.print (pressure);
		Serial.print (" ");
		Serial.print (temperature);
		Serial.print (" ");

		Serial.print (batteryPercent);

		Serial.println ();
	}

	void saveHeader (ExFile* sdFile)
	{
		sdFile->print ("time_Milliseconds");
		sdFile->print (",");

		sdFile->print ("vertVal");
		sdFile->print (",");
		sdFile->print ("horizVal");
		sdFile->print (",");

		sdFile->print ("vertTrim");
		sdFile->print (",");
		sdFile->print ("horizTrim");
		sdFile->print (",");

		sdFile->print ("ping_Microseconds");
		sdFile->print (",");
		sdFile->print ("packetLoss_Percent");
		sdFile->print (",");

		sdFile->print ("gravityX_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("gravityY_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("gravityZ_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("linearAccelerationX_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("linearAccelerationY_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("linearAccelerationZ_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("accelerationX_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("accelerationY_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("accelerationZ_MetersPerSecondSquared");
		sdFile->print (",");
		sdFile->print ("gyroX_RadiansPerSecond");
		sdFile->print (",");
		sdFile->print ("gyroY_RadiansPerSecond");
		sdFile->print (",");
		sdFile->print ("gyroZ_RadiansPerSecond");
		sdFile->print (",");
		sdFile->print ("magX_MicroTesla");
		sdFile->print (",");
		sdFile->print ("magY_MicroTesla");
		sdFile->print (",");
		sdFile->print ("magZ_MicroTesla");
		sdFile->print (",");
		sdFile->print ("orientationX_Radians");
		sdFile->print (",");
		sdFile->print ("orientationY_Radians");
		sdFile->print (",");
		sdFile->print ("orientationZ_Radians");
		sdFile->print (",");

		sdFile->print ("pressure_Pascals");
		sdFile->print (",");
		sdFile->print ("temperature_Celsius");
		sdFile->print (",");

		sdFile->print ("battery_Percent");

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

		sdFile->print (pressure);
		sdFile->print (",");
		sdFile->print (temperature);
		sdFile->print (",");

		sdFile->print (batteryPercent);

		sdFile->println ();
	}
};