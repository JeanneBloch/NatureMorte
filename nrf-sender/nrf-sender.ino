#include <SPI.h>
#include "RF24.h"
#include "printf.h"

#define DEBUG 1

RF24 radio(7, 8);

byte addresses[][6] = {"1Node", "2Node"};

// from https://github.com/martin2250/ADCTouch/blob/master/src/ADCTouch.cpp

int touch_read(byte ADCChannel, int samples)
{
  long _value = 0;
  for(int _counter = 0; _counter < samples; _counter ++)
  {
    // set the analog pin as an input pin with a pullup resistor
    // this will start charging the capacitive element attached to that pin
    pinMode(ADCChannel, INPUT_PULLUP);

    // connect the ADC input and the internal sample and hold capacitor to ground to discharge it
    ADMUX |=   0b11111;

    // start the conversion
    ADCSRA |= (1 << ADSC);

    // ADSC is cleared when the conversion finishes
    while((ADCSRA & (1 << ADSC)));

    // ADCSRA |= (1<<ADIF); //reset the flag

    pinMode(ADCChannel, INPUT);

    _value += analogRead(ADCChannel);
  }
  return _value / samples;
}

void setup() {
  Serial.begin(9600);
  printf_begin();

  radio.begin();

  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(false);
  radio.setChannel(125);
  radio.printDetails();

  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);

  Serial.println(F("Now sending"));
}

void loop() {
  digitalWrite(LED_BUILTIN, (millis() & 128) ? HIGH : LOW);
  
  unsigned long start_time = micros();
  int sensorValue = touch_read(A0, 64);

  if (!radio.write( &sensorValue, sizeof(int) )) {
    Serial.println(F("failed"));
  }
  

  //Serial.println(F("sent time"));

  radio.txStandBy();
#if DEBUG
  Serial.print(sensorValue);
//  Serial.print(" ");
//  Serial.print(micros() - start_time);
  Serial.println("");
#endif
  delay(50);
}

