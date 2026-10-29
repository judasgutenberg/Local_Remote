/* 
Local Remote, Gus Mueller, April 22 2024
Provides a local control-panel for the remote-control system here:
https://github.com/judasgutenberg/Esp8266_RemoteControl
*/

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


#include <LittleFS.h>
#include <NTPClient.h>

#include "commandhandlers.h"
#include "commandregistry.h"
#include "filefunctions.h"

#include "config.h"
#include "globals.h"
#include "utilities.h"



void setup(){
  Serial.begin(115200);
  Serial.println("Starting up Local Remote");
  backlightOn();
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("Starting...");
  wiFiConnect();
 
  enableInterrupts();
 
  EEPROM.begin(sizeof(nonVolatileStruct));
  if(EEPROM.percentUsed()>=0) {
    EEPROM.get(0, eepromData);
    Serial.println("EEPROM has data from a previous run.");
    Serial.print(EEPROM.percentUsed());
    Serial.println("% of ESP flash space currently used");
    controlIpAddress = (String)eepromData.controlIpAddress;
    weatherIpAddress = (String)eepromData.weatherIpAddress;
  } else {
    Serial.println("EEPROM size changed - EEPROM data zeroed - commit() to make permanent");    
  }
}
  
void loop(){
  if(millis() - (long)backlightOnTime > ci[BACKLIGHT_TIMEOUT] * 1000 && millis()/1000 - (long)backlightOnTime /1000  != 0) {
    Serial.print(millis());
    Serial.print(" | ");
    Serial.print(backlightOnTime);
    Serial.print(" | ");
    Serial.print(ci[BACKLIGHT_TIMEOUT]);
    Serial.print(" | ");
    Serial.print((millis() - backlightOnTime)/1000 );
    Serial.println();
    lcd.noBacklight(); //power down the backlight if no buttons have been pushed in awhile (configured as backlight_timeout in config.c)
  }
  if(totalMenuItems == 0 || specialUrl != "" || (millis() % 5000  == 0 && ( millis() - timeOutForServerDataUpdates > ci[UI_HIATUS] * 1000 || timeOutForServerDataUpdates == 0))) {
    if(millis() - connectTimes[modeDeviceSwitcher] > ci[REBOOT_POLLING_TIMEOUT] * 1000 && millis() > ci[ALLOWANCE_FOR_BOOT] * 1000) {
      Serial.print("Too long to poll! Seconds:");
      Serial.println((millis() - connectTimes[modeDeviceSwitcher]) / 1000);
      rebootEsp();
    }
    if(millis() - timeOutForServerDataUpdates > 60 * 1000 && millis() - lastScreenInit  > ci[UI_HIATUS] * 1000 ) {  //every ci[UI_HIATUS] seconds of idle, reset screen
      lcd.init();
      lcd.noBacklight();
      lastScreenInit = millis();
    }
    if(deviceJson == "") {
      getDeviceInfo();
      lcd.setCursor(0,1);
      lcd.print("Getting device data");
    } else {
      if(temperatureValue == -100 || millis() - updateTimes[modeWeather] > ci[WEATHER_UPDATE_INTERVAL] * 1000) { //get temperatures every ci[WEATHER_UPDATE_INTERVAL] seconds
        if(temperatureValue == -100) {
          lcd.setCursor(0,2);
          lcd.print("Getting weather data");
        }
        getWeatherData();
        updateTimes[modeWeather] = millis();
      }
      if (batPercent == -1000 || millis() - updateTimes[modePower] >  ci[ENERGY_UPDATE_INTERVAL] * 1000) { //get values every energy_update_interval (maybe 69) seconds, alright!
        if(batPercent == -1000) {
          lcd.setCursor(0,3);
          lcd.print("Getting energy data");
        }
        getEnergyInfo();
        updateTimes[modePower] = millis();
      } 
      if(updateTimes[modeDeviceSwitcher] == 0 || millis() - updateTimes[0] > ci[POLLING_GRANULARITY] * 1000) {
        Serial.println("get form data");
        getControlFormData();
        updateTimes[modeDeviceSwitcher] = millis();
      }
    }
  }
}




void wiFiConnect() {
  if(WiFi.status() == WL_CONNECTED) {
    return;
  }
  const int NUM_WIFI_CREDENTIALS = 5;
  unsigned long lastDotTime = 0;
  unsigned long lastOfflineReconnectAttemptTime = millis();
  bool connected = false;
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(false);
  WiFi.persistent(false);
  uint8_t validWiFiAttempts = 0;
  for (int attempt = 0; attempt < NUM_WIFI_CREDENTIALS; attempt++) {
    int ssidIndex = (currentWifiIndex + attempt) % NUM_WIFI_CREDENTIALS;
    const char* wifiSsid = cs[WIFI_SSID + ssidIndex * 2];
    const char* wifiPassword = cs[WIFI_PASSWORD + ssidIndex * 2];
    if (wifiSsid && wifiSsid[0] != '\0') {
      validWiFiAttempts++;
      if(ci[DEBUG] > 1) {
        Serial.printf("\nAttempting WiFi connection to: %s\n", wifiSsid);
      }
      WiFi.disconnect(true);
      unsigned long disconnectTime = millis();
      // short wait for disconnect to take effect (non-blocking)
      while (millis() - disconnectTime < 100) {
        yield();
      }
      WiFi.begin(wifiSsid, wifiPassword);
      int wiFiSeconds = 0;
      bool initialAttemptPhase = true;
      lastOfflineReconnectAttemptTime = millis();
      while (WiFi.status() != WL_CONNECTED) {
        delay(5);
        unsigned long now = millis();
        // print dot every second
        if (now - lastDotTime >= 1000) {
          if(ci[DEBUG] > 1) {
            Serial.print(".");
          }
          lastDotTime = now;
          wiFiSeconds++;
        }
        // print asterisk and retry every 10 seconds
        if (now - lastOfflineReconnectAttemptTime > 10000) {
          WiFi.disconnect();
          WiFi.begin(wifiSsid, wifiPassword);
          lastOfflineReconnectAttemptTime = now;
          if(ci[DEBUG] > 1) {
            Serial.print("*");
          }
        }
        if (WiFi.status() == WL_NO_SSID_AVAIL) {
          if(ci[DEBUG] > 1) {
            Serial.printf("\nSSID not found: %s\n", wifiSsid);
          }
          break; // try next SSID
        }
        // timeout handling
        uint32_t wifiTimeoutToUse = ci[WIFI_TIMEOUT];
 
        if (wiFiSeconds > wifiTimeoutToUse) {
          if(ci[DEBUG] > 1) {
            Serial.println("");
            Serial.println(F("WiFi taking too long"));
          }
 
          if(ci[DEBUG] > 1) {
            Serial.println(F("trying another"));
          }
          wiFiSeconds = 1;
          initialAttemptPhase = false;
          break; // move to next SSID
        }
      }
      if (!initialAttemptPhase && wiFiSeconds > (ci[WIFI_TIMEOUT] / 2)) {
        offlineMode = true;
        haveReconnected = false;
        return;
      }
      yield(); // keep Wi-Fi background tasks alive
    }
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      currentWifiIndex = (ssidIndex + 1) % NUM_WIFI_CREDENTIALS; // next SSID next time
      if(ci[DEBUG] > 1) {
        Serial.printf("\nConnected to %s, IP: %s\n", wifiSsid, WiFi.localIP().toString().c_str());
      }
      ipAddress = WiFi.localIP().toString();
      break;
    }
  }
  if (!connected) {
    if(validWiFiAttempts > 0) {
      if(ci[DEBUG] > 1) {
        Serial.println(F("\nAll WiFi attempts failed."));
      }
    }
 
    //had required a FRAM_ADDRESS but perhaps not
    offlineMode = true;
    haveReconnected = false;
  } else {
    offlineMode = false;
    haveReconnected = true;
  }
}

void backlightOn() {
  lcd.backlight();
  backlightOnTime = millis();
}

void getWeatherData() {
  WiFiClient clientGet;
  const int httpGetPort = 80;
  String url;
  url = "/weatherdata";
  const char* localDataSource = weatherIpAddress.c_str();
  int attempts = 0;
  while(!clientGet.connect(localDataSource, httpGetPort) && attempts < ci[CONNECTION_RETRY_NUMBER]) {
    attempts++;
    delay(100);
  }
  Serial.println();
  if (attempts >= ci[CONNECTION_RETRY_NUMBER]) {
    Serial.print("Weather info connection failed");
    clientGet.stop();
    return;
  } else {
     Serial.println(url);
     clientGet.println("GET " + url + " HTTP/1.1");
     clientGet.print("Host: ");
     clientGet.println(localDataSource);
     clientGet.println("User-Agent: ESP8266/1.0");
     clientGet.println("Accept-Encoding: identity");
     clientGet.println("Connection: close\r\n\r\n");
     unsigned long timeoutP = millis();
     while (clientGet.available() == 0) {
       if (millis() - timeoutP > 10000) {
        if(clientGet.connect(localDataSource, httpGetPort)){
         //timeOffset = timeOffset + timeSkewAmount; //in case two probes are stepping on each other, make this one skew a 20 seconds from where it tried to upload data
         clientGet.println("GET / HTTP/1.1");
         clientGet.print("Host: ");
         clientGet.println(localDataSource);
         clientGet.println("User-Agent: ESP8266/1.0");
         clientGet.println("Accept-Encoding: identity");
         clientGet.println("Connection: close\r\n\r\n");
        }//if (clientGet.connect(
        //clientGet.stop();
        return;
       } //if( millis() -  
     }
    delay(2); //see if this improved data reception. OMG IT TOTALLY WORKED!!!
    while(clientGet.available()){
      connectTimes[modeWeather] = millis();
      String retLine = clientGet.readStringUntil('\r');//when i was reading string until '\n' i didn't get any JSON most of the time!
      retLine.trim();
      if(retLine.indexOf('*') >0 && retLine.indexOf('|') >0) {
        String lineArray[1];
        String dataArray[3];
        splitString(retLine, '|', lineArray, 1);
        String firstLine = lineArray[0];
        splitString(firstLine, '*', dataArray, 3);
        Serial.println(firstLine);
        oldTemperatureValue = temperatureValue;
        oldHumidityValue = humidityValue;
        oldPressureValue = pressureValue;
        temperatureValue = dataArray[0].toDouble();
        pressureValue = dataArray[1].toDouble();
        humidityValue = dataArray[2].toDouble();
      } else {
        Serial.print("weather non-data line returned: ");
        Serial.println(retLine);
      }
    }
  }  
  clientGet.stop();
}

void getDeviceInfo() { //goes on the internet to get the latest ip addresses of the various things to connect to. otherwise get them from like EEPROM
  WiFiClient clientGet;
 
  String url;
  String remoteDatastring = (String)ci[WEATHER_DEVICE] + "*" + (String)ci[CONTROL_DEVICE] +  "*" + (String)millis();
  url = String(cs[URL_GET]) + "?storagePassword=" + (String)cs[STORAGE_PASSWORD] + "&device_id=" + ci[DEVICE_ID] + "&mode=getDevices&data=" + encryptStoragePassword(remoteDatastring);

  
  int attempts = 0;
  while(!clientGet.connect(cs[HOST_GET], 80) && attempts < ci[CONNECTION_RETRY_NUMBER]) {
    attempts++;
    delay(100);
  }
  Serial.println();
  if (attempts >= ci[CONNECTION_RETRY_NUMBER]) {
    Serial.print("Device info connection failed");
    clientGet.stop();
    return;
  } else {
     Serial.println(url);
     clientGet.println("GET " + url + " HTTP/1.1");
     clientGet.print("Host: ");
     clientGet.println(cs[HOST_GET]);
     clientGet.println("User-Agent: ESP8266/1.0");
     clientGet.println("Accept-Encoding: identity");
     clientGet.println("Connection: close\r\n\r\n");
     unsigned long timeoutP = millis();
     while (clientGet.available() == 0) {
       if (millis() - timeoutP > 10000) {
        if(clientGet.connect(cs[HOST_GET], 80)){
         //timeOffset = timeOffset + timeSkewAmount; //in case two probes are stepping on each other, make this one skew a 20 seconds from where it tried to upload data
         clientGet.println("GET / HTTP/1.1");
         clientGet.print("Host: ");
         clientGet.println(cs[HOST_GET]);
         clientGet.println("User-Agent: ESP8266/1.0");
         clientGet.println("Accept-Encoding: identity");
         clientGet.println("Connection: close\r\n\r\n");
        }//if (clientGet.connect(
        //clientGet.stop();
        return;
       } //if( millis() -  
     }
    delay(2); //see if this improved data reception. OMG IT TOTALLY WORKED!!!
    while(clientGet.available()){
      String retLine = clientGet.readStringUntil('\r');//when i was reading string until '\n' i didn't get any JSON most of the time!
      retLine.trim();
      if(retLine.charAt(0) == '{') {
        Serial.println(retLine);
        deviceJson = (String)retLine.c_str();
        DeserializationError error = deserializeJson(deviceJsonBuffer, deviceJson);
        String ipAddress;
        int deviceId;
        Serial.println((int)deviceJsonBuffer["devices"].size());
        bool eepromUpdateNeeded = false;
        for(int i=0; i<deviceJsonBuffer["devices"].size(); i++) {
            ipAddress = (String)deviceJsonBuffer["devices"][i]["ip_address"];
            deviceId = (int)deviceJsonBuffer["devices"][i]["device_id"];
            Serial.print(ipAddress);
            Serial.print(" ");
            Serial.println(deviceId);
            if(deviceId == (int)ci[WEATHER_DEVICE]){
              if(weatherIpAddress != ipAddress){
                weatherIpAddress = ipAddress; 
                const char* cString = weatherIpAddress.c_str();
                std::strncpy(eepromData.weatherIpAddress, cString, sizeof(eepromData.weatherIpAddress));
                eepromData.weatherIpAddress[sizeof(eepromData.weatherIpAddress) - 1] = '\0';
                eepromUpdateNeeded = true;
              }            
            } else if(deviceId == (int)ci[CONTROL_DEVICE]){
              if(controlIpAddress != ipAddress){
                controlIpAddress = ipAddress; 
                const char* cString = controlIpAddress.c_str();
                std::strncpy(eepromData.controlIpAddress, cString, sizeof(eepromData.controlIpAddress));
                eepromData.controlIpAddress[sizeof(eepromData.controlIpAddress) - 1] = '\0';
                eepromUpdateNeeded = true;
              }
            }
        }
        if(eepromUpdateNeeded) {
          EEPROM.put(0, eepromData);
          if(!EEPROM.commit()){
            Serial.println("Eeprom update failed.");
          } else {
            Serial.println("Eeprom update SUCCEEDED!!!!.");
          }
        }
      } else {
        Serial.print("device non-JSON line returned: ");
        Serial.println(retLine);
      }
    }
  }  
  clientGet.stop();
}

void getEnergyInfo() { //goes on the internet to get the latest solar energy data for display
  WiFiClient clientGet;
 
  String url;
  String remoteDatastring = (String)millis();
  url = String(cs[URL_GET]) + "?storage_password=" + (String)cs[STORAGE_PASSWORD] + "&device_id=" + ci[DEVICE_ID] + "&mode=getEnergyInfo&data=" + encryptStoragePassword(remoteDatastring) ;

  
  int attempts = 0;
  while(!clientGet.connect(cs[HOST_GET], 80) && attempts < ci[CONNECTION_RETRY_NUMBER]) {
    attempts++;
    delay(100);
  }
  Serial.println();
  if (attempts >= ci[CONNECTION_RETRY_NUMBER]) {
    Serial.print("Solar info connection failed");
    clientGet.stop();
    return;
  } else {
     Serial.println(url);
     clientGet.println("GET " + url + " HTTP/1.1");
     clientGet.print("Host: ");
     clientGet.println(cs[HOST_GET]);
     clientGet.println("User-Agent: ESP8266/1.0");
     clientGet.println("Accept-Encoding: identity");
     clientGet.println("Connection: close\r\n\r\n");
     unsigned long timeoutP = millis();
     while (clientGet.available() == 0) {
       if (millis() - timeoutP > 10000) {
        if(clientGet.connect(cs[HOST_GET], 80)){
         //timeOffset = timeOffset + timeSkewAmount; //in case two probes are stepping on each other, make this one skew a 20 seconds from where it tried to upload data
         clientGet.println("GET / HTTP/1.1");
         clientGet.print("Host: ");
         clientGet.println(cs[HOST_GET]);
         clientGet.println("User-Agent: ESP8266/1.0");
         clientGet.println("Accept-Encoding: identity");
         clientGet.println("Connection: close\r\n\r\n");
        }//if (clientGet.connect(
        //clientGet.stop();
        return;
       } //if( millis() -  
     }
    delay(2); //see if this improved data reception. OMG IT TOTALLY WORKED!!!
    while(clientGet.available()){
      connectTimes[modePower] = millis();
      String retLine = clientGet.readStringUntil('\r');//when i was reading string until '\n' i didn't get any JSON most of the time!
      retLine.trim();
      if(retLine.charAt(0) == '{') {
        Serial.println(retLine);
        String energyJson = (String)retLine.c_str();
        DeserializationError error = deserializeJson(energyJsonBuffer, energyJson);
        pvPower = (int)energyJsonBuffer["energy_info"]["pv_power"];
        batPower = (int)energyJsonBuffer["energy_info"]["bat_power"];
        batPercent = (int)energyJsonBuffer["energy_info"]["bat_percent"];
        loadPower = (int)energyJsonBuffer["energy_info"]["load_power"];
      } else {
        Serial.print("power non-JSON line returned: ");
        Serial.println(retLine);
      }
    }
  }  
  clientGet.stop();
}

void getControlFormData() {
  Serial.print("control ip address:");
  Serial.println(controlIpAddress);
  WiFiClient clientGet;
 
  String url;
  if(specialUrl != "") {
    url =  (String)specialUrl;
    specialUrl = "";
  } else {
    url =  (String)"/readLocalData";
  }
  Serial.println(url);
  int attempts = 0;
  while(!clientGet.connect(controlIpAddress, 80) && attempts < ci[CONNECTION_RETRY_NUMBER]) {
    attempts++;
    delay(2);
  }
  Serial.println();
  if (attempts >= ci[CONNECTION_RETRY_NUMBER]) {
    Serial.print("Connection failed");
    connectionFailureTime = millis();
    connectionFailureMode = true;
  } else {
     connectionFailureTime = 0;
     connectionFailureMode = false;
     Serial.println(url);
     clientGet.println("GET " + url + " HTTP/1.1");
     clientGet.print("Host: ");
     clientGet.println(controlIpAddress);
     clientGet.println("User-Agent: ESP8266/1.0");
     clientGet.println("Accept-Encoding: identity");
     clientGet.println("Connection: close\r\n\r\n");
     unsigned long timeoutP = millis();
     while (clientGet.available() == 0) {
       if (millis() - timeoutP > 10000) {
        if(clientGet.connect(controlIpAddress, 80)){
         //timeOffset = timeOffset + timeSkewAmount; //in case two probes are stepping on each other, make this one skew a 20 seconds from where it tried to upload data
         clientGet.println("GET / HTTP/1.1");
         clientGet.print("Host: ");
         clientGet.println(controlIpAddress);
         clientGet.println("User-Agent: ESP8266/1.0");
         clientGet.println("Accept-Encoding: identity");
         clientGet.println("Connection: close\r\n\r\n");
        }//if (clientGet.connect(
        //clientGet.stop();
        return;
       } //if( millis() -  
     }
    delay(1); //see if this improved data reception. OMG IT TOTALLY WORKED!!!
    bool receivedData = false;
    bool receivedDataJson = false;
    while(clientGet.available()){
      connectTimes[modeDeviceSwitcher] = millis();
      receivedData = true;
      String retLine = clientGet.readStringUntil('\r');//when i was reading string until '\n' i didn't get any JSON most of the time!
      retLine.trim();
      if(retLine.charAt(0) == '{') {
        Serial.println(retLine);
        localCopyOfJson = (String)retLine.c_str();
        updateScreen(localCopyOfJson, 0, false); 
        receivedDataJson = true;
        break; 
      } else {
        Serial.print("non-JSON line returned: ");
        Serial.println(retLine);
      }
    }
  } //if (attempts >= connection_retry_number)....else....    
  Serial.println("\r>>> Closing host for json: ");
  Serial.println(controlIpAddress);
  clientGet.stop();
}

void updateScreen(String json, char startLine, bool withInit) { //handles the display of everything
  disableInterrupts();
  allowInterrupts = false;
  bool showTimingDebugInfo = false;
  lcd.clear();
  char buffer[12];
  if(currentMode == modeWeather) {
    char directionIndication = ' ';
    char format[] = "%7.2f";
    lcd.setCursor(0, 0);
    lcd.print("Current Weather");
    if (oldTemperatureValue == -100) { //this condition a proxy for there being no old weather data
      directionIndication = ' ';
    } else if(oldTemperatureValue-temperatureValue > ci[TEMPERATURE_DELTA_REPONSIVENESS]) {//we want a change in temperature to be at least as big as temperature_delta_for_change to produce a change arrow
      directionIndication  = 'v';
    } else if (oldTemperatureValue-temperatureValue < -ci[TEMPERATURE_DELTA_REPONSIVENESS]) {
      directionIndication  = '^';
    } else {
      directionIndication  = '=';
    }
    lcd.setCursor(19, 1);
    lcd.print(directionIndication);
    lcd.setCursor(0, 1);
    sprintf(buffer, format, temperatureValue * 1.8 +32);
    lcd.print(buffer);
    lcd.setCursor(13, 1);
    lcd.print("deg F");
    if (oldTemperatureValue == -100) { //this condition a proxy for there being no old weather data
      directionIndication = ' ';
    } else if(oldPressureValue - pressureValue > ci[PRESSURE_DELTA_REPONSIVENESS]) {//we want a change in pressure to be at least as big as pressure_delta_for_change to produce a change arrow
      directionIndication  = 'v';
    } else if (oldPressureValue - pressureValue < -ci[PRESSURE_DELTA_REPONSIVENESS]) {
      directionIndication  = '^';
    } else {
      directionIndication  = '=';
    }
    lcd.setCursor(19, 2);
    lcd.print(directionIndication);  
    lcd.setCursor(0, 2);
    sprintf(buffer, format, pressureValue);
    lcd.print(buffer);
    lcd.setCursor(13, 2);
    lcd.print("mm Hg");
    if (oldTemperatureValue == -100) { //this condition a proxy for there being no old weather data
      directionIndication = ' ';
    } else if(oldHumidityValue - humidityValue > ci[HUMIDITY_DELTA_REPONSIVENESS]) { //we want a change in humidity to be at least as big as humidity_delta_for_change to produce a change arrow
      directionIndication  = 'v';
    } else if (oldHumidityValue - humidityValue < -ci[HUMIDITY_DELTA_REPONSIVENESS]) {
      directionIndication  = '^';
    } else {
      directionIndication  = '=';
    }
    lcd.setCursor(19, 3);
    lcd.print(directionIndication);
    lcd.setCursor(0, 3);
    sprintf(buffer, format, humidityValue);
    lcd.print(buffer);
    lcd.setCursor(13, 3);
    lcd.print("% rel");
    if(showTimingDebugInfo) {
      lcd.setCursor(17, 0);
      lcd.print((millis() - updateTimes[1])/1000);
    }
  } else if(currentMode == modePower) {
    char format[] = "%6d";
    lcd.setCursor(0, 0);
    lcd.print("Solar: ");
    sprintf(buffer, format, pvPower);
    lcd.setCursor(12, 0);
    lcd.print(buffer);
    lcd.setCursor(19, 0);
    lcd.print("w");
    lcd.setCursor(0, 1);
    lcd.print("Load: ");
    lcd.setCursor(12, 1);
    sprintf(buffer, format, loadPower);
    lcd.print(buffer);
    lcd.setCursor(19, 1);
    lcd.print("w");
    lcd.setCursor(0, 2);
    lcd.print("Battery: ");
    lcd.setCursor(12, 2);
    sprintf(buffer, format, batPercent);
    lcd.print(buffer);
    lcd.setCursor(19, 2);
    lcd.print("%");
    lcd.setCursor(0, 3);
    if(batPower < 0) {
      lcd.print("Charging: ");
    } else {
      lcd.print("Draining: "); 
    }
    lcd.setCursor(12, 3);
    sprintf(buffer, format, abs(batPower));
    lcd.print(buffer);
    lcd.setCursor(19, 3);
    lcd.print("w");
    if(showTimingDebugInfo) {
      lcd.setCursor(10, 0);
      lcd.print((millis() - updateTimes[2])/1000);
    }
  } else {
    if(specialUrl != "") {
      return;
    }
    allowInterrupts = true;
    if(withInit == true) {
      menuBegin = 0;
      menuCursor = 0;
    }
    if(startLine == 0){
      menuBegin = 0;
    }
    
    int value = -1;
    int serverSaved = 0;
    String friendlyPinName = "";
    String id = "";
    if(json != "") {
      DeserializationError error = deserializeJson(jsonBuffer, json);
    }
    if(jsonBuffer["device"]) { //deviceName is a global
      deviceName = (String)jsonBuffer["device"];
    }
    char * nodeName="pins";
    if(jsonBuffer[nodeName]) {
      char totalShown = 0;
      totalMenuItems = jsonBuffer[nodeName].size();
      for(int i=0; i<jsonBuffer[nodeName].size(); i++) {
        if(i >= startLine && totalShown < totalScreenLines) {
          lcd.setCursor(0, i - startLine);
          friendlyPinName = (String)jsonBuffer[nodeName][i]["name"];
          Serial.println(friendlyPinName);
          value = (int)jsonBuffer[nodeName][i]["value"];
          id = (String)jsonBuffer[nodeName][i]["id"];
          if(value == 1) {
            lcd.print(" *" + friendlyPinName.substring(0, 18));
          } else {
            lcd.print("  " + friendlyPinName.substring(0, 18));
          }
          totalShown++; 
        }
      }
    }
    Serial.print("total menu items:");
    Serial.println((int)totalMenuItems);
    if(showTimingDebugInfo) {
      lcd.setCursor(17, 0);
      lcd.print((millis() - updateTimes[0])/1000);
    }
  }
  allowInterrupts = true;
  enableInterrupts();
}

void moveCursorUp(){
  if(currentMode != modeDeviceSwitcher) {
    return;
  }
  Serial.print("up: ");
  Serial.print((int)menuCursor);
  Serial.println(" * " );

  if(menuCursor > totalScreenLines - 1) {
    lcd.setCursor(0, totalScreenLines - 1);
  } else {
    lcd.setCursor(0, menuCursor);
  }
  lcd.print(" ");
  if(menuBegin > 0) {
    //scroll screen up:
    menuBegin--;
    menuCursor--;
    updateScreen("", menuBegin, false);
  } else {
    menuCursor--;
    if(menuCursor < 0  || menuCursor > 250) {
      Serial.print(" less than zero ");
      Serial.println((int)menuCursor);
      menuCursor = 0;
    }
    updateScreen("", menuBegin, false);
  }
  if(menuCursor == totalScreenLines) {
    //menuCursor = totalScreenLines - 1;
  }
  //lcd.setCursor(0, menuCursor);
  lcd.setCursor(0, (menuCursor - menuBegin));
  lcd.print(">");
  Serial.println((int)menuCursor);
}
void moveCursorDown(){
  if(currentMode != modeDeviceSwitcher) {
    return;
  }
  Serial.println("down: ");
  Serial.print((int)menuCursor);
  lcd.setCursor(0, menuCursor);
  lcd.print(" ");
  menuCursor++;
  if(menuCursor > totalMenuItems-1) {
    menuCursor = totalMenuItems-1;
  }
  if(menuCursor > totalScreenLines - 1) {
    //scroll screen up:
    menuBegin = menuCursor-(totalScreenLines - 1);
    Serial.print(" Menu begin: ");
    Serial.println((int)menuBegin);
    updateScreen("", menuBegin, false);
    lcd.setCursor(0, totalScreenLines - 1);
  } else {
    updateScreen("", menuBegin, false);
    lcd.setCursor(0, menuCursor);
  }
  lcd.print(">");
  Serial.println((int)menuCursor);
}

void toggleDevice(){
  if(currentMode == modeWeather) { //reboot the system if toggle is hit while we're looking at weather
    rebootEsp();
  }
  if(currentMode != modeDeviceSwitcher) {
    return;
  }
  Serial.print("TOGGLE! screen: ");
  if(menuCursor > totalScreenLines - 1) {
    lcd.setCursor(0, totalScreenLines - 1);
    Serial.print(totalScreenLines - 1);
  } else {
    lcd.setCursor(0, menuCursor);
     Serial.print(menuCursor);
  }
  Serial.print(" for menu item: ");
  if(getPinValue(menuCursor) == 0) {
    lcd.print(">*");
    sendDataToController(menuCursor, 1);
  } else {
    lcd.print("> ");
    sendDataToController(menuCursor, 0);
  }
  Serial.println((int)menuCursor);
}

void advanceMode() {
  modeCursor++;
  if(modeCursor >= modeCount) {
    modeCursor = 0;
  }
  currentMode = modes[modeCursor];
  Serial.print("current mode: ");
  Serial.println(currentMode);
  updateScreen("", menuBegin, false);
}
 
void buttonPushed() {
  //Serial.print("button pushed ");
  //Serial.println(allowInterrupts);
  if(!allowInterrupts) {
    return;
  }
  disableInterrupts();
  boolean skipButtonFunction = false;
  if(lastTimeButtonPushed == 0 || millis() - lastTimeButtonPushed > 150) { //150 millisecond debounce
    lastTimeButtonPushed = millis();
  } else {
    //too soon, so debounce!
    skipButtonFunction = true;
  }
  timeOutForServerDataUpdates = millis();
  volatile int val;
  volatile int hardwarePinNumber;
  //backlightOn();
  for(volatile char i=0; i<buttonNumber; i++) {
    hardwarePinNumber = buttonPins[i];
    val = digitalRead(hardwarePinNumber);
    
    if(val == 1) {
      if(millis() - backlightOnTime > ci[BACKLIGHT_TIMEOUT] * 1000) {
        skipButtonFunction = true; //if we're in LCD blackout, don't do button function, just turn on backlight
      }
      
      if(!skipButtonFunction){
        if(hardwarePinNumber == buttonUp) {
          moveCursorUp();
        }
        if(hardwarePinNumber == buttonDown) {
          moveCursorDown();
        }
        if(hardwarePinNumber == buttonChange) {
          toggleDevice();
        }
        if(hardwarePinNumber == buttonMode) {
          advanceMode();
        }
      }
    }
  }
  backlightOn();
  enableInterrupts();
}

void disableInterrupts(){
  for(char i=0; i<buttonNumber; i++) {
    detachInterrupt(digitalPinToInterrupt(buttonPins[i]));
  }
}

void enableInterrupts(){
   for(char i=0; i<buttonNumber; i++) {
    pinMode(buttonPins[i], OUTPUT);
    attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonPushed, RISING);
  }
}

char getPinValue(char ordinal) {
  char * nodeName="pins";
  char value = -1;
  if(jsonBuffer[nodeName]) {
    for(int i=0; i<jsonBuffer[nodeName].size(); i++) {
      value = (int)jsonBuffer[nodeName][i]["value"];
      if(ordinal == i) {
        return value;
      }
    }
  }
  return 0;
}

void sendDataToController(char ordinal, char value) {
  char * nodeName="pins";
  String id = "";
  if(jsonBuffer[nodeName]) {
    for(int i=0; i<jsonBuffer[nodeName].size(); i++) {
      id = (String)jsonBuffer[nodeName][i]["id"];
      if((int)ordinal == i) {
        //we just snag one of the updates and use it to send data to the remote controller
        specialUrl =  (String)"/writeLocalData?id=" + id + "&on=" + (int)value + "&ipaddress=" + ourIpAddress;
      }
    }
  }
}

void rebootEsp() {
  Serial.println("Rebooting ESP");
  backlightOn();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Forced to restart...");
  ESP.restart();
}



//////////////////////////////

void stopWebSocket(){
  //lastGoodKey = "";
  if(outputMode == 3) {
    responseBuffer = fastResponseBuffer; //anything heading to the output now has to take the responseBuffer route
    fastResponseBuffer = "";
    //Serial.println("OUTPUTMODE CHANGEO1: " + String(outputMode) + " TO " + String(oldOutputMode));
    outputMode = oldOutputMode;
    lastTimeOutputModeChanged = millis();

    //Serial.println("---------------");
    //Serial.println(outputMode);
    saveCommandState(0, 0, 0, 0);
  }
  webSocket.disconnect();
}

void startWebSocket(int origin){
  //origin only for debugging
  if(ci[DEBUG] > 2) {
    Serial.println(F("Origin: ") + String(origin));
  }
  //if we have a pending lastCommandLogId we need to stay out of here
  if(lastCommandLogId > 0) {
    return;
  }

  if(outputMode != 3) {
    oldOutputMode = outputMode;
    //oldOutputMode = 0;
    lastTimeOutputModeChanged = millis();
    //Serial.println("OUTPUTMODE CHANGEO2: " + String(outputMode) + " TO " + String(3));
  }
  outputMode = 3;
  if(webSocket.isConnected()) {
    return;
  }
  saveCommandState(0, VERSION, -3, 0);
  wiFiConnect();
 
  char socketUrl[128];
  snprintf(
    socketUrl,
    sizeof(socketUrl),
    "/?k2=%s&type=device&device_id=%d",
    lastGoodKey.c_str(),
    ci[DEVICE_ID]
  );

  if(ci[DEBUG] > 1) {
    Serial.println(socketUrl);
  }
  webSocket.begin(cs[HOST_GET], 8080, socketUrl);
}

 

void saveCommandState(uint32_t lastCommandLogId, uint16_t version, int32_t commandId, int32_t commandType) {
    File file = LittleFS.open(F("/commandstate.txt"), "w");
    if (!file) {
        // optional: Serial.println("Failed to open file for writing");
        return;
    }
    file.println(lastCommandLogId);
    file.println(version);
    file.println(commandId);
    file.println(commandType);
    file.close();
}



///////////////////////////////////////////////
//command functions
///////////////////////////////////////////////
#define CFG_REQ_FS        0b00000001
#define CFG_REQ_SLAVE     0b00000010
#define CFG_REQ_FRAM      0b00000100
#define CFG_REQ_RTC       0b00001000
///////////////////////////////
#define CFG_REQ_IR        0b00010000
#define CFG_UNUSED_1      0b00100000
#define CFG_UNUSED_2      0b01000000
#define CFG_REQ_DEFER     0b10000000
  
void runCommand(const char * commandText, bool deferred){
  //can change the default values of some config data for things like polling
  //dumpMemoryStats(99);
  //return;
  String command;
  int commandId;
  String commandData;
  String commandArray[4];
  int latency;
  //first get rid of the first character, since all it does is signal that we are receiving a command:
  const char* cmd = commandText + 1;
  splitString(cmd, '|', commandArray, 3);
  commandId = commandArray[0].toInt();
  command = commandArray[1];
  commandData = commandArray[2];
  latencyCount++;
  latency = commandArray[3].toInt();
  latencySum += latency;
  //Serial.println(commandId);
  if(commandId < -1) {
    /*
      Serial.println("***********");
      Serial.print(oldOutputMode);
      Serial.print(" ");
      Serial.println(outputMode);
     */
    //this can occasionally switch us to fast commmand mode simply by sending a command
    if(outputMode != abs(commandId)) {
      oldOutputMode = outputMode;
      lastTimeOutputModeChanged = millis();
      /*
      Serial.print("Changing outputmode from ");
      Serial.print(outputMode);
      Serial.print(" to ");
      Serial.println(abs(commandId));
      */
      //Serial.println("OUTPUTMODE CHANGEO5: " + String(outputMode) + " TO " + String(abs(commandId)));
      outputMode = abs(commandId);
    }
    /*
      Serial.print(oldOutputMode);
      Serial.print(" ");
      Serial.println(outputMode);
      */
  }
  String* results[4];
  int resultCount;
  command.trim();
  if(commandId) {
    //Serial.println("------------");
    //Serial.println(command);
    handleCommand(command, deferred);
    if(!deferred && (commandRequiresDeferment(command))) {
      //Serial.println("--------+"  + command + "*-----");
      notYetDeferred(commandText, commandId, (int32_t)(command.indexOf(F("update firmware"))>-1));
    }
    command = "";
    if(commandId > 0) { //don't reset lastCommandId if the command came via serial port
      lastCommandId = commandId;
    }
  }
}

void handleCommand(String input, bool deferred) {
  String results[5];
  int resultCount = 0;
  if (input == "") {
    return;
  }
  int commandNumber = commandCount;//from global//sizeof(commands) / sizeof(commands[0]);
  for (int i = 0; i < commandNumber; i++) {
    yield();
    if (parseCommand(input, commands[i].name, results, resultCount, commands[i].maxArgs)) {
      uint8_t cfg = commands[i].configuration;
      // ?? Capability checks
 
 
 
      if ((cfg & CFG_REQ_IR) && !(ci[IR_PIN] > -1)) {
        textOut(F("Error: IR capability required\n"));
        return;
      }
      /*
      // ? Deferred requirement check (if you want it enforced)
      if ((cfg & CFG_REQ_DEFER) && !deferred) {
        textOut(F("Error: Command must be deferred\n"));
        return;
      }
      */
      // ? All checks passed, execute
      commands[i].handler(results, resultCount, deferred);
      return;
    }
  }
  textOut(F("Command '") + input + F("' does not exist\n"));
}

void notYetDeferred(const char * commandText, int commandId, int commandType){
  //Serial.println("^^^^^^^^^^^^^^");
  //Serial.print(lastCommandLogId);
  //Serial.print(" $ ");
  //Serial.println(commandId);
  if(lastCommandLogId > 0 || commandId <0) {
    saveCommandState(lastCommandLogId, VERSION, commandId, commandType);
  }
  String command;
  if (commandText == nullptr) {
    return;
  }
  size_t len = strlen(commandText);
  if (deferredCommand != nullptr) {
    delete[] deferredCommand;
  }
  deferredCommand = new char[len + 1];  // +1 for null terminator
  strcpy(deferredCommand, commandText);
  if(commandId == -1) {
    //our command is via serial, so handle deferred commands immediately
    runCommand(deferredCommand, true);
  }
  return;
}

bool parseCommand(const String& input, const String& command, String* results, int& resultCount, int maxResults) {
    resultCount = 0;
    String s = input;
    s.trim();
    // Check if it starts with the command
    if (!s.startsWith(command)) {
        return false;
    }
    // Remove the command part
    s = s.substring(command.length());
    s.trim();
    bool inQuotes = false;
    String current = "";
    for (unsigned int i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '"') {
            inQuotes = !inQuotes;
            continue; // don't include the quote
        }
        if (c == ' ' && !inQuotes) {
            if (current.length() > 0) {
                if (resultCount < maxResults) {
                    current = numericEquivalents(current);
                    results[resultCount++] = current;
                }
                current = "";
            }
        } else {
            current += c;
        }
    }
    // Add last token
    if (current.length() > 0 && resultCount < maxResults) {
        current = numericEquivalents(current);
        results[resultCount++] = current;
    }
    return true;
}

bool commandRequiresDeferment(String input) {
  String results[5];
  int resultCount = 0;
  if (input == "") {
    return false;
  }
  int commandNumber = commandCount;
  for (int i = 0; i < commandNumber; i++) {
    yield();

    if (parseCommand(input, commands[i].name, results, resultCount, commands[i].maxArgs)) {
      return (commands[i].configuration & CFG_REQ_DEFER) != 0;
    }
  }
  // Command not found ? no deferment requirement (you could argue either way)
  return false;
}

void saveCommandState(uint32_t lastCommandLogId, uint16_t version, int32_t commandId, int32_t commandType) {
    File file = LittleFS.open(F("/commandstate.txt"), "w");
    if (!file) {
        // optional: Serial.println("Failed to open file for writing");
        return;
    }
    file.println(lastCommandLogId);
    file.println(version);
    file.println(commandId);
    file.println(commandType);
    file.close();
}

int32_t loadCommandStateVersion(int dataToReturn) {
    File file = LittleFS.open("/commandstate.txt", "r");
    if (!file) {
        // optional: Serial.println("Failed to open file for reading");
        return 0;
    }
    int lineCount = 4;
    int32_t lines[4];
    for(int lineNumber = 0; lineNumber<lineCount; lineNumber++){
      lines[lineNumber] = (int32_t)file.readStringUntil('\n').toInt();
    }
    return lines[dataToReturn];
}
 
 
///////////////////////////////////////////////
//end of command functions
///////////////////////////////////////////////

 
