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
#define SCREEN_TIME 2000

LiquidCrystal_I2C lcd (0x27 , 16, 2);

float c1,c2,c3,c4;
int currentScreen = 0;
unsigned long screenTimer = 0;
int previousScreen = -1;
unsigned long lcdTimer = 0;


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

}

void loop() 
{
  c1= analogRead(CELL1) * 3.3 / 4095.0;
  c2= analogRead(CELL2) * 3.3 / 4095.0;
  c3= analogRead(CELL3) * 3.3 / 4095.0;
  c4= analogRead(CELL4) * 3.3 / 4095.0;
  float average = (c1 + c2 + c3 + c4) / 4.0;
  float highest = max(max(c1, c2), max(c3, c4));
  float lowest = min(min(c1, c2), min(c3, c4));
  float difference = highest - lowest;
 String fault = "NO FAULT";

if(lowest < 2.5)
{
    fault = "LOW CELL";
}
else if(highest > 3.0)
{
    fault = "HIGH CELL";
}

if(fault != "NO FAULT")
{
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("WARNING");

    lcd.setCursor(0,1);
    lcd.print(fault);
    
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    tone(BUZZER, 2000);

    delay(500);

    return;
}
else
{
    
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER);
}

  Serial.print("c1: ");
  Serial.println(c1);
  Serial.print("c2: ");
  Serial.println(c2);
  Serial.print("c3: ");
  Serial.println(c3);
  Serial.print("c4: ");
  Serial.println(c4);

  Serial.println("----------------");

    if (millis() - screenTimer >= SCREEN_TIME)
    {
      currentScreen++;
      if (currentScreen > 4)
      {
        currentScreen = 0;
      }
      screenTimer = millis();
    }
  
  if(millis() - lcdTimer < 300)
{
    return;
}

lcdTimer = millis();

     if(currentScreen ==0)
     {
      if(previousScreen != currentScreen)
      {
       lcd.clear();
       previousScreen = currentScreen;
      }
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
    if(previousScreen != currentScreen)
    {
        lcd.clear();
        previousScreen = currentScreen;
    }
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
    if(previousScreen != currentScreen)
    {
        lcd.clear();
        previousScreen = currentScreen;
    }
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("PACK INFO");
      lcd.setCursor(0,1);
      lcd.print("AVG:");
      lcd.print(average,2); 
     }
     else if(currentScreen == 3)
      {
    if(previousScreen != currentScreen)
    {
        lcd.clear();
        previousScreen = currentScreen;
    }

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("STATUS");
      lcd.setCursor(0,1);
      lcd.print(fault);
     }
     else if(currentScreen == 4)
{
  if(previousScreen != currentScreen)
    {
        lcd.clear();
        previousScreen = currentScreen;
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("DIFF:");
    lcd.print(difference,2);

    lcd.setCursor(0,1);
    lcd.print("LOW:");
    lcd.print(lowest,1);
}
}