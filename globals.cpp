#include "globals.h"
#include "config.h"
#include <map>


nonVolatileStruct eepromData;

void ICACHE_RAM_ATTR buttonPushed();
StaticJsonDocument<1000> jsonBuffer;
StaticJsonDocument<2000> deviceJsonBuffer;  //this has to be big for some reason
StaticJsonDocument<1000> energyJsonBuffer;
String deviceName = "Your Device";
String specialUrl = "";
String ourIpAddress;

String controlIpAddress; //stored in EEPROM
String weatherIpAddress; //stored in EEPROM
 
 
byte buttonMode = 13;
byte buttonUp = 15;
byte buttonDown = 12;
byte buttonChange = 14;
byte buttonPins[buttonNumber] = {buttonChange, buttonDown, buttonMode, buttonUp}; //connect to the column pinouts of the keypad left to right: 13, 15, 12, 14
String localCopyOfJson;
long connectionFailureTime = 0;
bool connectionFailureMode = false;
bool allowInterrupts = false;
long timeOutForServerDataUpdates;
long lastTimeButtonPushed = 0;
long oldWeatherRecordingTime = 0;
long backlightOnTime = 0;
byte menuCursor = 0;
byte menuBegin = 0;
byte currentMode = 0;
byte modeCursor = 0;

byte modeDeviceSwitcher = 0;
byte modeWeather = 1;
byte modePower = 2;
extern byte modes[modeCount] = {modeDeviceSwitcher, modeWeather, modePower};
extern long updateTimes[modeCount] = {0,0,0};
extern long connectTimes[modeCount] = {0,0,0};
long lastScreenInit = 0;
signed char totalMenuItems = 1;
char totalScreenLines = 4;
String deviceJson = "";

double temperatureValue = -100;
double humidityValue;
double pressureValue;

double oldTemperatureValue = -100;
double oldHumidityValue;
double oldPressureValue;

int pvPower;
int batPower;
int batPercent = -1000;
int loadPower;

int currentWifiIndex = 0;
bool offlineMode = false;
bool haveReconnected = true;
String ipAddress;
uint8_t outputMode = 0;
String responseBuffer = "";
String fastResponseBuffer = "";
uint8_t oldOutputMode = 0;
uint32_t lastTimeOutputModeChanged = 0;
uint32_t lastTimeSentFastModeCommand = 0;
uint32_t lastTimeCommandRan = 0;
uint32_t lastCommandLogId = 0;
String lastGoodKey = "";
String fileToUpload;
uint32_t fileUploadPosition = 0;
char * deferredCommand = nullptr;

WebSocketsClient webSocket;
WiFiUDP ntpUDP; //i guess i need this for time lookup
NTPClient timeClient(ntpUDP, "pool.ntp.org");

 LiquidCrystal_I2C lcd(0x27,20,totalScreenLines); 
 
