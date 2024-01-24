
/************************************************************************************************* 
                                      PROGRAMMINFO
************************************************************************************************** 
Funktion: Testprogramm für CLOUDYA: NEO-Pixel-Ring - OLED - Buzzer - TM1637
**************************************************************************************************
Version: 05.12.2022
**************************************************************************************************
Board: ES32 UNO ioT V1.0.4
**************************************************************************************************
C++ Arduino IDE V1.8.19
**************************************************************************************************
Einstellungen:
https://dl.espressif.com/dl/package_esp32_index.json
http://dan.drown.org/stm32duino/package_STM32duino_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
**************************************************************************************************
Librarys
- Adafruit_NeoPixel.h V 1.7.0
**************************************************************************************************

 **************************************************************************************************/

 
 //----------TM1637------------------------------
#include <TM1637Display.h>
#define CLK 18
#define DIO 5
TM1637Display display = TM1637Display(CLK, DIO);
const uint8_t test1[] = {
  SEG_A | SEG_B | SEG_C | SEG_D |  SEG_G ,    // 3
  SEG_A | SEG_B | SEG_C | SEG_D |  SEG_G ,    // 3  
  SEG_A | SEG_B | SEG_C | SEG_D |  SEG_G ,    // 3   
  SEG_A | SEG_B | SEG_C | SEG_D |  SEG_G ,    // 3    
};
const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
  };
#define TEST_DELAY   1000
//----------------------------------------------------------

 
//-----OLED-----------------------
#include <Arduino.h>
#include <U8g2lib.h>
//----------------------------

//------------NEOPIXEL ----------------------------------------------
  #include <Adafruit_NeoPixel.h>
  #ifdef __AVR__
    #include <avr/power.h>
  #endif
  
  #define NUMPIXELS 20
  #define PIN 14
  
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//----------------------------------------------------------


// -----Buzzer----------------------------------
#define BUZ 4
int freq = 2000;
int channel = 0;
int resolution = 8;
//------------------------------------------------

//---------OLED-------------------------------------------------
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
unsigned long delayTime;


  int BildschirmBreite = u8g2.getDisplayWidth();
  int BildschirmHoehe = u8g2.getDisplayHeight();


  //Bitmap hier einfügen:
  //Smiley XBM erstellt mit GIMP
#define smiley_width 64
#define smiley_height 64
  static unsigned char smiley[] = {
    0x00, 0x00, 0x00, 0xd0, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
    0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x07, 0x00, 0x00,
    0x00, 0x00, 0xfc, 0x5f, 0xfd, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
    0x80, 0x7f, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00, 0x00, 0xfc, 0x01, 0x00,
    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xf0, 0x01, 0x00,
    0x00, 0xc0, 0x0f, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00,
    0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1e, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0x00, 0x00, 0x1f, 0x00, 0x03, 0x80, 0x01, 0xf8, 0x00,
    0x80, 0x07, 0x80, 0x03, 0xc0, 0x03, 0xf0, 0x00, 0xc0, 0x07, 0x80, 0x07,
    0xc0, 0x03, 0xe0, 0x01, 0xc0, 0x03, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x03,
    0xe0, 0x01, 0xc0, 0x07, 0xe0, 0x07, 0x80, 0x07, 0xe0, 0x00, 0xc0, 0x0f,
    0xe0, 0x07, 0x80, 0x07, 0xf0, 0x00, 0xc0, 0x0f, 0xe0, 0x07, 0x00, 0x0f,
    0x70, 0x00, 0xc0, 0x0f, 0xe0, 0x07, 0x00, 0x0e, 0x78, 0x00, 0xc0, 0x0f,
    0xe0, 0x07, 0x00, 0x1e, 0x38, 0x00, 0xe0, 0x0f, 0xe0, 0x07, 0x00, 0x1e,
    0x3c, 0x00, 0xc0, 0x0f, 0xe0, 0x07, 0x00, 0x1c, 0x1c, 0x00, 0xc0, 0x07,
    0xe0, 0x07, 0x00, 0x3c, 0x1c, 0x00, 0xc0, 0x07, 0xe0, 0x07, 0x00, 0x38,
    0x1c, 0x00, 0xc0, 0x07, 0xc0, 0x03, 0x00, 0x38, 0x1e, 0x00, 0x80, 0x07,
    0xc0, 0x03, 0x00, 0x38, 0x1e, 0x00, 0x80, 0x03, 0x80, 0x01, 0x00, 0x38,
    0x0e, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x78, 0x0e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x38, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78,
    0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x78, 0x0e, 0x7c, 0x00, 0x00,
    0x00, 0x00, 0x3e, 0x70, 0x0e, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x18, 0x38,
    0x1e, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x78, 0x1e, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x38, 0x1e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x08, 0x38,
    0x1c, 0x20, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x38, 0x1c, 0x20, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x38, 0x1c, 0x60, 0x00, 0x00, 0x00, 0x00, 0x06, 0x3c,
    0x3c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1c, 0x38, 0xc0, 0x00, 0x00,
    0x00, 0x00, 0x03, 0x1c, 0x78, 0x80, 0x01, 0x00, 0x00, 0x00, 0x01, 0x1e,
    0x78, 0x00, 0x01, 0x00, 0x00, 0x80, 0x01, 0x0e, 0xf0, 0x00, 0x07, 0x00,
    0x00, 0xc0, 0x00, 0x0f, 0xf0, 0x00, 0x04, 0x00, 0x00, 0x60, 0x00, 0x07,
    0xe0, 0x01, 0x1c, 0x00, 0x00, 0x38, 0x80, 0x07, 0xc0, 0x03, 0x70, 0x00,
    0x00, 0x0c, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x01, 0x00, 0x07, 0xe0, 0x03,
    0x80, 0x07, 0x80, 0x17, 0xf0, 0x01, 0xe0, 0x01, 0x00, 0x0f, 0x00, 0xfc,
    0x3f, 0x00, 0xf0, 0x00, 0x00, 0x1f, 0x00, 0x40, 0x01, 0x00, 0x78, 0x00,
    0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0xfc, 0x00, 0x00,
    0x00, 0x00, 0x1f, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00, 0x80, 0x0f, 0x00,
    0x00, 0xe0, 0x07, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xc0, 0x1f, 0x00,
    0x00, 0xf8, 0x03, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0x2f, 0xf4, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff,
    0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xfa, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };
//---------------------------------------------
  
void setup() {
  
    //-------Buzzer--------------
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(BUZ, channel);
//---------------------------------

  
  //---NEOPIXEL-------------------------------------------
  strip.begin();
  strip.setBrightness(50);
//  strip.show();

//-----OLED----------------------------------------
  u8g2.begin();
  u8g2.setFont(u8g2_font_courR10_tf);
  u8g2.firstPage();

  do {
    u8g2.clearBuffer();
    u8g2.drawXBM(32, 1, smiley_width, smiley_height, smiley);
  } while ( u8g2.nextPage() );
  delay(1000);
  //-----------------------------------------------


  //----------Buzzer-------------------------------------
    ledcWrite(channel, 150);
    ledcWriteTone(channel, 4000);
    delay(200);
    ledcWriteTone(channel, 0);
    delay(200);
    ledcWriteTone(channel, 4000);
    delay(200);
    ledcWriteTone(channel, 0);
    delay(10);
  //------------------------------------------------------

    //--------NEOPIXEL------------------------------
  colorWipe(strip.Color(255, 0, 0), 50); // Rot
  colorWipe(strip.Color(0, 255, 0), 50); // Gruen
  colorWipe(strip.Color(0, 0, 255), 50); // Blau
//-----------------------------------------------

}

void loop() {
    display.clear();
    display.setBrightness(1);

  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

  display.clear();
  // All segments on
  display.setSegments(data);
  delay(TEST_DELAY);

  // Selectively set different digits
  data[0] = display.encodeDigit(8);
  data[1] = display.encodeDigit(0);
  data[2] = display.encodeDigit(8);
  data[3] = display.encodeDigit(0);
  display.setSegments(data);
  delay(TEST_DELAY);
     display.setBrightness(7); 
  data[0] = display.encodeDigit(0);
  data[1] = display.encodeDigit(8);
  data[2] = display.encodeDigit(0);
  data[3] = display.encodeDigit(8);
  display.setSegments(data);
  delay(TEST_DELAY);
    display.setBrightness(1);
  data[0] = display.encodeDigit(8);
  data[1] = display.encodeDigit(0);
  data[2] = display.encodeDigit(8);
  data[3] = display.encodeDigit(0);
  display.setSegments(data);
  delay(TEST_DELAY);

    display.showNumberDec(8, true); // Expect: 0301
  delay(TEST_DELAY);
    display.showNumberDec(80, true); // Expect: 0301
  delay(TEST_DELAY);
    display.showNumberDec(800, true); // Expect: 0301
  delay(TEST_DELAY);
    display.showNumberDec(8000, true); // Expect: 0301
  delay(TEST_DELAY);
    display.clear();
  delay(TEST_DELAY);

    // Done!
  display.setSegments(SEG_DONE);

  while(1);

  
  /*
// -------- TM1637-------------
  display.setBrightness(1);

 for (int i=8888; i <= 9999; i++){
     display.showNumberDec(i, true);  // i=value, true to padd the blank segments with 0
     delay(25);
  };
 //--------------------------------
 */
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }

  }
