#include <WiFi.h>

#define CELL1 34
#define CELL2 35
#define CELL3 32
#define CELL4 33
#define GREEN_LED 25
#define YELLOW_LED 26
#define RED_LED 27
#define BUZZER 14


void setup() {
  pinMode(GREEN_LED,OUTPUT);
  pinMode(YELLOW_LED,OUTPUT);
  pinMode(RED_LED,OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Serial.begin(115200);
  Blynk.begin(
  BLYNK_AUTH_TOKEN,
  ssid,
  pass
);

}

void loop() {
 
  Blynk.run();

  float c1 = analogRead(CELL1) * 3.3 / 4095.0;
  float c2 = analogRead(CELL2) * 3.3 / 4095.0;
  float c3 = analogRead(CELL3) * 3.3 / 4095.0;
  float c4 = analogRead(CELL4) * 3.3 / 4095.0;
  float Average = (c1 + c2 + c3 + c4) / 4.0;

  float minCell = c1;
  int weakest = 1;

  if (c2 < minCell) {
    minCell=c2;
    weakest=2;
  }
  if (c3 < minCell) {
    minCell=c3;
    weakest=3;
  }
  if (c4 < minCell) {
    minCell=c4;
    weakest=4;
  }

  float maxCell = c1;
  int strongest = 1;

  if (c2 > maxCell) {
    maxCell = c2;
    strongest = 2;
  }
  if (c3 > maxCell) {
    maxCell = c3;
    strongest = 3;
  }
  if (c4 > maxCell) {
    maxCell = c4;
    strongest = 4;
  }
  
  float imbalance = ((maxCell - minCell) / Average) * 100;

  String status;

  if (imbalance < 2) {
    status = "Healthy";
  }
  else if (imbalance < 5) {
    status = "minor imbalance";
  }
  else {
    status = "pack failure";
  }

  digitalWrite(GREEN_LED,LOW);
  digitalWrite(YELLOW_LED,LOW);
  digitalWrite(RED_LED,LOW);
  noTone(BUZZER);

if (imbalance < 2)
{
  status = "Healthy";
  digitalWrite(GREEN_LED, HIGH);
}
else if (imbalance < 5)
{
  status = "minor imbalance";
  digitalWrite(YELLOW_LED, HIGH);
}
else if (imbalance < 10)
{
  status = "critical imbalance";
  digitalWrite(RED_LED, HIGH);
  tone(BUZZER,1000);
}
else
{
  status = "pack failure";
  digitalWrite(RED_LED, HIGH);
  tone(BUZZER,2000);
}
  Blynk.virtualWrite(V0, c1);
  Blynk.virtualWrite(V1, c2);
  Blynk.virtualWrite(V2, c3);
  Blynk.virtualWrite(V3, c4);

  Blynk.virtualWrite(V4, Average);

  Blynk.virtualWrite(V5, imbalance);

  Blynk.virtualWrite(V6, status);

Serial.println("================================");

  Serial.print("Cell 1 Voltage : ");
  Serial.print(c1);
  Serial.println(" V");

  Serial.print("Cell 2 Voltage : ");
  Serial.print(c2);
  Serial.println(" V");

  Serial.print("Cell 3 Voltage : ");
  Serial.print(c3);
  Serial.println(" V");

  Serial.print("Cell 4 Voltage : ");
  Serial.print(c4);
  Serial.println(" V");

  Serial.println();

  Serial.print("Pack Average Voltage : ");
  Serial.print(Average);
  Serial.println(" V");

  Serial.print("Weakest Cell : Cell ");
  Serial.println(weakest);

  Serial.print("Strongest Cell : Cell ");
  Serial.println(strongest);

  Serial.print("Imbalance Percentage : ");
  Serial.print(imbalance);
  Serial.println(" %");

  Serial.print("Battery Status : ");
  Serial.println(status);

  Serial.println("================================");
  Serial.println();

  delay(3000);
}