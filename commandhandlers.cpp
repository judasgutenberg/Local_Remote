 #include "commandhandlers.h"


void cmdDeferredReboot(String* param, int argCount, bool deferred) {
  if(!deferred) {
    textOut(F("Rebooting... \n"));
    //notYetDeferred();
  } else {
    rebootEsp();
  }
}


void cmdRebootMasterFromSlave(String* param, int argCount, bool deferred) {
 
}


void cmdLocalUpdateFirmware(String* param, int argCount, bool deferred) {
  if(!deferred) {
    textOut(F("Attempting firmware update using file: ") + String(param[0]) + F("...\n"));
    //notYetDeferred();
  } else {
    flashFromLittleFS(param[0].c_str());
  }
}

void cmdUpdateFirmware(String* param, int argCount, bool deferred) {
 
}

/////////////////////

void cmdBeforeBoot(String* param, int argCount, bool deferred) {
 
}

void cmdQuitSafeMode(String* param, int argCount, bool deferred) {
  rtcMarkStable();
  textOut(F("Startup safe mode disabled\n"));
}

void cmdSetPreboot(String* param, int argCount, bool deferred) {
 
}
////////////////////


void cmdfastCom(String* param, int argCount, bool deferred) {
  int value = param[0].toInt();
  String blurb = F("Fast communications changing to ");
  if(!deferred) {
    if(value == 1) {
      textOut(blurb + "on\n"); 
    } else {
      textOut(blurb + "off\n"); 
    }   
  } else {
    if(value == 1) {
      startWebSocket(3);
      //prevents nasty loops:
      oldOutputMode = 0;
      outputMode = 3;
    } else {
      stopWebSocket();
      //prevents nasty loops:
      oldOutputMode = 0;
      outputMode = 0;
    }   
  }
}

void cmdResetFastCom(String* param, int argCount, bool deferred) {
  if(outputMode != 3) {
    oldOutputMode = outputMode;
    outputMode = 3;
    lastTimeOutputModeChanged = millis();
    //Serial.println("OUTPUTMODE CHANGEO7: " + String(outputMode) + " TO " + String(3));
  }
  webSocket.disconnect();
  startWebSocket(4);
  textOut(F("Fast communications reset"));
}

//////////////////////
void cmdSetGPIO(String* param, int argCount, bool deferred){
 
}

void cmdSetSlaveGPIO(String* param, int argCount, bool deferred){
 
}

void cmdGetSlaveGPIO(String* param, int argCount, bool deferred){
 
}

void cmdGetGPIO(String* param, int argCount, bool deferred){
  int pinNumber = param[0].toInt();
  int value = digitalRead(pinNumber);
  textOut(String(value) + "\n");
}

//////////////////////
void cmdInitSensors(String* param, int argCount, bool deferred) {
  //startWeatherSensors(ci[SENSOR_ID],  ci[SENSOR_SUB_TYPE], ci[SENSOR_I2C], ci[SENSOR_DATA_PIN], ci[SENSOR_POWER_PIN]);
  textOut(F("Sensors re-initialized\n"));
}

void cmdVersion(String* param, int argCount, bool deferred) {
  textOut(F("Version: ") + String(VERSION) + String("\n"));
}

void cmdRunSlaveSketch(String* param, int argCount, bool deferred) {
 
}

void cmdRunSlaveBootloader(String* param, int argCount, bool deferred) {
 
}

void cmdPetWatchdog(String* param, int argCount, bool deferred) {
 
}


void cmdGetWeatherSensors(String* param, int argCount, bool deferred) {
  /*
  int weatherLineOrdinal = (boolean)param[0].toInt();
  String transmissionString;
  if(weatherLineOrdinal == 0) {
    transmissionString = weatherDataString(ci[SENSOR_ID], ci[SENSOR_SUB_TYPE], ci[SENSOR_DATA_PIN], ci[SENSOR_POWER_PIN], ci[SENSOR_I2C], NULL, 0, deviceName, nullptr, ci[CONSOLIDATE_ALL_SENSORS_TO_ONE_RECORD]);
  } else {
    //textOut(String(additionalSensorInfo));
    transmissionString = weatherDataStringFromConfigString((char *)additionalSensorInfo.c_str(), weatherLineOrdinal);
  }
  textOut(transmissionString + "\n");
  */
}

void cmdRebootEsp(String* param, int argCount, bool deferred) {
  rebootEsp();
}

void cmdOnePinAtATime(String* param, int argCount, bool deferred) {
 
}

void cmdClearLatencyAverage(String* param, int argCount, bool deferred) {
 
}

void cmdIr(String* param, int argCount, bool deferred) {
 
}

void cmdClearFram(String* param, int argCount, bool deferred) {
 
}

void cmdDumpFram(String* param, int argCount, bool deferred) {
  
}

void cmdDumpFramHex(String* param, int argCount, bool deferred) {
 
}

void cmdDumpFramHexAt(String* param, int argCount, bool deferred) {
  
}

void cmdDumpFramRecord(String* param, int argCount, bool deferred) {
 
}

void cmdGetFramIndex(String* param, int argCount, bool deferred) {
 
}

void cmdRebootSlave(String* param, int argCount, bool deferred) {
 
}

void cmdSetDate(String* param, int argCount, bool deferred) { //params must be comma-delimited
 
}

void cmdGetDate(String* param, int argCount, bool deferred) {
 
}

void cmdGetWatchdogInfo(String* param, int argCount, bool deferred) {
 
}

void cmdGetWatchdogData(String* param, int argCount, bool deferred) {
 
}

//only works for EPS8266:
void cmdDumpGpioState(String* param, int argCount, bool deferred) {
  textOut(F("GPIO STATE DUMP\n"));
  textOut(F("GPIO | DIR | VAL\n"));
  uint32_t outReg = GPO;
  uint32_t enableReg = GPE;
  for(int gpio = 0; gpio <= 16; gpio++) {
    // skip nonexistent GPIOs
    if(gpio == 6 || gpio == 7 || gpio == 8 || gpio == 11) {
      continue;
    }
    bool isOutput;
    bool value;
    if(gpio == 16) {
      // GPIO16 is weird and separate
      isOutput = (GP16E & 1);
      value = (GP16O & 1);
    } else {
      isOutput = (enableReg & (1 << gpio));
      value = (outReg & (1 << gpio));
    }
    char sprintBuffer[60];
    sprintf(sprintBuffer, 
      "%4d | %s | %s\n",
      gpio,
      isOutput ? "OUT" : " IN",
      value ? "HIGH" : " LOW"
    );
    textOut(sprintBuffer);
  }

  textOut("\n");
}

void cmdDumpPinState(String* param, int argCount, bool deferred) {
 
}

void cmdListFiles(String* param, int argCount, bool deferred) {
  listFiles();
}

void cmdSaveMasterConfig(String* param, int argCount, bool deferred) {
  String toWord = param[0];
  String dest = param[1];
  String initialBlurb = F("Configuration saved to");
  saveAllConfigToFlash(0);
  textOut(initialBlurb + F(" local flash\n"));
}

void cmdSaveSlaveConfig(String* param, int argCount, bool deferred) {
 
}

void cmdInitMasterDefaults(String* param, int argCount, bool deferred) {
  initMasterDefaults();
  textOut("Master config initialized\n");
}

void cmdInitSlaveDefaults(String* param, int argCount, bool deferred) {
 
}

void cmdGetUptime(String* param, int argCount, bool deferred) {
  textOut(F("Last booted: ") + timeAgo("") + "\n");
}

void cmdGetWifiUptime(String* param, int argCount, bool deferred) {
  //textOut(F("WiFi up: ") + msTimeAgo(wifiOnTime) + "\n");
}

void cmdGetLastpoll(String* param, int argCount, bool deferred) {
 
}

void cmdGetLastdatalog(String* param, int argCount, bool deferred) {
  //textOut(F("Last data: ") + msTimeAgo(lastDataLogTime) + "\n");
}

void cmdTiming(String* param, int argCount, bool deferred) {
 
}

void cmdMemory(String* param, int argCount, bool deferred) {
  dumpMemoryStats(0);
}

void cmdResetInfo(String* param, int argCount, bool deferred) {
  textOut(F("Reset reason: ") + String(ESP.getResetReason()));
  textOut(F(", Reset info: ") + String(ESP.getResetInfo()));
  //textOut(F(", Reset pointer: ") + ESP.getResetInfoPtr());
  textOut("\n");
}

void cmdWifiInfo(String* param, int argCount, bool deferred) {
  textOut(F("Signal: ") + String(WiFi.RSSI()));
  textOut(F(", SSID: ") + WiFi.SSID());
  textOut("\n");
  textOut(F(" Local IP: ") + WiFi.localIP().toString());
  textOut(F(", MAC: ") + WiFi.macAddress());
  textOut("\n");
  textOut(F(" Gateway IP: ") + WiFi.gatewayIP().toString());
  textOut(F(", DNS IP: ") + WiFi.dnsIP().toString());
  textOut(F(", Subnet mask: ") + WiFi.subnetMask().toString());
  textOut("\n");
  textOut(F(" WiFi channel: ") + String(WiFi.channel()));
  textOut(F(", WiFi mode: ") + String(WiFi.getPhyMode()));
  textOut("\n");
}

ADC_MODE(ADC_VCC);

void cmdChipInfo(String* param, int argCount, bool deferred) {
  textOut(F("Flash chip size: ") + String(ESP.getFlashChipSize()));
  textOut(F(", Flash chip real size: ") + String(ESP.getFlashChipRealSize()));
  textOut(F(", Flash chip speed: ") + String(ESP.getFlashChipSpeed()));
  textOut(F(", Flash chip mode: ") + String(ESP.getFlashChipMode()));
  textOut("\n");
}

void cmdCpuInfo(String* param, int argCount, bool deferred) {
  textOut(F("Cpu frequency: ") + String(ESP.getCpuFreqMHz()));
  textOut(F(", Cycle count: ") + String(ESP.getCycleCount()));
  textOut(F("\n Chip ID: ") + String(ESP.getChipId()));
  textOut(F(", Core version: ") + String(ESP.getCoreVersion()));
  textOut(F(", SDK version: ") + String(ESP.getSdkVersion()));
  textOut(F(", Boot version: ") + String(ESP.getBootVersion()));
  textOut(F(", Boot mode: ") + String(ESP.getBootMode()));
  textOut("\n");
  textOut(F(" VCC voltage: ") + String(ESP.getVcc()));
  textOut("\n");
}

void cmdFlashInfo(String* param, int argCount, bool deferred) {
  textOut(F("Free sketch space: ") + String(ESP.getFreeSketchSpace()));
  textOut(F(", Sketch size: ") + String(ESP.getSketchSize()) + "\n");
}

void cmdDumpSlaveSerialData(String* param, int argCount, bool deferred) {
 
}

void cmdDumpMasterSerialData(String* param, int argCount, bool deferred){
 
}

void cmdFormatFileSystem(String* param, int argCount, bool deferred) {
  formatFileSystem();
  //textOut(F("File system formatted\n"));
}
///////////////////////

void cmdAnomalyLogTest(String* param, int argCount, bool deferred) {
  anomalyLog(param[0]);
}

void cmdRenameFile(String* param, int argCount, bool deferred) {
  renameFile(param[0].c_str(), param[1].c_str());
}

void cmdDel(String* param, int argCount, bool deferred) {
  deleteFile(param[0].c_str());
}

void cmdDownload(String* param, int argCount, bool deferred) {
  downloadFile(param[0].c_str(), extractFilename(param[0]).c_str());
}

void cmdUpload(String* param, int argCount, bool deferred) {
  if(fileToUpload != "") {
    textOut(fileToUpload + F(" is still uploading; please wait\n"));
    return;
  }
  fileToUpload = param[0];
  File f = LittleFS.open(fileToUpload, "r");
  if (!f) {
      textOut(fileToUpload + F(": file not found\n"));
      fileToUpload = "";
      return;
  }
  f.close();
  //do it all in one go instead of as a series of polls
  int result = postFileUpload(fileToUpload.c_str(), String(ci[DEVICE_ID]).c_str());
  //textOut(String(result));
  fileToUpload = "";
  fileUploadPosition = 0;
}

void cmdCat(String* param, int argCount, bool deferred) {
  param[0].trim();
  dumpFile(param[0].c_str());
}

void cmdReadSlaveEeprom(String* param, int argCount, bool deferred) {
 
}

///////////////////////////
 
void cmdSetSerialSwap(String* param, int argCount, bool deferred) {
 serialSwap(param[0].toInt());
 textOut(F("Serial swap set to: ") + String(param[0]) + "\n");
}

void cmdGetSerialSwap(String* param, int argCount, bool deferred) {
 
}

void cmdSetSerialLogging(String* param, int argCount, bool deferred) {
 
}

void cmdGetSerialLogging(String* param, int argCount, bool deferred) {
 
}

void cmdDumpParserConfig(String* param, int argCount, bool deferred) {
 
}

void cmdInitSerialParser(String* param, int argCount, bool deferred) {
 
}


/////////////////////

void cmdResetSerial(String* param, int argCount, bool deferred) {
  setSerialRate(115200); 
  ETS_UART_INTR_DISABLE();
  ETS_UART_INTR_ENABLE();
  textOut(F("Serial reset\n"));
}

void cmdDumpConfig(String* param, int argCount, bool deferred) {
  String fromWord = param[0];
  String source = param[1];
  loadAllConfigFromFlash(1, 0);
}

void cmdDumpSlaveEeprom(String* param, int argCount, bool deferred) {
 
}

void cmdSendSlaveSerial(String* param, int argCount, bool deferred) {
 
}

void cmdSetSlaveTime(String* param, int argCount, bool deferred) {
 
}

void cmdGetSlaveTime(String* param, int argCount, bool deferred) {
 
}

void cmdInitSlaveSerial(String* param, int argCount, bool deferred) {
 
}

void getSlaveSerial(String* param, int argCount, bool deferred) {
 
}

void getSlaveParsedDatum(String* param, int argCount, bool deferred) {
 
}

void updateSlaveFirmware(String* param, int argCount, bool deferred) {
 
}

void getMasterEepromUsed(String* param, int argCount, bool deferred) {
 
}

void getSlaveEepromUsed(String* param, int argCount, bool deferred) {
 
}

void getSlave(String* param, int argCount, bool deferred) {
 
}

void setSlaveParserBasis(String* param, int argCount, bool deferred) {
 
}


void setSlaveBasis(String* param, int argCount, bool deferred) {
 
}

void setSlave(String* param, int argCount, bool deferred) {
 
}


void runSlave(String* param, int argCount, bool deferred) {
 
}

void cmdSet(String* param, int argCount, bool deferred) {
  String key = param[0];
  String value = param[1];
 if(!isInteger(key)) {
  textOut(F("Configuration '") + key + F("' does not exist:\n"));
  return;
 }
 int configIndex = key.toInt();
 if(value != "") {
    value.trim();
    if(configIndex>=CONFIG_STRING_COUNT) {
      ci[configIndex] = value.toInt();
      //handle some specific sets in terms of what they do:
 
    } else {   
      char buffer[50];    
      value.toCharArray(buffer, sizeof(buffer));
      cs[configIndex] = strdup(buffer); 
      

    }
  }
  textOut(F("Configuration set to: ") + value + "\n");
}  


void cmdGet(String* param, int argCount, bool deferred) {
  String key = param[0];
  if(!isInteger(key)) {
    textOut(F("Configuration '") + key + "' does not exist:\n");
    return;
  }
  int configIndex = key.toInt();
  String value;
  if(configIndex>=CONFIG_STRING_COUNT) {
    value = String(ci[configIndex]);
  } else {
    value = String(cs[configIndex]);
  }
  textOut(F("Sought configuration: ") + value + "\n");
}
