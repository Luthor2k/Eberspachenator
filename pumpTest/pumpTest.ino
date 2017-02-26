//Fuel pump test routine, and other IO too.

int sensorPin = A0;    // select the input pin for the potentiometer

const int glowRelay = 6;
const int fuelPump = 8;
const int blowerMotor = 9;

unsigned long previousMillis = 0;
unsigned long previousPumpMillis = 0;

int pumpRate = 0;

const int pumpPulseTime = 100;
const int lowFuelRate = 600;
const int mediumFuelRate = 400;
const int highFuelRate = 250;

void setup() {
  pinMode(glowRelay, OUTPUT);
  pinMode(fuelPump, OUTPUT);
  pinMode(blowerMotor, OUTPUT);
}

void loop() {

  pumpRate = analogRead(sensorPin) * 4;

  if (pumpRate > 2000) {
    digitalWrite(glowRelay, HIGH);
  }
  else
  {
    digitalWrite(glowRelay, LOW);
  }

  analogWrite(blowerMotor, (pumpRate/16));
  pump();

}

void pump() { //based on the setting of the pump, adjust the time between pulses, pulse time is 100ms

  unsigned long currentMillis = millis();

  if (currentMillis - previousPumpMillis >= pumpPulseTime) {
    digitalWrite(fuelPump, LOW);
  }

  if (currentMillis - previousPumpMillis >= pumpRate) {
    previousPumpMillis = currentMillis;
    digitalWrite(fuelPump, HIGH);
  }

}
