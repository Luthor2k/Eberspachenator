/*
    Eberspacher D1L Controller Firmware

    Arthur Hazleden
    December 2016

    D10 - standby LED
    D11 - startup LED
    D12 - running LED
    D13 - shutdown LED

    D3 - flame switch sensor
    D4 - overtemperature switch sensor
    D5 - thermostat switch

    D6 - glow plug relay, on open collector NPN
    D - extra open collector NPN
    D8 - fuel pump mosfet
    D9 - blower motor PWM control mosfet

    A0 - battery voltage across a 22k / 10k resistor divider network
    A1 - cabin temperature from and LM
*/


// pin assignments
const unsigned char PIN_FLAME_SWITCH = 3;
const int overTempSwitch = 4;
const int tStatSwitch = 5;

const int glowRelay = 6;
//const int spareOC = 7;
const int fuelPump = 8;
const int blowerMotor = 9;


// variables used for timing
unsigned long previousMillis = 0;

//variables used for heater control
int pumpRate = 0;
int heaterState = 0;

int tStatState = 0;
int lastTStatState = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;
int callForHeat = 0;  //set to 1 when the thermostat indicates heating is needed

unsigned long previousPumpMillis = 0;

// constants defining heater startup function timing, times are when these operations start after the thermostat engauges the heater
const long justGlow = 500; // engauge the glowplug
const long blowerStartup = 3000;  //start the blower on low speed
const long startupFueling = 60000;  //start with pump at low rate
const long glowCut = 60000; //cut glow after flame detected and this time
const long powerIncrease = 100000; //blower and pump to medium speed at this point, warm up heater housing in prep for run mode
const long fullPower = 260000; //blower and pump to full speed at this point, switch to run mode after this

// constants defining heater shutdown function timing, times are when these operations start after the thermostat stops the heater
const long blowerHigh = 20000; //cut fuel, engauge glowplug for cleaning and start cooling down the heater
const long glowCleaning = 40000; //disengauge glow plug, is clean by now
const long blowerLow = 240000;  //stop blower after the cooldown phase, proceed to standby phase


//constants defining heater function rates (all in milliseconds, a shorter time means more pumps per second)
const int pumpPulseTime = 100;
const int lowFuelRate = 600;
const int mediumFuelRate = 400;
const int highFuelRate = 250;


void setup() {

  for (int thisPin = 3; thisPin <= 5; thisPin++) {
    pinMode(thisPin, INPUT);
  }

  for (int thisPin = 6; thisPin <= 9; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }
  digitalWrite(glowRelay, LOW);
  digitalWrite(spareOC, LOW);
  digitalWrite(fuelPump, LOW);
  digitalWrite(blowerMotor, LOW);

}

void loop() {

  thermostat(); //check the temperature and state, change state if necessary

  pump(); //update the super slow fuel pump PWM
  
  heaterStateLoop();

}


void heaterStateLoop()
{

   /* Heater States:
     0 - standby
     1 - startup
     2 - running
     3 - shutdown
  */

  unsigned long currentMillis = millis();

  switch (heaterState) {
    case 0: // ********** standby **********
      digitalWrite(glowRelay, LOW);
      digitalWrite(spareOC, LOW);
      digitalWrite(blowerMotor, LOW);

      pumpRate = 0;

      break;

    case 1: // ********** startup **********

      if (currentMillis - previousMillis < justGlow) {  // engauge the glowplug
        digitalWrite(glowRelay, HIGH);
        previousMillis = currentMillis;
      }

      if (currentMillis - previousMillis < blowerStartup) { //start the blower on low speed
        analogWrite(blowerMotor, 50);
        previousMillis = currentMillis;
      }

      if (currentMillis - previousMillis < startupFueling) {  //start with pump at low rate
        pumpRate = lowFuelRate;
        previousMillis = currentMillis;
      }

      if (currentMillis - previousMillis < glowCut) { //cut glow after flame detected and this time
        digitalWrite(glowRelay, LOW);
        previousMillis = currentMillis;
      }

      if (currentMillis - previousMillis < powerIncrease) { //blower and pump to medium speed at this point, warm up heater housing in prep for run mode
        analogWrite(blowerMotor, 150);
        pumpRate = mediumFuelRate;
        previousMillis = currentMillis;
      }

      if (currentMillis - previousMillis < fullPower) { //blower and pump to full speed at this point, switch to run mode after this
        analogWrite(blowerMotor, 255);
        pumpRate = highFuelRate;
        previousMillis = currentMillis;
      }

      if (currentMillis - previousMillis >= fullPower) {  //advance to run phase
        heaterState = 2;
        previousMillis = currentMillis;
      }


      break;

    case 2: // ********** running **********
      //in future, add multiple power levels for PI control? avoid the glow plug current draw on startup!

      break;

    case 3: // ********** shutdown **********

      if (currentMillis - previousMillis < blowerHigh) {  //cut fuel, engauge glowplug for cleaning and start cooling down the heater
        analogWrite(blowerMotor, 255);
        pumpRate = 0;
        digitalWrite(glowRelay, HIGH);
        previousMillis = currentMillis;
      }

      if (currentMillis - previousMillis < glowCleaning) {  //disengauge glow plug, is clean by now
        digitalWrite(glowRelay, LOW);
        previousMillis = currentMillis;
      }

      if (currentMillis - previousMillis >= blowerLow) {  //stop blower after the cooldown phase, proceed to standby phase
        analogWrite(blowerMotor, 0);
        heaterState = 0;
        previousMillis = currentMillis;
      }

      break;

  }
}

void thermostat() { //updates thermostat reading, checks for overtemperature condition, changes heater state

  // ********** Debounced thermostat checkup: **********
  int reading = digitalRead(tStatSwitch);
  // If the switch changed, due to noise or pressing:
  if (reading != lastTStatState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the tStatState is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != tStatState) {
      tStatState = reading;

      // only toggle the call for heat if the new tstat state is HIGH
      if (tStatState == HIGH) {
        callForHeat = !callForHeat;
      }
    }
  }
  // save the tStatState.  Next time through the loop,
  // it'll be the lastTStatState:
  lastTStatState = reading;

  // ********** Heater state change: **********
  if (callForHeat == 1 && heaterState == 0) { //if tstat says low temp and the heater is idle, start it up
    heaterState = 1;
  }

  if (callForHeat == 0 && heaterState == 2) { //if tstat says high temp and heater is running, shut it down
    heaterState = 3;
  }


  // ********** Overtemperature switch checkup: **********
  if (overTempSwitch == 1) {
    heaterState = 3;
  }

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

