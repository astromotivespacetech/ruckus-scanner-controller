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
Button b1, b2, b3, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, mode1, mode2;           
Knob k1, k2;
Slider scanlength, tubeoffset, stepover, stepdown, tubediameter;

int state = 0;
int scanLength = 1000;
int tubeOffset = 555;
int motorOneSpeed = 50;
int motorTwoSpeed = 90;
int mode = 1;
int stepOver = 5;
int stepDown = 100;
int tubeDiameter = 75;

int sectionOneLeft = 100;
int sectionOneTop = 200;
int sectionTwoLeft = 700;
int sectionThreeLeft = 1350;
int buttonWidth = 200;
int buttonHeight = 70;
int buttonSpacing = 100;
int margin = 100;
int scanLengthTop = sectionOneTop;
int tubeOffsetTop = scanLengthTop + margin;
int stepoverTop = scanLengthTop + margin*2;
int stepdownTop = scanLengthTop + margin*3;
int tubediameterTop = scanLengthTop + margin*4;
int sliderLength = 400;





void setup() {
  //size(1792, 1120);  
  size(1792, 800);  
  font1 = createFont("Monospaced", 16);
  
  // List all the available serial ports:
  printArray(ports);
  
  
  try {
    // Open the port you are using at the rate you want:
    port = new Serial(this, ports[ports.length-1], 115200);
    //port = new Serial(this, "/dev/cu.usbmodem142401", 115200);
    port_exists = true;
  } catch (Exception e) {
    e.printStackTrace();
  }
  
  
  if (port_exists) {
    delay(1000);
    port.write("x");
  }



  cp5 = new ControlP5(this);
  
  b1 = cp5.addButton("save")
    .setCaptionLabel("Save")
    .setPosition(sectionOneLeft, sectionOneTop + buttonSpacing*2)
    .setSize(200, 70)
    .setFont(font1);
    
  b2 = cp5.addButton("run")
    .setCaptionLabel("Start")
    .setPosition(sectionOneLeft, sectionOneTop + buttonSpacing)
    .setSize(200, 70)
    .setFont(font1);
    
  b3 = cp5.addButton("home")
    .setCaptionLabel("Home")
    .setPosition(sectionOneLeft+buttonWidth+10, sectionOneTop + buttonSpacing)
    .setSize(200, 70)
    .setFont(font1);
    
  b5 = cp5.addButton("plus")
    .setCaptionLabel("+")
    .setPosition(sectionTwoLeft+sliderLength+35, scanLengthTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b6 = cp5.addButton("minus")
    .setCaptionLabel("-")
    .setPosition(sectionTwoLeft, scanLengthTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b7 = cp5.addButton("plusTubeOffset")
    .setCaptionLabel("+")
    .setPosition(sectionTwoLeft+sliderLength+35, tubeOffsetTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b8 = cp5.addButton("minusTubeOffset")
    .setCaptionLabel("-")
    .setPosition(sectionTwoLeft, tubeOffsetTop)
    .setSize(25, 25)
    .setFont(font1);
        
  b9 = cp5.addButton("plusStepover")
    .setCaptionLabel("+")
    .setPosition(sectionTwoLeft+sliderLength+35, stepoverTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b10 = cp5.addButton("minusStepover")
    .setCaptionLabel("-")
    .setPosition(sectionTwoLeft, stepoverTop)
    .setSize(25, 25)
    .setFont(font1);
  
  b11 = cp5.addButton("plusStepdown")
    .setCaptionLabel("+")
    .setPosition(sectionTwoLeft+sliderLength+35, stepdownTop)
    .setSize(25, 25)
    .setFont(font1);
  
  b12 = cp5.addButton("minusStepdown")
    .setCaptionLabel("-")
    .setPosition(sectionTwoLeft, stepdownTop)
    .setSize(25, 25)
    .setFont(font1);
    
  b13 = cp5.addButton("plusTubediameter")
    .setCaptionLabel("+")
    .setPosition(sectionTwoLeft+sliderLength+35, tubediameterTop)
    .setSize(25, 25)
    .setFont(font1);
  
  b14 = cp5.addButton("minusTubediameter")
    .setCaptionLabel("-")
    .setPosition(sectionTwoLeft, tubediameterTop)
    .setSize(25, 25)
    .setFont(font1);
    
  k1 = cp5.addKnob("speed1")
   .setCaptionLabel("Linear Speed (mm/s)")
   .setRange(20,300)
   .setValue(30)
   .setPosition(sectionThreeLeft,200)
   .setRadius(100)
   .setNumberOfTickMarks(28)
   .setTickMarkLength(4)
   .snapToTickMarks(true)
   .setDragDirection(Knob.VERTICAL)
   .setFont(font1);
 
  k2 = cp5.addKnob("speed2")
   .setCaptionLabel("Angular Speed (deg/s)")
   .setRange(90,540)
   .setValue(180)
   .setPosition(sectionThreeLeft,550)
   .setRadius(100)
   .setNumberOfTickMarks(10)
   .setTickMarkLength(4)
   .snapToTickMarks(true)
   .setDragDirection(Knob.VERTICAL)
   .setFont(font1);
   
  cp5.addSlider("scanlength")
     .setCaptionLabel("")
     .setPosition(sectionTwoLeft + 30, scanLengthTop)
     .setSize(sliderLength,25)
     .setRange(100,1000)
     .setValue(500)
     .setNumberOfTickMarks(901)
     .setFont(font1);
     
  cp5.addSlider("tubeoffset")
     .setCaptionLabel("")
     .setPosition(sectionTwoLeft + 30, tubeOffsetTop)
     .setSize(sliderLength,25)
     .setRange(0,1000)
     .setValue(500)
     .setNumberOfTickMarks(1001)
     .setFont(font1);    

  cp5.addSlider("stepover")
     .setCaptionLabel("")
     .setPosition(sectionTwoLeft + 30, stepoverTop)
     .setSize(sliderLength,25)
     .setRange(1,10)
     .setValue(stepOver)
     .setNumberOfTickMarks(10)
     .setFont(font1);
     
  cp5.addSlider("stepdown")
    .setCaptionLabel("")
    .setPosition(sectionTwoLeft + 30, stepdownTop)
    .setSize(sliderLength,25)
    .setRange(0.5,8.0)
    .setValue(stepDown*0.01)
    .setNumberOfTickMarks(16)
    .setFont(font1);
    
   cp5.addSlider("tubediameter")
    .setCaptionLabel("")
    .setPosition(sectionTwoLeft + 30, tubediameterTop)
    .setSize(sliderLength,25)
    .setRange(10,100)
    .setValue(tubeDiameter)
    .setNumberOfTickMarks(91)
    .setFont(font1);
    
  mode1 = cp5.addButton("mode1")
    .setCaptionLabel("Mode 1")
    .setPosition(sectionOneLeft, sectionOneTop)
    .setSize(buttonWidth, buttonHeight)
    .setFont(font1)
    .setColorBackground(#2266cc);
  
  mode2 = cp5.addButton("mode2")
    .setCaptionLabel("Mode 2")
    .setPosition(sectionOneLeft + buttonWidth + 10, sectionOneTop)
    .setSize(buttonWidth, buttonHeight)
    .setFont(font1)
    .setColorBackground(#444444);
  

  
  b1.getCaptionLabel().toUpperCase(false);
  b2.getCaptionLabel().toUpperCase(false);
  b3.getCaptionLabel().toUpperCase(false);
  //b4.getCaptionLabel().toUpperCase(false);
  k1.getCaptionLabel().toUpperCase(false);
  k2.getCaptionLabel().toUpperCase(false);
  mode1.getCaptionLabel().toUpperCase(false);
  mode2.getCaptionLabel().toUpperCase(false);
 
  
}



void draw() {
  background(20, 20, 25);
  
  textSize(24);
  text("Ruckus Composites", sectionOneLeft, 100);
  
  textSize(16);
  text("Scan Length (mm)", sectionTwoLeft, scanLengthTop-20);
  text("Tube Offset (mm)", sectionTwoLeft, tubeOffsetTop-20);
  text("Stepover (mm)", sectionTwoLeft, stepoverTop-20);
  text("Stepdown (deg)", sectionTwoLeft, stepdownTop-20);
  text("Tube Diameter (mm)", sectionTwoLeft, tubediameterTop-20);
  
  
  if (port_exists) {
    if ( port.available() > 0 ) {
      String val = port.readStringUntil('\n'); 
      
      try {
        println(val);
      
            
        if (val != null) {
          String[] vals = val.split(",");
          
          if (vals.length == 9) {
            scanLength = Integer.parseInt(vals[0]);
            tubeOffset = Integer.parseInt(vals[1]);
            motorOneSpeed = Integer.parseInt(vals[2]);
            motorTwoSpeed = Integer.parseInt(vals[3]);
            mode = Integer.parseInt(vals[4]);
            stepOver = Integer.parseInt(vals[5]);
            stepDown = Integer.parseInt(vals[6]);
            tubeDiameter = Integer.parseInt(vals[7]);
            
            cp5.getController("scanlength").setValue(parseFloat(scanLength));
            cp5.getController("tubeoffset").setValue(parseFloat(tubeOffset));
            cp5.getController("stepover").setValue(parseFloat(stepOver));
            cp5.getController("stepdown").setValue(stepDown*0.01);
            cp5.getController("tubediameter").setValue(parseFloat(tubeDiameter));
            k1.setValue(motorOneSpeed);
            k2.setValue(motorTwoSpeed);
            
            if (mode == 1) {
              mode1();
            } else if (mode == 2) {
              mode2();
            }
          }
        }
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
  }
}




void save() {
  sendCommand(); 
}

void run() {  
  int s = (b2.getCaptionLabel().getText() == "Start") ? 1 : 0;
  String x = (b2.getCaptionLabel().getText() == "Start") ? "Stop" : "Start";
  b2.setCaptionLabel(x);
  state = s;
  sendCommand();
}


void home() {
  state = 2;
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
  scanLength = int(cp5.getController("scanlength").getValue());
}

void tubeoffset() {
  tubeOffset = int(cp5.getController("tubeoffset").getValue());
}

void stepover() {
  stepOver = int(cp5.getController("stepover").getValue());
}

void stepdown() {
  stepDown = int(cp5.getController("stepdown").getValue()*100);
}

void tubediameter() {
  tubeDiameter = int(cp5.getController("tubediameter").getValue());
}

void plus() {
  Controller sl = cp5.getController("scanlength");
  sl.setValue(sl.getValue() + 1.0);
  scanLength = int(sl.getValue());
}

void minus() {
  Controller sl = cp5.getController("scanlength");
  sl.setValue(sl.getValue() - 1.0);
  scanLength = int(sl.getValue());
}

void plusTubeOffset() {
  Controller to = cp5.getController("tubeoffset");
  to.setValue(to.getValue() + 1.0);
  tubeOffset = int(to.getValue());
}

void minusTubeOffset() {
  Controller to = cp5.getController("tubeoffset");
  to.setValue(to.getValue() - 1.0);
  tubeOffset = int(to.getValue());
}

void plusStepover() {
  Controller so = cp5.getController("stepover");
  so.setValue(so.getValue() + 1.0);
  stepOver = int(so.getValue());
}

void minusStepover() {
  Controller so = cp5.getController("stepover");
  so.setValue(so.getValue() - 1.0);
  stepOver = int(so.getValue());
}

void plusStepdown() {
  Controller sd = cp5.getController("stepdown");
  sd.setValue(sd.getValue() + 0.5);
  stepDown = int(sd.getValue());
}

void minusStepdown() {
  Controller sd = cp5.getController("stepdown");
  sd.setValue(sd.getValue() - 0.5);
  stepDown = int(sd.getValue());
}

void plusTubediameter() {
  Controller td = cp5.getController("tubediameter");
  td.setValue(td.getValue() + 1.0);
  tubeDiameter = int(td.getValue());
}

void minusTubediameter() {
  Controller td = cp5.getController("tubediameter");
  td.setValue(td.getValue() - 1.0);
  tubeDiameter = int(td.getValue());
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
    writeInt(tubeDiameter);
    port.write(">");
  }
  
  //println(state);
  //println(scanLength);
  //println(tubeOffset);
  //println(motorOneSpeed);
  //println(motorTwoSpeed);
  //println(mode);
  //println(stepOver);
  //println(stepDown);

}

void writeInt(int x) {
  port.write(x >> 8);
  port.write(x & 255);
}
  
  
