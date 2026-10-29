#ifndef CONFIG_H
#define CONFIG_H
#include "globals.h"

#define CONFIG_PERSIST_METHOD_FLASH 0
#define CONFIG_PERSIST_METHOD_I2C_SLAVE 1
#define CONFIG_PERSIST_METHOD_FRAM 2
#define TELEMETRY_TYPE_MEMORY 1
#define TELEMETRY_TYPE_TIMING 2

// ===== AUTO-GENERATED CONFIGURATION INDEX DEFINITIONS =====

// String configuration options (0–11)
#define STORAGE_PASSWORD 0
#define URL_GET 1
#define HOST_GET 2
#define ENCRYPTION_SCHEME 3
#define SENSOR_CONFIG_STRING 4
#define PINS_TO_START_LOW 5
#define WIFI_SSID 6
#define WIFI_PASSWORD 7
#define WIFI_SSID_1 8
#define WIFI_PASSWORD_1 9
#define WIFI_SSID_2 10
#define WIFI_PASSWORD_2 11

// Integer configuration options (12+)
#define SENSOR_ID 16
#define SENSOR_I2C 17
#define BACKLIGHT_TIMEOUT 18
#define DEVICE_ID 19
#define POLLING_GRANULARITY 20
#define DATA_LOGGING_GRANULARITY 21
#define OFFLINE_LOG_GRANULARITY 22
#define WIFI_TIMEOUT 23
#define OFFLINE_RECONNECT_INTERVAL 24

#define REBOOT_POLLING_TIMEOUT 25
#define ALLOWANCE_FOR_BOOT 26
#define CONNECTION_FAILURE_RETRY_SECONDS 27
#define CONNECTION_RETRY_NUMBER 28
#define WEATHER_DEVICE  29
#define CONTROL_DEVICE  30

#define UI_HIATUS 31
#define WEATHER_UPDATE_INTERVAL 32
#define ENERGY_UPDATE_INTERVAL 33
#define TEMPERATURE_DELTA_REPONSIVENESS 34
#define PRESSURE_DELTA_REPONSIVENESS  35
#define HUMIDITY_DELTA_REPONSIVENESS  36

#define DEBUG 37


// ==========================================================

#define CONFIG_STRING_COUNT 16
#define CONFIG_TOTAL_COUNT 38

extern char* cs[CONFIG_STRING_COUNT]; // string configuration values
extern int ci[CONFIG_TOTAL_COUNT];    // integer configuration values

void initMasterDefaults();
void initSlaveDefaults();

#endif
