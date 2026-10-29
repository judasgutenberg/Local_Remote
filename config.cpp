#include "config.h"
#include "globals.h"

char* cs[CONFIG_STRING_COUNT]; //used for string configurations
int ci[CONFIG_TOTAL_COUNT]; //used for int configurations

 
void initMasterDefaults() {
  cs[STORAGE_PASSWORD] = "your_secret";  // must match storage_password in backend
  cs[URL_GET] = "/weather/server.php";
  cs[HOST_GET] = "yoursite.com";
  cs[ENCRYPTION_SCHEME] = "9876543210ABCDEF"; // 16-char hex string
 
  cs[SENSOR_CONFIG_STRING] = "";//"0*-1*5273*0*NULL*NULL*propane level*0*15*-1*-1*-1*1";
  cs[PINS_TO_START_LOW] = ""; //comma-delimited integers of pins to start low
  cs[WIFI_SSID] = "your_ssd1";
  cs[WIFI_PASSWORD] = "";
  cs[WIFI_SSID_1] = "your_ssd2";
  cs[WIFI_PASSWORD_1] = "12341234";

  cs[WIFI_SSID_2] = "your_ssd3";
  cs[WIFI_PASSWORD_2] = "";
  
  ci[SENSOR_ID] = 280;          //if we have an onboard sensor, which we might decide to support
  ci[SENSOR_I2C] = 0x76;
  ci[BACKLIGHT_TIMEOUT] = 100;
  ci[DEVICE_ID] = 11;
  ci[POLLING_GRANULARITY] = 2;
  ci[DATA_LOGGING_GRANULARITY] = 300;
  ci[OFFLINE_LOG_GRANULARITY] = 100;
  ci[WIFI_TIMEOUT] = 100;
  ci[OFFLINE_RECONNECT_INTERVAL] = 20;
  
  ci[REBOOT_POLLING_TIMEOUT] = 80;
  ci[ALLOWANCE_FOR_BOOT] = 70;
  ci[CONNECTION_FAILURE_RETRY_SECONDS] = 4;
  ci[CONNECTION_RETRY_NUMBER] = 22;
  
  ci[WEATHER_DEVICE] = 3;
  ci[CONTROL_DEVICE] = 17;
  
  ci[UI_HIATUS] = 35;
  ci[WEATHER_UPDATE_INTERVAL] = 67;
  ci[ENERGY_UPDATE_INTERVAL] = 116;
  
  ci[TEMPERATURE_DELTA_REPONSIVENESS] = 2; //these are in decidegrees
  ci[PRESSURE_DELTA_REPONSIVENESS] = 5;   //these are in decipascals or whatevs
  ci[HUMIDITY_DELTA_REPONSIVENESS] = 3;   //these are in decipercents
 
  ci[DEBUG] = 1;
}

 
