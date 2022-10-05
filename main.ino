#include <EEPROM.h>

#define dir1Pin   0b00000001 // GPIO 8
#define step1Pin  0b00000010 // GPIO 9
#define dir2Pin   0b00000100 // GPIO 10
#define step2Pin  0b00001000 // GPIO 11
#define prox1Pin  0b00010000 // GPIO 12
#define prox2Pin  0b00100000 // GPIO 13

int state = 0;

struct Motor {
  int dirPin;
  int stepPin;
  int dirState;
  int stepState;
  unsigned long prevStep;     // micros
  int stepCounter;
  unsigned long stepDelay;    // micros
  int stepDelayAddress;       // EEPROM address where stepDelay lives
}


Motor motor1 { dir1Pin, step1Pin, LOW, LOW, 0, 0, 0, 0 }
Motor motor2 { dir2Pin, step2Pin, LOW, LOW, 0, 0, 0, 2 }


void setup() {
  Serial.begin(115200);

  // get stepDelay from addresses
  motor1.stepDelay = readIntFromEEPROM(motor1.stepDelayAddress);
  motor2.stepDelay = readIntFromEEPROM(motor2.stepDelayAddress);

  
}

void loop() {
  if (Serial.available()) {
    state = (int)Serial.read();
  }
  


  
}


bool motorHoming(motor) {
  bool homed = false;
  int proxState = fastDigitalRead(PINB, prox1pin);
  if (proxState) {
    homed = true;
  } else {
    motorStep(motor, 0);
  }
  return homed;
}


void motorStep(motor, dir) {
  if (micros() - motor.prevStep > motor.stepDelay) {
    motor.prevStep = micros();
    motor.stepState = motorStepState ? HIGH : LOW;  // toggle from high to low or vice versa
    fastDigitalWrite(motor.dirPin, motor.dirState);
    fastDigitalWrite(motor.stepPin, motor.stepState);
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
