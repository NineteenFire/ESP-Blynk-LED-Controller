// This function will run every time Blynk connection is established
BLYNK_CONNECTED() 
{
  if (isFirstConnect)
  {
    Blynk.syncVirtual(V12);//check fade time before schedule
    Blynk.syncVirtual(V10,V11,V22); //Read start/stop/sunrise/sunset times, fade duration, fan on temp
    //Blynk.syncVirtual(V15);//make sure we have schedule loaded before starting normal operation
    Blynk.virtualWrite(V15,1);
    isFirstConnect = false;
  }
}
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
  int difference;
  if(LEDMode == 1) //normal operation
  {
    if (year() != 1970)
    {
      nowseconds = ((hour() * 3600) + (minute() * 60) + second());
      //s = start, e=end
      int ramp1s = sunriseSecond;
      int ramp1e = (sunriseSecond + fadeTimeSeconds) % 86400;
      int ramp2s = startsecond;
      int ramp2e = (startsecond + fadeTimeSeconds) % 86400;
      int ramp3s = stopsecond;
      int ramp3e = (stopsecond + fadeTimeSeconds) % 86400;
      int ramp4s = sunsetSecond;
      int ramp4e = (sunsetSecond + fadeTimeSeconds) % 86400;
      
      //0=moonPWM, 1=dimPWM, 2=maxPWM, 3=Ramp moon->dim, 4=Ramp dim->max, 5=Ramp max->dim, 6=Ramp dim->moon
      byte LEDStartMode = 0; 
  
      /* General shape of ramps:
       * |                2e-----3s
       * |               /         \
       * |       1e----2s           3e-----4s
       * |      /                            \
       * |----1s                              4e----
       * |__________________________________________
       */
  
      //ramp 1 ends after midnight
           if((ramp1e < ramp1s)&&(nowseconds > ramp1s))LEDStartMode=3;
      else if((ramp1e < ramp1s)&&(nowseconds < ramp1e))LEDStartMode=3;
      else if((ramp1e < ramp1s)&&(nowseconds > ramp1e)&&(nowseconds < ramp2s))LEDStartMode=1;
      else if((ramp1e < ramp1s)&&(nowseconds > ramp2s)&&(nowseconds < ramp2e))LEDStartMode=4;
      else if((ramp1e < ramp1s)&&(nowseconds > ramp2e)&&(nowseconds < ramp3s))LEDStartMode=2;
      else if((ramp1e < ramp1s)&&(nowseconds > ramp3s)&&(nowseconds < ramp3e))LEDStartMode=5;
      else if((ramp1e < ramp1s)&&(nowseconds > ramp3e)&&(nowseconds < ramp4s))LEDStartMode=1;
      else if((ramp1e < ramp1s)&&(nowseconds > ramp4s)&&(nowseconds < ramp4e))LEDStartMode=6;
      else if((ramp1e < ramp1s)&&(nowseconds > ramp4e)&&(nowseconds < ramp1s))LEDStartMode=0;
  
      //ramp 2 starts after midnight
      else if((ramp2s < ramp1e)&&(nowseconds > ramp1e))LEDStartMode=1;
      else if((ramp2s < ramp1e)&&(nowseconds < ramp2s))LEDStartMode=1;
      else if((ramp2s < ramp1e)&&(nowseconds > ramp2s)&&(nowseconds < ramp2e))LEDStartMode=4;
      else if((ramp2s < ramp1e)&&(nowseconds > ramp2e)&&(nowseconds < ramp3s))LEDStartMode=2;
      else if((ramp2s < ramp1e)&&(nowseconds > ramp3s)&&(nowseconds < ramp3e))LEDStartMode=5;
      else if((ramp2s < ramp1e)&&(nowseconds > ramp3e)&&(nowseconds < ramp4s))LEDStartMode=1;
      else if((ramp2s < ramp1e)&&(nowseconds > ramp4s)&&(nowseconds < ramp4e))LEDStartMode=6;
      else if((ramp2s < ramp1e)&&(nowseconds > ramp4e)&&(nowseconds < ramp1s))LEDStartMode=0;
      else if((ramp2s < ramp1e)&&(nowseconds > ramp1s)&&(nowseconds < ramp1e))LEDStartMode=3;
      
      //ramp 2 ends after midnight
      else if((ramp2e < ramp2s)&&(nowseconds > ramp2s))LEDStartMode=4;
      else if((ramp2e < ramp2s)&&(nowseconds < ramp2e))LEDStartMode=4;
      else if((ramp2e < ramp2s)&&(nowseconds > ramp2e)&&(nowseconds < ramp3s))LEDStartMode=2;
      else if((ramp2e < ramp2s)&&(nowseconds > ramp3s)&&(nowseconds < ramp3e))LEDStartMode=5;
      else if((ramp2e < ramp2s)&&(nowseconds > ramp3e)&&(nowseconds < ramp4s))LEDStartMode=1;
      else if((ramp2e < ramp2s)&&(nowseconds > ramp4s)&&(nowseconds < ramp4e))LEDStartMode=6;
      else if((ramp2e < ramp2s)&&(nowseconds > ramp4e)&&(nowseconds < ramp1s))LEDStartMode=0;
      else if((ramp2e < ramp2s)&&(nowseconds > ramp1s)&&(nowseconds < ramp1e))LEDStartMode=3;
      else if((ramp2e < ramp2s)&&(nowseconds > ramp1e)&&(nowseconds < ramp2s))LEDStartMode=1;
  
      //ramp 3 starts after midnight
      else if((ramp3s < ramp2e)&&(nowseconds > ramp2e))LEDStartMode=2;
      else if((ramp3s < ramp2e)&&(nowseconds < ramp3s))LEDStartMode=2;
      else if((ramp3s < ramp2e)&&(nowseconds > ramp3s)&&(nowseconds < ramp3e))LEDStartMode=5;
      else if((ramp3s < ramp2e)&&(nowseconds > ramp3e)&&(nowseconds < ramp4s))LEDStartMode=1;
      else if((ramp3s < ramp2e)&&(nowseconds > ramp4s)&&(nowseconds < ramp4e))LEDStartMode=6;
      else if((ramp3s < ramp2e)&&(nowseconds > ramp4e)&&(nowseconds < ramp1s))LEDStartMode=0;
      else if((ramp3s < ramp2e)&&(nowseconds > ramp1s)&&(nowseconds < ramp1e))LEDStartMode=3;
      else if((ramp3s < ramp2e)&&(nowseconds > ramp1e)&&(nowseconds < ramp2s))LEDStartMode=1;
      else if((ramp3s < ramp2e)&&(nowseconds > ramp2s)&&(nowseconds < ramp2e))LEDStartMode=4;
  
      //ramp 3 ends after midnight
      else if((ramp3e < ramp3s)&&(nowseconds > ramp3s))LEDStartMode=5;
      else if((ramp3e < ramp3s)&&(nowseconds < ramp3e))LEDStartMode=5;
      else if((ramp3e < ramp3s)&&(nowseconds > ramp3e)&&(nowseconds < ramp4s))LEDStartMode=1;
      else if((ramp3e < ramp3s)&&(nowseconds > ramp4s)&&(nowseconds < ramp4e))LEDStartMode=6;
      else if((ramp3e < ramp3s)&&(nowseconds > ramp4e)&&(nowseconds < ramp1s))LEDStartMode=0;
      else if((ramp3e < ramp3s)&&(nowseconds > ramp1s)&&(nowseconds < ramp1e))LEDStartMode=3;
      else if((ramp3e < ramp3s)&&(nowseconds > ramp1e)&&(nowseconds < ramp2s))LEDStartMode=1;
      else if((ramp3e < ramp3s)&&(nowseconds > ramp2s)&&(nowseconds < ramp2e))LEDStartMode=4;
      else if((ramp3e < ramp3s)&&(nowseconds > ramp2e)&&(nowseconds < ramp3s))LEDStartMode=2;
  
      //ramp 4 starts after midnight
      else if((ramp4s < ramp3e)&&(nowseconds > ramp3e))LEDStartMode=1;
      else if((ramp4s < ramp3e)&&(nowseconds < ramp4s))LEDStartMode=1;
      else if((ramp4s < ramp3e)&&(nowseconds > ramp4s)&&(nowseconds < ramp4e))LEDStartMode=6;
      else if((ramp4s < ramp3e)&&(nowseconds > ramp4e)&&(nowseconds < ramp1s))LEDStartMode=0;
      else if((ramp4s < ramp3e)&&(nowseconds > ramp1s)&&(nowseconds < ramp1e))LEDStartMode=3;
      else if((ramp4s < ramp3e)&&(nowseconds > ramp1e)&&(nowseconds < ramp2s))LEDStartMode=1;
      else if((ramp4s < ramp3e)&&(nowseconds > ramp2s)&&(nowseconds < ramp2e))LEDStartMode=4;
      else if((ramp4s < ramp3e)&&(nowseconds > ramp2e)&&(nowseconds < ramp3s))LEDStartMode=2;
      else if((ramp4s < ramp3e)&&(nowseconds > ramp3s)&&(nowseconds < ramp3e))LEDStartMode=5;
  
      //ramp 4 ends after midnight
      else if((ramp4e < ramp4s)&&(nowseconds > ramp4s))LEDStartMode=6;
      else if((ramp4e < ramp4s)&&(nowseconds < ramp4e))LEDStartMode=6;
      else if((ramp4e < ramp4s)&&(nowseconds > ramp4e)&&(nowseconds < ramp1s))LEDStartMode=0;
      else if((ramp4e < ramp4s)&&(nowseconds > ramp1s)&&(nowseconds < ramp1e))LEDStartMode=3;
      else if((ramp4e < ramp4s)&&(nowseconds > ramp1e)&&(nowseconds < ramp2s))LEDStartMode=1;
      else if((ramp4e < ramp4s)&&(nowseconds > ramp2s)&&(nowseconds < ramp2e))LEDStartMode=4;
      else if((ramp4e < ramp4s)&&(nowseconds > ramp2e)&&(nowseconds < ramp3s))LEDStartMode=2;
      else if((ramp4e < ramp4s)&&(nowseconds > ramp3s)&&(nowseconds < ramp3e))LEDStartMode=5;
      else if((ramp4e < ramp4s)&&(nowseconds > ramp3e)&&(nowseconds < ramp4s))LEDStartMode=1;
   
      //ramp 1 starts after midnight
      else if((ramp1s < ramp4e)&&(nowseconds > ramp4e))LEDStartMode=0;
      else if((ramp1s < ramp4e)&&(nowseconds < ramp1s))LEDStartMode=0;
      else if((ramp1s < ramp4e)&&(nowseconds > ramp1s)&&(nowseconds < ramp1e))LEDStartMode=3;
      else if((ramp1s < ramp4e)&&(nowseconds > ramp1e)&&(nowseconds < ramp2s))LEDStartMode=1;
      else if((ramp1s < ramp4e)&&(nowseconds > ramp2s)&&(nowseconds < ramp2e))LEDStartMode=4;
      else if((ramp1s < ramp4e)&&(nowseconds > ramp2e)&&(nowseconds < ramp3s))LEDStartMode=2;
      else if((ramp1s < ramp4e)&&(nowseconds > ramp3s)&&(nowseconds < ramp3e))LEDStartMode=5;
      else if((ramp1s < ramp4e)&&(nowseconds > ramp3e)&&(nowseconds < ramp4s))LEDStartMode=1;
      else if((ramp1s < ramp4e)&&(nowseconds > ramp4s)&&(nowseconds < ramp4e))LEDStartMode=6;
  
      //set LEDs based on LEDStartMode
      if(LEDStartMode == 0)
      {
        Serial.println("Setting lights to moonlight mode based on current time...");
        for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].moonPWM;}
        fadeInProgress=false;
        writeLEDs();
      }
      if(LEDStartMode == 1)
      {
        Serial.println("Setting lights to sunrise/set levels based on current time...");
        for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].dimPWM;}
        fadeInProgress=false;
        writeLEDs();
      }
      if(LEDStartMode == 2)
      {
        Serial.println("Setting lights to daylight levels based on current time...");
        for (i = 0; i < numCh; i = i + 1){LEDsettings[i].currentPWM = LEDsettings[i].maxPWM;}
        fadeInProgress=false;
        writeLEDs();
      }
      if(LEDStartMode == 3)
      {
        Serial.println("Setting lights to sunrise mode based on current time...");
        fadeInProgress = true;
        if(nowseconds > ramp1s) //normal operation
        {
          fadeStartTimeSeconds = now() - (nowseconds - ramp1s);
          fadeStartTimeMillis = millis() - ((nowseconds - ramp1s) * 1000);
        }
        if(nowseconds < ramp1s) //ramp 1 started before midnight but currently after midnight
        {
          fadeStartTimeSeconds = now() - (nowseconds + 86400 - ramp1s);
          fadeStartTimeMillis = millis() - ((nowseconds + 86400 - ramp1s) * 1000);
        }
        for (i = 0; i < numCh; i = i + 1) {
          LEDsettings[i].targetPWM = LEDsettings[i].dimPWM;
          LEDsettings[i].lastPWM = LEDsettings[i].moonPWM;
          if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
          if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
          LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
        }
      }
      if(LEDStartMode == 4)
      {
        Serial.println("Setting lights to ramp up to daylight mode based on current time...");
        fadeInProgress = true;
        if(nowseconds > ramp2s) //normal operation
        {
          fadeStartTimeSeconds = now() - (nowseconds - ramp2s);
          fadeStartTimeMillis = millis() - ((nowseconds - ramp2s) * 1000);
        }
        if(nowseconds < ramp2s) //ramp 2 started before midnight but currently after midnight
        {
          fadeStartTimeSeconds = now() - (nowseconds + 86400 - ramp2s);
          fadeStartTimeMillis = millis() - ((nowseconds + 86400 - ramp2s) * 1000);
        }
        for (i = 0; i < numCh; i = i + 1)
        {
          LEDsettings[i].targetPWM = LEDsettings[i].maxPWM;
          LEDsettings[i].lastPWM = LEDsettings[i].dimPWM;
          if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
          if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
          LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
        }
      }
      if(LEDStartMode == 5)
      {
        Serial.println("Setting lights to ramp down from daylight based on current time...");
        fadeInProgress = true;
        if(nowseconds > ramp3s) //normal operation
        {
          fadeStartTimeSeconds = now() - (nowseconds - ramp3s);
          fadeStartTimeMillis = millis() - ((nowseconds - ramp3s) * 1000);
        }
        if(nowseconds < ramp3s) //ramp 3 started before midnight but currently after midnight
        {
          fadeStartTimeSeconds = now() - (nowseconds + 86400 - ramp3s);
          fadeStartTimeMillis = millis() - ((nowseconds + 86400 - ramp3s) * 1000);
        }
        for (i = 0; i < numCh; i = i + 1)
        {
          LEDsettings[i].targetPWM = LEDsettings[i].dimPWM;
          LEDsettings[i].lastPWM = LEDsettings[i].maxPWM;
          if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
          if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
          LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
        }
      }
      if(LEDStartMode == 6)
      {
        Serial.println("Setting lights to sunset mode based on current time...");
        fadeInProgress = true;
        if(nowseconds > ramp4s) //normal operation
        {
          fadeStartTimeSeconds = now() - (nowseconds - ramp4s);
          fadeStartTimeMillis = millis() - ((nowseconds - ramp4s) * 1000);
        }
        if(nowseconds < ramp4s) //ramp 3 started before midnight but currently after midnight
        {
          fadeStartTimeSeconds = now() - (nowseconds + 86400 - ramp4s);
          fadeStartTimeMillis = millis() - ((nowseconds + 86400 - ramp4s) * 1000);
        }
        for (i = 0; i < numCh; i = i + 1)
        {
          LEDsettings[i].targetPWM = LEDsettings[i].moonPWM;
          LEDsettings[i].lastPWM = LEDsettings[i].dimPWM;
          if(LEDsettings[i].targetPWM > LEDsettings[i].lastPWM){difference = (LEDsettings[i].targetPWM - LEDsettings[i].lastPWM);}
          if(LEDsettings[i].targetPWM < LEDsettings[i].lastPWM){difference = (LEDsettings[i].lastPWM - LEDsettings[i].targetPWM);}
          LEDsettings[i].fadeIncrementTime = fadeTimeMillis / difference;
        }
      }
    }
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
