
const char* ssid = "xxxxxx"; //mine was Moxee Hotspot83_2.4G
const char* password = "xxxx";
const char* storagePassword = "xxxx"; //to ensure someone doesn't store bogus data on your server. should match value in the storage_password column in you user table for your user
//data posted to remote server so we can keep a historical record
//url will be in the form: http://your-server.com:80/weather/data.php?data=
const char* urlGet = "/readLocalData";
const char* hostGet = "192.168.2.42";
const int sensorType = 0; //2301;//2301;//680; //SENSORS! -- we support these: 75 for LM75, 85 for BMP085, 180 for BMP180, 2301 for DHT 2301, 680 for BME680.  0 for no sensor.  No support for multiple sensors for now.
const int locationId = 11; //3 really is watchdog
const int pollingGranularity = 4; //how often to poll backend in seconds, 4 makes sense
const int dataLoggingGranularity = 300; //how often to store data in backend, 300 makes sense
const int connectionFailureRetrySeconds = 4;
const int connectionRetryNumber = 22;
const int hiatusLengthOfUiUpdatesAfterUserInteraction = 35;
const int granularityWhenInConnectionFailureMode = 5; //40 was too little time for everything to come up and start working reliably, at least with my sketchy cellular connection
 