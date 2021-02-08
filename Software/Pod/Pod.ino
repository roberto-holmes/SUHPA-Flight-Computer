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
#define ACK_PAYLOAD_SIZE 2

Servo servoEle, servoRud;

int elePin = 5;
int rudPin = 6;	 //Pins 5 and 6 for pod board

float eleAngle, rudAngle;

RF24 radio(radioCE, radioCSN);
uint8_t address[][6] = {"CRASH", "Poddd"};
byte package[PACKAGE_SIZE];
byte ackPayload[ACK_PAYLOAD_SIZE] = "E";

float rud, ele;

void setup()
{
	Serial.begin(115200);

	// Radio is not responding -> turn on warning LED
	if (!radio.begin())
	{
		digitalWrite(4, HIGH);
	}

	//! For some reason, higher values lead to greater packet loss
	//! RF24_PA_LOW isn't powerful enough for consistent packets
	// From 0 to 3
	radio.setPALevel(1);

	// Set up ack payloads
	radio.enableDynamicPayloads();
	radio.enableAckPayload();

	// Set up pipes
	radio.openWritingPipe(address[1]);
	radio.openReadingPipe(1, address[0]);

	// setup the ACK payload & load the first response into the FIFO
	// memcpy(ackPayload, "E", 2);	 // set the payload message
	// load the payload for the first received transmission on pipe 0
	radio.writeAckPayload(1, &ackPayload, sizeof(ackPayload));

	// Put radio in RX mode
	radio.startListening();

	servoEle.attach(elePin);
	servoRud.attach(rudPin);
}
void loop()
{
	uint8_t pipe;
	if (radio.available(&pipe))
	{  // is there a payload? get the pipe number that recieved it
		uint8_t bytes = radio.getDynamicPayloadSize();	// get the size of the payload
		char received[8];
		radio.read(&package, sizeof(package));	// get incoming payload

		memcpy(&ele, package + 0, 4);
		memcpy(&rud, package + 4, 4);

		servoEle.write(ele);
		servoRud.write(rud);

		Serial.print(ele);
		Serial.print("\t");
		Serial.print(rud);
		Serial.print("\n");

		// load the payload for the first received transmission on pipe 0
		radio.writeAckPayload(1, &ackPayload, sizeof(ackPayload));
	}
}
