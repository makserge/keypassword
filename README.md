# Password manager based on LILYGO® T-Dongle-S3 ESP32-S3

Setup

1. Copy libraries

libraries/ArduinoJson
libraries/OneButton
libraries/SimpleTimer
libraries/TFT_eSPI
libraries/WiFiManager

to

~/Documents/arduino/libraries

2. Copy libraries/lv_conf.h to ~/Documents/arduino/libraries/TFT_eSPI/

3. In Arduino IDE 2.x select board "Adafruit Feather ESP32-S3 No PSRAM"

4. In Arduino IDE 2.x switch board to USB OTG Mode: 

Tools->USB CDC On Boot->Disabled
Tools->USB Mode->USB-OTG(TinyUSB)
Tools->Partition Scheme->Default

NB: For upload sketch temporary enable USB CDC
Tools->USB CDC On Boot->Enabled

5. Create SPIFFS .

5.1 Copy tool/esp32fs.jar to ~/Documents/arduino/tools/ESP32FS/tool
5.2 Press boot button and reconnect T-Dongle-S3
5.3 Open Arduino 1.x, open sketch folder, select board "Adafruit Feather ESP32-S3 No PSRAM",
select Partition Scheme->Default, correct Port (/dev/cu.usbmodem*) and run Tools -> ESP32 Sketch Data Upload



Usage
1. To apply selected password press button on T-Dongle-S3 once
2. To enable web interface:

6.1 Long press button on T-Dongle-S3
6.2 Connect to Wi-Fi network "KeyPassword"
6.3 Open Web interface http://192.168.4.1/ (on phone captive portal opens automatically after connect to Wi-Fi)
6.4 Once configuration done, press "Exit" to apply settings and reboot T-Dongle-S3
6.5 If nothing changes more than 60s web interface automatically exit, T-Dongle-S3 reboots and new settings will be applied
