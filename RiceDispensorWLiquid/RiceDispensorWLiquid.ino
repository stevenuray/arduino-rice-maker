#include <Servo.h>

Servo myServo;

//informational consts, not used in program but to guide config decisions
unsigned const int minServoRange = 600;
unsigned const int maxServoRange = 2400;
unsigned const int minBeamRange = 1000;
unsigned const int maxBeamRange = 1975;

//config consts
unsigned const int openDispensorPosition = 1750;
unsigned const int closeDispensorPosition = 1150;

unsigned const int servoPwmPin = 9;

unsigned const int relayPinRed = 10;
unsigned const int relayPinBlack = 11;

unsigned const int waterSensorInputPin = 3;
unsigned const int liquidDispensorEnablePowerPin = 4;
unsigned const int liquidDispensorEnablePin = A0;
unsigned const int buttonPressThreshold = 850;

const boolean drainModeEnabled = false;
unsigned const int drainModeMillis = 240000;

// The calibration factor for hall effect sensor of liquid dispensor
float calibrationFactor = 54;
float totalMilliLitres = 0;
float flowMilliLitres = 0;

volatile byte pulseCount;
unsigned long lastFlowCalculatedMS;
float targetMilliLitresToDispense = 250;
long liquidDispenseStartTimeMS;
const unsigned long maxLiquidDispenseMS = 60000;

unsigned const int moveToPositionDelayInMilliseconds = 5500;
unsigned const int dispenseCycles = 1;
boolean dispenseCycleCompleted = false;

void setup() {
  Serial.begin(9600);
  Serial.println("START");
  myServo.attach(9);
  setupPinModes();
  
  //If special drain only mode is enabled, just drain and quit, don't dispense any cereal or liquid
  if(drainModeEnabled){
    setupForValve();
    drainMode();
    exit(0);
  }
  
  closeDispensor();
  Serial.println("READY");
  setupForValve();
}

void loop() {
  if(isLiquidDispensorEnabled()) {
    loopForValve();
    if(isLiquidDispenseCycleCompleted() && dispenseCycleCompleted == false){
      dispenseCycle();
      dispenseCycleCompleted = true;
    }
  } else {
      if(dispenseCycleCompleted == false){
        dispenseCycle();
        dispenseCycleCompleted = true;
      }
      closeValve();
      delay(2000);
  }
}

void dispenseCycle() {
  for (int i = 0; i < dispenseCycles; i++){
    openDispensor();
    closeDispensor();
    delay(500);
  }
}

void openDispensor() {
  myServo.write(openDispensorPosition);
  delay(moveToPositionDelayInMilliseconds);
}

void closeDispensor() {
  myServo.write(closeDispensorPosition);
  delay(moveToPositionDelayInMilliseconds);
}

void setupPinModes(){
  pinMode(relayPinRed, OUTPUT);
  pinMode(relayPinBlack, OUTPUT);
  pinMode(liquidDispensorEnablePin, INPUT_PULLUP);
}