// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
#include <Adafruit_NeoPixel.h>
#include <ADCTouch.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      7

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGBW + NEO_KHZ800);

int delayval = 20; 

int ref0;

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  ref0 = ADCTouch.read(A0, 500);    //create reference value
  delay(1000);
  ref0 = ADCTouch.read(A0, 500);    //create reference value
}

int brightness = 0;
int direction = 0;

void setBrightness(int brightness) {
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(brightness, brightness, brightness, brightness)); 
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void loop() {
  brightness = constrain(brightness + direction, 0, 30);
  setBrightness(brightness);

  int value0 = ADCTouch.read(A0, 500);
  value0 -= ref0;       //remove offset
  
  if(value0 >= 20) {
    direction = 1;
  } else {
    direction = -2;
  }
  delay(delayval); // Delay for a period of time (in milliseconds).*/
}
