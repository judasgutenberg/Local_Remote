void compileAndSendDeviceData(const String& weatherData, const String& whereWhenData, const String& powerData, bool doPinCursorChanges, uint16_t fRAMOrdinal);
void startWeatherSensors(int sensorIdLocal, int sensorSubTypeLocal, int i2c, int pinNumber, int powerPin);
void cleanup();

 
void sendIr(String rawDataStr);
void serialSwap(int8_t value);

String weatherDataStringFromConfigString(char * sensorData, int sensorOrdinal);

void stopWebSocket();
void startWebSocket(int origin);
int initSerialParser();

 

String weatherDataString(int sensorId, int sensorSubtype, int dataPin, int powerPin, int i2c, int deviceFeatureId, char objectCursor, String sensorName,  int* ordinalOfOverwrite, int consolidateAllSensorsToOneRecord);

void rtcMarkStable();

void rebootEsp();
int freeMemory();



void notYetDeferred(const char * commandText, int commandId, int commandType);
void saveCommandState(uint32_t lastCommandLogId, uint16_t version, int32_t commandId, int32_t commandType);
bool commandRequiresDeferment(String input) ;
bool parseCommand(const String& input, const String& command, String* results, int& resultCount, int maxResults);
int32_t loadCommandStateVersion(int dataToReturn) ;
