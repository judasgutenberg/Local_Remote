
const char* ssid = "xxxx"; //mine was Moxee Hotspot83_2.4G
const char* password = "xxx";
const char* storagePassword = "xxxx"; //to ensure someone doesn't store bogus data on your server. should match value in the storage_password column in you user table for your user
//data posted to remote server so we can keep a historical record
//url will be in the form: http://your-server.com:80/weather/data.php?data=
const char* dataSourceHost = "xxxx.com";
const int weatherDevice  = 13;
const int controlDevice  = 11; 
 const int locationId = 11; //3 really is watchdog
const int pollingGranularity = 4; //how often to poll backend in seconds, 4 makes sense
const int rebootPollingTimeout = 80; //if no polling in 300 seconds, reboot the ESP
const int connectionFailureRetrySeconds = 4;
const int connectionRetryNumber = 22;
const int allowanceForBoot = 70;
const int hiatusLengthOfUiUpdatesAfterUserInteraction = 35;

const int weatherRecordingInterval = 3600; //across a whole hour!
const int weatherUpdateInterval = 67;
const int energyUpateInterval = 116;

const double temperatureDeltaForChange = 0.2;
const double pressureDeltaForChange = 0.5;
const double humidityDeltaForChange = 0.3;

const int backlightTimeout = 600; //how long the backlight stays lit after we start up or a button is pushed.  the backlight uses 100 times more power than the rest of the LCD module
 
