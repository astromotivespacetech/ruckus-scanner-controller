#include <EEPROM.h>

#define STEPPIN1  22 // GPIO 23
#define DIRPIN1   23 // GPIO 22
#define STEPPIN2  24 // GPIO 25
#define DIRPIN2   25 // GPIO 24
#define PROXPIN1  26 // GPIO 26
#define PROXPIN2  27 // GPIO 27

const byte numChars = 16;
byte message[numChars];
boolean newData = false;
int checkSerialDelay = 1000;  // micros
unsigned long prevCheck;


struct Motor {
  int dirPin;
  int stepPin;
  int dirState;
  int stepState;
  unsigned long prevStep;     // micros
  unsigned int stepCounter;
  unsigned long stepDelay;     // micros
  float dPerStep;
  float pos;                  // mm or angle (deg);
};


unsigned int scanLength;      // mm
unsigned int tubeOffset;      // mm
unsigned int motorOneSpeed;   // mm/s
unsigned int motorTwoSpeed;   // deg/s
unsigned int stepOver;        // mm
float stepDown;               // deg

const float mmPerRev = 10.0;      // mm
const int fullStepsPerRev = 200;
unsigned int microsteps = 2;
int stepsPerRev;
float distPerStep;
float degPerStep;
unsigned int mode;
int state = 0;


int scanLengthAddr = 0;
int tubeOffsetAddr = 2;
int modeAddr = 4;
int motorOneSpeedAddr = 6;
int motorTwoSpeedAddr = 8;
int stepDownAddr = 10;
int stepOverAddr = 12;

Motor motorOne { DIRPIN1, STEPPIN1, LOW, LOW, 0, 0, 0, 0.0, 0.0 };
Motor motorTwo { DIRPIN2, STEPPIN2, LOW, LOW, 0, 0, 0, 0.0, 0.0 };

int proxDebounce = 0;


void setup() {
  Serial.begin(115200);

  // sets Pins 8,9,10,11 to Outputs, Pins 12,13 to Inputs
  pinMode(DIRPIN1, OUTPUT);
  pinMode(DIRPIN2, OUTPUT);
  pinMode(STEPPIN1, OUTPUT);
  pinMode(STEPPIN2, OUTPUT);
  pinMode(PROXPIN1, INPUT);
  pinMode(PROXPIN2, INPUT);
  

  // get motor speeds
  motorOneSpeed = readIntFromEEPROM(motorOneSpeedAddr);
  motorTwoSpeed = readIntFromEEPROM(motorTwoSpeedAddr);


  // get tube length and offset
  scanLength = readIntFromEEPROM(scanLengthAddr);
  tubeOffset = readIntFromEEPROM(tubeOffsetAddr);

  // get stepover and stepdown 
  stepDown = (float)(readIntFromEEPROM(stepDownAddr)) * 0.1;
  stepOver = readIntFromEEPROM(stepOverAddr);

  // get mode
  mode = readIntFromEEPROM(modeAddr);

  // calc steps per revolution
  stepsPerRev = fullStepsPerRev * microsteps;  

  // calc distance/degrees per step
  distPerStep = mmPerRev / stepsPerRev;
  degPerStep = 360.0 / stepsPerRev;

  motorOne.dPerStep = distPerStep;
  motorTwo.dPerStep = degPerStep;

  // set step delays
  motorOne.stepDelay = (unsigned int)((1 / (motorOneSpeed / distPerStep) * 0.5) * 1e6);
  motorTwo.stepDelay = (unsigned int)((1 / (motorTwoSpeed / degPerStep) * 0.5) * 1e6);


  Serial.print("Scan Length: ");
  Serial.println(scanLength);
  Serial.print("Tube Offset: ");
  Serial.println(tubeOffset);
  Serial.print("Motor 1 Speed: ");
  Serial.println(motorOneSpeed); 
  Serial.print("Motor 2 Speed: ");
  Serial.println(motorTwoSpeed); 
  Serial.print("Mode: ");
  Serial.println(mode); 
  Serial.print("Microsteps: ");
  Serial.println(microsteps); 
  
}

void loop() {

  if (micros() - prevCheck > checkSerialDelay) {
    recvWithStartEndMarkers();     
    prevCheck = micros();
  }
  

  if (state == 1) {
    
    while (motorOne.pos < tubeOffset) {
      Motor *ptr = &motorOne;
      motorStep( ptr );
    }

    delay(1000);

    // sequence depends on the mode. if mode 1, then the linear motor
    // will go the full length, and then the rotary motor will move a 
    // small amount. if mode 2, then the rotar motor will do a full
    // rotation, and then the linear motor will move a small amount.

    if (mode) {    

//      // calc how many steps the linear motor has to move each time
//      int  linearSteps = distPerStep * scanLength;
      
      // calc how many steps the rotary motor has to move each time
      int angSteps = degPerStep * stepDown;
  
      // calc how many total sequences to scan entire tube
      int num = (int)(360.0 / angSteps);

      // go through scanning sequence _num_ times, until tube has done a full revolution
      for (int i = 0; i < num; i++) {
        if (motorOne.dirState) {
          while (motorOne.pos < (tubeOffset + scanLength)) {
            Motor *ptr = &motorOne;
            motorStep( ptr );
          }
        } else {
          while (motorOne.pos > tubeOffset) {
            Motor *ptr = &motorOne;
            motorStep( ptr );
          }
        }
        
        // change direction of linear motor each time
        motorOne.dirState = (motorOne.dirState) ? LOW : HIGH; 

        // rotate the tube each time
        for (int j = 0; j < angSteps; j++) {
          Motor *ptr = &motorTwo;
          motorStep( ptr );
        }
      }

    } else {
      
    }
    
  } else if (state == 2) {

    motorOne.dirState = HIGH;
    
    // run motor homing function until it returns true i.e. until 
    // it hits the proximity sensor, so we start with a known position
    Motor *ptr = &motorOne;
    while (!motorHoming( ptr )); 
    
    state = 0; 
    motorOne.dirState = LOW;

  }


  
}


bool motorHoming(Motor *m) {
  bool homed = false;

  int x = digitalRead(PROXPIN1);
  int y = digitalRead(PROXPIN2);

  
  if (!x || !y) {
    proxDebounce += 1;

    if (proxDebounce > 10) {
      homed = true;
      m->pos = 0;      // set position to zero
    }
  } else {
    motorStep( m );
    proxDebounce = 0;
  }
  return homed;
}


void motorStep(Motor *m) {
  if (micros() - m->prevStep > m->stepDelay) {
    m->prevStep = micros();
    m->stepState = (m->stepState) ? LOW : HIGH;  // toggle from high to low or vice versa
    
    // only increment when stepState is HIGH. If dir is 0, then it will multiply by 
    // -1, thus decrementing stepCounter, otherwise if dir is 1, then it will multiply 
    // by 1, incrementing stepCounter
    m->stepCounter += (1 * m->stepState) * (2 * m->dirState - 1);

    // update position based on the distance per step for this motor
    m->pos += (1 * m->stepState) * (2 * m->dirState - 1) * m->dPerStep;
    
    digitalWrite(m->dirPin, m->dirState);
    digitalWrite(m->stepPin, m->stepState);
  }
}

void writeIntIntoEEPROM(int address, unsigned int number) { 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

unsigned int readIntFromEEPROM(int address) {
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}






void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    byte rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                message[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                message[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }



    if (newData) {

      unsigned int tempState = (message[0]<<8) + (message[1]);
      unsigned int tempScanLength = (message[2]<<8) + (message[3]);
      unsigned int tempTubeOffset = (message[4]<<8) + (message[5]);
      unsigned int tempMotorOneSpeed = (message[6]<<8) + (message[7]);
      unsigned int tempMotorTwoSpeed = (message[8]<<8) + (message[9]);
      unsigned int tempMode = (message[10]<<8) + (message[11]);
      unsigned int tempStepOver = (message[12]<<8) + (message[13]);
      unsigned int tempStepDown = (message[14]<<8) + (message[15]);

      if (state != tempState) {
         state = tempState;
      }
      if (scanLength != tempScanLength) {
        scanLength = tempScanLength;
        writeIntIntoEEPROM(scanLengthAddr, scanLength);
      }
      if (tubeOffset != tempTubeOffset) {
        tubeOffset = tempTubeOffset;
        writeIntIntoEEPROM(tubeOffsetAddr, tubeOffset);
      }
      if (mode != tempMode) {
        mode = tempMode;
        writeIntIntoEEPROM(modeAddr, mode);
      }
      if (motorOneSpeed != tempMotorOneSpeed) {
        motorOneSpeed = tempMotorOneSpeed;
        writeIntIntoEEPROM(motorOneSpeedAddr, motorOneSpeed);
        motorOne.stepDelay = (unsigned int)((1 / (motorOneSpeed / distPerStep) * 0.5) * 1e6);
      }
      if (motorTwoSpeed != tempMotorTwoSpeed) {
        motorTwoSpeed = tempMotorTwoSpeed;
        writeIntIntoEEPROM(motorTwoSpeedAddr, motorTwoSpeed);
        motorTwo.stepDelay = (unsigned int)((1 / (motorTwoSpeed / degPerStep) * 0.5) * 1e6);
      }
      if (stepOver != tempStepOver) {
        stepOver = tempStepOver;
        writeIntIntoEEPROM(stepOverAddr, stepOver);
      }
      if (stepDown != (float)(tempStepDown)*0.1) {
        stepDown = (float)(tempStepDown)*0.1;
        writeIntIntoEEPROM(stepOverAddr, stepDown);
      }

      newData = false;
          
    }
    
}
