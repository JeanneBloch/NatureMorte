#include <Adafruit_NeoPixel.h>
#include <ADCTouch.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            2

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      7

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGBW + NEO_KHZ800);

int delayval = 20; 

#define NUMADCPINS 6

#define DEBUGPIN 8

int refs[NUMADCPINS];

#define MAXBRIGHTNESS 255
//int brightnessValues[MAXBRIGHTNESS+1] = { 0, 1, 1, 2, 2, 3, 4, 6, 8, 10, 10 };

int touch_read(byte ADCChannel, int samples)
{
  long _value = 0;
  for(int _counter = 0; _counter < samples; _counter ++)
  {
    pinMode(ADCChannel, INPUT_PULLUP);
    
    ADMUX |=   0b11111;
    ADCSRA |= (1<<ADSC); //start conversion
    while(!(ADCSRA & (1<<ADIF))); //wait for conversion to finish
    ADCSRA |= (1<<ADIF); //reset the flag
    
    pinMode(ADCChannel, INPUT);
    _value += analogRead(ADCChannel);
  }
  return _value / samples;
}


void setup() {
  pixels.begin(); // This initializes the NeoPixel library.

  Serial.begin(115200);

  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(3, 0, 0, 0));
  pixels.show();

  pinMode(DEBUGPIN, INPUT_PULLUP);

  delay(1000);
  for(int i=0; i < NUMADCPINS; i++) {
    refs[i] = touch_read(A0 + i, 500);
  }
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

#define MAXCOLOR 20

#define SPEED 10.0

float currentBrightness[NUMADCPINS] = { 0, 0, 0, 0, 0, 0 };

void loop() {
  pixels.clear();
  //pixels.setPixelColor(0, pixels.Color(0, 0, 1, 0));
  int raws[NUMADCPINS];
  for(int i=0; i < NUMADCPINS; i++) {
    raws[i] = touch_read(A0 + i, 50);
  }
  for(int i=0; i < NUMADCPINS; i++) {
     int raw = raws[i];
     int value = raw - refs[i];
     Serial.print(value);
     Serial.print(" ");
     float newBrightness = constrain(map(value, 20, 50, 0, MAXBRIGHTNESS), 0, MAXBRIGHTNESS);
     float brightValue = (newBrightness + SPEED * currentBrightness[i]) / (SPEED + 1);
     currentBrightness[i] = brightValue;
     Serial.print(brightValue - 100.0);
     Serial.print(" ");

     int redValue = 0, blueValue = 0, greenValue = 0;

     if(digitalRead(DEBUGPIN) == LOW) {
       redValue = constrain(map(value, 0, 20, 0, MAXCOLOR), 0, MAXCOLOR);
       blueValue = constrain(map(value, 0, -20, 0, MAXCOLOR), 0, MAXCOLOR);
     }

     if(value < -20) {
        refs[i] = (refs[i] + raw) / 2;
        greenValue = 255;
     }

     pixels.setPixelColor(i + 1, pixels.Color(greenValue, redValue, blueValue, brightValue));
  }
  pixels.show();
  Serial.println();
  //delay(100);
}
