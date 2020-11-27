/*!
 * @file Adafruit_SSD1362.h
 *
 * This is part of for Adafruit's SSD1362 library for monochrome
 * OLED displays: http://www.adafruit.com/category/63_98
 *
 * These displays use I2C or SPI to communicate. I2C requires 2 pins
 * (SCL+SDA) and optionally a RESET pin. SPI requires 4 pins (MOSI, SCK,
 * select, data/command) and optionally a reset pin. Hardware SPI or
 * 'bitbang' software SPI are both supported.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries, with
 * contributions from the open source community.
 *
 * BSD license, all text above, and the splash screen header file,
 * must be included in any redistribution.
 *
 */

#ifndef _Adafruit_SSD1362_H_
#define _Adafruit_SSD1362_H_

// ONE of the following three lines must be #defined:
#define SSD1362_256_64
//#define SSD1362_128_64 ///< DEPRECTAED: old way to specify 128x64 screen
//#define SSD1362_128_32 ///< DEPRECATED: old way to specify 128x32 screen
//#define SSD1362_96_16  ///< DEPRECATED: old way to specify 96x16 screen
// This establishes the screen dimensions in old Adafruit_SSD1362 sketches
// (NEW CODE SHOULD IGNORE THIS, USE THE CONSTRUCTORS THAT ACCEPT WIDTH
// AND HEIGHT ARGUMENTS).

#if defined(ARDUINO_STM32_FEATHER)
typedef class HardwareSPI SPIClass;
#endif

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>

#if defined(__AVR__)
typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;
#define HAVE_PORTREG
#elif defined(__SAM3X8E__)
typedef volatile RwReg PortReg;
typedef uint32_t PortMask;
#define HAVE_PORTREG
#elif (defined(__arm__) || defined(ARDUINO_FEATHER52)) &&                      \
    !defined(ARDUINO_ARCH_MBED)
typedef volatile uint32_t PortReg;
typedef uint32_t PortMask;
#define HAVE_PORTREG
#endif

/// The following "raw" color names are kept for backwards client compatability
/// They can be disabled by predefining this macro before including the Adafruit
/// header client code will then need to be modified to use the scoped enum
/// values directly
#ifndef NO_ADAFRUIT_SSD1362_COLOR_COMPATIBILITY
#define BLACK SSD1362_BLACK     ///< Draw 'off' pixels
#define WHITE SSD1362_WHITE     ///< Draw 'on' pixels
#define INVERSE SSD1362_INVERSE ///< Invert pixels
#endif
/// fit into the SSD1362_ naming scheme
#define SSD1362_BLACK 0   ///< Draw 'off' pixels
#define SSD1362_WHITE 255   ///< Draw 'on' pixels
#define SSD1362_INVERSE 2 ///< Invert pixels

#define SSD1362_COMMANDLOCK 0xFD
#define SSD1362_VDDREGULATOR 0xAB
#define SSD1362_VDDIREF 0xAD
#define SSD1362_PHASELENGTH 0xB1
#define SSD1362_CLOCKDIVIDER 0xB3
#define SSD1362_PRECHARGEPERIOD 0xB6
#define SSD1362_LinearLUT 0xB9
#define SSD1362_PRECHARGEVOLTAGE 0xBC
#define SSD1362_PRECHARGECAP 0xBD
#define SSD1362_COMVOLTAGE 0xBE

#define SSD1362_COLUMNADDR 0x15
#define SSD1362_PAGEADDR 0x75
#define SSD1362_SETCONTRAST 0x81
#define SSD1362_SEGREMAP 0xA0
#define SSD1362_NORMALDISPLAY 0xA4
#define SSD1362_DISPLAYALLON 0xA5        
#define SSD1362_DISPLAYALLOFF 0xA6
#define SSD1362_INVERTDISPLAY 0xA7       
#define SSD1362_SETMULTIPLEX 0xA8
#define SSD1362_DISPLAYOFF 0xAE
#define SSD1362_DISPLAYON 0xAF
#define SSD1362_COMSCANINC 0xC0          ///< Not currently used
#define SSD1362_COMSCANDEC 0xC8          ///< See datasheet
#define SSD1362_SETDISPLAYOFFSET 0xA2
#define SSD1362_SETDISPLAYCLOCKDIV 0xB3
#define SSD1362_SETCOMPINS 0xDA          ///< See datasheet
#define SSD1362_SETVCOMDETECT 0xDB       ///< See datasheet

#define SSD1362_SETLOWCOLUMN 0x00  ///< Not currently used
#define SSD1362_SETHIGHCOLUMN 0x10 ///< Not currently used
#define SSD1362_SETSTARTLINE 0xA1

#define SSD1362_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1362_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

#define SSD1362_RIGHT_HORIZONTAL_SCROLL 0x26              ///< Init rt scroll
#define SSD1362_LEFT_HORIZONTAL_SCROLL 0x27               ///< Init left scroll
#define SSD1362_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1362_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A  ///< Init diag scroll
#define SSD1362_DEACTIVATE_SCROLL 0x2E                    ///< Stop scroll
#define SSD1362_ACTIVATE_SCROLL 0x2F                      ///< Start scroll
#define SSD1362_SET_VERTICAL_SCROLL_AREA 0xA3             ///< Set scroll range

// #define SSD1362_MEMORYMODE 0x20 ///< See datasheet
// #define SSD1362_COLUMNADDR 0x21 ///< See datasheet
// #define SSD1362_PAGEADDR 0x22 ///< See datasheet
// #define SSD1362_SETCONTRAST 0x81 ///< See datasheet
// #define SSD1362_CHARGEPUMP 0x8D ///< See datasheet
// #define SSD1362_SEGREMAP 0xA0 ///< See datasheet
// #define SSD1362_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
// #define SSD1362_DISPLAYALLON 0xA5 ///< Not currently used
// #define SSD1362_NORMALDISPLAY 0xA6 ///< See datasheet
// #define SSD1362_INVERTDISPLAY 0xA7 ///< See datasheet
// #define SSD1362_SETMULTIPLEX 0xA8 ///< See datasheet
// #define SSD1362_DISPLAYOFF 0xAE ///< See datasheet
// #define SSD1362_DISPLAYON 0xAF ///< See datasheet
// #define SSD1362_COMSCANINC 0xC0 ///< Not currently used
// #define SSD1362_COMSCANDEC 0xC8 ///< See datasheet
// #define SSD1362_SETDISPLAYOFFSET 0xD3 ///< See datasheet
// #define SSD1362_SETDISPLAYCLOCKDIV 0xD5 ///< See datasheet
// #define SSD1362_SETPRECHARGE 0xD9 ///< See datasheet
// #define SSD1362_SETCOMPINS 0xDA ///< See datasheet
// #define SSD1362_SETVCOMDETECT 0xDB ///< See datasheet

// #define SSD1362_SETLOWCOLUMN 0x00 ///< Not currently used
// #define SSD1362_SETHIGHCOLUMN 0x10 ///< Not currently used
// #define SSD1362_SETSTARTLINE 0x40 ///< See datasheet

// #define SSD1362_EXTERNALVCC 0x01 ///< External display voltage source
// #define SSD1362_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

// #define SSD1362_RIGHT_HORIZONTAL_SCROLL 0x26 ///< Init rt scroll
// #define SSD1362_LEFT_HORIZONTAL_SCROLL 0x27 ///< Init left scroll
// #define SSD1362_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
// #define SSD1362_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A ///< Init diag scroll
// #define SSD1362_DEACTIVATE_SCROLL 0x2E ///< Stop scroll
// #define SSD1362_ACTIVATE_SCROLL 0x2F ///< Start scroll
// #define SSD1362_SET_VERTICAL_SCROLL_AREA 0xA3 ///< Set scroll range

// Deprecated size stuff for backwards compatibility with old sketches
#if defined SSD1362_256_64
#define SSD1362_LCDWIDTH 256 ///< DEPRECATED: width w/SSD1362_128_64 defined
#define SSD1362_LCDHEIGHT 64 ///< DEPRECATED: height w/SSD1362_128_64 defined
#endif
#if defined SSD1362_128_64
#define SSD1362_LCDWIDTH 128 ///< DEPRECATED: width w/SSD1362_128_64 defined
#define SSD1362_LCDHEIGHT 64 ///< DEPRECATED: height w/SSD1362_128_64 defined
#endif
#if defined SSD1362_128_32
#define SSD1362_LCDWIDTH 128 ///< DEPRECATED: width w/SSD1362_128_32 defined
#define SSD1362_LCDHEIGHT 32 ///< DEPRECATED: height w/SSD1362_128_32 defined
#endif
#if defined SSD1362_96_16
#define SSD1362_LCDWIDTH 96  ///< DEPRECATED: width w/SSD1362_96_16 defined
#define SSD1362_LCDHEIGHT 16 ///< DEPRECATED: height w/SSD1362_96_16 defined
#endif

/*!
    @brief  Class that stores state and functions for interacting with
            SSD1362 OLED displays.
*/
class Adafruit_SSD1362 : public Adafruit_GFX // GFXcanvas8
{
public:
  // NEW CONSTRUCTORS -- recommended for new projects
	Adafruit_SSD1362 (uint16_t w, uint16_t h, TwoWire* twi = &Wire, int8_t rst_pin = -1, uint32_t clkDuring = 400000UL, uint32_t clkAfter = 100000UL);
	Adafruit_SSD1362 (uint16_t w, uint16_t h, int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin);
	Adafruit_SSD1362 (uint16_t w, uint16_t h, SPIClass* spi, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin, uint32_t bitrate = 8000000UL);

	// DEPRECATED CONSTRUCTORS - for back compatibility, avoid in new projects
	Adafruit_SSD1362 (int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin);
	Adafruit_SSD1362 (int8_t dc_pin, int8_t rst_pin, int8_t cs_pin);
	Adafruit_SSD1362 (int8_t rst_pin = -1);

	~Adafruit_SSD1362 (void);

	bool begin (uint8_t switchvcc = SSD1362_SWITCHCAPVCC, uint8_t i2caddr = 0, bool reset = true, bool periphBegin = true);
	void display (void);
	void clearDisplay (void);
	void invertDisplay (bool i);
	void dim (bool dim);
	void drawPixel (int16_t x, int16_t y, uint16_t color);
	virtual void drawFastHLine (int16_t x, int16_t y, int16_t w, int16_t color);
	virtual void drawFastVLine (int16_t x, int16_t y, int16_t h, int16_t color);
	void startscrollright (uint8_t start, uint8_t stop);
	void startscrollleft (uint8_t start, uint8_t stop);
	void startscrolldiagright (uint8_t start, uint8_t stop);
	void startscrolldiagleft (uint8_t start, uint8_t stop);
	void stopscroll (void);
	void ssd1362_command (uint8_t c);
	bool getPixel (int16_t x, int16_t y);
	uint8_t* getBuffer (void);

private:
  inline void SPIwrite(uint8_t d) __attribute__((always_inline));
  void drawFastHLineInternal (int16_t x, int16_t y, int16_t w, int16_t color);
  void drawFastVLineInternal (int16_t x, int16_t y, int16_t h, int16_t color);
  void ssd1362_command1(uint8_t c);
  void ssd1362_commandList(const uint8_t *c, uint8_t n);

  SPIClass *spi;
  TwoWire *wire;
  uint8_t *buffer;
  int8_t i2caddr, vccstate, page_end;
  int8_t mosiPin, clkPin, dcPin, csPin, rstPin;
#ifdef HAVE_PORTREG
  PortReg *mosiPort, *clkPort, *dcPort, *csPort;
  PortMask mosiPinMask, clkPinMask, dcPinMask, csPinMask;
#endif
#if ARDUINO >= 157
  uint32_t wireClk;    // Wire speed for SSD1362 transfers
  uint32_t restoreClk; // Wire speed following SSD1362 transfers
#endif
  uint8_t contrast; // normal contrast setting for this device
#if defined(SPI_HAS_TRANSACTION)
protected:
  // Allow sub-class to change
  SPISettings spiSettings;
#endif
};

#endif // _Adafruit_SSD1362_H_
