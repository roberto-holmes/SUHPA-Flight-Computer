#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <Servo.h>
#include <nRF24L01.h>

// Radio
#define radioCE 9
#define radioCSN 8
#define radioINT 25
#define PACKAGE_SIZE 8

Servo servoEle, servoRud;

int elePin = 5;
int rudPin = 6;	 //Pins 5 and 6 for pod board

float eleAngle, rudAngle;

RF24 radio(radioCE, radioCSN);
const byte address[6] = "00001";

byte package[PACKAGE_SIZE];

float rud, ele;

void setup()
{
	Serial.begin(115200);
	radio.begin();
	radio.openReadingPipe(0, address);
	// radio.setPALevel(RF24_PA_MIN);
	//radio.setPayloadSize(PACKAGE_SIZE);
	radio.startListening();

	servoEle.attach(elePin);
	servoRud.attach(rudPin);
}
void loop()
{
	if (radio.available())
	{
		radio.read(&package, PACKAGE_SIZE);
		memcpy(&ele, package + 0, 4);
		memcpy(&rud, package + 4, 4);

		servoEle.write(ele);
		servoRud.write(rud);

		Serial.print(ele);
		Serial.print("\t");
		Serial.print(rud);
		Serial.print("\n");
	}
}
