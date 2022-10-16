import controlP5.*;
import processing.serial.*;

Serial port;   
boolean port_exists = false;

String[] ports = Serial.list();


// screen dimensions
int W = 1792;
int H = 1120;

PFont font1;

ControlP5 cp5;
Button b1, b2, b3, b4, b5, b6, b7, b8, mode1, mode2;           
Knob k1, k2;
Slider s1, s2, s3;

int tubeLength = 1000;
int tubeOffset = 555;
int motorOneSpeed = 50;
int motorTwoSpeed = 90;
int mode = 1;
int stepOver = 50;
int stepDown = 35;


void setup() {
  //size(1792, 1120);  
  size(1300, 1000);  
  font1 = createFont("Monospaced", 16);
  
  // List all the available serial ports:
  printArray(ports);
  
  
  try {
    // Open the port you are using at the rate you want:
    port = new Serial(this, ports[5], 115200);
    port_exists = true;
  } catch (Exception e) {
    e.printStackTrace();
  }
  

  cp5 = new ControlP5(this);
  
  b1 = cp5.addButton("save")
    .setCaptionLabel("Save")
    .setPosition(100, 800)
    .setSize(200, 70)
    .setFont(font1);

  
  k1 = cp5.addKnob("speed1")
   .setCaptionLabel("Linear Speed (mm/s)")
   .setRange(1,30)
   .setValue(3)
   .setPosition(700,200)
   .setRadius(100)
   .setNumberOfTickMarks(29)
   .setTickMarkLength(4)
   .snapToTickMarks(true)
   .setDragDirection(Knob.VERTICAL)
   .setFont(font1);
  
  
  k2 = cp5.addKnob("speed2")
   .setCaptionLabel("Angular Speed (deg/s)")
   .setRange(10,360)
   .setValue(100)
   .setPosition(700,550)
   .setRadius(100)
   .setNumberOfTickMarks(35)
   .setTickMarkLength(4)
   .snapToTickMarks(true)
   .setDragDirection(Knob.VERTICAL)
   .setFont(font1);
   
  
  s1 = cp5.addSlider("tubelength")
     .setCaptionLabel("")
     .setPosition(135, 380)
     .setSize(400,25)
     .setRange(1,1000)
     .setValue(100)
     .setNumberOfTickMarks(1000)
     .setFont(font1);
    
  b3 = cp5.addButton("plus")
    .setCaptionLabel("+")
    .setPosition(545, 380)
    .setSize(25, 25)
    .setFont(font1);
    
  b4 = cp5.addButton("minus")
    .setCaptionLabel("-")
    .setPosition(100, 380)
    .setSize(25, 25)
    .setFont(font1);
    
    
    
  s2 = cp5.addSlider("stepover")
     .setCaptionLabel("")
     .setPosition(135, 480)
     .setSize(400,25)
     .setRange(1,50)
     .setValue(stepOver)
     .setNumberOfTickMarks(50)
     .setFont(font1);
    
  b5 = cp5.addButton("plusStepover")
    .setCaptionLabel("+")
    .setPosition(545, 480)
    .setSize(25, 25)
    .setFont(font1);
    
  b6 = cp5.addButton("minusStepover")
    .setCaptionLabel("-")
    .setPosition(100, 480)
    .setSize(25, 25)
    .setFont(font1);
    
    
  s3 = cp5.addSlider("stepdown")
    .setCaptionLabel("")
    .setPosition(135, 580)
    .setSize(400,25)
    .setRange(1,5)
    .setValue(stepDown*0.1)
    .setNumberOfTickMarks(9)
    .setFont(font1);
  
  b7 = cp5.addButton("plusStepdown")
    .setCaptionLabel("+")
    .setPosition(545, 580)
    .setSize(25, 25)
    .setFont(font1);
  
  b8 = cp5.addButton("minusStepdown")
    .setCaptionLabel("-")
    .setPosition(100, 580)
    .setSize(25, 25)
    .setFont(font1);
    
    
    
    
    
    
  

  mode1 = cp5.addButton("mode1")
    .setCaptionLabel("Mode 1")
    .setPosition(100, 200)
    .setSize(200, 70)
    .setFont(font1)
    .setColorBackground(#2266cc);
  
  mode2 = cp5.addButton("mode2")
    .setCaptionLabel("Mode 2")
    .setPosition(310, 200)
    .setSize(200, 70)
    .setFont(font1)
    .setColorBackground(#444444);
  

  
  b1.getCaptionLabel().toUpperCase(false);
  k1.getCaptionLabel().toUpperCase(false);
  k2.getCaptionLabel().toUpperCase(false);

}



void draw() {
  background(20, 20, 25);
  
  textSize(24);
  text("Ruckus Composites", 100, 100);
  
  textSize(16);
  text("Tube Length (mm)", 100, 360);
  text("Stepover (mm)", 100, 460);
  text("Stepdown (deg)", 100, 560);
}




void save() {
  sendCommand(); 
}


void mode1() {
  mode = 1;
  mode1.setColorBackground(#2266cc);
  mode2.setColorBackground(#444444);
}

void mode2() {
  mode = 2;
  mode2.setColorBackground(#2266cc);
  mode1.setColorBackground(#444444);
}

  



void tubelength() {
  tubeLength = int(s1.getValue());
}

void stepover() {
  stepOver = int(s2.getValue());
}

void stepdown() {
  stepDown = int(s3.getValue()*10);
}

void plus() {
  float f = s1.getValue();
  s1.setValue(f + 1.0);
  tubeLength = int(s1.getValue());
}

void minus() {
  float f = s1.getValue();
  s1.setValue(f - 1.0);
  tubeLength = int(s1.getValue());
}

void plusStepover() {
  float f = s2.getValue();
  s2.setValue(f + 1.0);
  stepOver = int(s2.getValue());
}

void minusStepover() {
  float f = s2.getValue();
  s2.setValue(f - 1.0);
  stepOver = int(s2.getValue());
}

void plusStepdown() {
  float f = s3.getValue();
  s3.setValue(f + 0.5);
  stepDown = int(s3.getValue()*10);
}

void minusStepdown() {
  float f = s3.getValue();
  s3.setValue(f - 0.5);
  stepDown = int(s3.getValue()*10);
}


void speed1() {
  float f = k1.getValue()*10;
  motorOneSpeed = int(f);
}
 
 
void speed2() {
  float f = k2.getValue();
  motorTwoSpeed = int(f);
}





void sendCommand() {
  
  if (port_exists) {
    port.write("<");
    writeInt(tubeLength);
    writeInt(tubeOffset);
    writeInt(motorOneSpeed);
    writeInt(motorTwoSpeed);
    writeInt(mode);
    writeInt(stepOver);
    writeInt(stepDown);
    port.write(">");
  }
  
  println(tubeLength);
  println(tubeOffset);
  println(motorOneSpeed);
  println(motorTwoSpeed);
  println(mode);
  println(stepOver);
  println(stepDown);

}

void writeInt(int x) {
  port.write(x >> 8);
  port.write(x & 255);
}
  
  
