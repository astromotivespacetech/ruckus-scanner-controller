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
Button b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, mode1, mode2;           
Knob k1, k2;
Slider s1, s2, s3, s4;

int state = 0;
int scanLength = 1000;
int tubeOffset = 555;
int motorOneSpeed = 50;
int motorTwoSpeed = 90;
int mode = 1;
int stepOver = 5;
int stepDown = 100;

int margin = 100;
int scanLengthTop = 380;
int tubeOffsetTop = scanLengthTop + margin;
int stepoverTop = scanLengthTop + margin*2;
int stepdownTop = scanLengthTop + margin*3;


void setup() {
  //size(1792, 1120);  
  size(1100, 1000);  
  font1 = createFont("Monospaced", 16);
  
  // List all the available serial ports:
  printArray(ports);
  
  
  try {
    // Open the port you are using at the rate you want:
    port = new Serial(this, ports[ports.length-1], 115200);
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
    
  b2 = cp5.addButton("run")
    .setCaptionLabel("Run")
    .setPosition(310, 800)
    .setSize(200, 70)
    .setFont(font1);

  b3 = cp5.addButton("plus")
    .setCaptionLabel("+")
    .setPosition(545, scanLengthTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b4 = cp5.addButton("minus")
    .setCaptionLabel("-")
    .setPosition(100, scanLengthTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b5 = cp5.addButton("plusTubeOffset")
    .setCaptionLabel("+")
    .setPosition(545, tubeOffsetTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b6 = cp5.addButton("minusTubeOffset")
    .setCaptionLabel("-")
    .setPosition(100, tubeOffsetTop)
    .setSize(25, 25)
    .setFont(font1);
        
  b7 = cp5.addButton("plusStepover")
    .setCaptionLabel("+")
    .setPosition(545, stepoverTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b8 = cp5.addButton("minusStepover")
    .setCaptionLabel("-")
    .setPosition(100, stepoverTop)
    .setSize(25, 25)
    .setFont(font1);
  
  b9 = cp5.addButton("plusStepdown")
    .setCaptionLabel("+")
    .setPosition(545, stepdownTop)
    .setSize(25, 25)
    .setFont(font1);
  
  b10 = cp5.addButton("minusStepdown")
    .setCaptionLabel("-")
    .setPosition(100, stepdownTop)
    .setSize(25, 25)
    .setFont(font1);
    
  k1 = cp5.addKnob("speed1")
   .setCaptionLabel("Linear Speed (mm/s)")
   .setRange(10,50)
   .setValue(30)
   .setPosition(700,200)
   .setRadius(100)
   .setNumberOfTickMarks(40)
   .setTickMarkLength(4)
   .snapToTickMarks(true)
   .setDragDirection(Knob.VERTICAL)
   .setFont(font1);
 
  k2 = cp5.addKnob("speed2")
   .setCaptionLabel("Angular Speed (deg/s)")
   .setRange(90,360)
   .setValue(180)
   .setPosition(700,550)
   .setRadius(100)
   .setNumberOfTickMarks(6)
   .setTickMarkLength(4)
   .snapToTickMarks(true)
   .setDragDirection(Knob.VERTICAL)
   .setFont(font1);
   
  s1 = cp5.addSlider("scanlength")
     .setCaptionLabel("")
     .setPosition(135, scanLengthTop)
     .setSize(400,25)
     .setRange(100,1000)
     .setValue(500)
     .setNumberOfTickMarks(901)
     .setFont(font1);
     
  s2 = cp5.addSlider("tubeoffset")
     .setCaptionLabel("")
     .setPosition(135, tubeOffsetTop)
     .setSize(400,25)
     .setRange(100,1000)
     .setValue(500)
     .setNumberOfTickMarks(901)
     .setFont(font1);    

  s3 = cp5.addSlider("stepover")
     .setCaptionLabel("")
     .setPosition(135, stepoverTop)
     .setSize(400,25)
     .setRange(1,10)
     .setValue(stepOver)
     .setNumberOfTickMarks(10)
     .setFont(font1);
     
  s4 = cp5.addSlider("stepdown")
    .setCaptionLabel("")
    .setPosition(135, stepdownTop)
    .setSize(400,25)
    .setRange(0.1,2.0)
    .setValue(stepDown*0.01)
    .setNumberOfTickMarks(20)
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
  b2.getCaptionLabel().toUpperCase(false);
  k1.getCaptionLabel().toUpperCase(false);
  k2.getCaptionLabel().toUpperCase(false);
  mode1.getCaptionLabel().toUpperCase(false);
  mode2.getCaptionLabel().toUpperCase(false);
}



void draw() {
  background(20, 20, 25);
  
  textSize(24);
  text("Ruckus Composites", 100, 100);
  
  textSize(16);
  text("Scan Length (mm)", 100, scanLengthTop-20);
  text("Tube Offset (mm)", 100, tubeOffsetTop-20);
  text("Stepover (mm)", 100, stepoverTop-20);
  text("Stepdown (deg)", 100, stepdownTop-20);
}




void save() {
  sendCommand(); 
}

void run() {
  state = 1;
  sendCommand();
  state = 0;
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

void scanlength() {
  scanLength = int(s1.getValue());
}

void tubeoffset() {
  tubeOffset = int(s2.getValue());
}

void stepover() {
  stepOver = int(s3.getValue());
}

void stepdown() {
  stepDown = int(s4.getValue()*100);
}

void plus() {
  s1.setValue(s1.getValue() + 1.0);
  scanLength = int(s1.getValue());
}

void minus() {
  s1.setValue(s1.getValue() - 1.0);
  scanLength = int(s1.getValue());
}

void plusTubeOffset() {
  s2.setValue(s2.getValue() + 1.0);
  tubeOffset = int(s2.getValue());
}

void minusTubeOffset() {
  s2.setValue(s2.getValue() - 1.0);
  tubeOffset = int(s2.getValue());
}

void plusStepover() {
  s3.setValue(s3.getValue() + 1.0);
  stepOver = int(s3.getValue());
}

void minusStepover() {
  s3.setValue(s3.getValue() - 1.0);
  stepOver = int(s3.getValue());
}

void plusStepdown() {
  s4.setValue(s4.getValue() + 0.5);
  stepDown = int(s4.getValue()*100);
}

void minusStepdown() {
  s4.setValue(s4.getValue() - 0.5);
  stepDown = int(s4.getValue()*100);
}

void speed1() {
  float f = k1.getValue();
  motorOneSpeed = int(f);
}
 
void speed2() {
  float f = k2.getValue();
  motorTwoSpeed = int(f);
}





void sendCommand() {
  
  if (port_exists) {
    port.write("<");
    writeInt(state);
    writeInt(scanLength);
    writeInt(tubeOffset);
    writeInt(motorOneSpeed);
    writeInt(motorTwoSpeed);
    writeInt(mode);
    writeInt(stepOver);
    writeInt(stepDown);
    port.write(">");
  }
  
  println(state);
  println(scanLength);
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
  
  
