#include <SPI.h>
#include "RF24.h"
#include "printf.h"

RF24 radio(7, 8);


byte addresses[][6] = {"1Node", "2Node"};

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
  
  unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
  int sensorValue = analogRead(A0);
  if (!radio.write( &sensorValue, sizeof(int) )) {
    Serial.println(F("failed"));
  }
  

  //Serial.println(F("sent time"));

  radio.txStandBy();
#if DEBUG
  Serial.print(sensorValue);
  Serial.print(" ");
  Serial.println(micros() - start_time);
#endif
  delay(50);
}

