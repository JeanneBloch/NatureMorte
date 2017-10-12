/*********************************************************
This is a library for the MPR121 12-channel Capacitive touch sensor

Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
  ----> https://www.adafruit.com/products/

These sensors use I2C communicate, at least 2 pins are required 
to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
**********************************************************/

#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIXELS1_PIN            23
#define PIXELS2_PIN            22

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8

#define NUMELECTRODES 12

Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, PIXELS1_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, PIXELS2_PIN, NEO_GRBW + NEO_KHZ800);

byte neopix_gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

uint32_t targetColors1[NUMELECTRODES] = {
  Adafruit_NeoPixel::Color(255,255,255,255),
  Adafruit_NeoPixel::Color(0,0,0,0),
  Adafruit_NeoPixel::Color(255,255,0,0),
  Adafruit_NeoPixel::Color(0,0,0,0),
  
  Adafruit_NeoPixel::Color(255,255,0,0),
  Adafruit_NeoPixel::Color(255,0,0,0),
  Adafruit_NeoPixel::Color(255,255,255,255),
  Adafruit_NeoPixel::Color(0,0,255,0),
  
  Adafruit_NeoPixel::Color(255,255,0,0),
  Adafruit_NeoPixel::Color(255,0,0,0),
  Adafruit_NeoPixel::Color(0,0,0,0),
  Adafruit_NeoPixel::Color(0,0,255,0),
};

uint32_t targetColors2[NUMELECTRODES] = {
  Adafruit_NeoPixel::Color(0,0,0,0),
  Adafruit_NeoPixel::Color(255,255,255,255),
  Adafruit_NeoPixel::Color(0,0,0,0),
  Adafruit_NeoPixel::Color(255,255,0,0),
  
  Adafruit_NeoPixel::Color(255,255,0,0),
  Adafruit_NeoPixel::Color(255,0,0,0),
  Adafruit_NeoPixel::Color(255,255,255,255),
  Adafruit_NeoPixel::Color(255,255,255,255),
  
  Adafruit_NeoPixel::Color(255,0,0,0),
  Adafruit_NeoPixel::Color(0,0,0,0),
  Adafruit_NeoPixel::Color(255,0,0,0),
  Adafruit_NeoPixel::Color(0,0,0,0),
};

uint32_t saturation[NUMELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
 
void setup() {
  Serial.begin(115200);

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  //cap.setThresholds(12,0);

  pixels1.begin();
  pixels2.begin();
}

void change(int electrodeIndex, bool direction) {
    uint32_t currentSaturation = saturation[electrodeIndex];

    int32_t increment = int(direction) ? 2 : -2;

    int32_t newSaturation = currentSaturation + increment;
    if (newSaturation < 0) newSaturation = 0;
    if (newSaturation > 255) newSaturation = 255;

    saturation[electrodeIndex] = newSaturation;
}

void show(Adafruit_NeoPixel& pixels, uint32_t *targetColors) {
  uint32_t sumr = 0, sumg = 0, sumb = 0, sumw = 0;

  for (uint8_t electrodeIndex=0; electrodeIndex<NUMELECTRODES; electrodeIndex++) {
    uint32_t targetColor = targetColors[electrodeIndex];

    uint32_t w = (uint8_t)(targetColor >> 24),
             r = (uint8_t)(targetColor >> 16),
             g = (uint8_t)(targetColor >>  8),
             b = (uint8_t)targetColor;

    sumr += r * saturation[electrodeIndex];
    sumg += g * saturation[electrodeIndex];
    sumb += b * saturation[electrodeIndex];
    sumw += w * saturation[electrodeIndex];
  }

  sumr = min(255, sumr >> 8);
  sumg = min(255, sumg >> 8);
  sumb = min(255, sumb >> 8);
  sumw = min(255, sumw >> 8);

  Serial.print(sumr);
  Serial.print("\t");
  Serial.print(sumg);
  Serial.print("\t");
  Serial.print(sumb);
  Serial.print("\t");
  Serial.print(sumw);
  Serial.print("\t");

  for (uint8_t i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, neopix_gamma[sumr], neopix_gamma[sumg], neopix_gamma[sumb], neopix_gamma[sumw]);
  }

  pixels.show();
}

void loop() {
  // Get the currently touched pads
  currtouched = cap.touched();
  
  for (uint8_t i=0; i<NUMELECTRODES; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
    }
  }

  for (uint8_t electrodeIndex=0; electrodeIndex<NUMELECTRODES; electrodeIndex++) {
    change(electrodeIndex, currtouched & _BV(electrodeIndex));

    Serial.print(saturation[electrodeIndex]);
    Serial.print("\t");
  }

  show(pixels1, targetColors1);
  show(pixels2, targetColors2);

  Serial.println();

  // reset our state
  lasttouched = currtouched;

  // comment out this line for detailed data from the sensor!
  return;
  
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i=0; i<NUMELECTRODES; i++) {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<NUMELECTRODES; i++) {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
  
  // put a delay so it isn't overwhelming
  delay(100);
}
