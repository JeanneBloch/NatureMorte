import processing.serial.*;
import controlP5.*;

ControlP5 cp5;

ArrayList<ArrayList<ColorPicker>> cps = new ArrayList<ArrayList<ColorPicker>>();
Serial arduinoPort;

void setup() {
  size(620, 980);
  noStroke();
  cp5 = new ControlP5(this);
  cp5.setAutoDraw(false);
  for(int electrode=0;electrode<12;electrode++) {
    float y = 20 + 80 * electrode;
    cp5.addTextlabel("label" + electrode)
                      .setText(electrode + "")
                      .setPosition(10,y+15)
                      .setColorValue(0xffffff00)
                      .setFont(createFont("Monaco",20))
                      ;

    
    ArrayList<ColorPicker> row = new ArrayList<ColorPicker>();
    cps.add(row);
    for(int light=0;light<2;light++) {
      ColorPicker cp = cp5.addColorPicker("picker" + electrode + "." + light)
              .setPosition(50 + 300 * light, y)
              .setColorValue(color(255, 128, 0, 128))
              ;
      row.add(cp);
    }
  }

  String[] serialPorts = Serial.list();
  String arduinoPortName = null;
  for(int i=0; i<serialPorts.length; i++) {
    if (serialPorts[i].startsWith("/dev/tty.usbmodem")) {
      arduinoPortName = serialPorts[i];
    }
  }
  
  if (arduinoPortName == null) {
    throw new RuntimeException("Arduino serial port not found!");
  }
  
  println("Arduino found on " + arduinoPortName);
  
  arduinoPort = new Serial(this, arduinoPortName, 9600);
}

void draw() {
  background(30, 30, 90);
  
  cp5.draw();

  for(int electrode=0;electrode<cps.size();electrode++) {
    ArrayList<ColorPicker> row = cps.get(electrode);
    for(int light=0; light<row.size(); light++) {
      ColorPicker cp = row.get(light);
      float[] pos = cp.getPosition();
      stroke(255); noFill();
      rect(pos[0] - 2, pos[1] - 2, 258, 62); 
      noStroke();
      fill(cp.getColorValue() | 0xff000000);
      rect(pos[0], pos[1] + 44, 130, 15);
      float b = cp.getArrayValue(3);
      fill(b, b, b);
      rect(pos[0]+130, pos[1] + 44, 125, 15);
    }
  }
}

public void controlEvent(ControlEvent c) {
  if (!state.equals("InitDone")) return;
  
  for(int electrode=0;electrode<cps.size();electrode++) {
    ArrayList<ColorPicker> row = cps.get(electrode);
    for(int light=0; light<row.size(); light++) {
      ColorPicker cp = row.get(light);
      if(c.isFrom(cp)) {
        int r = int(c.getArrayValue(0));
        int g = int(c.getArrayValue(1));
        int b = int(c.getArrayValue(2));
        int w = int(c.getArrayValue(3));
        String cmd = "C " + light + " " + electrode + " " + r + " " + g + " " + b + " " + w;
        sendLineToArduino(cmd);
        state = "Csent";
      }
    }
  }
}

String serialIn = "";

void serialEvent(Serial port) {
  // read a byte from the serial port:
  int inByte = port.read();
  switch(inByte) {
    case '\r':
      break;
    case '\n':
      println("> " + serialIn);
      handleLineFromArduino(serialIn);
      serialIn = "";
      break;
    default:
      serialIn += (char)inByte;
      break;
  }
}

String state = "Init";

void handleLineFromArduino(String lineIn) {
  if (state.equals("Init") || state.equals("InitDone")) {
    if (lineIn.equals("Ready")) {
      sendLineToArduino("Q");
      state = "Qsent";
      return;
    }
  } else if (state.equals("Qsent")) {
    if (lineIn.startsWith("OK") || lineIn.startsWith("ECHO")) {
    } else {
      try {
        String[] words = lineIn.split(" ");
        int light = Integer.parseInt(words[0]);
        int electrode = Integer.parseInt(words[1]);
        float values[] = new float[4];
        values[0] = Integer.parseInt(words[2]);
        values[1] = Integer.parseInt(words[3]);
        values[2] = Integer.parseInt(words[4]);
        values[3] = Integer.parseInt(words[5]);
        if (light >=0 && light < 2
            && electrode >= 0 && electrode < 12) {
          cps.get(electrode).get(light).setArrayValue(values);
        }
        if (electrode == 11 && light == 1) {
          state = "InitDone";
        }
      } catch (Exception e) {
        println(e);
        state = "Init";
      }
    }
  } else if (state.equals("Csent")) {
    if(lineIn.startsWith("OK")) {
      state = "InitDone";
    }
  }
}

void sendLineToArduino(String lineOut) {
  println("< " + lineOut);
  arduinoPort.write(lineOut + "\r\n");
}