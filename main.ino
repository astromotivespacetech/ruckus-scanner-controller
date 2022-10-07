#include <EEPROM.h>

#define DIRPIN1   0b00000001 // GPIO 8
#define STEPPIN1  0b00000010 // GPIO 9
#define DIRPIN2   0b00000100 // GPIO 10
#define STEPPIN2  0b00001000 // GPIO 11
#define PROXPIN1  0b00010000 // GPIO 12
#define PROXPIN2  0b00100000 // GPIO 13


char message[255];

bool delimiterFlag = false;

struct Motor {
  int dirPin;
  int stepPin;
  int dirState;
  int stepState;
  unsigned long prevStep;     // micros
  unsigned int stepCounter;
  unsigned int stepDelay;     // micros
  int stepDelayAddr;       // EEPROM address where stepDelay lives
};


unsigned int tubeLength;      // mm
unsigned int tubeOffset;      // mm
int tubeLengthAddr = 4;
int tubeOffsetAddr = 6;


const int mmPerRev = 10;      // mm
const int fullStepsPerRev = 200;
unsigned int microsteps;
int microstepsAddr = 8;
int stepsPerRev;
float distPerStep;


int mode;
int modeAddr = 10;
int state = 0;


Motor motorOne { DIRPIN1, STEPPIN1, LOW, LOW, 0, 0, 0, 0 };
Motor motorTwo { DIRPIN2, STEPPIN2, LOW, LOW, 0, 0, 0, 2 };


void setup() {
  Serial.begin(115200);

  // sets Pins 8,9,10,11 to Outputs, Pins 12,13 to Inputs
  DDRB = DDRB | 0b00001111;
  DDRB = DDRB & 0b11001111;

  // get stepDelay
  motorOne.stepDelay = readIntFromEEPROM(motorOne.stepDelayAddr);
  motorTwo.stepDelay = readIntFromEEPROM(motorTwo.stepDelayAddr);

  // get tube length and offset
  tubeLength = readIntFromEEPROM(tubeLengthAddr);
  tubeOffset = readIntFromEEPROM(tubeOffsetAddr);

  // get mode
  mode = readIntFromEEPROM(modeAddr);

  // get microstepping 
  microsteps = readIntFromEEPROM(microstepsAddr);

  // calc steps per revolution
  stepsPerRev = fullStepsPerRev * microsteps;  

  // calc distance per step
  distPerStep = mmPerRev / stepsPerRev;
  
  // run motor homing function until it returns true i.e. until 
  // it hits the proximity sensor, so we start with a known position
  while (!motorHoming(motorOne)); 
  
}

void loop() {

  if (state == 0) {
  
    while (Serial.available()) {
      char x = Serial.read();
      if (x != "x") {
        strcat(message, x); 
      } else {
        delimiterFlag = true;
      }
    }
  
    if (delimiterFlag) {
      delimiterFlag = false;

      int tempState = message[0];
      unsigned int tempTubeLength = message[1]<<8 + message[2];
      unsigned int tempTubeOffset = message[3]<<8 + message[4];
      unsigned int tempMotorOneStepDelay = message[5]<<8 + message[6];
      unsigned int tempMotorTwoStepDelay = message[7]<<8 + message[8];
      int tempMode = message[9];
      int tempMicrosteps = message[10];

      if (state != tempState) {
        state = tempState;
      }
      if (tubeLength != tempTubeLength) {
        tubeLength = tempTubeLength;
        writeIntIntoEEPROM(tubeLengthAddr, tubeLength);
      }
      if (tubeOffset != tempTubeOffset) {
        tubeOffset = tempTubeOffset;
        writeIntIntoEEPROM(tubeOffsetAddr, tubeOffset);
      }
      if (motorOne.stepDelay != tempMotorOneStepDelay) {
        motorOne.stepDelay = tempMotorOneStepDelay;
        writeIntIntoEEPROM(motorOne.stepDelayAddr, motorOne.stepDelay);
      }
      if (motorTwo.stepDelay != tempMotorTwoStepDelay) {
        motorTwo.stepDelay = tempMotorTwoStepDelay;
        writeIntIntoEEPROM(motorTwo.stepDelayAddr, motorTwo.stepDelay);
      }
      if (mode != tempMode) {
        mode = tempMode;
        writeIntIntoEEPROM(modeAddr, mode);
      }
      if (microsteps != tempMicrosteps) {
        microsteps = tempMicrosteps;
        writeIntIntoEEPROM(microstepsAddr, microsteps);
        stepsPerRev = fullStepsPerRev * microsteps;  
        distPerStep = mmPerRev / stepsPerRev;
      }
          
    }

  } else if (state == 1) {

    // move the first motor to the start position of the tube


    // sequence depends on the mode


    // 
    
  } 
  
  


  
}


bool motorHoming(Motor m) {
  bool homed = false;
  int proxState = fastDigitalRead(PINB, PROXPIN1);
  if (proxState) {
    homed = true;
  } else {
    motorStep(m, 0);
  }
  return homed;
}


void motorStep(Motor m, int dir) {
  if (micros() - m.prevStep > m.stepDelay) {
    m.prevStep = micros();
    m.stepState = m.stepState ? HIGH : LOW;  // toggle from high to low or vice versa
    m.stepCounter += (1 * m.stepState);
    fastDigitalWrite(m.dirPin, m.dirState);
    fastDigitalWrite(m.stepPin, m.stepState);
  }
}


int fastDigitalRead(int registr, int pin) {
  int x = registr;
  x = x & pin;
  return x;
}

void fastDigitalWrite(int pin, int state) {
  if (state) {
    PORTB = PORTB | pin;
  } else {
    PORTB = PORTB | ~pin;
  }
}




void writeIntIntoEEPROM(int address, int number) { 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}
int readIntFromEEPROM(int address) {
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}



