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
  float pos;                  // mm or angle (deg);
};


unsigned int tubeLength;      // mm
unsigned int tubeOffset;      // mm
int tubeLengthAddr = 4;
int tubeOffsetAddr = 6;

unsigned int motorOneSpeed;   // mm/s
unsigned int motorTwoSpeed;   // deg/s
int motorOneSpeedAddr = 0;
int motorTwoSpeedAddr = 2;


const int mmPerRev = 10;      // mm
const int fullStepsPerRev = 200;
unsigned int microsteps;
int microstepsAddr = 8;
int stepsPerRev;
float distPerStep;
float degPerStep;


int mode;
int modeAddr = 10;
int state = 0;


Motor motorOne { DIRPIN1, STEPPIN1, LOW, LOW, 0, 0, 0, 0 };
Motor motorTwo { DIRPIN2, STEPPIN2, LOW, LOW, 0, 0, 0, 0 };


void setup() {
  Serial.begin(115200);

  // sets Pins 8,9,10,11 to Outputs, Pins 12,13 to Inputs
  DDRB = DDRB | 0b00001111;
  DDRB = DDRB & 0b11001111;

  // get motor speeds
  motorOneSpeed = readIntFromEEPROM(motorOneSpeedAddr);
  motorTwoSpeed = readIntFromEEPROM(motorTwoSpeedAddr);

  // get tube length and offset
  tubeLength = readIntFromEEPROM(tubeLengthAddr);
  tubeOffset = readIntFromEEPROM(tubeOffsetAddr);

  // get mode
  mode = readIntFromEEPROM(modeAddr);

  // get microstepping 
  microsteps = readIntFromEEPROM(microstepsAddr);

  // calc steps per revolution
  stepsPerRev = fullStepsPerRev * microsteps;  

  // calc distance/degrees per step
  distPerStep = mmPerRev / stepsPerRev;
  degPerStep = 360.0 / stepsPerRev;

  // set step delays
  motorOne.stepDelay = (unsigned int)((1 / (motorOneSpeed / distPerStep) * 0.5) * 1e6);
  motorTwo.stepDelay = (unsigned int)((1 / (motorTwoSpeed / degPerStep) * 0.5) * 1e6);

  
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
      unsigned int tempMotorOneSpeed = message[5]<<8 + message[6];
      unsigned int tempMotorTwoSpeed = message[7]<<8 + message[8];
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
      if (mode != tempMode) {
        mode = tempMode;
        writeIntIntoEEPROM(modeAddr, mode);
      }
      if (microsteps != tempMicrosteps) {
        microsteps = tempMicrosteps;
        writeIntIntoEEPROM(microstepsAddr, microsteps);
        stepsPerRev = fullStepsPerRev * microsteps;  
        degPerStep = 360.0 / stepsPerRev;
        distPerStep = mmPerRev / stepsPerRev;
      }
      if (motorOneSpeed!= tempMotorOneSpeed) {
        motorOneSpeed = tempMotorOneSpeed;
        writeIntIntoEEPROM(motorOneSpeed, motorOneSpeedAddr);
        motorOne.stepDelay = (unsigned int)((1 / (motorOneSpeed / distPerStep) * 0.5) * 1e6);
      }
      if (motorTwoSpeed!= tempMotorTwoSpeed) {
        motorTwoSpeed = tempMotorTwoSpeed;
        writeIntIntoEEPROM(motorTwoSpeed, motorTwoSpeedAddr);
        motorTwo.stepDelay = (unsigned int)((1 / (motorTwoSpeed / degPerStep) * 0.5) * 1e6);
      }
          
    }

  } else if (state == 1) {

    // move the first motor to the start position of the tube
    motorOne.dirState = HIGH;
    
    while (motorOne.pos < tubeOffset) {
      motorStep(motorOne);
    }

    delay(1000);

    // sequence depends on the mode. if mode 1, then the linear motor
    // will go the full length, and then the rotary motor will move a 
    // small amount. if mode 2, then the rotar motor will do a full
    // rotation, and then the linear motor will move a small amount.

    // calc how many steps the linear motor has to move each time

    // calc how many steps the rotary motor has to move each time

    // calc how many total sequences to scan entire tube
    
  } 
  
  


  
}


bool motorHoming(Motor m) {
  bool homed = false;
  int proxState = fastDigitalRead(PINB, PROXPIN1);
  if (proxState) {
    homed = true;
    m.pos = 0;      // set position to zero
  } else {
    motorStep(m);
  }
  return homed;
}


void motorStep(Motor m) {
  if (micros() - m.prevStep > m.stepDelay) {
    m.prevStep = micros();
    m.stepState = m.stepState ? HIGH : LOW;  // toggle from high to low or vice versa

    // only increment when stepState is HIGH. If dir is 0, then it will multiply by 
    // -1, thus decrementing stepCounter, otherwise if dir is 1, then it will multiply 
    // by 1, incrementing stepCounter
    m.stepCounter += (1 * m.stepState) * (2 * m.dirState - 1);
    
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



