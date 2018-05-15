// This function will run every time Blynk connection is established
BLYNK_CONNECTED() 
{
  if (isFirstConnect)
  {
    Blynk.syncVirtual(V12);//check fade time before schedule
    Blynk.syncVirtual(V10,V11,V13,V22); //Read start/stop/sunrise/sunset times, fan on temp
    Blynk.virtualWrite(V15,1);
    isFirstConnect = false;
    Blynk.run();
  }
}
// This is called when Smartphone App is opened
BLYNK_APP_CONNECTED() {
  BLYNK_LOG("App Connected");
  appConnected = true;
  if(LEDMode == 1)updateBlynkSliders();
}
// This is called when Smartphone App is closed
BLYNK_APP_DISCONNECTED() {
  BLYNK_LOG("App Disconnected");
  appConnected = false;
}
BLYNK_WRITE(V0) {// slider widget to set the maximum led level from the Blynk App.
  int value;// = param.asInt();
  float valueF = param.asFloat();
  valueF = valueF * 40.95;
  value = valueF;
  value = constrain(value,0,4095);
  //if in mode 1 or 2 take new value as currentPWM and set LED to that value for viewing purposes
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[0].tempPWM = value;
    pwm.setPWM(0, 0, LEDsettings[0].tempPWM);
    BLYNK_LOG("Setting ch1 current to %d",LEDsettings[0].tempPWM);
  }
}
BLYNK_WRITE(V1) {// slider widget to set the maximum led level from the Blynk App.
  int value;
  float valueF = param.asFloat();
  valueF = valueF * 40.95;
  value = valueF;
  value = constrain(value,0,4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[1].tempPWM = value;
    pwm.setPWM(1, 0, LEDsettings[1].tempPWM);
    BLYNK_LOG("Setting ch2 current to %d",LEDsettings[1].tempPWM);
  }
}
BLYNK_WRITE(V2) {// slider widget to set the maximum led level from the Blynk App.
  int value;
  float valueF = param.asFloat();
  valueF = valueF * 40.95;
  value = valueF;
  value = constrain(value,0,4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[2].tempPWM = value;
    pwm.setPWM(2, 0, LEDsettings[2].tempPWM);
    BLYNK_LOG("Setting ch3 current to %d",LEDsettings[2].tempPWM);
  }
}
BLYNK_WRITE(V3) {// slider widget to set the maximum led level from the Blynk App.
  int value;
  float valueF = param.asFloat();
  valueF = valueF * 40.95;
  value = valueF;
  value = constrain(value,0,4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[3].tempPWM = value;
    pwm.setPWM(3, 0, LEDsettings[3].tempPWM);
    BLYNK_LOG("Setting ch4 current to %d",LEDsettings[3].tempPWM);
  }
}
BLYNK_WRITE(V4) {// slider widget to set the maximum led level from the Blynk App.
  int value;
  float valueF = param.asFloat();
  valueF = valueF * 40.95;
  value = valueF;
  value = constrain(value,0,4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[4].tempPWM = value;
    pwm.setPWM(4, 0, LEDsettings[4].tempPWM);
    BLYNK_LOG("Setting ch5 current to %d",LEDsettings[4].tempPWM);
  }
}
BLYNK_WRITE(V5) {// slider widget to set the maximum led level from the Blynk App.
  int value;
  float valueF = param.asFloat();
  valueF = valueF * 40.95;
  value = valueF;
  value = constrain(value,0,4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[5].tempPWM = value;
    pwm.setPWM(5, 0, LEDsettings[5].tempPWM);
    BLYNK_LOG("Setting ch6 current to %d",LEDsettings[5].tempPWM);
  }
}
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
    sunsetSecond = 72000 - fadeTimeSeconds;
  }

  sprintf(Time, "%02d:%02d:%02d", sunriseSecond/3600 , (sunriseSecond / 60) % 60, 0);
  BLYNK_LOG("Sunrise time is: %s",Time);
  sprintf(Time, "%02d:%02d:%02d", sunsetSecond/3600 , (sunsetSecond / 60) % 60, 0);
  BLYNK_LOG("Sunset time is: %s",Time);
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
    stopsecond = 57600 - fadeTimeSeconds;
  }
  
  sprintf(Time, "%02d:%02d:%02d", startsecond/3600 , ((startsecond / 60) % 60), 0);
  BLYNK_LOG("Daylight start time is: %s",Time);
  sprintf(Time, "%02d:%02d:%02d", stopsecond/3600 , ((stopsecond / 60) % 60), 0);
  BLYNK_LOG("Daylight stop time is: %s",Time);
}
BLYNK_WRITE(V12) // slider widget to set the led fade duration up tp 3 hours.
{
  int value = param.asInt();
  fadeTimeSeconds = map(value, 0, 180, 60, 10800);      // 1 minute fade duration is minimum
  fadeTimeMillis  = map(value, 0, 180, 60000, 10800000);// 3 hour fade duration is maximum

  BLYNK_LOG("Fade Time in seconds: %d",fadeTimeSeconds);
}
BLYNK_WRITE(V13) //Blynk pin for moonlight time
{
  TimeInputParam t(param);

  if (t.hasStartTime())
  {
    moonStartSecond = (t.getStartHour() * 3600) + (t.getStartMinute() * 60);
  }else
  {
    //if start time is not set assume 11PM
    moonStartSecond = 82800;
  }
  if (t.hasStopTime())
  {
    moonStopSecond = ((t.getStopHour() * 3600) + (t.getStopMinute() * 60) - fadeTimeSeconds + 86400) % 86400;
  }else
  {
    //if stop time is not set assume 2AM
    moonStopSecond = (86400 + 7200 - fadeTimeSeconds) % 86400;
  }

  sprintf(Time, "%02d:%02d:%02d", moonStartSecond/3600 , (moonStartSecond / 60) % 60, 0);
  BLYNK_LOG("Moonlight start time is: %s",Time);
  sprintf(Time, "%02d:%02d:%02d", moonStopSecond/3600 , (moonStopSecond / 60) % 60, 0);
  BLYNK_LOG("Moonlight stop time is: %s",Time);
}
BLYNK_WRITE(V15) {// menu input to select LED mode
  LEDMode = param.asInt();
  int i;
  int value;
  if(LEDMode == 1) //normal operation
  {
    if (year() != 1970)
    {
      //Set LEDs to expected value based on time as long as time is set
      smartLEDStartup();
    }
  }
  if(LEDMode == 2)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPWM(i, 0, LEDsettings[i].maxPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].maxPWM;
      }
  }
  if(LEDMode == 3)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPWM(i, 0, LEDsettings[i].dimPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].dimPWM;
      }
  }
  if(LEDMode == 4)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPWM(i, 0, LEDsettings[i].moonPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].moonPWM;
      }
  }
  float valueF;
  for (i = 0; i < numCh; i = i + 1)
  {
    valueF = (float)LEDsettings[i].tempPWM / (float)40.95;
    BLYNK_LOG("Channel %d Value: %.1f",i,valueF);
    switch (i) {
      case 0:
        Blynk.virtualWrite(V0, valueF);
        break;
      case 1:
        Blynk.virtualWrite(V1, valueF);
        break;
      case 2:
        Blynk.virtualWrite(V2, valueF);
        break;
      case 3:
        Blynk.virtualWrite(V3, valueF);
        break;
      case 4:
        Blynk.virtualWrite(V4, valueF);
        break;
      case 5:
        Blynk.virtualWrite(V5, valueF);
        break;
    }
    Blynk.run();
  }
}
BLYNK_WRITE(V16) {// Save button for LED settings
  int buttonState = param.asInt();
  if(buttonState == 1)
  {
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
}
BLYNK_WRITE(V22) {
  fanOnTemp = param.asInt();
}
//Updates Blynk sliders to current value, channel is optional and if not passed to function
//the function will update all sliders
void updateBlynkSliders(boolean updateAll)
{
  float valueF;
  int i;
  if(updateAll)
  {
    for (i = 0; i < numCh; i = i + 1)
    {
      valueF = (float)LEDsettings[i].currentPWM / (float)40.95;
      BLYNK_LOG("Channel %d Current: %.1f%",i,valueF);
      switch (i) {
        case 0:
          Blynk.virtualWrite(V0, valueF);
          break;
        case 1:
          Blynk.virtualWrite(V1, valueF);
          break;
        case 2:
          Blynk.virtualWrite(V2, valueF);
          break;
        case 3:
          Blynk.virtualWrite(V3, valueF);
          break;
        case 4:
          Blynk.virtualWrite(V4, valueF);
          break;
        case 5:
          Blynk.virtualWrite(V5, valueF);
          break;
      }
      Blynk.run();
    }
  }else
  {
    unsigned int channel = second() % numCh;
    valueF = (float)LEDsettings[channel].currentPWM / (float)40.95;
    switch (channel) {
        case 0:
          Blynk.virtualWrite(V0, valueF);
          break;
        case 1:
          Blynk.virtualWrite(V1, valueF);
          break;
        case 2:
          Blynk.virtualWrite(V2, valueF);
          break;
        case 3:
          Blynk.virtualWrite(V3, valueF);
          break;
        case 4:
          Blynk.virtualWrite(V4, valueF);
          break;
        case 5:
          Blynk.virtualWrite(V5, valueF);
          break;
      }
  }
}
