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
#include "Record.h"

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
#define ACK_PAYLOAD_SIZE 2
#define PACKET_LOSS_SAMPLE_SIZE 100
#define PING_SAMPLE_SIZE 100

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
// It should be about 100000 cycles, but in theory it could be be a problem
// If so comment out this line to dissable it
#define ENABLE_EEPROM

// You can also change to a different address as the limitation is per address not total
// Also these address need to be at least 4 apart as the trim values are integers (although they don't really need to be)
#define EEPROM_VERT_TRIM_ADDRESS 0
#define EEPROM_HORIZ_TRIM_ADDRESS 4

// Buzzer
int beepStopTime = 0;
bool beepOn = false;

// Values for battery voltage calculations
const float v100 = 3 * 4.2;
const float v0 = 3 * 3.5;
const float batVoltFactor = (3.3 * 4.09) / 1024;
float batPercent;

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
const int vertSteps = 100;
const int horizSteps = 100;
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
uint8_t address[][6] = {"CRASH", "Poddd"};
byte package[PACKAGE_SIZE];

// Variables frequency of communication
const unsigned long txInterval = 10;
unsigned long lastTXMillis;

// Control when the display gets updated
const unsigned long displayUpdateInterval = 10;
unsigned long lastDisplayUpdateMillis;

// Variables for calculating packet loss
bool ack[PACKET_LOSS_SAMPLE_SIZE];
int ackPos = 0;
int truers = 0;
float packetLoss;  // Fraction of packets that don't receive ack

// Variables for calculating return trip time
unsigned long pingArray[PACKET_LOSS_SAMPLE_SIZE];
int pingPos = 0;
float ping;	 // Average return trip time in ms

// Sensors
#define IMU_I2C_ADDRESS BNO055_ADDRESS_A
Adafruit_BNO055 imuDevice = Adafruit_BNO055(0, IMU_I2C_ADDRESS);
Adafruit_BMP3XX barometerDevice = Adafruit_BMP3XX();
DataRecord currentSensorData;
const unsigned long sensorReadInterval = 50;
unsigned long lastSensorUpdateMillis;

// Storage
#define STORAGE_DATA_DIRECTORY "Data"
#define STORAGE_FILE_NAME_FORMAT "Data/XXXX.csv\0"
#define STORAGE_FILE_NAME_LENGTH 14
#define STORAGE_FILE_NAME_NUMBER_START 5
#define STORAGE_FILE_NAME_MAX_NUMBER 9999

SdExFat sdio;
ExFile sdFile;
char* fileName = NULL;
bool sdCardInitialised = false;
SdioConfig sdConfig = SdioConfig();

// the setup routine runs once when you press reset:
void setup()
{
	Display::Init();
	initSensors();
	initSD();

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

	// Radio is not responding -> turn on warning LED
	if (!radio.begin())
	{
		digitalWrite(led2, HIGH);
	}

	// Initialise ack array
	for (int i = 0; i < PACKET_LOSS_SAMPLE_SIZE; i++)
		ack[i] = true;

	radio.setPALevel(RF24_PA_MAX);

	// Set up ack payloads
	radio.enableDynamicPayloads();
	radio.enableAckPayload();

	// Set up pipes
	radio.openWritingPipe(address[0]);
	radio.openReadingPipe(1, address[1]);

	// Put radio in TX mode
	radio.stopListening();

	lastTXMillis = millis();
	lastSensorUpdateMillis = millis();
	lastDisplayUpdateMillis = millis();

	readFromEEPROM();

	updateVertTrim(0);
	updateHorizTrim(0);
}

// the loop routine runs over and over again forever:
void loop()
{
	// Asynchronous beep
	if (beepOn && millis() >= beepStopTime)
	{
		analogWrite(buzzer, 0);
		beepOn = false;
	}

	// Calculate battery state
	float batVoltage = analogRead(batSense) * batVoltFactor;
	batPercent = constrain(map(batVoltage, v0, v100, 0.0, 1.0), 0.0, 1.0);
	if (batPercent < .25)
		digitalWrite(led1, HIGH);
	else
		digitalWrite(led1, LOW);

	// Read Stick inputs and normalise
	float vertStickValue = -2.0 * (analogRead(stickVert) - vertStickZero) / (vertStickMax - vertStickMin);
	float horizStickValue = 2.0 * (analogRead(stickHoriz) - horizStickZero) / (horizStickMax - horizStickMin);

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

	// Serial.println(batPercent);

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

	unsigned long currentMillis = millis();

	// Decide whether it is time to transmit
	if (currentMillis - lastTXMillis > txInterval)
	{
		lastTXMillis = currentMillis;
		// Transmit PWM values
		transmit(vertVal, horizVal);
	}

	// Decide whether it is time to update screen
	if (currentMillis - lastDisplayUpdateMillis > displayUpdateInterval)
	{
		lastDisplayUpdateMillis = currentMillis;
		Display::Update(vertVal, horizVal, horizMinVal, horizMaxVal, vertMinVal, vertMaxVal, horizTrim, vertTrim, batPercent, ping, packetLoss, sdCardInitialised);
	}

	// Decide whether it is time to read and save sensor data
	if (currentMillis - lastSensorUpdateMillis > sensorReadInterval)
	{
		lastSensorUpdateMillis = currentMillis;

		// Read sensor data
		readFromSensors();

		// Use to print data to console
		//currentSensorData.printData ();

		// Save the data to SD card
		saveCurrentDataRecord();
	}
}

void beep(int level, int time)
{
	analogWrite(buzzer, 64 + (level * 16));
	beepStopTime = millis() + time;
	beepOn = true;
}

void updateVertTrim(int trimInput)
{
	if (trimInput != 0)
	{
		if (vertTrim + trimInput <= vertSteps && vertTrim + trimInput >= -vertSteps)
		{
			vertTrim += trimInput;
			beep(0, 100);
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

	writeToEEPROM();
}

void updateHorizTrim(int trimInput)
{
	if (trimInput != 0)
	{
		if (horizTrim + trimInput <= horizSteps && horizTrim + trimInput >= -horizSteps)
		{
			horizTrim += trimInput;
			beep(0, 100);
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
		digitalWrite(horizMax, HIGH);

	writeToEEPROM();
}

void transmit(float ele, float rud)
{
	memcpy(package + 0, &ele, 4);
	memcpy(package + 4, &rud, 4);

	unsigned long start_timer = micros();  // start the timer
	bool report = radio.write(&package, PACKAGE_SIZE);	// transmit & save the report
	unsigned long end_timer = micros();	 // end the timer

	if (report)
	{
		//Serial.print(F("Transmission successful! "));  // payload was delivered
		// Serial.print(F("Time to transmit = "));
		// Serial.println(end_timer - start_timer);  // print the timer result
		// Serial.print(F(" us. Sent: "));
		// Serial.print(payload);	// print the outgoing message

		ack[ackPos] = true;
		pingArray[pingPos++] = end_timer - start_timer;
		pingPos %= PING_SAMPLE_SIZE;

		uint8_t pipe;
		if (radio.available(&pipe))
		{  // is there an ACK payload? grab the pipe number that received it
			byte received[ACK_PAYLOAD_SIZE];
			radio.read(&received, sizeof(received));  // get incoming ACK payload
			// Serial.print(F(" Received "));
			// Serial.print(radio.getDynamicPayloadSize());  // print incoming payload size
			// Serial.print(F(" bytes on pipe "));
			// Serial.print(pipe);	 // print pipe number that received the ACK
			// Serial.print(F(": "));
			// Serial.println(received);
		}
		else
		{
			Serial.print(F("Received: an empty ACK packet, "));	 // empty ACK packet received
		}
	}
	else
	{
		Serial.print(F("Transmission failed or timed out, "));	// payload was not delivered
		ack[ackPos] = false;
	}

	// Calculate average ping over sample size
	ping = 0;
	for (int i = 0; i < PING_SAMPLE_SIZE; i++)
		ping += pingArray[i];
	ping /= (float)PING_SAMPLE_SIZE;
	ping /= 1000.;	// Convert from us to ms;

	// Calculate packet loss over sample size
	++ackPos %= PACKET_LOSS_SAMPLE_SIZE;
	truers = 0;
	for (int i = 0; i < PACKET_LOSS_SAMPLE_SIZE; i++)
	{
		if (ack[i])
			truers++;
	}
	packetLoss = 1 - (float)truers / (float)PACKET_LOSS_SAMPLE_SIZE;

	//Serial.print("Packet Loss: ");
	//Serial.print(packetLoss);
	//Serial.print(", Ping: ");
	//Serial.println(ping);
}

void readFromEEPROM()
{
#ifdef ENABLE_EEPROM

	// Read in the values
	vertTrim = readIntToEEPROM(EEPROM_VERT_TRIM_ADDRESS);
	horizTrim = readIntToEEPROM(EEPROM_HORIZ_TRIM_ADDRESS);

	// Validate the values
	// If they are not correct, give them default values
	// This is needed if the EEPROM gets corrupted
	if (vertTrim > vertSteps || vertTrim < -vertSteps) vertTrim = 0;
	if (horizTrim > horizSteps || horizTrim < -horizSteps) horizTrim = 0;

#endif	// ENABLE_EEPROM
}

void writeToEEPROM()
{
#ifdef ENABLE_EEPROM

	writeIntToEEPROM(EEPROM_VERT_TRIM_ADDRESS, vertTrim);
	writeIntToEEPROM(EEPROM_HORIZ_TRIM_ADDRESS, horizTrim);

#endif	// ENABLE_EEPROM
}

void writeIntToEEPROM(int address, int value)
{
	// Use the update method, so that we don't write to the EEPROM unless it is needed
	EEPROM.update(address + 0, (byte)(value));
	EEPROM.update(address + 1, (byte)(value >> 8));
	EEPROM.update(address + 2, (byte)(value >> 16));
	EEPROM.update(address + 3, (byte)(value >> 24));
}

int readIntToEEPROM(int address)
{
	int value = 0;

	value |= EEPROM.read(address);
	value |= EEPROM.read(address + 1) << 8;
	value |= EEPROM.read(address + 2) << 16;
	value |= EEPROM.read(address + 3) << 24;

	return value;
}

void initSensors()
{
	imuInit();
	barometerInit();
}

void readFromSensors()
{
	currentSensorData.time = millis();
	currentSensorData.vertVal = vertVal;
	currentSensorData.horizVal = horizVal;
	currentSensorData.vertTrim = vertTrim;
	currentSensorData.horizTrim = horizTrim;
	currentSensorData.ping = ping;
	currentSensorData.packetLoss = packetLoss;
	currentSensorData.batteryPercent = batPercent;
	imuRead();
	barometerRead();
}

bool imuInit()
{
	if (!imuDevice.begin(Adafruit_BNO055::adafruit_bno055_opmode_t::OPERATION_MODE_NDOF)) return false;

	//imuDevice.setAxisRemap (Adafruit_BNO055::adafruit_bno055_axis_remap_config_t::REMAP_CONFIG_P3);
	//imuDevice.setAxisSign (Adafruit_BNO055::adafruit_bno055_axis_remap_sign_t::REMAP_SIGN_P3);
	imuDevice.setExtCrystalUse(true);
	imuDevice.enterNormalMode();

	//while (!imuDevice.isFullyCalibrated ()) delay (10);

#ifdef VERBOSE_LOGGING
	Serial.println("BNO055 Configured");
#endif	// VERBOSE_LOGGING
	return true;
}

bool barometerInit()
{
	if (!barometerDevice.begin_I2C()) return false;

	// Configure device
	if (!barometerDevice.setTemperatureOversampling(BMP3_OVERSAMPLING_2X)) return false;
	if (!barometerDevice.setPressureOversampling(BMP3_OVERSAMPLING_8X)) return false;
	if (!barometerDevice.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3)) return false;
	if (!barometerDevice.setOutputDataRate(BMP3_ODR_50_HZ)) return false;

#ifdef VERBOSE_LOGGING
	Serial.println("BMP388 Configured");
#endif	// VERBOSE_LOGGING
	return true;
}

bool imuRead()
{
	sensors_event_t imuEvent;
	imuDevice.getEvent(&imuEvent, Adafruit_BNO055::VECTOR_ACCELEROMETER);
	currentSensorData.acceleration[0] = imuEvent.acceleration.x;
	currentSensorData.acceleration[1] = imuEvent.acceleration.y;
	currentSensorData.acceleration[2] = imuEvent.acceleration.z;
	imuDevice.getEvent(&imuEvent, Adafruit_BNO055::VECTOR_MAGNETOMETER);
	currentSensorData.mag[0] = imuEvent.magnetic.x;
	currentSensorData.mag[1] = imuEvent.magnetic.y;
	currentSensorData.mag[2] = imuEvent.magnetic.z;
	imuDevice.getEvent(&imuEvent, Adafruit_BNO055::VECTOR_GYROSCOPE);
	currentSensorData.gyro[0] = imuEvent.gyro.x * DEG_TO_RAD;
	currentSensorData.gyro[1] = imuEvent.gyro.y * DEG_TO_RAD;
	currentSensorData.gyro[2] = imuEvent.gyro.z * DEG_TO_RAD;
	imuDevice.getEvent(&imuEvent, Adafruit_BNO055::VECTOR_GRAVITY);
	currentSensorData.gravity[0] = imuEvent.acceleration.x;
	currentSensorData.gravity[1] = imuEvent.acceleration.y;
	currentSensorData.gravity[2] = imuEvent.acceleration.z;
	imuDevice.getEvent(&imuEvent, Adafruit_BNO055::VECTOR_LINEARACCEL);
	currentSensorData.linearAcceleration[0] = imuEvent.acceleration.x;
	currentSensorData.linearAcceleration[1] = imuEvent.acceleration.y;
	currentSensorData.linearAcceleration[2] = imuEvent.acceleration.z;

	// Orientation is given as heading roll pitch, rather than as rotations around x y z axis
	// As such
	//	x := pitch
	//	y:= roll
	//	z:= heading
	// We then also need to remap to have the correct ranges of angles
	imuDevice.getEvent(&imuEvent, Adafruit_BNO055::VECTOR_EULER);
	currentSensorData.orientation[0] = imuEvent.orientation.z * DEG_TO_RAD;
	currentSensorData.orientation[1] = imuEvent.orientation.y * DEG_TO_RAD;
	currentSensorData.orientation[2] = imuEvent.orientation.x * DEG_TO_RAD;
	// currentSensorData.orientation[0] = HALF_PI + (imuEvent.orientation.z * DEG_TO_RAD);
	// currentSensorData.orientation[1] = (imuEvent.orientation.y * DEG_TO_RAD) - PI;
	// currentSensorData.orientation[2] = (imuEvent.orientation.x * DEG_TO_RAD) - PI;

	// if (currentSensorData.orientation[0] < 0)
	// {
	// 	currentSensorData.orientation[0] = -currentSensorData.orientation[0];
	// 	currentSensorData.orientation[1] += PI;
	// 	currentSensorData.orientation[2] += PI;
	// }

	// if (currentSensorData.orientation[0] > PI)
	// {
	// 	currentSensorData.orientation[0] = TWO_PI - currentSensorData.orientation[0];
	// 	currentSensorData.orientation[1] += PI;
	// 	currentSensorData.orientation[2] += PI;
	// }

	// currentSensorData.orientation[1] -= floor (currentSensorData.orientation[1] / TWO_PI) * TWO_PI;
	// currentSensorData.orientation[1] -= PI;
	// currentSensorData.orientation[1] = -currentSensorData.orientation[1];

	// currentSensorData.orientation[2] -= floor (currentSensorData.orientation[2] / TWO_PI) * TWO_PI;
	// currentSensorData.orientation[2] -= PI;

	return true;
}

bool barometerRead()
{
	if (!barometerDevice.performReading()) return false;

	currentSensorData.pressure = barometerDevice.readPressure ();
	currentSensorData.temperature = barometerDevice.readTemperature();

	return true;
}

bool isFile()
{
	return fileName != NULL;
}

void startNewFile()
{
	if (!sdCardInitialised) initSD();
	if (sdCardInitialised)
	{
		findNextFileName();

		// Make sure there is a Data folder
		if (!sdFile.open(STORAGE_DATA_DIRECTORY, O_CREAT))
		{
			Serial.print("Failed to open directory with error: ");
			Serial.println(sdFile.getError());
			sdCardInitialised = false;
			return;
		}
		else
		{
			sdFile.close();
		}

		if (!sdFile.open(fileName, O_APPEND | O_RDWR | O_CREAT))
		{
			Serial.print("Failed to open file with error: ");
			Serial.println(sdFile.getError());
			sdCardInitialised = false;
			return;
		}
		else
		{
			currentSensorData.saveHeader(&sdFile);
			sdFile.close();
		}

		Serial.print("Created new file: ");
		Serial.println(fileName);
	}
}

void findNextFileName()
{
	if (fileName == NULL) fileName = (char*)malloc(STORAGE_FILE_NAME_LENGTH);
	memcpy(fileName, STORAGE_FILE_NAME_FORMAT, STORAGE_FILE_NAME_LENGTH);

	for (int i = 0; i <= STORAGE_FILE_NAME_MAX_NUMBER; i++)
	{
		setFileName(i);
		if (!sdio.exists(fileName)) return;
	}

	Serial.println("Reached maximum number of files");
}

void setFileName(uint16_t value)
{
	fileName[STORAGE_FILE_NAME_NUMBER_START + 0] = getNumbersCharacter((value / 1000) % 10);
	fileName[STORAGE_FILE_NAME_NUMBER_START + 1] = getNumbersCharacter((value / 100) % 10);
	fileName[STORAGE_FILE_NAME_NUMBER_START + 2] = getNumbersCharacter((value / 10) % 10);
	fileName[STORAGE_FILE_NAME_NUMBER_START + 3] = getNumbersCharacter((value / 1) % 10);
}

char getNumbersCharacter(uint8_t value)
{
	if (value < 10)
		return value + 48;
	else
		return '-';
}

void initSD()
{
	sdio.chvol();
	if (sdio.begin(sdConfig))
	{
		sdCardInitialised = true;
	}
	else
	{
		sdCardInitialised = false;
		Serial.println("No SD card found");
		return;
	}
}

bool saveCurrentDataRecord()
{
	//initSD ();
	if (sdCardInitialised)
	{
		if (!isFile()) startNewFile();

		if (!sdFile.isOpen())
		{
			if (!sdFile.open(fileName, O_APPEND | O_RDWR | O_CREAT))
			{
				Serial.print("Failed to open file with error: ");
				Serial.println(sdFile.getError());
				sdCardInitialised = false;
				return false;
			}
		}

		currentSensorData.saveData(&sdFile);
		sdFile.close();

		return true;
	}
	else
	{
		return false;
	}
}