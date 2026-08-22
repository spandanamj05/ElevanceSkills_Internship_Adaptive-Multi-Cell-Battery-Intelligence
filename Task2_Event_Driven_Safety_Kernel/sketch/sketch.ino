#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

LiquidCrystal_I2C lcd (0x27 , 16, 2);

float c1,c2,c3,c4;
float prevC1 = 0;
float prevC2 = 0;
float prevC3 = 0;
float prevC4 = 0;
unsigned long recoveryStart = 0;
bool recoveryRunning = false;

void setup() {
  // put your setup code here, to run once:
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

  prevC1=c1;
  prevC2=c2;
  prevC3=c3;
  prevC4=c4;

  bool faultPresent =
  sensorFault ||
  fluctuationFault ||
  weakCell ||
  overVoltage;

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

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SYSTEM");
    lcd.setCursor(0,1);
    lcd.print("NORMAL");
    }

    else {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RECOVERY");
      lcd.setCursor(0,1);
      lcd.print("WAIT 5 SECS");

    }
  }
}
