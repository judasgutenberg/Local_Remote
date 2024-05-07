 
  
extern const char* ssid; //mine was Moxee Hotspot83_2.4G
extern const char* password;
extern const char* storagePassword; //to ensure someone doesn't store bogus data on your server. should match value in config.php
//data posted to remote server so we can keep a historical record
//url will be in the form: http://your-server.com:80/weather/data.php?data=
extern const char* dataSourceHost;
extern const int weatherDevice;
extern const int controlDevice; 
extern const int sensorType; //SENSORS! -- we support these: 75 for LM75, 85 for BMP085, 180 for BMP180, 2301 for DHT 2301, 680 for BME680.  0 for no sensor.  No support for multiple sensors for now.
extern const int locationId; //3 really is watchdog
extern const int pollingGranularity; //how often to poll backend in seconds, 4 makes sense
extern const int rebootPollingTimeout;
extern const int connectionFailureRetrySeconds;
extern const int connectionRetryNumber;
extern const  int hiatusLengthOfUiUpdatesAfterUserInteraction;

extern const int weatherRecordingInterval;
extern const int weatherUpdateInterval;
extern const int energyUpateInterval;

extern const double temperatureDeltaForChange;
extern const double pressureDeltaForChange;
extern const double humidityDeltaForChange;

extern const int backlightTimeout;
