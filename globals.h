#ifndef GLOBALS_H
#define GLOBALS_H

#define VERSION 2294
#define RTC_MAGIC 0xDEADCA75

#include <Arduino.h>  
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include <LiquidCrystal_I2C.h>
#include <ESP_EEPROM.h>
#include "config.h"
 

const byte modeCount = 3;
const byte buttonNumber = 4; 

struct nonVolatileStruct {
  char  controlIpAddress[30];
  char  weatherIpAddress[30];
};

extern nonVolatileStruct eepromData;

extern void ICACHE_RAM_ATTR buttonPushed();
extern StaticJsonDocument<1000> jsonBuffer;
extern StaticJsonDocument<2000> deviceJsonBuffer;  //this has to be big for some reason
extern StaticJsonDocument<1000> energyJsonBuffer;
extern String deviceName;
extern String specialUrl;
extern String ourIpAddress;

extern String controlIpAddress; //stored in EEPROM
extern String weatherIpAddress; //stored in EEPROM
 
extern const byte buttonNumber;
extern byte buttonMode;
extern byte buttonUp;
extern byte buttonDown;
extern byte buttonChange;
extern byte buttonPins[buttonNumber]; //connect to the column pinouts of the keypad left to right: 13, 15, 12, 14
extern String localCopyOfJson;
extern long connectionFailureTime;
extern bool connectionFailureMode;
extern bool allowInterrupts;
extern long timeOutForServerDataUpdates;
extern long lastTimeButtonPushed;
extern long oldWeatherRecordingTime;
extern long backlightOnTime;
extern byte menuCursor;
extern byte menuBegin;
extern byte currentMode;
extern byte modeCursor;
extern const byte modeCount;
extern byte modeDeviceSwitcher;
extern byte modeWeather;
extern byte modePower;
extern byte modes[modeCount];
extern long updateTimes[modeCount];
extern long connectTimes[modeCount];
extern long lastScreenInit;
extern signed char totalMenuItems;
extern char totalScreenLines;
extern String deviceJson;

extern double temperatureValue;
extern double humidityValue;
extern double pressureValue;

extern double oldTemperatureValue;
extern double oldHumidityValue;
extern double oldPressureValue;

extern int pvPower;
extern int batPower;
extern int batPercent;
extern int loadPower;

extern int currentWifiIndex;
extern bool offlineMode;
extern bool haveReconnected;
extern String ipAddress;
extern uint8_t outputMode;
extern String responseBuffer;
extern String fastResponseBuffer;
extern uint8_t oldOutputMode;
extern uint32_t lastTimeOutputModeChanged;
extern uint32_t lastTimeSentFastModeCommand;
extern uint32_t lastTimeCommandRan;
extern uint32_t lastCommandLogId;
extern String lastGoodKey;
extern String fileToUpload;
extern uint32_t fileUploadPosition;
extern char * deferredCommand;

extern WebSocketsClient webSocket;
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

extern LiquidCrystal_I2C lcd; 



#endif
