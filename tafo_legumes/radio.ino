#include "RF24.h"

#if ARDUINO_AVR_MEGA2560
RF24 radio(49, 48);
#else
RF24 radio(7, 8);
#endif

static int sensor_ref = 0;

void radio_init() {
  const byte addresses[][6] = {"1Node", "2Node"};

  radio.begin();

  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);
  radio.setChannel(125);
  radio.printDetails();

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  // Start the radio listening for data
  radio.startListening();

  Serial.println(F("Now receiving"));

}

static int last_sensor_value = 0;

bool radio_read() {
  if ( radio.available()) {
    while (radio.available()) {                                   // While there is data ready
      radio.read( &last_sensor_value, sizeof(last_sensor_value) );             // Get the payload
    }
    if (sensor_ref == 0) {
      sensor_ref = last_sensor_value;
      Serial.print(F("Touch sensor reference is "));
      Serial.println(sensor_ref);
    } else {
      /*Serial.print(F("New touch sensor value "));
      Serial.println(last_sensor_value);*/
    }
  }
  
  return last_sensor_value >= (sensor_ref + 8);
}

