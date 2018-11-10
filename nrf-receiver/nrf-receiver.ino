#include <SPI.h>
#include "RF24.h"

#define LED 2

#if ARDUINO_AVR_MEGA2560
RF24 radio(49, 48);
#else
RF24 radio(7, 8);
#endif

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

void setup() {
  Serial.begin(9600);
  printf_begin();

  radio_init();

  pinMode(LED, OUTPUT);
}

void loop() {

#define NSAMPLES 8

  static long last_micros = micros();
  static long samples[NSAMPLES] = {0};
  static int sample_idx = 0;
  static long sample_total = 0;
  static int led = 0;
  
  if ( radio.available()) {
    int got_sensor;

    int batch_size = 0;

    while (radio.available()) {                                   // While there is data ready
      batch_size++;
      radio.read( &got_sensor, sizeof(got_sensor) );             // Get the payload
    }

    digitalWrite(LED, (millis() & 128) ? HIGH : LOW);

    analogWrite(3, got_sensor / 4);

    long now = micros();
    long time_since_last = now - last_micros;
    last_micros = now;

    sample_total = sample_total - samples[sample_idx] + got_sensor;
    samples[sample_idx] = got_sensor;
    sample_idx = (sample_idx + 1) & (NSAMPLES - 1);

    Serial.print(1024);
    Serial.print(" ");

    Serial.print(got_sensor);
    Serial.print(" ");

    /*Serial.print(time_since_last);
    Serial.print(" ");*/
    /*
    Serial.print(batch_size * 100);
    Serial.print(" ");*/

    int sample_min = 1024, sample_max = 0;
    for(int i=0; i<NSAMPLES; i++) {
      int s = samples[i];
      if(s < sample_min) sample_min = s;
      if(s > sample_max) sample_max = s;
    }

    Serial.print(10*(sample_max - sample_min) + 100);
    Serial.print(" ");

    Serial.println(sample_total / NSAMPLES + 200);
  }
}

