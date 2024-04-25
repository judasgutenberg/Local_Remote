/* 
Local Remote, Gus Mueller, April 22 2024
Provides a local control-panel for the remote-control system here:
https://github.com/judasgutenberg/Esp8266_RemoteControl
*/
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <ESP_EEPROM.h>

#include "config.h"

//so far i have failed to get the "EEProm" to work
struct nonVolatileStruct {
  char  controlIpAddress[30];
  char  weatherIpAddress[30];
} eepromData;


void ICACHE_RAM_ATTR buttonPushed();
StaticJsonDocument<1000> jsonBuffer;
StaticJsonDocument<2000> deviceJsonBuffer;  //this has to be big for some reason
String deviceName = "Your Device";
String specialUrl = "";
String ourIpAddress;

String controlIpAddress; //would like to store in EEPROM
String weatherIpAddress; //would like to store in EEPROM
 
const byte buttonNumber = 4;
byte buttonMode = 13;
byte buttonUp = 15;
byte buttonDown = 12;
byte buttonChange = 14;
byte buttonPins[buttonNumber] = {buttonChange, buttonDown, buttonMode, buttonUp}; //connect to the column pinouts of the keypad left to right: 13, 15, 12, 14
String localCopyOfJson;
long connectionFailureTime = 0;
bool connectionFailureMode = false;
long timeOutForServerDataUpdates;
char menuCursor = 0;
char menuBegin = 0;
signed char totalMenuItems = 1;
char totalScreenLines = 4;
String deviceJson = "";

LiquidCrystal_I2C lcd(0x27,20,totalScreenLines); 

void setup(){
  Serial.begin(115200);
  Serial.println("Starting up Local Remote");
  WiFiConnect();
  for(char i=0; i<buttonNumber; i++) {
    pinMode(buttonPins[i], OUTPUT); //seems to work to make an unconnected INPUT default as low
    //Serial.println(digitalPinToInterrupt(buttonPins[i]));
    attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonPushed, CHANGE);
  }
 
  EEPROM.begin(sizeof(nonVolatileStruct));
  if(EEPROM.percentUsed()>=0) {
    EEPROM.get(0, eepromData);
    Serial.println("EEPROM has data from a previous run.");
    Serial.print(EEPROM.percentUsed());
    Serial.println("% of ESP flash space currently used");
    controlIpAddress = (String) eepromData.controlIpAddress;
    weatherIpAddress = (String) eepromData.weatherIpAddress;
  } else {
    Serial.println("EEPROM size changed - EEPROM data zeroed - commit() to make permanent");    
  }
  
  // set up the LCD's number of rows and columns:
  lcd.init();
 

  // Print a message to the LCD.
 
}
  
void loop(){
  for(char i=0; i<buttonNumber; i++) {
      //Serial.print(digitalRead(buttonPins[i]));
  }
  //Serial.println();
  if(totalMenuItems == 0 || specialUrl != "" || (millis() % 5000  == 0 && ( millis() - timeOutForServerDataUpdates > hiatusLengthOfUiUpdatesAfterUserInteraction * 1000 || timeOutForServerDataUpdates == 0))) {
    
    if(deviceJson == "") {
      getDeviceInfo();
    } else {
      getJson();
    }
  }
  
 
 
}


void WiFiConnect() {
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println();
  // Wait for connection
  int wiFiSeconds = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    wiFiSeconds++;
    if(wiFiSeconds > 80) {
      Serial.println("WiFi taking too long");
 
      wiFiSeconds = 0; //if you don't do this, you'll be stuck in a rebooting loop if WiFi fails once
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  ourIpAddress =  WiFi.localIP().toString();
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}


void getDeviceInfo() { //goes on the internet to get the latest ip addresses of the various things to connect to. otherwise get them from like EEPROM
  WiFiClient clientGet;
  const int httpGetPort = 80;
  String url;
  url = "/weather/data.php?storagePassword=" + (String)storagePassword + "&mode=getDevices";

  char * dataSourceHost = "randomsprocket.com";
  int attempts = 0;
  while(!clientGet.connect(dataSourceHost, httpGetPort) && attempts < connectionRetryNumber) {
    attempts++;
    delay(200);
  }
  Serial.println();
  if (attempts >= connectionRetryNumber) {
    Serial.print("Device info connection failed");
    clientGet.stop();
    return;
  } else {
 
     Serial.println(url);
     clientGet.println("GET " + url + " HTTP/1.1");
     clientGet.print("Host: ");
     clientGet.println(dataSourceHost);
     clientGet.println("User-Agent: ESP8266/1.0");
     clientGet.println("Accept-Encoding: identity");
     clientGet.println("Connection: close\r\n\r\n");
     unsigned long timeoutP = millis();
     while (clientGet.available() == 0) {
       if (millis() - timeoutP > 10000) {

        if(clientGet.connect(dataSourceHost, httpGetPort)){
         //timeOffset = timeOffset + timeSkewAmount; //in case two probes are stepping on each other, make this one skew a 20 seconds from where it tried to upload data
         clientGet.println("GET / HTTP/1.1");
         clientGet.print("Host: ");
         clientGet.println(dataSourceHost);
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
            if(deviceId == (int)weatherDevice){
              if(weatherIpAddress != ipAddress){
                weatherIpAddress = ipAddress; 
                const char* cString = weatherIpAddress.c_str();
                std::strncpy(eepromData.weatherIpAddress, cString, sizeof(eepromData.weatherIpAddress));
                eepromData.weatherIpAddress[sizeof(eepromData.weatherIpAddress) - 1] = '\0';
                eepromUpdateNeeded = true;
              }            
            } else if(deviceId == (int)controlDevice){
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
            Serial.println("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxEeprom update failed.");
          } else {
            Serial.println("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxEeprom update SUCCEEDED!!!!.");
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

void getJson() {
  Serial.print("control ip address:");
  Serial.println(controlIpAddress);
  WiFiClient clientGet;
  const int httpGetPort = 80;
  String url;
  if(specialUrl != "") {
    url =  (String)specialUrl;
    specialUrl = "";
 
  } else {
    url =  (String)"/readLocalData";
  }
  
  int attempts = 0;
  while(!clientGet.connect(controlIpAddress, httpGetPort) && attempts < connectionRetryNumber) {
    attempts++;
    delay(200);
  }
  
  Serial.println();
  if (attempts >= connectionRetryNumber) {
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

        if(clientGet.connect(controlIpAddress, httpGetPort)){
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
    delay(2); //see if this improved data reception. OMG IT TOTALLY WORKED!!!
    bool receivedData = false;
    bool receivedDataJson = false;
    while(clientGet.available()){
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
 
   
  } //if (attempts >= connectionRetryNumber)....else....    
  Serial.println("\r>>> Closing host for json: ");
  Serial.println(controlIpAddress);
  clientGet.stop();
}


void updateScreen(String json, char startLine, bool withInit) {
  lcd.clear();
  if(specialUrl != "") {
    return;
  }
  if(withInit == true) {
    menuBegin = 0;
    menuCursor = 0;
  }
  if(startLine == 0){
    menuBegin = 0;
  }
  lcd.backlight();
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
          lcd.print(" *" + friendlyPinName);
        } else {
          lcd.print("  " + friendlyPinName);
        }
        totalShown++; 
      }
    }
     
  }
  Serial.print("total menu items:");
  Serial.println((int)totalMenuItems);
}


void moveCursorUp(){
  Serial.print("up: ");
  Serial.print((int)menuCursor);
  Serial.println(" * " );
  lcd.setCursor(0, menuCursor);
  lcd.print(" ");
 
  if(menuBegin > 0) {
    //scroll screen up:
    menuBegin--;
    updateScreen("", menuBegin, false);
  } else {
    menuCursor--;
    if(menuCursor < 0  || menuCursor > 250) {
      Serial.print(" less than zero ");
      Serial.println((int)menuCursor);
      menuCursor = 0;
    }
  }
  if(menuCursor == totalScreenLines) {
    menuCursor = totalScreenLines - 2;
  }
  lcd.setCursor(0, menuCursor);
  lcd.print(">");
  Serial.println((int)menuCursor);
}
void moveCursorDown(){
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
    lcd.setCursor(0, menuCursor);
  }
  
  lcd.print(">");
  Serial.println((int)menuCursor);
}

void toggleDevice(){
  Serial.println("toggle");
  Serial.println((int)menuCursor);
  lcd.setCursor(0, menuCursor);
 
  lcd.setCursor(0, menuCursor);
  if(getPinValue(menuCursor) == 0) {
    lcd.print(">*");
    sendDataToController(menuCursor, 1);
  } else {
    lcd.print("> ");
    sendDataToController(menuCursor, 0);
  }
  Serial.println((int)menuCursor);
}
 
void buttonPushed() {
  for(char i=0; i<buttonNumber; i++) {
    detachInterrupt(digitalPinToInterrupt(buttonPins[i]));
  }
  timeOutForServerDataUpdates = millis();
  
  volatile int val;
  volatile int hardwarePinNumber;
  for(volatile char i=0; i<buttonNumber; i++) {
    hardwarePinNumber = buttonPins[i];
    val = digitalRead(hardwarePinNumber);
    if(val == 1) {
      if(hardwarePinNumber == buttonUp) {
        moveCursorUp();
      }
      if(hardwarePinNumber == buttonDown) {
        moveCursorDown();
      }
      if(hardwarePinNumber == buttonChange) {
        toggleDevice();
      }
    }
  }
  for(char i=0; i<buttonNumber; i++) {
    attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonPushed, CHANGE);
  }
 
}

char getPinValue(char ordinal) {
  char * nodeName="pins";
  char value = -1;
  if(jsonBuffer[nodeName]) {
    for(int i=0; i<jsonBuffer[nodeName].size(); i++) {
      lcd.setCursor(0, i);
      value = (int)jsonBuffer[nodeName][i]["value"];
      if(ordinal == i) {
        return value;
      }
    }
     
  }
}


void sendDataToController(char ordinal, char value) {
  char * nodeName="pins";
  String id = "";
  if(jsonBuffer[nodeName]) {
    for(int i=0; i<jsonBuffer[nodeName].size(); i++) {
      id = (String)jsonBuffer[nodeName][i]["id"];
      if((int)ordinal == i) {
        specialUrl =  (String)"/writeLocalData?id=" + id + "&on=" + (int)value;
      }
    }
     
  }

  
}
