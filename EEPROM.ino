void saveMaxPWMValues()
{
  //Save tempPWM values to maxPWM EEPROM
  EEPROM.begin(512);
  int addr=0;
  int i;
  BLYNK_LOG("Saving maxPWM values.");
  for (i = 0; i < numCh; i = i + 1)
  {
    BLYNK_LOG("     Address: %d, Value: %d",addr,LEDsettings[i].tempPWM);
    LEDsettings[i].maxPWM = LEDsettings[i].tempPWM;
    EEPROM.put(addr, LEDsettings[i].tempPWM);
    addr = addr + 4;
  }
  EEPROM.end();
}

void saveDimPWMValues()
{
  //Save tempPWM values to dimPWM EEPROM
  EEPROM.begin(512);
  int addr = 4*numCh;  //4 bytes per int
  int i;
  BLYNK_LOG("Saving dimPWM values.");
  for (i = 0; i < numCh; i = i + 1)
  {
    BLYNK_LOG("     Address: %d, Value: %d",addr,LEDsettings[i].tempPWM);
    LEDsettings[i].dimPWM = LEDsettings[i].tempPWM;
    EEPROM.put(addr, LEDsettings[i].tempPWM);
    addr = addr + 4;
  }
  EEPROM.end();
}

void saveMoonPWMValues()
{
  //Save tempPWM values to dimPWM EEPROM
  EEPROM.begin(512);
  int addr = 8*numCh;  //4 bytes per int
  int i;
  BLYNK_LOG("Saving moonPWM values.");
  for (i = 0; i < numCh; i = i + 1)
  {
    BLYNK_LOG("     Address: %d, Value: %d",addr,LEDsettings[i].tempPWM);
    LEDsettings[i].moonPWM = LEDsettings[i].tempPWM;
    EEPROM.put(addr, LEDsettings[i].currentPWM);
    addr = addr + 4;
  }
  EEPROM.end();
}

void readPWMValues()
{
  //read maxPWM and dimPWM values from EEPROM
  EEPROM.begin(512);
  int addr = 0;
  int i;
  BLYNK_LOG("LED Values (max): ");
  for (i = 0; i < numCh; i = i + 1)
  {
    EEPROM.get(addr, LEDsettings[i].maxPWM);
    BLYNK_LOG("%d,",LEDsettings[i].maxPWM);
    addr = addr + 4;
  }
  BLYNK_LOG("LED Values (dim): ");
  for (i = 0; i < numCh; i = i + 1)
  {
    EEPROM.get(addr, LEDsettings[i].dimPWM);
    BLYNK_LOG("%d,",LEDsettings[i].dimPWM);
    addr = addr + 4;
  }
  BLYNK_LOG("LED Values (moon): ");
  for (i = 0; i < numCh; i = i + 1)
  {
    EEPROM.get(addr, LEDsettings[i].moonPWM);
    BLYNK_LOG("%d,",LEDsettings[i].moonPWM);
    addr = addr + 4;
  }
  EEPROM.end();
}

void clearPWMValues()
{
  //Clear out all data from EEPROM range we need to store values in
  EEPROM.begin(512);
  int addr = 0;
  int i;
  for (i = 0; i < (numCh*3); i = i + 1)
  {
    EEPROM.put(addr, 0L);
    addr = addr + 4;
  }
  EEPROM.write(500,numCh);
  EEPROM.end();
}

boolean checkEEPROMPWM()
{
  int val;
  EEPROM.begin(512);
  val = EEPROM.read(500);
  EEPROM.end();
  if(val == numCh){return true;}else {return false;}
}
