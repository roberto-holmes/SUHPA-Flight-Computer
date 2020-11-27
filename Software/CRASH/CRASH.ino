#ifndef TRUE
#define TRUE 1
#endif	// !TRUE

#ifndef FALSE
#define FALSE 0
#endif	// !FALSE

#ifndef NULL
#define NULL 0
#endif	// !NULL

#ifndef VSCODE
#include <WProgram.h>
#endif

#include <Adafruit_BMP3XX.h>
#include <Adafruit_BNO055.h>
#include <SdFat.h>
#include <TimeLib.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <printf.h>
#include <stdint.h>
#include <usb_serial.h>
#include <util/crc16.h>

// #include "sdios.h"
// #include <DS1307RTC.h>
#include <EEPROM.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>
// #include <SerialFlash.h>
#include <Wire.h>

int leds[] = {22, 21, 40, 39, 38, 37, 35, 0, 1, 2, 3, 4};

// Trim LEDs
#define vertMin 4
#define vertLow 3
#define vertMid 2
#define vertHigh 1
#define vertMax 0

#define horizMin 40
#define horizLow 39
#define horizMid 38
#define horizHigh 37
#define horizMax 35

// Trim inputs
#define trimUp 31
#define trimDown 30

#define trimLeft 28
#define trimRight 29

// Stick inputs
#define stickVert 14
#define stickHoriz 15

// Battery voltage (If you multiply by 4.09)
#define batSense 23

// Radio
#define radioCE 27
#define radioCSN 26
#define radioINT 25
#define PACKAGE_SIZE 8

// Display
#define displayFR 41
#define displayCSB 16
#define displayRSTB 17
#define displayDC 20

// Other outputs
#define buzzer 33
#define led1 21
#define led2 22

// PWM range (Absolute max 588 to 2400 ish)
const int vertMaxVal = 2400;
const int vertMinVal = 600;
const int horizMaxVal = 2400;
const int horizMinVal = 600;
float vertZero, horizZero;

// Stick input range
const int vertStickMax = 100;
const int vertStickMin = 900;
const int horizStickMax = 100;
const int horizStickMin = 900;
float vertStickZero, horizStickZero;

const float vertStickMultiplier = 1000.0;
const float horizStickMultiplier = 1000.0;

// Number of Trim steps / 2
const int vertSteps = 10;
const int horizSteps = 10;
float vertRes, horizRes;

// Values to store Trim and PWM
int vertTrim = 0;
int horizTrim = 0;
int vertVal = 0;
int horizVal = 0;

// Store state of trim buttons
bool trimUpDown = false;
bool trimDownDown = false;
bool trimLeftDown = false;
bool trimRightDown = false;

// Values for debouncing
unsigned long trimUpLastDebounceTime = 0;
unsigned long trimDownLastDebounceTime = 0;
unsigned long trimLeftLastDebounceTime = 0;
unsigned long trimRightLastDebounceTime = 0;

unsigned long trimUpLastPressTime = 0;
unsigned long trimDownLastPressTime = 0;
unsigned long trimLeftLastPressTime = 0;
unsigned long trimRightLastPressTime = 0;

const unsigned long debounceDelay = 50;
const unsigned long holdDelay = 250;

// Radio Setup
RF24 radio(radioCE, radioCSN);
const byte address[6] = "00001";
byte package[PACKAGE_SIZE];

// the setup routine runs once when you press reset:
void setup()
{
	pinMode(vertMin, OUTPUT);
	pinMode(vertLow, OUTPUT);
	pinMode(vertMid, OUTPUT);
	pinMode(vertHigh, OUTPUT);
	pinMode(vertMax, OUTPUT);

	pinMode(horizMin, OUTPUT);
	pinMode(horizLow, OUTPUT);
	pinMode(horizMid, OUTPUT);
	pinMode(horizHigh, OUTPUT);
	pinMode(horizMax, OUTPUT);
	pinMode(buzzer, OUTPUT);

	pinMode(trimUp, INPUT);
	pinMode(trimDown, INPUT);
	pinMode(trimLeft, INPUT);
	pinMode(trimRight, INPUT);

	pinMode(stickVert, INPUT);
	pinMode(stickHoriz, INPUT);

	// Ensure trim LEDs are off
	digitalWrite(vertMin, LOW);
	digitalWrite(vertLow, LOW);
	digitalWrite(vertMid, LOW);
	digitalWrite(vertHigh, LOW);
	digitalWrite(vertMax, LOW);
	digitalWrite(horizMin, LOW);
	digitalWrite(horizLow, LOW);
	digitalWrite(horizMid, LOW);
	digitalWrite(horizHigh, LOW);
	digitalWrite(horizMax, LOW);

	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	pinMode(13, OUTPUT);

	for (int i = 0; i < 11; i++)
	{
		digitalWrite(leds[i], HIGH);
		delay(100);
		digitalWrite(leds[i], LOW);
	}

	for (int i = 12 - 1; i >= 1; i--)
	{
		digitalWrite(leds[i], HIGH);
		delay(100);
		digitalWrite(leds[i], LOW);
	}

	digitalWrite(leds[0], HIGH);
	delay(200);
	digitalWrite(leds[0], LOW);

	vertRes = (vertMaxVal - vertMinVal) / (2 * vertSteps);
	horizRes = (horizMaxVal - horizMinVal) / (2 * horizSteps);

	vertStickZero = (vertStickMax - vertStickMin) / 2;
	horizStickZero = (horizStickMax - horizStickMin) / 2;

	vertZero = (vertMaxVal - vertMinVal) / 2;
	horizZero = (horizMaxVal - horizMinVal) / 2;

	radio.begin();
	radio.openWritingPipe(address);
	radio.setPALevel(RF24_PA_MAX);
	//radio.setPayloadSize(PACKAGE_SIZE);
	radio.stopListening();

	updateVertTrim(0);
	updateHorizTrim(0);

	// updateTrim();
}

// the loop routine runs over and over again forever:
void loop()
{
	// float vertStickValue = 1 - (analogRead(stickVert) / 1024.0);
	// float horizStickValue = analogRead(stickHoriz) / 1024.0;

	// vertStickValue += (vertTrim * 0.2);
	// horizStickValue += (horizTrim * 0.2);

	// Read Stick inputs and normalise
	float vertStickValue = (1024 - analogRead(stickVert) - vertStickZero) / 1024.0;
	float horizStickValue = (analogRead(stickHoriz) - horizStickZero) / 1024.0;

	// Calculate PWM values in both axis by combining stick inputs with trim
	float tempVertVal = vertStickValue * vertStickMultiplier + vertTrim * vertRes + vertZero;
	float tempHorizVal = horizStickValue * horizStickMultiplier + horizTrim * horizRes + horizZero;

	// Ensure PWM values are valid
	if (tempVertVal <= vertMaxVal && tempVertVal >= vertMinVal)
		vertVal = tempVertVal;
	if (tempHorizVal <= horizMaxVal && tempHorizVal >= horizMinVal)
		horizVal = tempHorizVal;

	// Debug PWM values
	// Serial.print(vertVal);
	// Serial.print(",");
	// Serial.println(horizVal);

	// Transmit PWM values
	transmit(vertVal, horizVal);

	delay(10);

	// Check if button is being pressed
	if (digitalRead(trimUp))
	{
		// Debounce and check if the button is being held down
		if ((!trimUpDown || millis() > trimUpLastPressTime + holdDelay) && millis() > trimUpLastPressTime + debounceDelay)
		{
			trimUpLastPressTime = millis();
			trimUpDown = true;
			// vertTrim++;
			updateVertTrim(1);
		}
	}
	else
	{
		trimUpDown = false;
	}

	if (digitalRead(trimDown))
	{
		if ((!trimDownDown || millis() > trimDownLastPressTime + holdDelay) && millis() > trimDownLastPressTime + debounceDelay)
		{
			trimDownLastPressTime = millis();
			trimDownDown = true;
			// vertTrim--;
			updateVertTrim(-1);
		}
	}
	else
	{
		trimDownDown = false;
	}

	if (digitalRead(trimLeft))
	{
		if ((!trimLeftDown || millis() > trimLeftLastPressTime + holdDelay) && millis() > trimLeftLastPressTime + debounceDelay)
		{
			trimLeftLastPressTime = millis();
			trimLeftDown = true;
			// horizTrim--;
			updateHorizTrim(-1);
		}
	}
	else
	{
		trimLeftDown = false;
	}

	if (digitalRead(trimRight))
	{
		if ((!trimRightDown || millis() > trimRightLastPressTime + holdDelay) && millis() > trimRightLastPressTime + debounceDelay)
		{
			trimRightLastPressTime = millis();
			trimRightDown = true;
			// horizTrim++;
			updateHorizTrim(1);
		}
	}
	else
	{
		trimRightDown = false;
	}
}

void beep(int level)
{
	analogWrite(buzzer, 64 + (level * 16));
	delay(100);
	analogWrite(buzzer, 0);
}

void updateVertTrim(int trimInput)
{
	if (vertTrim + trimInput <= vertSteps && vertTrim + trimInput >= -vertSteps)
	{
		vertTrim += trimInput;
		beep(0);
	}

	digitalWrite(vertMin, LOW);
	digitalWrite(vertLow, LOW);
	digitalWrite(vertMid, LOW);
	digitalWrite(vertHigh, LOW);
	digitalWrite(vertMax, LOW);

	if (vertTrim == -vertSteps)
		digitalWrite(vertMin, HIGH);
	else if (vertTrim < 0)
		digitalWrite(vertLow, HIGH);
	else if (vertTrim == 0)
		digitalWrite(vertMid, HIGH);
	else if (vertTrim < vertSteps)
		digitalWrite(vertHigh, HIGH);
	else if (vertTrim == vertSteps)
		digitalWrite(vertMax, HIGH);
}

void updateHorizTrim(int trimInput)
{
	if (horizTrim + trimInput <= horizSteps && horizTrim + trimInput >= -horizSteps)
	{
		horizTrim += trimInput;
		beep(0);
	}

	digitalWrite(horizMin, LOW);
	digitalWrite(horizLow, LOW);
	digitalWrite(horizMid, LOW);
	digitalWrite(horizHigh, LOW);
	digitalWrite(horizMax, LOW);

	if (horizTrim == -horizSteps)
		digitalWrite(horizMin, HIGH);
	else if (horizTrim < 0)
		digitalWrite(horizLow, HIGH);
	else if (horizTrim == 0)
		digitalWrite(horizMid, HIGH);
	else if (horizTrim < horizSteps)
		digitalWrite(horizHigh, HIGH);
	else if (horizTrim == horizSteps)
		digitalWrite(horizMax, HIGH);
}

void transmit(float ele, float rud)
{
	memcpy(package + 0, &ele, 4);
	memcpy(package + 4, &rud, 4);

	for (int i = 0; i < PACKAGE_SIZE; i++)
	{
		// Serial.print("Radio: ");
		Serial.print(package[i]);
		Serial.print(" ");
	}
	Serial.println();
	radio.write(&package, PACKAGE_SIZE);
}

// void updateTrim()
// {
// 	if (vertTrim > 2) vertTrim = 2;
// 	if (vertTrim < -2) vertTrim = -2;
// 	if (horizTrim > 2) horizTrim = 2;
// 	if (horizTrim < -2) horizTrim = -2;

// 	digitalWrite(vertMin, LOW);
// 	digitalWrite(vertLow, LOW);
// 	digitalWrite(vertMid, LOW);
// 	digitalWrite(vertHigh, LOW);
// 	digitalWrite(vertMax, LOW);
// 	digitalWrite(horizMin, LOW);
// 	digitalWrite(horizLow, LOW);
// 	digitalWrite(horizMid, LOW);
// 	digitalWrite(horizHigh, LOW);
// 	digitalWrite(horizMax, LOW);

// 	if (vertTrim == -2) digitalWrite(vertMin, HIGH);
// 	else if (vertTrim == -1)
// 		digitalWrite(vertLow, HIGH);
// 	else if (vertTrim == 0)
// 		digitalWrite(vertMid, HIGH);
// 	else if (vertTrim == 1)
// 		digitalWrite(vertHigh, HIGH);
// 	else if (vertTrim == 2)
// 		digitalWrite(vertMax, HIGH);

// 	if (horizTrim == -2) digitalWrite(horizMin, HIGH);
// 	else if (horizTrim == -1)
// 		digitalWrite(horizLow, HIGH);
// 	else if (horizTrim == 0)
// 		digitalWrite(horizMid, HIGH);
// 	else if (horizTrim == 1)
// 		digitalWrite(horizHigh, HIGH);
// 	else if (horizTrim == 2)
// 		digitalWrite(horizMax, HIGH);
// }