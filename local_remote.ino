/* @file CustomKeypad.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates changing the keypad size and key values.
|| #
*/
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>

#include "config.h"

LiquidCrystal_I2C lcd(0x27,40,4); 

StaticJsonDocument<1000> jsonBuffer;
String deviceName = "Your Device";
String ipAddress;
const byte pinNumber = 3;
byte buttonPins[pinNumber] = {0,2,14}; //connect to the column pinouts of the keypad
String localCopyOfJson;
long connectionFailureTime = 0;
bool connectionFailureMode = false;
 
void setup(){
  WiFiConnect();
  Serial.begin(115200);
  for(char i=0; i<pinNumber; i++) {
    //pinMode(buttonPins[i], OUTPUT);
    
  }
  Serial.println("LCD Character Backpack I2C Test.");

  // set up the LCD's number of rows and columns:
  lcd.init();
  Serial.println("Backpack init'd.");

  // Print a message to the LCD.
 
}
  
void loop(){
  for(char i=0; i<pinNumber; i++) {
      //Serial.print(digitalRead(buttonPins[i]));
  }
  //Serial.println();
  if(millis() % 5000  == 0) {
    Serial.println("get json");
    getJson();
  }
 
  // print the number of seconds since reset:

 
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
  ipAddress =  WiFi.localIP().toString();
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

void getJson() {
  WiFiClient clientGet;
  const int httpGetPort = 80;
  String url;
  String mode = "readLocalData";
 
  url =  (String)urlGet;
  int attempts = 0;
  while(!clientGet.connect(hostGet, httpGetPort) && attempts < connectionRetryNumber) {
    attempts++;
    delay(200);
  }
  Serial.println();
  if (attempts >= connectionRetryNumber) {
    Serial.print("Connection failed, moxee rebooted: ");
    connectionFailureTime = millis();
    connectionFailureMode = true;
  } else {
     connectionFailureTime = 0;
     connectionFailureMode = false;
     Serial.println(url);
     clientGet.println("GET " + url + " HTTP/1.1");
     clientGet.print("Host: ");
     clientGet.println(hostGet);
     clientGet.println("User-Agent: ESP8266/1.0");
     clientGet.println("Accept-Encoding: identity");
     clientGet.println("Connection: close\r\n\r\n");
     unsigned long timeoutP = millis();
     while (clientGet.available() == 0) {
       if (millis() - timeoutP > 10000) {
        //let's try a simpler connection and if that fails, then reboot moxee
        //clientGet.stop();
        if(clientGet.connect(hostGet, httpGetPort)){
         //timeOffset = timeOffset + timeSkewAmount; //in case two probes are stepping on each other, make this one skew a 20 seconds from where it tried to upload data
         clientGet.println("GET / HTTP/1.1");
         clientGet.print("Host: ");
         clientGet.println(hostGet);
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
        updateScreen(localCopyOfJson);
        receivedDataJson = true;
        break; 
      } else {
        Serial.print("non-JSON line returned: ");
        Serial.println(retLine);
      }
    }
 
   
  } //if (attempts >= connectionRetryNumber)....else....    
  Serial.println("\r>>> Closing host: ");
  Serial.println(hostGet);
  clientGet.stop();
}


void updateScreen(String json) {
  
  lcd.backlight();
  int pinNumber = 0;
  int value = -1;
  int canBeAnalog = 0;
  int enabled = 0;
  int pinCounter = 0;
  int serverSaved = 0;
  String friendlyPinName = "";
  String id = "";
  DeserializationError error = deserializeJson(jsonBuffer, json);
 
  if(jsonBuffer["device"]) { //deviceName is a global
    deviceName = (String)jsonBuffer["device"];
  }
  char * nodeName="pins";
  if(jsonBuffer[nodeName]) {
    
    pinCounter = 0;
    for(int i=0; i<jsonBuffer[nodeName].size(); i++) {
      lcd.setCursor(0, i);
      friendlyPinName = (String)jsonBuffer[nodeName][i]["name"];
      Serial.println(friendlyPinName);
      value = (int)jsonBuffer[nodeName][i]["value"];
      id = (String)jsonBuffer[nodeName][i]["id"];
      if(value == 1) {
        lcd.print("* " + friendlyPinName);
      } else {
        lcd.print("_ " + friendlyPinName);
      }
      
      }
    }

}
