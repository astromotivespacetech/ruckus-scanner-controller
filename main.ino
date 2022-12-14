#include <EEPROM.h>

#define STEPPIN1  22 
#define DIRPIN1   23
#define STEPPIN2  24 
#define DIRPIN2   25
#define PROXPIN1  26
#define PROXPIN2  27

const byte numChars = 20;
byte message[numChars];
boolean newData = false;
unsigned long checkSerialDelay = 100000;  // micros
unsigned long prevCheck;

unsigned int scanLength;      // mm
unsigned int tubeOffset;      // mm
unsigned int motorOneSpeed;   // mm/s
unsigned int motorTwoSpeed;   // deg/s
unsigned int stepOver;        // mm
float stepDown;               // deg
unsigned int tubeDiameter;    // mm 

const float mmPerRev = 10.0;      // mm
const int fullStepsPerRev = 200;
unsigned int linearMicrosteps = 2;
unsigned int angularMicrosteps = 4;
int linearStepsPerRev;
int angularStepsPerRev;
float distPerStep;
float degPerStep;
const float wheelDiameter = 63.5; // mm

unsigned int mode;
int state = 0;

// addresses
int scanLengthAddr = 0;
int tubeOffsetAddr = 2;
int modeAddr = 4;
int motorOneSpeedAddr = 6;
int motorTwoSpeedAddr = 8;
int stepDownAddr = 10;
int stepOverAddr = 12;
int tubeDiameterAddr = 14;

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
  int type;
};

Motor motorOne { DIRPIN1, STEPPIN1, LOW, LOW, 0, 0, 0, 0.0, 0.0, 0 };
Motor motorTwo { DIRPIN2, STEPPIN2, LOW, LOW, 0, 0, 0, 0.0, 0.0, 1 };

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
  stepDown = (float)(readIntFromEEPROM(stepDownAddr)) * 0.01;
  stepOver = readIntFromEEPROM(stepOverAddr);

  // get mode
  mode = readIntFromEEPROM(modeAddr);

  // get diameter
  tubeDiameter = readIntFromEEPROM(tubeDiameterAddr);

  // calc steps per revolution
  linearStepsPerRev = fullStepsPerRev * linearMicrosteps;  
  angularStepsPerRev = fullStepsPerRev * angularMicrosteps;  

  // calc circumference ratio
  float tubeCirc = tubeDiameter * PI;
  float wheelCirc = wheelDiameter * PI;
  float ratio = wheelCirc / tubeCirc;

  // calc distance/degrees per step
  distPerStep = mmPerRev / linearStepsPerRev; // 0.0125 mm
  degPerStep = (360.0 / angularStepsPerRev) * ratio; 

  motorOne.dPerStep = distPerStep;
  motorTwo.dPerStep = degPerStep;

  // set step delays
  motorOne.stepDelay = (unsigned int)((1 / (motorOneSpeed / distPerStep) * 0.5) * 1e6);
  motorTwo.stepDelay = (unsigned int)((1 / (motorTwoSpeed / degPerStep) * 0.5) * 1e6);

}

void loop() {

  if (micros() - prevCheck > checkSerialDelay) {
    recvWithStartEndMarkers();     
    prevCheck = micros();
  }
  

  if (state == 1) {

    Motor *ptrOne = &motorOne;
    Motor *ptrTwo = &motorTwo;
 
    while (motorOne.pos <= tubeOffset) {
      motorStep( ptrOne );
    }

    delay(1000);

    // sequence depends on the mode. if mode 1, then the linear motor
    // will go the full length, and then the rotary motor will move a 
    // small amount. if mode 2, then the rotar motor will do a full
    // rotation, and then the linear motor will move a small amount.

    if (mode == 1) {    

      motorTwo.dirState = LOW;

      // calc how many total sequences to scan entire tube
      int num = (int)(360.0 / stepDown);
      
      // go through scanning sequence _num_ times, until tube has done a full revolution
      for (int i = 0; i < num; i++) {

        if (state == 0) {
          state = 2;
          break;
        }
        
        if (!motorOne.dirState) {
          while (motorOne.pos <= (tubeOffset + scanLength)) {
            motorStep( ptrOne );
          }
        } else {
          while (motorOne.pos >= tubeOffset) {
            motorStep( ptrOne );
          }
        }
                
        // change direction of linear motor each time
        motorOne.dirState = (motorOne.dirState) ? LOW : HIGH; 

        delay(500);

        // rotate the tube each time
        while (motorTwo.pos <= (stepDown*i)) {
          motorStep( ptrTwo );
        }

        delay(500);

      }

      // reset motor two pos for next sequence
      motorTwo.pos = 0;

    } else if (mode == 2) {


      // calc how many total sequences to scan entire tube
      int num = (int)(scanLength / stepOver);

      motorTwo.dirState = LOW;

      for (int i = 0; i < num; i++) {

        if (state == 0) {
          state = 2;
          break;
        }

        // rotate the tube a full rotation
        if (!motorTwo.dirState) {
          while (motorTwo.pos <= 360.0 ) {
            motorStep( ptrTwo );
          }
        } else {
          while (motorTwo.pos > 0 ) {
            motorStep( ptrTwo );
          }
        }
        
        motorTwo.dirState = (motorTwo.dirState) ? LOW : HIGH; 

        delay(500);

        int dist = motorOne.pos + stepOver;

        // move linear 
        while (motorOne.pos <= dist) {
          motorStep( ptrOne );
        }

        delay(500);
      }
    }
    
    state = 2;
    
  } else if (state == 2) {

    motorOne.dirState = HIGH;
    
    // run motor homing function until it returns true i.e. until 
    // it hits the proximity sensor, so we start with a known position
    Motor *ptrOne = &motorOne;
    while (!motorHoming( ptrOne )); 
    
    state = 0; 
    motorOne.dirState = LOW;

  }


  
}


bool motorHoming(Motor *m) {
  bool homed = false;

  // set linear speed to 50 mm/s 
  m->stepDelay = (unsigned int)((1 / (50 / distPerStep) * 0.5) * 1e6);
  
  if (!digitalRead(PROXPIN1)) {
    proxDebounce += 1;

    if (proxDebounce > 5) {
      homed = true;
      m->pos = 0;      // set position to zero
    }
  } else {
    motorStep( m );
    proxDebounce = 0;
  }

  // reset linear speed 
  m->stepDelay = (unsigned int)((1 / (motorOneSpeed / distPerStep) * 0.5) * 1e6);
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
    m->pos -= (1 * m->stepState) * (2 * (m->dirState) - 1) * m->dPerStep;


    // check angular position and reset if needed (always 0 to 360)
    if (m->type) {
      if (m->pos > 360.0) {
        m->pos -= 360.0;
      } else if (m->pos < 0.0) {
        m->pos += 360.0;
      }
    }


    if (micros() - prevCheck > checkSerialDelay) {
      recvWithStartEndMarkers();     
      prevCheck = micros();
    }

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
  static boolean jogData = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char requestData = 'x';
  char jogMarker = 'y';
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
      } else {
        message[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    } else if (rc == startMarker) {
      recvInProgress = true;
    } else if (rc == requestData) {
      Serial.print(scanLength);
      Serial.print(",");
      Serial.print(tubeOffset);
      Serial.print(",");
      Serial.print(motorOneSpeed); 
      Serial.print(",");
      Serial.print(motorTwoSpeed); 
      Serial.print(",");
      Serial.print(mode); 
      Serial.print(",");
      Serial.print(stepOver);
      Serial.print(",");
      Serial.print(readIntFromEEPROM(stepDownAddr));
      Serial.print(",");
      Serial.print(tubeDiameter);
      Serial.print(",");
      Serial.println();
      break;
    } else if (rc == jogMarker) {
      recvInProgress = true;
      jogData = true;
    }
  }



  if (newData) {

    if (jogData) {

      jogData = false;

      Motor *ptrOne = &motorOne;
      Motor *ptrTwo = &motorTwo;

      unsigned int linearPos = (message[0]<<8) + (message[1]);
      unsigned int angularPos = (message[2]<<8) + (message[3]);
      
      // determine linear direction
      if (motorOne.pos < linearPos) {
        motorOne.dirState = LOW;
        // move linear 
        while (motorOne.pos < linearPos) {
          motorStep( ptrOne );
        }
      } else if (motorOne.pos > linearPos) {
        motorOne.dirState = HIGH;
        // move linear 
        while (motorOne.pos > linearPos) {
          motorStep( ptrOne );
        }
      }

      if (motorTwo.pos < angularPos) {
        motorTwo.dirState = LOW;
        // rotate
        while (motorTwo.pos < angularPos) {
          motorStep( ptrTwo );
        }
      } else if (motorTwo.pos > angularPos) {
        motorTwo.dirState = HIGH;
        // rotate
        while (motorTwo.pos > angularPos) {
          motorStep( ptrTwo );
        }
      }

      // reset to low
      motorTwo.dirState = LOW;

    } else {

      unsigned int tempState = (message[0]<<8) + (message[1]);
      unsigned int tempScanLength = (message[2]<<8) + (message[3]);
      unsigned int tempTubeOffset = (message[4]<<8) + (message[5]);
      unsigned int tempMotorOneSpeed = (message[6]<<8) + (message[7]);
      unsigned int tempMotorTwoSpeed = (message[8]<<8) + (message[9]);
      unsigned int tempMode = (message[10]<<8) + (message[11]);
      unsigned int tempStepOver = (message[12]<<8) + (message[13]);
      unsigned int tempStepDown = (message[14]<<8) + (message[15]);
      unsigned int tempTubeDiameter = (message[16]<<8) + (message[17]);


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
      if (stepDown != (float)(tempStepDown)*0.01) {
        stepDown = (float)(tempStepDown)*0.01;
        writeIntIntoEEPROM(stepDownAddr, tempStepDown);
      }
      if (tubeDiameter != tempTubeDiameter) {
        tubeDiameter = tempTubeDiameter;
        writeIntIntoEEPROM(tubeDiameterAddr, tempTubeDiameter);
        float tubeCirc = tubeDiameter * PI;
        float wheelCirc = wheelDiameter * PI;
        float ratio = wheelCirc / tubeCirc;
        degPerStep = (360.0 / angularStepsPerRev) * ratio; 
        motorTwo.dPerStep = degPerStep;
      }
          
    }
  
    newData = false; 
  }       
}
