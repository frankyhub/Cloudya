/*************************************************************************************************
                                      PROGRAMMINFO
**************************************************************************************************
                                     Cloudya FP2023
  Funktion: ESP32 WEB Server Internet-Uhr, Wecker, LED-Stripe, 4 Digit Display, DHT11 Raumklima, Wetterdaten vom Tegernsee, Gas-Sensor
  OLED Display 1,3" mit Wochentag, Datum, Uhrzeit, Bewölkung und IP-Adresse, Raumklima, Wetterdaten

**************************************************************************************************
  Version: 01.03.2023   Cloudya A155
  ---------------------------------------------------------------

*************************************************************************************************
  Board: ESP32vn IoT UNO
**************************************************************************************************
  C++ Arduino IDE V1.8.19
**************************************************************************************************
  Einstellungen:
  https://dl.espressif.com/dl/package_esp32_index.json
  http://dan.drown.org/stm32duino/package_STM32duino_index.json
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
**************************************************************************************************
  Inbetriebnahme:

  1. Versorge deine Cloudya mit Strom
  2. Warte bis das "Cloudya" WLAN erscheint (Einstellungen, WLAN, Meine Netzwerke)
  3. Verbinde dich mit dem "Cloudya" WLAN und gebe 192.168.4.1 in der Adresszeile deines Webbrowsers ein (KEINE WEB/GOOGLE SUCHE)
  4. Du solltest jetzt ein Webinterface sehen in dem du den Namen und das Passwort deines Heimnetzwerks eingeben kannst
  5. Wenn du jetzt "Speichern" drückst, sollte sich deine Cloudya mit deinem Heimnetzwerk verbinden
  6. Die IP Adresse wird nach dem Reboot im OLED Display und im Seriellen Monitor angezeigt.
  7. Du kannst jetzt das Webinterface zur Steuerung des Lichts etc. aufrufen indem du die IP Adresse deiner Cloudya in der Adresszeile deines Webbrowsers eingibst.


  ### Electronik Verbindungen
**OLED-Display 1,3": VDD an V, GND an G, SCK an SCL, SDA an SDA,
**4-stellige LED 0.56 Display: CLK an D18, DIO an D5, 5V an 5V, GND an GND
**LED-Strip: Data an D14, 5V an 5V, GND an GND
**Buzzer Wecken  D4 und GND
**Buzzer Gas D26 und GND
** led_rot 25
** led_gelb 17
** led_gruen 16
** DHT11  5V, GND, OUT an IO27
** Gas Sensor MQ135 IO39 AO an IO39, 5V und GND; MQ135 eichen!
 **************************************************************************************************/

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FastLED.h>
#include <SPIFFS.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <time.h>
#include "./src/hardware/LightController.h"
#include "./src/network/MDNSController.h"
#include "./src/AlarmController.h"
#include "./src/PasswordController.h"
#include "./src/hardware/System.h"
#include "./src/Webserver.h"

//------------openWeatherMap--------------------------------
#include <HTTPClient.h>
#include <Arduino_JSON.h>
//---------------------------------------------------------


//------- Gas Sensor MQ135 ------------
int Buzzer = 26;
int Gas_analog = 39;
//int Gas_digital = 2;

#define led_rot 25
#define led_gelb 17
#define led_gruen 16
//----------------------------------------


//----------DHT11------------------------------------------
#include <SimpleDHT.h> //by Winlin 1.0.14       
int pinDHT11 = 27;
SimpleDHT11 dht11(pinDHT11);
//---------------------------------------------------------
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
unsigned long delayTime;

#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variablen
String formattedDate;
String dayStamp;
String timeStamp;
char *name = "Cloudya";
char *hostname = "Cloudya";
char *passwort = "";

// 4-Digit Display
#define CLK 18
#define DIO 5
TM1637Display display = TM1637Display(CLK, DIO);

// Internetzeit
#define MY_NTP_SERVER "de.pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03" // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
time_t now;
tm tm;
void showTime() {
  time(&now);
  localtime_r(&now, &tm); // update the structure tm with the current time
  /* Serial.print("Jahr:");
    Serial.print(tm.tm_year + 1900); // years since 1900
    Serial.print("\tmonth:");
    Serial.print(tm.tm_mon + 1); // January = 0 (!)
    Serial.print("\tday:");
    Serial.print(tm.tm_mday); // day of month
    Serial.print("\thour:");
    Serial.print(tm.tm_hour); // hours since midnight 0-23
    Serial.print("\tmin:");
    Serial.print(tm.tm_min); // minutes after the hour 0-59
    Serial.print("\tsec:");
    Serial.print(tm.tm_sec); // seconds after the minute 0-61*
    Serial.print("\twday");
    Serial.print(tm.tm_wday); // days since Sunday 0-6
  */
  Serial.println();
}


MDNSController mdnsController = MDNSController(hostname, name);
LightController lightController = LightController();
AlarmController alarmController = AlarmController(&lightController);
Webserver webserver = Webserver(80, lightController, &alarmController, &display);
Webserver apWebserver = Webserver(80, lightController, &alarmController, &display);

bool isSetup = false;

struct Button
{
  uint8_t pin;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button buttonRight = {12, 0, false};
Button buttonLeft = {33, 0, false};

void IRAM_ATTR buttonRightIRS()
{
  buttonRight.numberKeyPresses += 1;
  buttonRight.pressed = true;
}

void IRAM_ATTR buttonLeftIRS()
{
  buttonLeft.numberKeyPresses += 1;
  buttonLeft.pressed = true;
}

//----------------OLED Clock----------------------------
// Define NTP Client to get time

//Wochentage
String weekDays[7] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

//Monate
String months[12] = {"Jan", "Feb", "M""\xE4""rz", "Apr", "Mai", "Juni", "Juli", "Aug", "Sept", "Okt", "Nov", "Dez"};
//-----------------------------------------------

//char tag[7] = {'Sonntag', 'Montag ', 'Dienstag', 'Mittwoch', 'Donnerstag', 'Freitag', 'Samstag'};

//--------------Wetter-----------------------------------------------
String openWeatherMapApiKey = "b2bc7fc61c4cc3545e6bbf151b58e1dd";
String city = "Tegernsee";
String countryCode = "DE";

unsigned long lastTime = 0;
// Timer  10 Minuten (600000)
//unsigned long timerDelay = 600000;
// Timer 10 Sekunden (10000)
unsigned long timerDelay = 10000;

String jsonBuffer;
int ktemp;
int mintemp;
int maxtemp;
int sicht;
//------------------------------------------------------------------------


void setup() //-------------------------------------------------------------------------------------------------------------
{
  //------- Gas Sensor MQ135 ------------
  pinMode(Buzzer, OUTPUT);
  //    pinMode(Gas_digital, INPUT);

  pinMode(led_rot, OUTPUT);
  pinMode(led_gruen, OUTPUT);
  pinMode(led_gelb, OUTPUT);
  //----------------------------------------


  u8g2.begin();
  Serial.begin(115200);
  // setup display
  display.setBrightness(0); // Set the display brightness (0-7)!!!!!
  display.clear();          // Clear the display

  System::initFS();

  PasswordController passwordController = PasswordController("/wifi.txt");

  pinMode(buttonRight.pin, INPUT_PULLUP);
  pinMode(buttonLeft.pin, INPUT_PULLUP);
  attachInterrupt(buttonRight.pin, buttonRightIRS, FALLING);
  attachInterrupt(buttonLeft.pin, buttonLeftIRS, FALLING);

  if (!passwordController.isExisting())
  {
    isSetup = true;
  }
  else
  {
    isSetup = !System::connectToWifi(&passwordController);
  }

  if (isSetup)
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(name, passwort);

    mdnsController.setup();

    apWebserver.setupAP();
    apWebserver.begin();
    //    Serial.println(WiFi.softAPIP());
  }

  if (isSetup)
  {
    return;
  }

  // Setup Cloudya domain local network
  mdnsController.setup();

  // Print IP Addresse
  // Serial.println(WiFi.localIP());

  // Init and get the time
  configTzTime(MY_TZ, MY_NTP_SERVER);

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    display.showNumberDec(0);
    return;
  }

  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);

  int displayTime = atoi(timeHour) * 100 + atoi(timeMinute);
  // Serial.println(displayTime);
  display.showNumberDecEx(displayTime, 0b11100000, true); //Display the time value;

  webserver.setup();
  webserver.begin();


  // NTPClient Zeit
  timeClient.begin();
  // timeClient.setTimeOffset(7200);

/*
  //------------ Smiley -------------------------------------------------------
  //Smiley 64x64
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

*/
//Smiley 50x50
#define smiley_width 50
#define smiley_height 50
static unsigned char smiley[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
   0xff, 0x03, 0x00, 0x00, 0x00, 0xc0, 0x07, 0xc0, 0x0f, 0x00, 0x00, 0x00,
   0xf0, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x78, 0x00,
   0x00, 0x00, 0x1c, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00,
   0xc0, 0x01, 0x00, 0x80, 0x07, 0x00, 0x00, 0x80, 0x03, 0x00, 0x80, 0x01,
   0x08, 0xc0, 0x00, 0x07, 0x00, 0xc0, 0x01, 0x1c, 0xc0, 0x00, 0x0e, 0x00,
   0xe0, 0x00, 0x1c, 0xe0, 0x01, 0x0c, 0x00, 0x60, 0x00, 0x1e, 0xe0, 0x01,
   0x18, 0x00, 0x30, 0x00, 0x1e, 0xe0, 0x01, 0x38, 0x00, 0x30, 0x00, 0x3e,
   0xe0, 0x01, 0x30, 0x00, 0x18, 0x00, 0x1e, 0xe0, 0x01, 0x70, 0x00, 0x18,
   0x00, 0x1e, 0xe0, 0x01, 0x60, 0x00, 0x1c, 0x00, 0x1e, 0xe0, 0x01, 0x60,
   0x00, 0x0c, 0x00, 0x1c, 0xe0, 0x01, 0x60, 0x00, 0x0c, 0x00, 0x1c, 0xc0,
   0x01, 0xc0, 0x00, 0x0c, 0x00, 0x1c, 0xc0, 0x00, 0xc0, 0x00, 0x0c, 0x00,
   0x00, 0x80, 0x00, 0xc0, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00,
   0x06, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x0e, 0x04, 0x00, 0x00, 0x80,
   0xc1, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x00, 0xc1, 0x00, 0x0c, 0x04, 0x00,
   0x00, 0x80, 0xc0, 0x00, 0x0c, 0x04, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x0c,
   0x04, 0x00, 0x00, 0x80, 0xc0, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x80, 0xe0,
   0x00, 0x1c, 0x08, 0x00, 0x00, 0x40, 0x60, 0x00, 0x18, 0x08, 0x00, 0x00,
   0x40, 0x60, 0x00, 0x18, 0x10, 0x00, 0x00, 0x20, 0x70, 0x00, 0x38, 0x20,
   0x00, 0x00, 0x10, 0x30, 0x00, 0x30, 0x40, 0x00, 0x00, 0x10, 0x38, 0x00,
   0x70, 0x80, 0x00, 0x00, 0x04, 0x18, 0x00, 0x60, 0x00, 0x01, 0x00, 0x02,
   0x1c, 0x00, 0xe0, 0x00, 0x06, 0x80, 0x01, 0x0c, 0x00, 0xc0, 0x01, 0x70,
   0x30, 0x00, 0x07, 0x00, 0x80, 0x03, 0x80, 0x05, 0x00, 0x07, 0x00, 0x00,
   0x07, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 0x0e, 0x00, 0x00, 0xe0, 0x01,
   0x00, 0x00, 0x3c, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00,
   0x3c, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x80,
   0xbf, 0xf4, 0x07, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x01, 0x00, 0x00,
   0x00, 0x00, 0xd0, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00 };
   



  u8g2.setFont(u8g2_font_courR10_tf);
  u8g2.firstPage();

  do {
    u8g2.clearBuffer();
    u8g2.drawXBM(40, 0, smiley_width, smiley_height, smiley);
        u8g2.setCursor(12, 62);
        u8g2.setFont(u8g2_font_courB08_tf);
        u8g2.print("Ich lade die Daten");

    
  } while ( u8g2.nextPage() );
  delay(3000);

  //----------------------------------------------------------------------------------
//FastLED.setBrightness(100);

} //Ende Setup



void loop()//----------------------------------------------------------------------------------------------------------
{
  //-------------DHT11-----------------------
  Serial.println("=================================");
  Serial.println("Abfrage DHT11 Raumklima:");
  float temperature = 0;
  float humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("DHT11 Error, err=");
    Serial.println(err);
    delay(2000);
    // return; //aktivieren, wenn ein DHT11 aktiv ist
  }
  Serial.print("Sample OK: ");
  Serial.print((float)temperature - 1); //Korrektur -1
  Serial.print(" *C, ");
  Serial.print((float)humidity);
  Serial.println(" relLF%");
  delay(1000); // DHT11 Abfragezeit

  //-------------------------------------------

  //------- Gas Sensor MQ135 ------------------
  //    int gassensorAnalog = analogRead(Gas_analog); //----------------------------------------------------------------aktivieren!!!
  int gassensorAnalog = 311;   //--------------------------------------------------------------------------------- DEMO deaktivieren!!!
  //  int gassensorDigital = digitalRead(Gas_digital);
  delay(100);

  //-------------------------------------------


  //----------------Clock----------------------
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  showTime();
  //  Serial.print("Epoch Time: ");
  //  Serial.println(epochTime);

  String formattedTime = timeClient.getFormattedTime();
  // Serial.print("Formatted Time: ");
  // Serial.println(formattedTime);

  int currentHour = timeClient.getHours();
  char stunden = timeClient.getHours();


  //  Serial.print("Hour: ");
  //  Serial.println(currentHour);

  int currentMinute = timeClient.getMinutes();
  //  Serial.print("Minutes: ");
  //  Serial.println(currentMinute);

  int currentSecond = timeClient.getSeconds();
  //  Serial.print("Seconds: ");
  //  Serial.println(currentSecond);

  String weekDay = weekDays[timeClient.getDay()];
  char tag = timeClient.getDay();
  //  Serial.println("**************** Datum und Uhrzeit ********************");
  //  Serial.println(tag);

  //  Serial.print("Wochentag: ");
  //  Serial.println(weekDay);

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime);

  int monthDay = ptm->tm_mday;
  //  Serial.print("Tag: ");
  //  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon + 1;
  //  Serial.print("Monat: ");
  //  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth - 1];
  //  Serial.print("Monat: ");
  //  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year + 1900;
  //  Serial.print("Jahr: ");
  //  Serial.println(currentYear);

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  //  Serial.print("Datum: ");
  //  Serial.println(currentDate);

  //  Serial.println("");

  //---------------------------------------------------------

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // Eingehendes Format
  // 2018-05-28T16:00:13Z
  // Extrahiere Zeit und Datum
  formattedDate = timeClient.getFormattedDate();
  //  Serial.println(formattedDate);

  // Extrahiere Datum
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  //  Serial.print("Datum: ");
  //  Serial.println(dayStamp);
  // Extrahiere Zeit
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  /*  Serial.print("Uhrzeit: ");
    Serial.print(tm.tm_hour);
    Serial.print(":");
    Serial.println(currentMinute);
  */
  delay(1000);

  if (isSetup)
  {
    lightController.bounce();
    return;
  }

  EVERY_N_MILLIS(600)
  {
    if (buttonRight.pressed && buttonLeft.pressed)
    {
      alarmController.stopAlarm();
    }
    else
    {
      if (buttonRight.pressed)
      {
        alarmController.snooze();
        buttonRight.pressed = false;
      }

      if (buttonLeft.pressed)
      {
        alarmController.snooze();
        buttonLeft.pressed = false;
      }
    }
  }

  EVERY_N_MILLIS(200)
  {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
      display.showNumberDec(0);
      return;
    }

    char timeHour[3];
    strftime(timeHour, 3, "%H", &timeinfo);
    char timeMinute[3];
    strftime(timeMinute, 3, "%M", &timeinfo);
    int displayTime = atoi(timeHour) * 100 + atoi(timeMinute);
    display.showNumberDecEx(displayTime, 0b11100000, true); //Display the time value;
    alarmController.loop(atoi(timeHour), atoi(timeMinute));
  }
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi Status
    if (WiFi.status() == WL_CONNECTED) {
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);

      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Eingabe fehlgeschlagen!");
        return;
      }
      ktemp = (myObject["main"]["temp"]);
      mintemp = (myObject["main"]["temp_min"]);
      maxtemp = (myObject["main"]["temp_max"]);


      //--------- OLED Seite 1 -----------------------
      u8g2.firstPage();
      do {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_courB10_tf);
        u8g2.setCursor(25, 10);
        u8g2.print(weekDay);
        u8g2.setCursor(10, 27);
        u8g2.print(monthDay);
        u8g2.print(". ");
        u8g2.print(currentMonthName);
        u8g2.print(" ");
        u8g2.print(currentYear);
        u8g2.setFont(u8g2_font_courR10_tf);

        u8g2.setCursor(8, 46);
        u8g2.print("Wolken: ");
        u8g2.print(myObject["clouds"]["all"]);
        u8g2.print(" %");

        //------- IP-Adresse --------------
        u8g2.setFont(u8g2_font_courB08_tf);
        u8g2.setCursor(20, 64);
        u8g2.print(WiFi.localIP());


      } while ( u8g2.nextPage() );
      delay(8000);;

      //--------- OLED Seite 2 Wetterdaten -----------------------------
      do {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_courB10_tf);
        u8g2.setCursor(18, 10);
        u8g2.print("Wetterdaten");
        u8g2.setCursor(28, 22);
        u8g2.print("Tegernsee");
        u8g2.setFont(u8g2_font_courR10_tf);


        u8g2.setCursor(8, 35);
        u8g2.print("Temp: ");
        u8g2.print(maxtemp - 273.15, 1);
        u8g2.print("\xB0""C");

        u8g2.setCursor(8, 49);
        u8g2.print("Wind: ");
        //      u8g2.setCursor(51, 49);
        u8g2.print(myObject["wind"]["speed"]);
        u8g2.print("m/s");

        u8g2.setCursor(8, 63);
        u8g2.print("LuDr: ");
        //      u8g2.setCursor(51, 63);
        u8g2.print(myObject["main"]["pressure"]);
        u8g2.print("hPa");

      } while ( u8g2.nextPage() );
      delay(8000);

      //--------- OLED Seite 3 Wetterdaten -----------------------------
      do {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_courB10_tf);
        u8g2.setCursor(18, 10);
        u8g2.print("Wetterdaten");
        u8g2.setCursor(28, 22);
        u8g2.print("Tegernsee");
        u8g2.setFont(u8g2_font_courR10_tf);


        u8g2.setCursor(4, 36);
        u8g2.print("Feuchte: ");
        u8g2.print(myObject["main"]["humidity"]);
        u8g2.print("%");

        u8g2.setCursor(4, 50);
        u8g2.print("Wolken:  ");
        //      u8g2.setCursor(51, 49);
        u8g2.print(myObject["clouds"]["all"]);
        u8g2.print("%");

        u8g2.setCursor(4, 64);
        u8g2.print("minTemp:");
        u8g2.print(mintemp - 273.15, 1);
        u8g2.print("\xB0""C");

      } while ( u8g2.nextPage() );
      delay(8000);


      //--------- OLED Seite 4 Raumklima---------------------
      do {
        u8g2.setFont(u8g2_font_courB10_tf);
        u8g2.clearBuffer();
        u8g2.setCursor(24, 10);
        u8g2.print("Raumklima");
        u8g2.setFont(u8g2_font_courR10_tf);

        u8g2.setCursor(8, 36);
        u8g2.print("Temp: ");
        //        u8g2.setCursor(51, 30);
        u8g2.print(temperature - 1); //mit Korrektur -1°C
        u8g2.print("\xB0""C");

        u8g2.setCursor(8, 60);
        u8g2.print("LuFe: ");
        //        u8g2.setCursor(51, 44);
        u8g2.print(humidity);
        u8g2.print("%");

        u8g2.setCursor(8, 60);
        //       u8g2.print("Gas:  ");                                 // ************************** GAS SENSOR *****************************
        //        u8g2.setCursor(51, 60);
        //       u8g2.print(gassensorAnalog);
        //       u8g2.print("ppm");

      } while ( u8g2.nextPage() );
      //      delay(1000);
      //---------------------------------------------------------
    }

    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }


  //------- Gas Sensor MQ135 ------------
  Serial.print("Gas Sensor: ");
  Serial.println(gassensorAnalog);

  if (gassensorAnalog <= 550)  {
    Serial.println("=================================");
    Serial.println("kein Gas");
    Serial.println("=================================");
    digitalWrite(led_gruen, HIGH);        // OK
  } else {
    digitalWrite(led_gruen, LOW);
  }


  if (gassensorAnalog >= 551 && gassensorAnalog <= 799) {
    digitalWrite(led_gelb, HIGH);
    Serial.println("=================================");
    Serial.println("Gas erkannt");
    Serial.println("=================================");
  } else {
    digitalWrite(led_gelb, LOW);
  }

  if (gassensorAnalog > 800) {
    Serial.println("=================================");
    Serial.println("Gas-Alarm");
    Serial.println("=================================");
    digitalWrite(led_rot, HIGH);         // Gas-Alarm

    digitalWrite (Buzzer, HIGH) ; //Buzzer aktiv

  } else {
    digitalWrite(led_rot, LOW);
    digitalWrite (Buzzer, LOW) ;  //Buzzer inaktiv
  }
  delay(100);
  //-----------------------------------------------------

} //Ende loop

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Domain-Name
  http.begin(client, serverName);

  // HTTP POST request 200=OK
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP-Antwort Code (200=OK): ");
    Serial.println(httpResponseCode);
    Serial.println("Server-Meldung: ");
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();

  return payload;
}
