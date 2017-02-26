/*
  Here used to test heater display
*/



#include <LiquidCrystal.h>

// ----------------- Heater -----------------

int sensorPin = A0;    // select the input pin for the potentiometer
int VBattMeasure = A1;  //input measures 1/3 of the battery voltage

const int glowRelay = 6;
const int fuelPump = 8;
const int blowerMotor = 9;
const int displayBacklight = 10;

unsigned long previousMillis = 0;
unsigned long previousPumpMillis = 0;

int pumpRate = 0;

const int pumpPulseTime = 150;
const int lowFuelRate = 600;
const int mediumFuelRate = 400;
const int highFuelRate = 250;

// ----------------- LCD -----------------
LiquidCrystal lcd(13, 12, A2, A3, A4, A5);

byte glowLeft[8] = {
  0, 13, 18, 5, 5, 5, 5, 2
};

byte glowRight[8] = {
  0, 22, 9, 20, 20, 20, 20, 8
};

/*
  byte fuelPump[8] = {
  28, 16, 24, 23, 21, 6, 4, 4
  };
*/

byte aniPump1[8] = {
  4, 4, 14, 10, 10, 10, 4, 4
};
byte aniPump2[8] = {
  4, 4, 14, 14, 10, 10, 4, 4
};
byte aniPump3[8] = {
  4, 4, 14, 10, 14, 10, 4, 4
};
byte aniPump4[8] = {
  4, 4, 14, 10, 10, 14, 4, 4
};

byte aniFan1[8] = {
  0, 0, 6, 6, 4, 12, 12, 0
};
byte aniFan2[8] = {
  0, 0, 24, 8, 4, 2, 3, 0
};
byte aniFan3[8] = {
  0, 0, 0, 24, 31, 3, 0, 0
};
byte aniFan4[8] = {
  0, 0, 1, 3, 4, 24, 16, 0
};

unsigned long previousLCDMillis = 0;
const long interval = 70;
int aniState = 3;

char ADC_Str[4];
char Vbatt_Str[6];


void setup() {
  pinMode(glowRelay, OUTPUT);
  pinMode(fuelPump, OUTPUT);
  pinMode(blowerMotor, OUTPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  lcd.createChar(0, glowLeft);
  lcd.createChar(1, glowRight);

  //lcd.createChar(2, fuelPump);

  lcd.createChar(3, aniPump1);
  lcd.createChar(4, aniPump2);
  lcd.createChar(5, aniPump3);
  lcd.createChar(6, aniPump4);

  //lcd.createChar(7, aniFan1);
  //lcd.createChar(8, aniFan2);
  //lcd.createChar(9, aniFan3);
  //lcd.createChar(10, aniFan4);
  lcd.clear();
  lcd.print("Eberspachenator");

}

void loop() {
  lcd.setCursor(0, 1);

  int ADC0 = analogRead(sensorPin);
  
  memset(ADC_Str, 0, 4);
  sprintf(ADC_Str, "%3d", (ADC0 / 4));

  lcd.print(ADC_Str);

  //for (int thisChar = 0; thisChar < 4; thisChar++) {
  //  lcd.print(ADC_Str[thisChar]);
  // }

  //lcd.write(byte(2));
  //lcd.write(byte(3));
  //lcd.write(byte(4));
  //lcd.write(byte(5));
  //lcd.write(byte(6));

  unsigned long currentMillis = millis();

  if (currentMillis - previousLCDMillis >= interval) {

    previousLCDMillis = currentMillis;

    if (aniState < 6) {
      aniState += 1;
    } else {
      aniState = 3;
    }

    lcd.setCursor(7, 1);
    lcd.write(byte(aniState));
    //lcd.write(byte(aniState+4));

  }
  //print the battery voltage
  float Vbatt = analogRead(VBattMeasure) * 0.0146627;
  lcd.setCursor(9, 1);
  dtostrf(Vbatt, 4, 1, Vbatt_Str);
  lcd.print(Vbatt_Str);
  lcd.print("V");

  pumpRate = 4092 - (ADC0 * 2);

  if (pumpRate > 3000) {
    digitalWrite(glowRelay, HIGH);
    digitalWrite(displayBacklight,LOW);
    lcd.setCursor(4, 1);
    lcd.write(byte(0));
    lcd.write(byte(1));
  }
  else if (pumpRate < 2900)
  {
    digitalWrite(glowRelay, LOW);
    digitalWrite(displayBacklight,HIGH);
    lcd.setCursor(4, 1);
    lcd.print("  ");
  }

  analogWrite(blowerMotor, (ADC0 / 4));
  pump();


}


void pump() { //based on the setting of the pump, adjust the time between pulses, pulse time is 100ms

  unsigned long currentMillis = millis();

  if (currentMillis - previousPumpMillis >= pumpPulseTime) {
    digitalWrite(fuelPump, LOW);
    lcd.setCursor(6, 1);
    lcd.print(" ");
  }

  if (currentMillis - previousPumpMillis >= pumpRate) {
    previousPumpMillis = currentMillis;
    digitalWrite(fuelPump, HIGH);
    lcd.setCursor(6, 1);
    lcd.write(byte(3));
  }

}
