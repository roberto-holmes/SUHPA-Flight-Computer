#include <Adafruit_BMP3XX.h>
#include <Adafruit_BNO055.h>
#include <EEPROM.h>
#include <RF24.h>
#include <SPI.h>
#include <SdFat.h>
#include <TimeLib.h>
#include <Wire.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <nRF24L01.h>
#include <printf.h>
#include <stdint.h>
#include <usb_serial.h>
#include <util/crc16.h>

#include "Display.h"

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
#define led1 22
#define led2 21

// EEPROM
// There are limited write cycles to any address in EEPROM
// It should be about 100000 cycles, but in theory it coulbe be a problem
// If so comment out this line to dissable it
#define ENABLE_EEPROM

// You can also change to a different address as the limitation is per address not total
// Also these address need to be at least 4 apart as the trim values are integers (although they don't really need to be)
#define EEPROM_VERT_TRIM_ADDRESS 0
#define EEPROM_HORIZ_TRIM_ADDRESS 4

// Values for battery voltage calculations
const float v100 = 3 * 4.2;
const float v0 = 3 * 3.5;
const float batVoltFactor = (3.3 * 4.09) / 1024;

// PWM range (Absolute max 588 to 2400 ish)
const int vertMaxVal = 2200;
const int vertMinVal = 750;
const int horizMaxVal = 2200;
const int horizMinVal = 750;
float vertZero, horizZero;

// Stick input range
const int vertStickMax = 107;
const int vertStickMin = 910;
const int horizStickMax = 107;
const int horizStickMin = 905;
float vertStickZero, horizStickZero;

const float vertStickMultiplier = 1.0;
const float horizStickMultiplier = 1.0;
float vertStickRes, horizStickRes;

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
const unsigned long holdDelayCentre = 500;

// Radio Setup
RF24 radio(radioCE, radioCSN);
const byte address[6] = "00001";
byte package[PACKAGE_SIZE];

// the setup routine runs once when you press reset:
void setup()
{
	Display::Init();

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

	pinMode(batSense, INPUT);

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

	vertRes = (vertMaxVal - vertMinVal) / (2.0 * (vertSteps));
	horizRes = (horizMaxVal - horizMinVal) / (2.0 * (horizSteps));

	vertStickZero = (vertStickMax + vertStickMin) / 2.0;
	horizStickZero = (horizStickMax + horizStickMin) / 2.0;

	vertZero = (vertMaxVal + vertMinVal) / 2.0;
	horizZero = (horizMaxVal + horizMinVal) / 2.0;

	vertStickRes = vertStickMultiplier * (vertMaxVal - vertMinVal) / 2.0;
	horizStickRes = horizStickMultiplier * (horizMaxVal - horizMinVal) / 2.0;

	radio.begin();
	radio.openWritingPipe(address);
	radio.setPALevel(RF24_PA_MAX);
	//radio.setPayloadSize(PACKAGE_SIZE);
	radio.stopListening();

	readFromEEPROM ();

	updateVertTrim (0);
	updateHorizTrim(0);
}

// the loop routine runs over and over again forever:
void loop()
{
	// Calculate battery state
	float batVoltage = analogRead(batSense) * batVoltFactor;
	float batPercent = constrain(map(batVoltage, v0, v100, 0.0, 1.0), 0.0, 1.0);
	if (batPercent < .25)
		digitalWrite(led1, HIGH);
	else
		digitalWrite(led1, LOW);

	// Read Stick inputs and normalise
	float vertStickValue = 2.0 * (analogRead(stickVert) - vertStickZero) / (vertStickMax - vertStickMin);
	float horizStickValue = -2.0 * (analogRead(stickHoriz) - horizStickZero) / (horizStickMax - horizStickMin);

	// Calculate PWM values in both axis by combining stick inputs with trim
	float tempVertVal = vertStickValue * vertStickRes + vertTrim * vertRes + vertZero;
	float tempHorizVal = horizStickValue * horizStickRes + horizTrim * horizRes + horizZero;

	vertStickValue = constrain(vertStickValue, -1, 1);
	horizStickValue = constrain(horizStickValue, -1, 1);

	vertVal = constrain(tempVertVal, vertMinVal, vertMaxVal);
	horizVal = constrain(tempHorizVal, horizMinVal, horizMaxVal);

	// Debug PWM values
	// Serial.print(vertVal);
	// Serial.print(",");
	// Serial.println(horizVal);

	// Transmit PWM values
	transmit(vertVal, horizVal);

	// Serial.print(analogRead(stickVert));
	// Serial.print(" : ");
	// Serial.print(vertStickValue);
	// Serial.print("\t");
	// Serial.print(analogRead(stickHoriz));
	// Serial.print(" : ");
	// Serial.print(horizStickValue);
	// Serial.print("\n");

	// Serial.print(tempVertVal);
	// Serial.print(" : ");
	// Serial.print(vertVal);
	// Serial.print("\t");
	// Serial.print(tempHorizVal);
	// Serial.print(" : ");
	// Serial.print(horizVal);
	// Serial.print("\n");

	Serial.println(batPercent);

	delay(10);

	// Check if button is being pressed
	if (digitalRead(trimUp))
	{
		// Debounce and check if the button is being held down
		if ((!trimUpDown || (vertTrim != 0 && millis() > trimUpLastPressTime + holdDelay) || (vertTrim == 0 && millis() > trimUpLastPressTime + holdDelayCentre)) && millis() > trimUpLastPressTime + debounceDelay)
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
		if ((!trimDownDown || (vertTrim != 0 && millis() > trimDownLastPressTime + holdDelay) || (vertTrim == 0 && millis() > trimDownLastPressTime + holdDelayCentre)) && millis() > trimDownLastPressTime + debounceDelay)
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
		if ((!trimLeftDown || (horizTrim != 0 && millis() > trimLeftLastPressTime + holdDelay) || (horizTrim == 0 && millis() > trimLeftLastPressTime + holdDelayCentre)) && millis() > trimLeftLastPressTime + debounceDelay)
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
		if ((!trimRightDown || (horizTrim != 0 && millis() > trimRightLastPressTime + holdDelay) || (horizTrim == 0 && millis() > trimRightLastPressTime + holdDelayCentre)) && millis() > trimRightLastPressTime + debounceDelay)
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

	Display::Update (vertVal, horizVal, horizMinVal, horizMaxVal, vertMinVal, vertMaxVal, horizTrim, vertTrim, batPercent);
}

void beep(int level)
{
	analogWrite(buzzer, 64 + (level * 16));
	delay(100);
	analogWrite(buzzer, 0);
}

void updateVertTrim(int trimInput)
{
	if (trimInput != 0)
	{
		if (vertTrim + trimInput <= vertSteps && vertTrim + trimInput >= -vertSteps)
		{
			vertTrim += trimInput;
			beep (0);
		}
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

	writeToEEPROM ();
}

void updateHorizTrim(int trimInput)
{
	if (trimInput != 0)
	{
		if (horizTrim + trimInput <= horizSteps && horizTrim + trimInput >= -horizSteps)
		{
			horizTrim += trimInput;
			beep (0);
		}
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
		digitalWrite (horizMax, HIGH);

	writeToEEPROM ();
}

void transmit(float ele, float rud)
{
	memcpy(package + 0, &ele, 4);
	memcpy(package + 4, &rud, 4);

	// for (int i = 0; i < PACKAGE_SIZE; i++)
	// {
	// 	// Serial.print("Radio: ");
	// 	Serial.print(package[i]);
	// 	Serial.print(" ");
	// }
	// Serial.println();
	radio.write(&package, PACKAGE_SIZE);
}

void readFromEEPROM ()
{
#ifdef ENABLE_EEPROM

	// Read in the values
	vertTrim = readIntToEEPROM (EEPROM_VERT_TRIM_ADDRESS);
	horizTrim = readIntToEEPROM (EEPROM_HORIZ_TRIM_ADDRESS);

	// Validate the values
	// If they are not correct, give them default values
	// This is needed if the EEPROM gets corrupted
	if (vertTrim > vertSteps || vertTrim < -vertSteps) vertTrim = 0;
	if (horizTrim > horizSteps || horizTrim < -horizSteps) horizTrim = 0;

#endif // ENABLE_EEPROM
}

void writeToEEPROM ()
{
#ifdef ENABLE_EEPROM

	writeIntToEEPROM (EEPROM_VERT_TRIM_ADDRESS, vertTrim);
	writeIntToEEPROM (EEPROM_HORIZ_TRIM_ADDRESS, horizTrim);

#endif // ENABLE_EEPROM
}

void writeIntToEEPROM (int address, int value)
{
	// Use the update method, so that we don't write to the EEPROM unless it is needed
	EEPROM.update (address + 0, (byte) (value));
	EEPROM.update (address + 1, (byte) (value >> 8));
	EEPROM.update (address + 2, (byte) (value >> 16));
	EEPROM.update (address + 3, (byte) (value >> 24));
}

int readIntToEEPROM (int address)
{
	int value = 0;

	value |= EEPROM.read (address);
	value |= EEPROM.read (address + 1) << 8;
	value |= EEPROM.read (address + 2) << 16;
	value |= EEPROM.read (address + 3) << 24;

	return value;
}