void saveMaxPWMValues()
{
  //Save tempPWM values to maxPWM EEPROM
  EEPROM.begin(512);
  int addr=0;
  int i;
  Serial.println("Saving maxPWM values: ");
  for (i = 0; i < numCh; i = i + 1)
  {
    Serial.print("Address: ");
    Serial.print(addr);
    Serial.print(", maxPWM: ");
    Serial.println(LEDsettings[i].tempPWM);
    //addr += EEPROM.put(addr, LEDsettings[i].tempPWM);
    EEPROM.put(addr, LEDsettings[i].tempPWM);
    LEDsettings[i].maxPWM = LEDsettings[i].tempPWM;
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
  Serial.println("Saving dimPWM values: ");
  for (i = 0; i < numCh; i = i + 1)
  {
    Serial.print("Address: ");
    Serial.print(addr);
    Serial.print(", maxPWM: ");
    Serial.println(LEDsettings[i].tempPWM);
    EEPROM.put(addr, LEDsettings[i].tempPWM);
    LEDsettings[i].dimPWM = LEDsettings[i].tempPWM;
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
  Serial.println("Saving moonPWM values: ");
  for (i = 0; i < numCh; i = i + 1)
  {
    Serial.print("Address: ");
    Serial.print(addr);
    Serial.print(", maxPWM: ");
    Serial.println(LEDsettings[i].tempPWM);
    EEPROM.put(addr, LEDsettings[i].tempPWM);
    LEDsettings[i].moonPWM = LEDsettings[i].tempPWM;
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
  Serial.print("LED Values (max): ");
  for (i = 0; i < numCh; i = i + 1)
  {
    EEPROM.get(addr, LEDsettings[i].maxPWM);
    Serial.print(LEDsettings[i].maxPWM);
    Serial.print(",");
    addr = addr + 4;
  }
  Serial.print("\n");
  Serial.print("LED Values (dim): ");
  for (i = 0; i < numCh; i = i + 1)
  {
    EEPROM.get(addr, LEDsettings[i].dimPWM);
    Serial.print(LEDsettings[i].dimPWM);
    Serial.print(",");
    addr = addr + 4;
  }
  Serial.print("\n");
  Serial.print("LED Values (moon): ");
  for (i = 0; i < numCh; i = i + 1)
  {
    EEPROM.get(addr, LEDsettings[i].moonPWM);
    Serial.print(LEDsettings[i].moonPWM);
    Serial.print(",");
    addr = addr + 4;
  }
  Serial.print("\n");
  EEPROM.end();
}

void clearPWMValues()
{
  //Save tempPWM values to dimPWM EEPROM
  EEPROM.begin(512);
  int addr = 0;
  int i;
  for (i = 0; i < (numCh*3); i = i + 1)
  {
    addr += EEPROM.put(addr, 0);
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

