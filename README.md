# Cloudya
Internetuhr, Wetterstation und RGB-Cloud

![Bild](pic/claudya.jpg)


## Inbetriebnahme:

  1. Versorge deine Cloudya mit Strom
  2. Warte bis das "Cloudya" WLAN erscheint (Einstellungen, WLAN, Meine Netzwerke)
  3. Verbinde dich mit dem "Cloudya" WLAN und gebe 192.168.4.1 in der Adresszeile deines Webbrowsers ein (KEINE WEB/GOOGLE SUCHE)
  4. Du solltest jetzt ein Webinterface sehen in dem du den Namen und das Passwort deines Heimnetzwerks eingeben kannst
  5. Wenn du jetzt "Speichern" drückst, sollte sich deine Cloudya mit deinem Heimnetzwerk verbinden
  6. Die IP Adresse wird nach dem Reboot im OLED Display und im Seriellen Monitor angezeigt.
  7. Du kannst jetzt das Webinterface zur Steuerung des Lichts etc. aufrufen indem du die IP Adresse deiner Cloudya in der Adresszeile deines Webbrowsers eingibst.


## Elektronik Verbindungen
+ OLED-Display 1,3": VDD an V, GND an G, SCK an SCL, SDA an SDA,
+ 4-stellige LED 0.56 Display: CLK an D18, DIO an D5, 5V an 5V, GND an GND
+ LED-Stripe: Data an D14, 5V an 5V, GND an GND
+ Buzzer Wecken  D4 und GND
+ Buzzer Gas D26 und GND
+ led_rot 25
+ led_gelb 17
+ led_gruen 16
+ DHT11  5V, GND, OUT an IO27
+ Gas Sensor MQ135 IO39 AO an IO39, 5V und GND; MQ135 eichen!


### ESP32 Shield

![Bild](pic/shield.png)


### LED Display

![Bild](pic/c_4digit.png)

  ### DHT11

![Bild](pic/c_dht11.png)

![Bild](pic/c_dht11_0.png)

![Bild](pic/c_dht11_2.png)


  ### LED Stripe

![Bild](pic/c_rgb.jpg)

---
