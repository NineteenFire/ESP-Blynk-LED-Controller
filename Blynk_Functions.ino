// This function will run every time Blynk connection is established
BLYNK_CONNECTED() 
{
  if (isFirstConnect)
  {
    Blynk.syncVirtual(V12);//check fade time before schedule
    Blynk.syncVirtual(V10,V11,V22); //Read start/stop/sunrise/sunset times, fade duration, fan on temp
    Blynk.virtualWrite(V15,1);
    isFirstConnect = false;
  }
}
BLYNK_WRITE(V0) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  value = map(value, 0, 100, 0, 4095);
  //if in mode 1 or 2 take new value as tempPWM and set LED to that value for viewing purposes
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[0].tempPWM = value;
    pwm.setPin(0, LEDsettings[0].tempPWM);
  }
}
BLYNK_WRITE(V1) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  value = map(value, 0, 100, 0, 4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[1].tempPWM = value;
    pwm.setPin(1, LEDsettings[1].tempPWM);
  }
}
BLYNK_WRITE(V2) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  value = map(value, 0, 100, 0, 4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[2].tempPWM = value;
    pwm.setPin(2, LEDsettings[2].tempPWM);
  }
}
BLYNK_WRITE(V3) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  value = map(value, 0, 100, 0, 4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[3].tempPWM = value;
    pwm.setPin(3, LEDsettings[3].tempPWM);
  }
}
BLYNK_WRITE(V4) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  value = map(value, 0, 100, 0, 4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[4].tempPWM = value;
    pwm.setPin(4, LEDsettings[4].tempPWM);
  }
}
BLYNK_WRITE(V5) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  value = map(value, 0, 100, 0, 4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[5].tempPWM = value;
    pwm.setPin(5, LEDsettings[5].tempPWM);
  }
}
/*
 * Can uncomment and add widgets to Blynk for additional channel control, make sure to update numCh variable
 * 
BLYNK_WRITE(V6) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  value = map(value, 0, 100, 0, 4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[6].tempPWM = value;
    pwm.setPin(6, LEDsettings[56].tempPWM);
  }
}
BLYNK_WRITE(V7) {// slider widget to set the maximum led level from the Blynk App.
  int value = param.asInt();
  value = map(value, 0, 100, 0, 4095);
  if((LEDMode == 2)||(LEDMode == 3)||(LEDMode == 4))
  {
    LEDsettings[7].tempPWM = value;
    pwm.setPin(7, LEDsettings[7].tempPWM);
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
    sunsetSecond = 72000 - fadeTimeSeconds;
  }

  sprintf(Time, "%02d:%02d:%02d", sunriseSecond/3600 , (sunriseSecond / 60) % 60, 0);
  Serial.print("Sunrise time is: ");
  Serial.println(Time);
  sprintf(Time, "%02d:%02d:%02d", sunsetSecond/3600 , (sunsetSecond / 60) % 60, 0);
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
    stopsecond = 57600 - fadeTimeSeconds;
  }
  
  sprintf(Time, "%02d:%02d:%02d", startsecond/3600 , ((startsecond / 60) % 60), 0);
  Serial.print("Daylight start time is: ");
  Serial.println(Time);
  sprintf(Time, "%02d:%02d:%02d", stopsecond/3600 , ((stopsecond / 60) % 60), 0);
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
  int value;
  if(LEDMode == 1) //normal operation
  {
    if (year() != 1970)
    {
      //Set LEDs to expected value based on time as long as time is set
      smartLEDStartup();
    }
    //Set sliders to 0 since we don't keep updating them during normal operation
    Blynk.virtualWrite(V0, 0);
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 0);
    Blynk.virtualWrite(V3, 0);
    Blynk.virtualWrite(V4, 0);
    Blynk.virtualWrite(V5, 0);
  }
  if(LEDMode == 2)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPin(i, LEDsettings[i].maxPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].maxPWM;
      }
      value = map(LEDsettings[0].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V0, value);
      value = map(LEDsettings[1].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V1, value);
      value = map(LEDsettings[2].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V2, value);
      value = map(LEDsettings[3].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V3, value);
      value = map(LEDsettings[4].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V4, value);
      value = map(LEDsettings[5].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V5, value);
  }
  if(LEDMode == 3)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPin(i, LEDsettings[i].dimPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].dimPWM;
      }
      value = map(LEDsettings[0].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V0, value);
      value = map(LEDsettings[1].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V1, value);
      value = map(LEDsettings[2].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V2, value);
      value = map(LEDsettings[3].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V3, value);
      value = map(LEDsettings[4].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V4, value);
      value = map(LEDsettings[5].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V5, value);
  }
  if(LEDMode == 4)
  {
    for (i = 0; i < numCh; i = i + 1) {
        pwm.setPin(i, LEDsettings[i].moonPWM);
        LEDsettings[i].tempPWM = LEDsettings[i].moonPWM;
      }
      value = map(LEDsettings[0].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V0, value);
      value = map(LEDsettings[1].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V1, value);
      value = map(LEDsettings[2].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V2, value);
      value = map(LEDsettings[3].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V3, value);
      value = map(LEDsettings[4].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V4, value);
      value = map(LEDsettings[5].tempPWM, 0, 4095, 0, 100);
      Blynk.virtualWrite(V5, value);
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
