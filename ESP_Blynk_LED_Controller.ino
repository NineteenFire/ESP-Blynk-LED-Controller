#include <FS.h>                   // https://github.com/tzapu/WiFiManager This Library needs to be included FIRST!
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>
#include <ArduinoOTA.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <OneWire.h> // network library to communicate with the DallasTemperature sensor, 
#include <DallasTemperature.h>  // library for the Temp sensor itself
//included libraries for WiFiManager - AutoConnectWithFSParameters
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <EEPROM.h>

byte pSDA=4;
byte pSCL=5;
byte pEnFAN=2;
byte pFANPWM=0;
byte pOneWire=13;

// WiFiManager
char blynk_token[34] = "BLYNK_TOKEN";//added from WiFiManager - AutoConnectWithFSParameters
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

bool isFirstConnect = true;

char Date[16];
char Time[16];
unsigned long startsecond = 0;        // time for LEDs to ramp to full brightness
unsigned long stopsecond = 0;         // finish time for LEDs to ramp to dim level from full brightness
unsigned long sunriseSecond = 0;      // time for LEDs to start ramping to dim level
unsigned long sunsetSecond = 0;       // finish time for LEDs to dim off
unsigned long nowseconds = 0;         // time  now  in seconds

boolean fadeInProgress = false;
unsigned long fadeStartTimeMillis = 0;
unsigned long fadeStartTimeSeconds = 0;
unsigned long fadeTimeSeconds = 0;
unsigned long fadeTimeMillis = 0;

struct LEDCh // for storing light intensity values
{
  unsigned int fadeIncrementTime;
  unsigned int currentPWM;
  unsigned int lastPWM;
  unsigned int targetPWM;
  unsigned int maxPWM;
  unsigned int dimPWM;
  unsigned int tempPWM;
  unsigned int moonPWM;
};
typedef struct LEDCh channel;
const int numCh = 6;
channel LEDsettings[numCh];

int LEDMode = 1; //1=operating, 2=Configure MaxPWM, 3=Configure dimPWM, 4=Configure moonPWM

// Data wire is plugged into port 2 on the Wemos
#define ONE_WIRE_BUS D7

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// array to hold device addresses (can increase array length if using more sensors)
DeviceAddress tempSensors[5];
int numTempSensors;

int fanOnTemp = 0;

WidgetRTC rtc;
SimpleTimer timer;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


BLYNK_WRITE(V0) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  //if in mode 1 or 2 take new value as tempPWM and set LED to that value for viewing purposes
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[0].tempPWM = value;
    pwm.setPWM(0, 0, LEDsettings[0].tempPWM);
  }
}
BLYNK_WRITE(V1) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[1].tempPWM = value;
    pwm.setPWM(1, 0, LEDsettings[1].tempPWM);
  }
}
BLYNK_WRITE(V2) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[2].tempPWM = value;
    pwm.setPWM(2, 0, LEDsettings[2].tempPWM);
  }
}
BLYNK_WRITE(V3) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[3].tempPWM = value;
    pwm.setPWM(3, 0, LEDsettings[3].tempPWM);
  }
}
BLYNK_WRITE(V4) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[4].tempPWM = value;
    pwm.setPWM(4, 0, LEDsettings[4].tempPWM);
  }
}
BLYNK_WRITE(V5) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[5].tempPWM = value;
    pwm.setPWM(5, 0, LEDsettings[5].tempPWM);
  }
}
/*
 * Can uncomment and add widgets to Blynk for additional channel control, make sure to update numCh variable
 * 
BLYNK_WRITE(V6) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[6].tempPWM = value;
    pwm.setPWM(6, 0, LEDsettings[56].tempPWM);
  }
}
BLYNK_WRITE(V7) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[7].tempPWM = value;
    pwm.setPWM(7, 0, LEDsettings[7].tempPWM);
  }
}*/


BLYNK_WRITE(V10) //Blynk pin for sunrise/sunset time (time LEDs start turning on / hit moon values)
{
  TimeInputParam t(param);

  if (t.hasStartTime())
  {
    sunriseSecond = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);
  }else
  {
    //if start time is not set assume 8AM
    sunriseSecond = 28800;
  }
  if (t.hasStopTime())
  {
    sunsetSecond = ((t.getStopHour() * 3600) + (t.getStopMinute() * 60) - fadeTimeSeconds + 86400) % 86400;
  }else
  {
    //if stop time is not set assume 8PM
    sunsetSecond = 72000;
  }

  sprintf(Time, "%02d:%02d:%02d", sunriseSecond/3600 , (sunriseSecond % 60) * 60, 0);
  Serial.print("Sunrise time is: ");
  Serial.println(Time);
  sprintf(Time, "%02d:%02d:%02d", sunsetSecond/3600 , (sunsetSecond % 60) * 60, 0);
  Serial.print("Sunset time is: ");
  Serial.println(Time);
}
BLYNK_WRITE(V11) //Blynk pin for daylight start/stop time
{
  TimeInputParam t(param);
  
  if (t.hasStartTime())
  {
    startsecond = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);
  }else
  {
    //if start time is not set assume noon
    startsecond = 43200;
  }
  if (t.hasStopTime())
  {
    stopsecond = ((t.getStopHour() * 3600) + (t.getStopMinute() * 60) - fadeTimeSeconds + 86400) % 86400;
  }else
  {
    //if stop time is not set assume 4PM
    stopsecond = 57600;
  }
  
  sprintf(Time, "%02d:%02d:%02d", startsecond/3600 , (startsecond % 60) * 60, 0);
  Serial.print("Daylight start time is: ");
  Serial.println(Time);
  sprintf(Time, "%02d:%02d:%02d", stopsecond/3600 , (stopsecond % 60) * 60, 0);
  Serial.print("Daylight stop time is: ");
  Serial.println(Time);
  /*
  if(startsecond < (sunriseSecond + fadeTimeSeconds))
  {
    //ramp to full brightness starting before sunrise finished
    Blynk.notify("Sunrise plus ramp time should be earlier than Daylight start time");
  }
  if(stopsecond > (sunsetSecond - fadeTimeSeconds))
  {
    //ramp to moonlight starting before ramp to sunset finished
    Blynk.notify("Daylight stop time should be earlier than sunset time (note: ramp finishes at sunset time)");
  }*/
}
BLYNK_WRITE(V12) // slider widget to set the led fade duration up tp 3 hours.
{
  int value = param.asInt();
  fadeTimeSeconds = map(value, 0, 180, 60, 10800);      // 1 minute fade duration is minimum
  fadeTimeMillis  = map(value, 0, 180, 60000, 10800000);// 3 hour fade duration is maximum

  Serial.print("Fade Time in seconds: ");
  Serial.println(fadeTimeSeconds);
}
BLYNK_WRITE(V15) {// menu input to select LED mode
  LEDMode = param.asInt();
  int i;
  if(LEDMode == 1) //normal operation
  {
    nowseconds = ((hour() * 3600) + (minute() * 60) + second());
    fadeInProgress=false;
    if(nowseconds < sunriseSecond)
    {
      //moonlight
      Serial.println("Setting lights to moonlight mode based on current time...");
      for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].moonPWM;}
    }else if(sunriseSecond < nowseconds < startsecond)
    {
      //sunrise 
      Serial.println("Setting lights to sunrise mode based on current time...");
      for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].dimPWM;}
    }else if(startsecond < nowseconds < stopsecond)
    {
      //daylight
      Serial.println("Setting lights to daylight mode based on current time...");
      for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].maxPWM;}
    }else if(stopsecond < nowseconds < sunsetSecond)
    {
      //sunset
      Serial.println("Setting lights to sunset mode based on current time...");
      for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].dimPWM;}
    }else if(sunsetSecond < nowseconds)
    {
      //moonlight
      Serial.println("Setting lights to moonlight mode based on current time...");
      for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].moonPWM;}
    }
    //Write current values to LEDs
    writeLEDs();
  }
  if(LEDMode == 2)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPWM(i, 0, LEDsettings[i].maxPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].maxPWM;
      }
      Blynk.virtualWrite(V0, LEDsettings[0].tempPWM);
      Blynk.virtualWrite(V1, LEDsettings[1].tempPWM);
      Blynk.virtualWrite(V2, LEDsettings[2].tempPWM);
      Blynk.virtualWrite(V3, LEDsettings[3].tempPWM);
      Blynk.virtualWrite(V4, LEDsettings[4].tempPWM);
      Blynk.virtualWrite(V5, LEDsettings[5].tempPWM);
  }
  if(LEDMode == 3)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPWM(i, 0, LEDsettings[i].dimPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].dimPWM;
      }
      Blynk.virtualWrite(V0, LEDsettings[0].tempPWM);
      Blynk.virtualWrite(V1, LEDsettings[1].tempPWM);
      Blynk.virtualWrite(V2, LEDsettings[2].tempPWM);
      Blynk.virtualWrite(V3, LEDsettings[3].tempPWM);
      Blynk.virtualWrite(V4, LEDsettings[4].tempPWM);
      Blynk.virtualWrite(V5, LEDsettings[5].tempPWM);
  }
  if(LEDMode == 4)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPWM(i, 0, LEDsettings[i].moonPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].moonPWM;
      }
      Blynk.virtualWrite(V0, LEDsettings[0].tempPWM);
      Blynk.virtualWrite(V1, LEDsettings[1].tempPWM);
      Blynk.virtualWrite(V2, LEDsettings[2].tempPWM);
      Blynk.virtualWrite(V3, LEDsettings[3].tempPWM);
      Blynk.virtualWrite(V4, LEDsettings[4].tempPWM);
      Blynk.virtualWrite(V5, LEDsettings[5].tempPWM);
  }
}
BLYNK_WRITE(V16) {// Save button for LED settings
  if(LEDMode == 2)
  {
    saveMaxPWMValues();
  }
  if(LEDMode == 3)
  {
    saveDimPWMValues();
  }
  if(LEDMode == 4)
  {
    saveMoonPWMValues();
  }
}
BLYNK_WRITE(V22) {
  fanOnTemp = param.asInt();
}

void checkSchedule()        // check if ramping should start
{
  if (year() != 1970) {
    sprintf(Date, "%02d/%02d/%04d",  day(), month(), year());
    sprintf(Time, "%02d:%02d:%02d", hour(), minute(), second());
    nowseconds = ((hour() * 3600) + (minute() * 60) + second());
  }
  
  int i;
  int difference;
  if (nowseconds == sunriseSecond) //ramp from 0 to sunrise values
  {
    Serial.println("Starting sunrise...");
    fadeInProgress = true;
    fadeStartTimeMillis = millis();
    fadeStartTimeSeconds = now();
    for (i = 0; i < numCh; i = i + 1) {
      LEDsettings[i].targetPWM = LEDsettings[i].dimPWM;
      LEDsettings[i].lastPWM = LEDsettings[i].currentPWM;
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
      LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
    }
  }
  if (nowseconds == startsecond) //ramp from sunrise to max 
  {
    Serial.println("Ramping to full brightness...");
    fadeInProgress = true;
    fadeStartTimeMillis = millis();
    fadeStartTimeSeconds = now();
    for (i = 0; i < numCh; i = i + 1) {
      LEDsettings[i].targetPWM = LEDsettings[i].maxPWM;
      LEDsettings[i].lastPWM = LEDsettings[i].currentPWM;
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
      LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
    }
  }
  if (nowseconds == stopsecond) //ramp from max to sunset values
  {
    Serial.println("Starting sunset...");
    fadeInProgress = true;
    fadeStartTimeMillis = millis();
    fadeStartTimeSeconds = now();
    for (i = 0; i < numCh; i = i + 1) {
      LEDsettings[i].targetPWM = LEDsettings[i].dimPWM;
      LEDsettings[i].lastPWM = LEDsettings[i].currentPWM;
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
      LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
    }
  }
  if (nowseconds == sunsetSecond) //ramp from sunset to off
  {
    Serial.println("Ramping LEDs off...");
    // code here to start the led fade off routine
    fadeInProgress = true;
    fadeStartTimeMillis = millis();
    fadeStartTimeSeconds = now();
    for (i = 0; i < numCh; i = i + 1) {
      LEDsettings[i].targetPWM = LEDsettings[i].moonPWM;
      LEDsettings[i].lastPWM = LEDsettings[i].currentPWM;
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
      LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
    }
  }
}

void ledFade() {
  int i;
  unsigned long timeElapsed = millis() - fadeStartTimeMillis; //time since the start of fade
  if(fadeInProgress)
  {
    for (i = 0; i < numCh; i = i + 1)
    {
      if(LEDsettings[i].lastPWM < LEDsettings[i].targetPWM)
      {
        //value increasing
        LEDsettings[i].currentPWM = LEDsettings[i].lastPWM + (timeElapsed / LEDsettings[i].fadeIncrementTime);
      }
      if(LEDsettings[i].lastPWM > LEDsettings[i].targetPWM)
      {
        //value decreasing
        LEDsettings[i].currentPWM = LEDsettings[i].lastPWM - (timeElapsed / LEDsettings[i].fadeIncrementTime);
      }
      LEDsettings[i].currentPWM = constrain(LEDsettings[i].currentPWM,0,4095);
    }
    if(now() > (fadeStartTimeSeconds + fadeTimeSeconds))
    {
      fadeInProgress = false;
      for (i = 0; i < numCh; i = i + 1) {
        LEDsettings[i].currentPWM = LEDsettings[i].targetPWM;
      }
    }
    if(LEDMode==1)
    {
      writeLEDs();
    }
  }
}

void writeLEDs() {
  int i;
  for (i = 0; i < numCh; i = i + 1) {
    pwm.setPWM(i, 0, LEDsettings[i].currentPWM);
  }
}

// Digital clock display of the time
void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(month()) + " " + day() + " " + year();
  nowseconds = ((hour() * 3600) + (minute() * 60) + second());
  Serial.print("Time =");
  Serial.println(currentTime);
  Serial.print("Nowseconds =");
  Serial.println(nowseconds);
  Serial.print("Start = ");
  Serial.println(startsecond);
  Serial.print("Stop = ");
  Serial.println(stopsecond);
  Serial.println();
  Blynk.virtualWrite(V8, currentTime);
}

void reconnectBlynk() {
  if (!Blynk.connected())
  {
    digitalWrite(BUILTIN_LED, HIGH);
    if (Blynk.connect()) {
      BLYNK_LOG("Reconnected");
    }
    else {
      digitalWrite(BUILTIN_LED, LOW);
      BLYNK_LOG("Not reconnected");
    }
  }else
  {
    digitalWrite(BUILTIN_LED, LOW);
  }
  //Print LED values to serial once per minute for debugging
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  nowseconds = ((hour() * 3600) + (minute() * 60) + second());
  Serial.print("Time: ");
  Serial.print(currentTime);
  Serial.print("\tLEDs: ");
  for (int i = 0; i < numCh; i = i + 1) {
        Serial.print(LEDsettings[i].currentPWM);
        if(i != numCh)Serial.print(", ");
      }
  Serial.print("\n");
}

void checkTemp()
{
  //read all sensors and keep track of maximum temperature
  sensors.requestTemperatures();
  float temperature[numTempSensors];
  float maxTemperature = 0.0;
  for(int i; i < numTempSensors ; i++)
  {
    temperature[i] = sensors.getTempC(tempSensors[i]);
    if(temperature[i] > maxTemperature)maxTemperature=temperature[i];
  }
  
  //write the maximum temperature to Blynk
  Blynk.virtualWrite(V20, maxTemperature);
  
  //if higher than fan set temperature then turn fan on
  int intTemp = maxTemperature; //temp as integer
  if(intTemp > fanOnTemp)
  {
    //Calculate PWM by temp over fanOnTemp, at 0C above run at 10%, 20C above 100%
    int fanPWM = intTemp - fanOnTemp;
    fanPWM = constrain(fanPWM,0,20);
    fanPWM = map(fanPWM,0,20,10,1023);
    analogWrite(pFANPWM,fanPWM);
    //turn fan on
    digitalWrite(pEnFAN,HIGH);
  }
  if(intTemp < (fanOnTemp-2))
  {
    //turn fan off
    digitalWrite(pEnFAN,LOW);
    analogWrite(pFANPWM,0);
  }
}

void flip()
{
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}


void setup()
{
  Wire.begin(pSDA,pSCL);
  
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(pFANPWM, OUTPUT);
  pinMode(pEnFAN, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  digitalWrite(pFANPWM, LOW);
  digitalWrite(pEnFAN, LOW);
  
  ArduinoOTA.begin();
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(500);

  //Start dallas temperature temp sensors
  sensors.begin();
  // locate devices on the bus
  numTempSensors = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(numTempSensors);
  Serial.println(" temperature devices.");
  for( int i = 0 ; i < numTempSensors ; i++)
  {
    if (!sensors.getAddress(tempSensors[i], i))
    {
      Serial.print("Unable to find address for Device "); 
      Serial.println(i);
    }
  }

  //The following code is borrowed from WiFiManager
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(blynk_token, json["blynk_token"]);

        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
  Serial.println(blynk_token);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_blynk_token);

  //reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(180);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("LED AP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("LED Controller connected :)");

  //read updated parameters
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Blynk.config(blynk_token);
  Blynk.connect();

  rtc.begin();
  Blynk.notify("LED Controller ONLINE");
  Serial.println("Done");
  
  //Blynk.setProperty(V0, "color", "#F2FAFF"); // COOL WHITE
  //Blynk.setProperty(V1, "color", "#FFEDE0"); // WARM WHITE
  //Blynk.setProperty(V2, "color", "#FD5635"); // RED
  //Blynk.setProperty(V3, "color", "#BFFFB3"); // LIME
  //Blynk.setProperty(V4, "color", "#2E28FF"); // ROYAL BLUE
  //Blynk.setProperty(V5, "color", "#2ECEFF"); // CYAN
  //Blynk.setProperty(V6, "color", "#FD5635"); // RED
  //Blynk.setProperty(V7, "color", "#BD1313"); // DEEP RED

  //Read LED settings from EEPROM
  if(checkEEPROMPWM() == false)
  {
    //if the EEPROM has not been set previously clear memory
    clearPWMValues();
  }
  readPWMValues();

  Blynk.virtualWrite(V15,1); //Ensure mode is set to normal at startup
  Blynk.syncVirtual(V12);//check fade time before schedule so we can check for ramp collisions
  Blynk.syncVirtual(V10,V11,V22); //Read start/stop/sunrise/sunset times, fade duration, fan on temp
  Blynk.syncVirtual(V15);//make sure we have schedule loaded before starting normal operation
  
  timer.setInterval(500L, ledFade);           // adjust the led lighting every 500ms
  timer.setInterval(1000L, checkSchedule);    // check every second if fade should start
  timer.setInterval(10000L, checkTemp);       // check heatsink temperature every 10 seconds
  timer.setInterval(60000L, reconnectBlynk);  // check every 60s if still connected to server

  digitalWrite(BUILTIN_LED, LOW); //ensure light is OFF

  Serial.println("End of startup.");
}
void loop()
{
  ArduinoOTA.handle();
  timer.run();
  if (Blynk.connected()) {
    Blynk.run();
  }
}
