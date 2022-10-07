#include <EEPROM.h>

#define dir1Pin   0b00000001 // GPIO 8
#define step1Pin  0b00000010 // GPIO 9
#define dir2Pin   0b00000100 // GPIO 10
#define step2Pin  0b00001000 // GPIO 11
#define prox1Pin  0b00010000 // GPIO 12
#define prox2Pin  0b00100000 // GPIO 13

int state = 0;

char message[255];

bool delimiterFlag = false;

struct Motor {
  int dirPin;
  int stepPin;
  int dirState;
  int stepState;
  unsigned long prevStep;     // micros
  int stepCounter;
  unsigned long stepDelay;    // micros
  int stepDelayAddress;       // EEPROM address where stepDelay lives
};


unsigned int tubeLength;
unsigned int tubeOffset;

int tubeLengthAddress = 4;
int tubeOffsetAddress = 6;


Motor motorOne { dir1Pin, step1Pin, LOW, LOW, 0, 0, 0, 0 };
Motor motorTwo { dir2Pin, step2Pin, LOW, LOW, 0, 0, 0, 2 };


void setup() {
  Serial.begin(115200);

  // get stepDelay from addresses
  motorOne.stepDelay = readIntFromEEPROM(motorOne.stepDelayAddress);
  motorTwo.stepDelay = readIntFromEEPROM(motorTwo.stepDelayAddress);

  tubeLength = readIntFromEEPROM(tubeLengthAddress);
  tubeOffset = readIntFromEEPROM(tubeOffsetAddress);

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
      unsigned long tempMotorOneStepDelay = message[5]<<24 + message[6]<<16 + message[7]<<8 + message[8];
      unsigned long tempMotorTwoStepDelay = message[9]<<24 + message[10]<<16 + message[11]<<8 + message[12];

      if (state != tempState) {
        state = tempState;
      }
      if (tubeLength != tempTubeLength) {
        tubeLength = tempTubeLength;
      }
      if (tubeOffset != tempTubeOffset) {
        tubeOffset = tempTubeOffset;
      }
      if (motorOne.stepDelay != tempMotorOneStepDelay) {
        motorOne.stepDelay = tempMotorOneStepDelay;
      }
      if (motorTwo.stepDelay != tempMotorTwoStepDelay) {
        motorTwo.stepDelay = tempMotorTwoStepDelay;
      }
          
    }

  } else if (state == 1) {
    
  } 
  
  


  
}


bool motorHoming(Motor m) {
  bool homed = false;
  int proxState = fastDigitalRead(PINB, prox1Pin);
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




void storeMotorSpeed(Motor m) {
  writeIntIntoEEPROM(m.stepDelayAddress, m.stepDelay);
}
