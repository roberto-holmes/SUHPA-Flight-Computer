#include "Display.h"

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>

#include "Adafruit_SSD1362.h"
#include "bitmaps.h"

#define XY_PLOT_CENTER_X 171
#define XY_PLOT_CENTER_Y 40
#define XY_PLOT_SIZE 18
#define XY_PLOT_TAIL_MERGE_DELAY 100

#define BAR_POSITION_CENTER_X 75
#define BAR_POSITION_HORIZ_Y 25
#define BAR_POSITION_VERT_Y 45
#define BAR_HEIGHT 13
#define BAR_WIDTH 60
#define BAR_TRIM_WIDTH 11
#define BAR_COLOUR SSD1362_WHITE / 2

#define SCREEN_WIDTH 256 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_DC 41
#define OLED_CS 17
#define OLED_RESET 16
#define OLED_FR 20

Adafruit_SSD1362 display (SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_DC, OLED_RESET, OLED_CS);

int Display::xyPlotPositionX[DISPLAY_XY_PLOT_BUFFER_SIZE];
int Display::xyPlotPositionY[DISPLAY_XY_PLOT_BUFFER_SIZE];
int Display::stickPositionBufferHead = 0;
uint32_t Display::mergeTailTime = 0;

void Display::Init ()
{
	pinMode (OLED_FR, INPUT);

	if (!display.begin (SSD1362_EXTERNALVCC))
	{
		Serial.println (F ("SSD1362 allocation failed"));
		for (;;)
			; // Don't proceed, loop forever
	}

	display.clearDisplay ();
	display.drawBitmap ((display.width () - SUHPALogo_width) / 2, (display.height () - SUHPALogo_height) / 2, SUHPALogo_data, SUHPALogo_width, SUHPALogo_height, SSD1362_WHITE);
	display.display ();
	delay (500);

	for (int i = 0; i < DISPLAY_XY_PLOT_BUFFER_SIZE; i++)
	{
		xyPlotPositionX[i] = XY_PLOT_CENTER_X;
		xyPlotPositionY[i] = XY_PLOT_CENTER_Y;
	}

	mergeTailTime = millis () + XY_PLOT_TAIL_MERGE_DELAY;
}

void Display::Update (int vertVal, int horizVal, int horizMinVal, int horizMaxVal, int vertMinVal, int vertMaxVal)
{
	display.clearDisplay ();
	display.drawGrayscaleBitmap (0, 0, baseUI_data, baseUI_width, baseUI_height);

	// int vertVal = 0;
	// int horizVal = 0;

	float horizValPercent = constrain ((float)(horizVal - ((horizMaxVal + horizMinVal) / 2)) * 2.0 / (float)(horizMaxVal - horizMinVal), -1.0, 1.0);
	float vertValPercent = constrain ((float)(vertVal - ((vertMaxVal + vertMinVal) / 2)) * 2.0 / (float)(vertMaxVal - vertMinVal), -1.0, 1.0);

	int horizBarValue = (int)(horizValPercent * BAR_WIDTH);
	int vertBarValue = (int)(vertValPercent * BAR_WIDTH);

	for (int y = 0; y < BAR_HEIGHT; y++)
	{
		if (horizBarValue > 0)
		{
			for (int x = 0; x < horizBarValue; x++) display.drawPixel (BAR_POSITION_CENTER_X + 1 + x, BAR_POSITION_HORIZ_Y + y, BAR_COLOUR);
		}
		else if (horizBarValue < 0)
		{
			for (int x = 0; x < -horizBarValue; x++) display.drawPixel (BAR_POSITION_CENTER_X - 1 - x, BAR_POSITION_HORIZ_Y + y, BAR_COLOUR);
		}

		if (vertBarValue > 0)
		{
			for (int x = 0; x < vertBarValue; x++) display.drawPixel (BAR_POSITION_CENTER_X + 1 + x, BAR_POSITION_VERT_Y + y, BAR_COLOUR);
		}
		else if (vertBarValue < 0)
		{
			for (int x = 0; x < -vertBarValue; x++) display.drawPixel (BAR_POSITION_CENTER_X - 1 - x, BAR_POSITION_VERT_Y + y, BAR_COLOUR);
		}
	}

	//Final stick position
	int outputX = XY_PLOT_CENTER_X + (int)(horizValPercent * XY_PLOT_SIZE);
	int outputY = XY_PLOT_CENTER_Y - (int)(vertValPercent * XY_PLOT_SIZE);

	for (int i = 1; i < DISPLAY_XY_PLOT_BUFFER_SIZE; i++)
		display.drawLine (
			xyPlotPositionX[(stickPositionBufferHead + i + DISPLAY_XY_PLOT_BUFFER_SIZE - 1) % DISPLAY_XY_PLOT_BUFFER_SIZE],
			xyPlotPositionY[(stickPositionBufferHead + i + DISPLAY_XY_PLOT_BUFFER_SIZE - 1) % DISPLAY_XY_PLOT_BUFFER_SIZE],
			xyPlotPositionX[(stickPositionBufferHead + i) % DISPLAY_XY_PLOT_BUFFER_SIZE],
			xyPlotPositionY[(stickPositionBufferHead + i) % DISPLAY_XY_PLOT_BUFFER_SIZE],
			i * 8 / DISPLAY_XY_PLOT_BUFFER_SIZE);

	int lastIndex = (stickPositionBufferHead + DISPLAY_XY_PLOT_BUFFER_SIZE - 1) % DISPLAY_XY_PLOT_BUFFER_SIZE;
	if (xyPlotPositionX[lastIndex] != outputX || xyPlotPositionY[lastIndex] != outputY || millis () >= mergeTailTime)
	{
		xyPlotPositionX[stickPositionBufferHead] = outputX;
		xyPlotPositionY[stickPositionBufferHead] = outputY;
		stickPositionBufferHead = (stickPositionBufferHead + 1) % DISPLAY_XY_PLOT_BUFFER_SIZE;
		mergeTailTime = millis () + XY_PLOT_TAIL_MERGE_DELAY;
	}

	display.drawPixel (outputX, outputY + 1, SSD1362_WHITE);
	display.drawPixel (outputX + 1, outputY, SSD1362_WHITE);
	display.drawPixel (outputX, outputY, SSD1362_WHITE);
	display.drawPixel (outputX - 1, outputY, SSD1362_WHITE);
	display.drawPixel (outputX, outputY - 1, SSD1362_WHITE);

	while (digitalRead (OLED_FR)) {}
	display.display ();
}