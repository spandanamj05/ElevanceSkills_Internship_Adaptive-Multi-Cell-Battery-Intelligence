#define CELL1 34
#define CELL2 35
#define CELL3 32
#define CELL4 33
#define GREEN_LED 25
#define RED_LED 27
#define BUZZER 14
#define RELAY 26
#define WEAK_CELL_LIMIT 2.5
#define OVERVOLTAGE_LIMIT 3.0
#define SENSOR_FAULT_LIMIT 0.1
#define RECOVERY_TIME 5000
#define SCREEN_TIME 2000
#define FREEZE_TIME 2000

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd (0x27 , 16, 2);

float c1,c2,c3,c4;
float prevC1 = 0;
float prevC2 = 0;
float prevC3 = 0;
float prevC4 = 0;
bool prevAdcFrozen = false;
unsigned long recoveryStart = 0;
bool recoveryRunning = false;
int currentScreen = 0;
unsigned long screenTimer = 0;
unsigned long freezeStart = 0;
bool freezeRunning = false;
enum SystemMode
{
  NORMAL,
  DEGRADED,
  FAILSAFE,
  SHUTDOWN
};
SystemMode currentMode = NORMAL;
 String mode = "NORMAL";
 String fault = "NO FAULT";

void logFault(String faultName)
{
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.println(faultName);
}

void setup() {
  
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SYSTEM");

  lcd.setCursor(0,1);
  lcd.print("NORMAL");

  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(RELAY, HIGH);

  noTone(BUZZER);
}

void loop() 
{
  c1= analogRead(CELL1) * 3.3 / 4095.0;
  c2= analogRead(CELL2) * 3.3 / 4095.0;
  c3= analogRead(CELL3) * 3.3 / 4095.0;
  c4= analogRead(CELL4) * 3.3 / 4095.0;
  float average = (c1 + c2 + c3 + c4) / 4.0;
  
  Serial.print("c1: ");
  Serial.println(c1);
  Serial.print("c2: ");
  Serial.println(c2);
  Serial.print("c3: ");
  Serial.println(c3);
  Serial.print("c4: ");
  Serial.println(c4);

  Serial.println("----------------");

  bool weakCell = false;
  if (c1 < WEAK_CELL_LIMIT) weakCell = true;
  if (c2 < WEAK_CELL_LIMIT) weakCell = true;
  if (c3 < WEAK_CELL_LIMIT) weakCell = true;
  if (c4 < WEAK_CELL_LIMIT) weakCell = true;

  bool overVoltage = false;
  if (c1 > OVERVOLTAGE_LIMIT) overVoltage = true;
  if (c2 > OVERVOLTAGE_LIMIT) overVoltage = true;
  if (c3 > OVERVOLTAGE_LIMIT) overVoltage = true;
  if (c4 > OVERVOLTAGE_LIMIT) overVoltage = true;

  bool sensorFault = false;
  if (c1 < SENSOR_FAULT_LIMIT) sensorFault = true;
  if (c2 < SENSOR_FAULT_LIMIT) sensorFault = true;
  if (c3 < SENSOR_FAULT_LIMIT) sensorFault = true;
  if (c4 < SENSOR_FAULT_LIMIT) sensorFault = true;

  bool fluctuationFault = false;
  if (abs(c1 - prevC1) > 0.5) fluctuationFault = true;
  if (abs(c2 - prevC2) > 0.5) fluctuationFault = true;
  if (abs(c3 - prevC3) > 0.5) fluctuationFault = true;
  if (abs(c4 - prevC4) > 0.5) fluctuationFault = true;

  bool adcFrozen = false;

if(
abs(c1-prevC1) < 0.01 &&
abs(c2-prevC2) < 0.01 &&
abs(c3-prevC3) < 0.01 &&
abs(c4-prevC4) < 0.01
)
{
  if(!freezeRunning)
  {
    freezeStart = millis();
    freezeRunning = true;
  }

  if(millis() - freezeStart >= FREEZE_TIME)
  {
    adcFrozen = true;
  }
}
else
{
  freezeRunning = false;
}

  prevC1=c1;
  prevC2=c2;
  prevC3=c3;
  prevC4=c4;

  bool faultPresent =
  sensorFault ||
  fluctuationFault ||
  weakCell ||
  overVoltage ||
  adcFrozen;

  bool relayMismatch = false;
int faultCount = 0;
if(sensorFault) faultCount++;
if(weakCell) faultCount++;
if(overVoltage) faultCount++;
if(fluctuationFault) faultCount++;
if(adcFrozen) faultCount++;
if(relayMismatch) faultCount++;

if(faultCount >= 3)
{
  currentMode = SHUTDOWN;
}
else if(sensorFault)
{
  currentMode = DEGRADED;
}
else if(
weakCell ||
overVoltage ||
fluctuationFault ||
adcFrozen
)
{
  currentMode = FAILSAFE;
}
else
{
  currentMode = NORMAL;
}
switch(currentMode)
{
  case NORMAL:
  mode = "NORMAL";
  break;

  case DEGRADED:
  mode = "DEGRADED";
  break;

  case FAILSAFE:
  mode = "FAILSAFE";
  break;

  case SHUTDOWN:
  mode = "SHUTDOWN";
  break;
}

fault = "NO FAULT";
if (sensorFault)
{
  fault = "SENSOR FAULT";
}
else if(overVoltage)
{
  fault = "OVER VOLTAGE";
}
else if(weakCell)
{
  fault = "WEAK CELL";
}
else if(fluctuationFault)
{
  fault = "UNSTABLE CELL";
}
else if(adcFrozen)
{
  fault = "ADC FROZEN";
}
else if(relayMismatch)
{
  fault = "RELAY ERROR";
}

if(sensorFault)
    logFault("Sensor Fault");

if(weakCell)
    logFault("Weak Cell");

if(overVoltage)
    logFault("Over Voltage");

if(fluctuationFault)
    logFault("Voltage Fluctuation");

if(adcFrozen && !prevAdcFrozen)
    logFault("ADC Frozen");

prevAdcFrozen = adcFrozen;

  if(currentMode == SHUTDOWN)
{
  digitalWrite(RELAY,LOW);

  digitalWrite(GREEN_LED,LOW);

  digitalWrite(RED_LED,HIGH);

  tone(BUZZER,3000);

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("SYSTEM");

  lcd.setCursor(0,1);
  lcd.print("SHUTDOWN");
  return;
}
  if(!faultPresent)
  {
    if (millis() - screenTimer >= SCREEN_TIME)
    {
      currentScreen++;
      if (currentScreen > 4)
      {
        currentScreen = 0;
      }
      screenTimer = millis();
    }
  }
  
  if (faultPresent) {
    recoveryRunning = false;
    digitalWrite(RELAY, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

  if (sensorFault) {
    digitalWrite(RELAY, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    tone(BUZZER,2000);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SENSOR FAULT!");
    lcd.setCursor(0,1);
    lcd.print("CHECK SENSOR!");

  }
  else if(relayMismatch)
{
    tone(BUZZER,3000);

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("RELAY ERROR");

    lcd.setCursor(0,1);
    lcd.print("CHECK RELAY");
}

  else if (fluctuationFault) {
    digitalWrite(RELAY, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    tone(BUZZER,2500);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("UNSTABLE CELL");
    lcd.setCursor(0,1);
    lcd.print("CHECK WIRING!");
  }
  else if (adcFrozen) {
    digitalWrite(RELAY, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    tone(BUZZER,2800);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ADC FROZEN");

    lcd.setCursor(0,1);
    lcd.print("CHECK INPUT");
}
  else if (weakCell) {
    digitalWrite(RELAY, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    tone(BUZZER,1000);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WEAK CELL");
    lcd.setCursor(0,1);
    lcd.print("RELAY OFF");
  }

  else if (overVoltage) {
    digitalWrite(RELAY, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER,1500);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("OVER VOLTAGE");
    lcd.setCursor(0,1);
    lcd.print("RELAY OFF");

  }
  }
  
  else {
    if (! recoveryRunning)
    {
      recoveryStart = millis();
      recoveryRunning = true;
    }
    if (millis () - recoveryStart >= RECOVERY_TIME)
    {
      digitalWrite(RELAY, HIGH);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      noTone(BUZZER);
    }
     if(currentScreen ==0)
     {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("CELL DATA");
      lcd.setCursor(0,1);
      lcd.print("C1:");
      lcd.print(c1,1);
      lcd.print(" ");
      lcd.print("C2:");
      lcd.print(c2,1);
      lcd.print(" ");
     }
     else if(currentScreen == 1)
     {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("CELL DATA");
      lcd.setCursor(0,1);
      lcd.print("C3:");
      lcd.print(c3,1);
      lcd.print(" ");
      lcd.print("C4:");
      lcd.print(c4,1);
      lcd.print(" ");
     }
     else if(currentScreen == 2)
     {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("PACK INFO");
      lcd.setCursor(0,1);
      lcd.print("AVG:");
      lcd.print(average,2); 
     }
     else if(currentScreen == 3)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("STATUS");

  lcd.setCursor(0,1);

  if(currentMode == NORMAL)
    lcd.print("NORMAL");

  else if(currentMode == DEGRADED)
    lcd.print("DEGRADED");

  else if(currentMode == FAILSAFE)
    lcd.print("FAILSAFE");

  else if(currentMode == SHUTDOWN)
    lcd.print("SHUTDOWN");
}
     else if(currentScreen == 4)
{
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("MODE");

  lcd.setCursor(0,1);

  if(currentMode == NORMAL)
    lcd.print("NORMAL");

  else if(currentMode == DEGRADED)
    lcd.print("DEGRADED");

  else if(currentMode == FAILSAFE)
    lcd.print("FAILSAFE");

  else if(currentMode == SHUTDOWN)
    lcd.print("SHUTDOWN");
}
  }
}