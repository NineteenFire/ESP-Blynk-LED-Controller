//#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space

#include <FS.h>                   // https://github.com/tzapu/WiFiManager This Library needs to be included FIRST!
#include <BlynkSimpleEsp8266.h>
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

void updateBlynkSliders(boolean updateAll=true);

byte pSDA=4;
byte pSCL=5;
byte pEnFAN=16;
byte pFANPWM=0;
byte pOneWire=13;
byte pOnBoardLED=2;

// WiFiManager
char blynk_token[34] = "BLYNK_TOKEN";//added from WiFiManager - AutoConnectWithFSParameters
//flag for saving data
bool shouldSaveConfig = false;
//callback notifying the need to save config
void saveConfigCallback () {
  BLYNK_LOG("Should save config");
  shouldSaveConfig = true;
}

bool isFirstConnect = true;
bool smartStartupRun = false;
bool appConnected = false;

char Time[16];

unsigned long startsecond = 0;        // time for LEDs to ramp to full brightness
unsigned long stopsecond = 0;         // finish time for LEDs to ramp to dim level from full brightness
unsigned long sunriseSecond = 0;      // time for LEDs to start ramping to dim level
unsigned long sunsetSecond = 0;       // finish time for LEDs to dim off
unsigned long moonStartSecond = 0;      // time for LEDs to start ramping to moonlight level
unsigned long moonStopSecond = 0;       // time for LEDs to ramp off
unsigned long nowseconds = 0;         // time  now  in seconds

//0=not ramping, 3=moon->dim, 4=dim->max, 5=max->dim, 6=dim->moon
unsigned int fadeInProgress = 0;
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
DeviceAddress tempSensors[3];
int numTempSensors;
int fanOnTemp = 0;

WidgetRTC rtc;
WidgetLED fanLED(V14);
BlynkTimer timer;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup()
{
  Wire.begin(pSDA,pSCL);
  //Wire.setClock(400000L);
  ArduinoOTA.begin();
  Serial.begin(115200);
  
  pinMode(pOnBoardLED, OUTPUT);
  pinMode(pFANPWM, OUTPUT);
  pinMode(pEnFAN, OUTPUT);
  digitalWrite(pOnBoardLED, LOW);
  digitalWrite(pFANPWM, LOW);
  digitalWrite(pEnFAN, LOW);
  
  pwm.begin();
  pwm.setPWMFreq(250);

  //Start dallas temperature temp sensors
  sensors.begin();
  // locate devices on the bus
  numTempSensors = sensors.getDeviceCount();
  BLYNK_LOG("Found %d temperature sensors.",numTempSensors);
  for( int i = 0 ; i < numTempSensors ; i++)
  {
    if (!sensors.getAddress(tempSensors[i], i))
    {
      BLYNK_LOG("Unable to find address for Device %d",i);
    }
  }

  //The following code is borrowed from WiFiManager
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  BLYNK_LOG("Mounting FS...");

  if (SPIFFS.begin()) {
    BLYNK_LOG("Mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      BLYNK_LOG("Config file exists");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        BLYNK_LOG("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          BLYNK_LOG("Parsed json");
          strcpy(blynk_token, json["blynk_token"]);
          BLYNK_LOG("Blynk Token in memory: %s",blynk_token);
        } else {
          BLYNK_LOG("Failed to load json config");
        }
      }
    }else{
      BLYNK_LOG("File config.json does not exist",blynk_token);
    }
  } else {
    BLYNK_LOG("Failed to mount FS");
  }
  //end read
  
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
  
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  // Set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Add blynk parameter here
  wifiManager.addParameter(&custom_blynk_token);

  // Reset settings - for testing
  //wifiManager.resetSettings();

  //set minimu quality of signal so it ignores AP's under that quality (default: 8%)
  wifiManager.setMinimumSignalQuality();

  // Sets timeout until configuration portal gets turned off
  // useful to make it all retry or go to sleep
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("LED AP")) {
    //Serial.println("failed to connect and hit timeout");
    BLYNK_LOG("Failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  BLYNK_LOG("LED Controller connected :)");

  //read updated parameters
  strcpy(blynk_token, custom_blynk_token.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    BLYNK_LOG("Saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["blynk_token"] = blynk_token;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      BLYNK_LOG("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  setSyncInterval(1000); //make sure time syncs right away
  Blynk.config(blynk_token);
  while (Blynk.connect() == false)
  {
    // Wait until connected, need to handle case where wifi connects but blynk auth invalid...
    
    if((shouldSaveConfig == false)&&(blynk_token == "BLYNK_TOKEN"))
    {
      //This will erase WiFi settings forcing a new AP where Blynk_token can be entered
      //WiFi.disconnect(true); 
      //ESP.reset();
      //OR
      WiFi.begin("FakeSSID","FakePW");     // Replace current WiFi credentials with fake ones
      delay(1000);
      ESP.restart();
    }
    
  }
  rtc.begin();

  //Can update slider colors here if desired
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
  
  timer.setInterval(500L, ledFade);           // adjust the led lighting every 500ms
  timer.setInterval(1000L, checkSchedule);    // check every second if fade should start
  timer.setInterval(15000L, checkTemp);       // check heatsink temperature every 15 seconds
  timer.setInterval(60000L, reconnectBlynk);  // check every 60s if still connected to server
  
  digitalWrite(pOnBoardLED, HIGH); //ensure light is OFF
  Blynk.notify("LED Controller ONLINE");
  BLYNK_LOG("End of startup.");
}
void loop()
{
  ArduinoOTA.handle();
  if (Blynk.connected()) {
    Blynk.run();
  }
  timer.run();
}

//===Functions===

void reconnectBlynk() {
  if (!Blynk.connected())
  {
    //Turn onboard LED on to indiciate there is an issue
    digitalWrite(pOnBoardLED, LOW);
    if (Blynk.connect())
    {
      digitalWrite(pOnBoardLED, HIGH);
      BLYNK_LOG("Reconnected");
    }
    else {
      digitalWrite(pOnBoardLED, LOW);
      BLYNK_LOG("Not reconnected");
    }
  }else
  {
    //Turn onboard LED off
    digitalWrite(pOnBoardLED, HIGH);
    clockDisplay();
  }
}

void checkSchedule()        // check if ramping should start
{
  if (year() != 1970) 
  {
    nowseconds = ((hour() * 3600) + (minute() * 60) + second());
    //nowseconds = elapsedSecsToday(now());
    unsigned long timingInfo = startsecond + sunriseSecond + moonStartSecond + fadeTimeSeconds;
    if((smartStartupRun == false)&&(timingInfo != 0))
    {
      smartLEDStartup();
      smartStartupRun = true;
      setSyncInterval(600000); //set time sync to every 10 minutes
    }
  }else
  {
    nowseconds = 86401; //set seconds out of range to avoid un-wanted ramps
  }
  
  int i;
  int difference;
  if((nowseconds == sunriseSecond)&&(fadeInProgress != 4)) //ramp from 0 to sunrise values
  {
    sprintf(Time, "%02d:%02d:%02d", hour() , minute(), second());
    BLYNK_LOG("%s\tStarting sunrise...",Time);
    
    fadeInProgress = 4;
    fadeStartTimeMillis = millis();
    fadeStartTimeSeconds = now();
    for(i = 0; i < numCh; i = i + 1) {
      LEDsettings[i].targetPWM = LEDsettings[i].dimPWM;
      LEDsettings[i].lastPWM = LEDsettings[i].currentPWM;
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
      LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
    }
  }
  else if((nowseconds == startsecond)&&(fadeInProgress != 5)) //ramp from sunrise to max 
  {
    sprintf(Time, "%02d:%02d:%02d", hour() , minute(), second());
    BLYNK_LOG("%s\tRamping to full brightness...",Time);
    
    fadeInProgress = 5;
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
  else if((nowseconds == stopsecond)&&(fadeInProgress != 6)) //ramp from max to sunset values
  {
    sprintf(Time, "%02d:%02d:%02d", hour() , minute(), second());
    BLYNK_LOG("%s\tStarting sunset...",Time);
    
    fadeInProgress = 6;
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
  else if((nowseconds == sunsetSecond)&&(fadeInProgress != 7)) //ramp from sunset to off
  {
    sprintf(Time, "%02d:%02d:%02d", hour() , minute(), second());
    BLYNK_LOG("%s\tRamping LEDs off...",Time);
    
    fadeInProgress = 7;
    fadeStartTimeMillis = millis();
    fadeStartTimeSeconds = now();
    for (i = 0; i < numCh; i = i + 1) {
      LEDsettings[i].targetPWM = 0;
      LEDsettings[i].lastPWM = LEDsettings[i].currentPWM;
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
      LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
    }
  }
  else if((nowseconds == moonStartSecond)&&(fadeInProgress != 8)) //ramp from off to moon
  {
    sprintf(Time, "%02d:%02d:%02d", hour() , minute(), second());
    BLYNK_LOG("%s\tRamping LEDs to moonlight...",Time);
    
    fadeInProgress = 8;
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
  else if((nowseconds == moonStopSecond)&&(fadeInProgress != 9)) //ramp off from moon
  {
    sprintf(Time, "%02d:%02d:%02d", hour() , minute(), second());
    BLYNK_LOG("%s\tRamping LEDs to moonlight...",Time);
    
    fadeInProgress = 9;
    fadeStartTimeMillis = millis();
    fadeStartTimeSeconds = now();
    for (i = 0; i < numCh; i = i + 1) {
      LEDsettings[i].targetPWM = 0;
      LEDsettings[i].lastPWM = LEDsettings[i].currentPWM;
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
      LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
    }
  }
  if(appConnected)
  {
    //if we're in normal mode and ramping keep the sliders updated
    if((LEDMode == 1)&&(fadeInProgress != 0))
    {
      //only update one slider per second to reduce traffic
      updateBlynkSliders(false);
    }
  }
}

void ledFade()
{
  int i;
  if(fadeInProgress != 0)
  {
    unsigned long timeElapsed = millis() - fadeStartTimeMillis; //time since the start of fade
    for (i = 0; i < numCh; i = i + 1)
    {
      if(LEDsettings[i].currentPWM != LEDsettings[i].targetPWM)
      {
        //if adjustments are necessary
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
        //if target == last set to target
        if(LEDsettings[i].lastPWM == LEDsettings[i].targetPWM)
        {
          LEDsettings[i].currentPWM = LEDsettings[i].targetPWM;
        }
      }
      LEDsettings[i].currentPWM = constrain(LEDsettings[i].currentPWM,0,4095);
    }
    if(now() > (fadeStartTimeSeconds + fadeTimeSeconds))
    {
      fadeInProgress = 0;
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
    pwm.setPWM(i,0,LEDsettings[i].currentPWM);
  }
}

void checkTemp()
{
  //read all sensors and keep track of maximum temperature
  sensors.requestTemperatures();
  float temperature[numTempSensors];
  float maxTemperature = 0.0;
  int i = 0;
  for(i = 0 ; i < numTempSensors ; i++)
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
    fanLED.on();
  }
  if(intTemp < (fanOnTemp-2))
  {
    //turn fan off
    digitalWrite(pEnFAN,LOW);
    analogWrite(pFANPWM,0);
    fanLED.off();
  }
}

void smartLEDStartup()
{
  int i = 0;
  int difference;
  int secondsIntoRamp = 0;
  unsigned long tempFadeTimeMillis;
  nowseconds = ((hour() * 3600) + (minute() * 60) + second());
  BLYNK_LOG("nowseconds: %d", nowseconds);

  //0=off, 1=moonPWM, 2=dimPWM, 3=maxPWM, 
  //4=Ramp off->dim, 5=Ramp dim->max, 6=Ramp max->dim, 7=Ramp dim->off, 8=Ramp off->moon, 9=Ramp moon->off
  byte LEDStartMode = 99; 

  /* General shape of ramps:
   * |                2e-----3s                        |         -(maxPWM)
   * |               /         \                       |
   * |              /           \                      |
   * |      1e----2s             3e-----4s             |         -(dimPWM)
   * |     /                              \         5e-|-6s      -(moonPWM)
   * |    /                                \       /   |   \
   * |--1s                                  4e---5s    |    6e-- -(off)
   * |_________________________________________________|________
   */

  for(i=0 ; i < nowseconds ; i++)//go through the schedule up to the current time
  {
    if(i == sunriseSecond){LEDStartMode = 4;secondsIntoRamp = 0;}
    if((i == (sunriseSecond + fadeTimeSeconds))&&(LEDStartMode == 4))LEDStartMode = 2;
    if(i == startsecond){LEDStartMode = 5;secondsIntoRamp = 0;}
    if((i == (startsecond + fadeTimeSeconds))&&(LEDStartMode == 5))LEDStartMode = 3;
    if(i == stopsecond){LEDStartMode = 6;secondsIntoRamp = 0;}
    if((i == (stopsecond + fadeTimeSeconds))&&(LEDStartMode == 6))LEDStartMode = 2;
    if(i == sunsetSecond){LEDStartMode = 7;secondsIntoRamp = 0;}
    if((i == (sunsetSecond + fadeTimeSeconds))&&(LEDStartMode == 7))LEDStartMode = 0;
    if(i == moonStartSecond){LEDStartMode = 8;secondsIntoRamp = 0;}
    if((i == (moonStartSecond + fadeTimeSeconds))&&(LEDStartMode == 8))LEDStartMode = 1;
    if(i == moonStopSecond){LEDStartMode = 9;secondsIntoRamp = 0;}
    if((i == (moonStopSecond + fadeTimeSeconds))&&(LEDStartMode == 9))LEDStartMode = 0;
    secondsIntoRamp++;
  }
  if(LEDStartMode == 99)//if we don't hit a ramp, ie if it's 12:01AM, go backwards
  {
    i = 86399;
    while((LEDStartMode == 99)&&(i > nowseconds))
    {
      if(i == sunriseSecond)LEDStartMode = 4;
      if(i == (sunriseSecond + fadeTimeSeconds))LEDStartMode = 2;
      if(i == startsecond)LEDStartMode = 5;
      if(i == (startsecond + fadeTimeSeconds))LEDStartMode = 3;
      if(i == stopsecond)LEDStartMode = 6;
      if(i == (stopsecond + fadeTimeSeconds))LEDStartMode = 2;
      if(i == sunsetSecond)LEDStartMode = 7;
      if(i == (sunsetSecond + fadeTimeSeconds))LEDStartMode = 0;
      if(i == moonStartSecond)LEDStartMode = 8;
      if(i == (moonStartSecond + fadeTimeSeconds))LEDStartMode = 1;
      if(i == moonStopSecond)LEDStartMode = 9;
      if(i == (moonStopSecond + fadeTimeSeconds))LEDStartMode = 0;
      secondsIntoRamp = 86400 - i;
      i--;
    }
  }
  
  //set LEDs based on LEDStartMode
  if(LEDStartMode == 0)
  {
    BLYNK_LOG("Setting lights off based on current time...");
    for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = 0;}
    fadeInProgress = 0;
  }
  else if(LEDStartMode == 1)
  {
    BLYNK_LOG("Setting lights to moonlight mode based on current time...");
    for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].moonPWM;}
    fadeInProgress = 0;
  }
  else if(LEDStartMode == 2)
  {
    BLYNK_LOG("Setting lights to sunrise/set levels based on current time...");
    for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].dimPWM;}
    fadeInProgress = 0;
  }
  else if(LEDStartMode == 3)
  {
    BLYNK_LOG("Setting lights to daylight levels based on current time...");
    for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].maxPWM;}
    fadeInProgress = 0;
  }
  else //if we're mid ramp
  {
    fadeInProgress = LEDStartMode;
    if(LEDStartMode == 4)
    {
      BLYNK_LOG("Setting lights to sunrise mode based on current time...");
      for (i = 0; i < numCh; i = i + 1) {
        LEDsettings[i].targetPWM = LEDsettings[i].dimPWM;
        LEDsettings[i].lastPWM = 0;
      }
    }
    if(LEDStartMode == 5)
    {
      BLYNK_LOG("Setting lights to ramp up to daylight mode based on current time...");
      for (i = 0; i < numCh; i = i + 1) {
        LEDsettings[i].targetPWM = LEDsettings[i].maxPWM;
        LEDsettings[i].lastPWM = LEDsettings[i].dimPWM;
      }
    }
    if(LEDStartMode == 6)
    {
      BLYNK_LOG("Setting lights to ramp down from daylight based on current time...");
      for (i = 0; i < numCh; i = i + 1) {
        LEDsettings[i].targetPWM = LEDsettings[i].dimPWM;
        LEDsettings[i].lastPWM = LEDsettings[i].maxPWM;
      }
    }
    if(LEDStartMode == 7)
    {
      BLYNK_LOG("Setting lights to sunset mode based on current time...");
      for (i = 0; i < numCh; i = i + 1) {
        LEDsettings[i].targetPWM = 0;
        LEDsettings[i].lastPWM = LEDsettings[i].dimPWM;
      }
    }
    if(LEDStartMode == 8)
    {
      BLYNK_LOG("Setting lights to moonrise mode based on current time...");
      for (i = 0; i < numCh; i = i + 1) {
        LEDsettings[i].targetPWM = LEDsettings[i].moonPWM;
        LEDsettings[i].lastPWM = 0;
      }
    }
    if(LEDStartMode == 9)
    {
      BLYNK_LOG("Setting lights to moonfall mode based on current time...");
      for (i = 0; i < numCh; i = i + 1) {
        LEDsettings[i].targetPWM = 0;
        LEDsettings[i].lastPWM = LEDsettings[i].moonPWM;
      }
    }
    
    //calculate currentPWM based on where we are in ramp
    for (i = 0; i < numCh; i = i + 1) {
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM)
      {
        difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);
        LEDsettings[i].currentPWM = LEDsettings[i].lastPWM + ((difference * secondsIntoRamp) / fadeTimeSeconds);
      }
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM)
      {
        difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);
        LEDsettings[i].currentPWM = LEDsettings[i].lastPWM - ((difference * secondsIntoRamp) / fadeTimeSeconds);
      }
    }
    //start a new ramp beginning at current time using time remaining of ramp
    fadeStartTimeMillis = millis();
    fadeStartTimeSeconds = now();
    tempFadeTimeMillis = (fadeTimeSeconds - secondsIntoRamp)*1000;
    for (i = 0; i < numCh; i = i + 1) {
      LEDsettings[i].lastPWM = LEDsettings[i].currentPWM;
      if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
      if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
      LEDsettings[i].fadeIncrementTime = tempFadeTimeMillis / difference;
    }
  }
  writeLEDs();
}

// returns scaling factor for current day in lunar cycle
byte lunarCycleScaling()
{
  byte scaleFactor[] = {25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,
                      95,90,85,80,75,70,65,60,55,50,45,40,35,30};
  tmElements_t fixedDate = {0,35,20,0,7,1,0};
  long lp = 2551443;
  time_t newMoonCycle = makeTime(fixedDate);
  long phase = (now() - newMoonCycle) % lp;
  long returnValue = ((phase / 86400) + 1);
  return scaleFactor[returnValue];
}

// returns day in lunar cycle 0-29
byte getLunarCycleDay()
{
  tmElements_t fixedDate = {0,35,20,0,7,1,0};
  long lp = 2551443;
  time_t newMoonCycle = makeTime(fixedDate);
  long phase = (now() - newMoonCycle) % lp;
  long returnValue = ((phase / 86400) + 1);
  return returnValue;
}
// Digital clock display of the time
void clockDisplay()
{
  if(isAM())sprintf(Time, "%d:%02d AM", hourFormat12() , minute());
  if(isPM())sprintf(Time, "%d:%02d PM", hourFormat12() , minute());
  Blynk.virtualWrite(V9, Time);
}
