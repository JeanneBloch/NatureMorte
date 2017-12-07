#include <SPI.h>
#include "RF24.h"
#include "printf.h"

RF24 radio(7,8);


byte addresses[][6] = {"1Node", "2Node"};

void setup() {
  Serial.begin(115200);
  printf_begin();
  
  radio.begin();

  radio.setPALevel(RF24_PA_MIN);
  radio.setAutoAck(false);
  radio.setChannel(125);
  radio.printDetails();

  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  
  Serial.println(F("Now sending"));
}

void loop() {
    unsigned long start_time = micros();                             // Take the time, and send it.  This will block until complete
     if (!radio.write( &start_time, sizeof(unsigned long) )){
       Serial.println(F("failed"));
     }
        
    //Serial.println(F("sent time"));

    radio.txStandBy();

   // delay(1);
}

