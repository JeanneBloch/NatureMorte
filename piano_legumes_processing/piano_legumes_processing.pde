import themidibus.*; //Import the library
import processing.serial.*;

MidiBus myBus; // The MidiBus

Serial myPort;  // Create object from Serial class

void setup() {
  size(400, 400);
  background(0);

  MidiBus.list(); // List all available Midi devices on STDOUT. This will show each device's index and name.

  // Either you can
  //                   Parent In Out
  //                     |    |  |
  //myBus = new MidiBus(this, 0, 1); // Create a new MidiBus using the device index to select the Midi input and output devices respectively.

  // or you can ...
  //                   Parent         In                   Out
  //                     |            |                     |
  //myBus = new MidiBus(this, "IncomingDeviceName", "OutgoingDeviceName"); // Create a new MidiBus using the device names to select the Midi input and output devices respectively.

  // or for testing you could ...
  //                 Parent  In        Out
  //                   |     |          |
  myBus = new MidiBus(this, -1, "Bus 1"); // Create a new MidiBus with no input device and the default Java Sound Synthesizer as the output device.

  String portName = "/dev/cu.usbmodem1421";
  myPort = new Serial(this, portName, 115200);
}

int KEY_COUNT = 6;

boolean[] key_status = new boolean[KEY_COUNT];
int[] key_values = new int[KEY_COUNT];

int channel = 0;
int pitches[] = {64, 66, 68, 69, 71, 73};
int velocity = 127;

void setKeyStatus(int key, boolean status) {
  if(status != key_status[key]) {
    key_status[key] = status;
    if(status) {
      myBus.sendNoteOn(channel, pitches[key], velocity); // Send a Midi noteOn
    } else {
      myBus.sendNoteOff(channel, pitches[key], velocity); // Send a Midi nodeOff
    }
  }
}

void readSerial() {
  if(myPort.available() <= 0)
    return;

  String myString = myPort.readStringUntil(10);
  if (myString == null)
    return;

  String[] values = splitTokens(myString);
  if(values.length != KEY_COUNT)
    return;
  
  for(int i = 0; i < KEY_COUNT; i++) {
    try {
      int level = Integer.parseInt(values[i]);
      boolean new_status = level >= 50 ? true : false;
      setKeyStatus(i, new_status);
    } catch(NumberFormatException e) {
    }
  }
}

void draw() {
 
  readSerial();

}

void delay(int time) {
  int current = millis();
  while (millis () < current+time) Thread.yield();
}