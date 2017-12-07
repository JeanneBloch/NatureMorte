#include <SPI.h>
#include "RF24.h"
#include "printf.h"

#if ARDUINO_AVR_MEGA2560
RF24 radio(49, 48);
#else
RF24 radio(7, 8);
#endif

#define LED 2

byte addresses[][6] = {"1Node", "2Node"};

void setup() {
  Serial.begin(115200);
  printf_begin();

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_MIN);
  radio.setAutoAck(false);
  radio.setChannel(125);
  radio.printDetails();

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  // Start the radio listening for data
  radio.startListening();

  Serial.println(F("Now receiving"));

  pinMode(LED, OUTPUT);
}

void loop() {

#define NSAMPLES 256

  static long last_micros = micros();
  static long samples[NSAMPLES] = {0};
  static int sample_idx = 0;
  static long sample_total = 0;
  static int led = 0;
  static unsigned long last_got_time = 0;
  
  if ( radio.available()) {
    unsigned long got_time;

    int batch_size = 0;

    while (radio.available()) {                                   // While there is data ready
      batch_size++;
      radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
    }

    digitalWrite(LED, (millis() & 128) ? HIGH : LOW);

    long now = micros();
    long time_since_last = now - last_micros;
    last_micros = now;

    sample_total = sample_total - samples[sample_idx] + time_since_last;
    samples[sample_idx] = time_since_last;
    sample_idx = (sample_idx + 1) & (NSAMPLES - 1);

    Serial.print(10000);
    Serial.print(" ");

    Serial.print(got_time - last_got_time);
    Serial.print(" ");
    last_got_time = got_time;

    Serial.print(time_since_last);
    Serial.print(" ");
    
    Serial.print(batch_size * 100);
    Serial.print(" ");

    Serial.println(sample_total / NSAMPLES);
  }
}

