// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      7

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGBW + NEO_KHZ800);

int delayval = 20; 

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
}

int brightness = 0;
int direction = 1;

void setBrightness(int brightness) {
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(brightness, brightness, brightness)); 
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void loop() {
/*  setBrightness(brightness);
  brightness = brightness + direction;
  if(brightness >= 255) {
    direction = -1;
    delay(300);
  } else if(brightness <= 0) {
    direction = 1;
  }
  delay(delayval); // Delay for a period of time (in milliseconds).
  */

  setBrightness(255);
}
