#include <Wire.h>
#include <EEPROM.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_NeoPixel.h>
#include "printf.h"

#ifdef ARDUINO_AVR_UNO
#define HAS_MPR121 0
#else
#define HAS_MPR121 1
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIXELS1_PIN            23
#define PIXELS2_PIN            25

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      8

#define NUMLIGHTS      2

#define NUMELECTRODES 12

Adafruit_NeoPixel pixels1 = Adafruit_NeoPixel(NUMPIXELS, PIXELS1_PIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel pixels2 = Adafruit_NeoPixel(NUMPIXELS, PIXELS2_PIN, NEO_GRBW + NEO_KHZ800);

const byte neopix_gamma[] = {
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

#define VERSION 0x00000001

struct {
  uint32_t version;
  uint32_t targetColors[NUMLIGHTS][NUMELECTRODES];
} config = {
  VERSION,
  {
    {
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
    },
    {
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
    }
  }
};

bool configDirty = false;
uint32_t lastConfigSave = 0;

uint32_t saturation[NUMELECTRODES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void loadConfig() {
  uint32_t version;
  EEPROM.get(0, version);
  if (version == VERSION) {
    Serial.println(F("Found config in EEPROM, loading it"));
    EEPROM.get(0, config);
  }
}

void saveConfig() {
  Serial.println(F("INFO Saving config"));
  EEPROM.put(0, config);
  configDirty = false;
  lastConfigSave = millis();
}

void setup() {
  Serial.begin(9600);

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }

  printf_begin();

  Serial.println(F("Starting up..."));

  loadConfig();

#if HAS_MPR121
  Serial.println(F("Looking for MPR121"));

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println(F("MPR121 not found, check wiring?"));
    while (1);
  }
  Serial.println(F("MPR121 found!"));
#endif

  //radio_init();

  pixels1.begin();
  pixels2.begin();

  Serial.println(F("Ready"));
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

  for (uint8_t i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, neopix_gamma[sumr], neopix_gamma[sumg], neopix_gamma[sumb], neopix_gamma[sumw]);
  }

  pixels.show();
}

#define CMDBUFFER_SIZE 256
#define CMDMAXWORDS 16
char cmdbuffer[CMDBUFFER_SIZE];
int cmdbuffer_index = 0;
char *cmdwords[CMDMAXWORDS];
int cmdwords_count = 0;

void processSerialInput() {
  // if there's any serial available, read it:
  while (Serial.available() > 0) {
    if(cmdbuffer_index >= CMDBUFFER_SIZE) {
      Serial.println(F("ERR Command buffer overflow"));
      cmdbuffer_index = 0;
    }
    char c = Serial.read();
    switch (c) {
      case '\r':
        break;
      case '\n':
        if (cmdbuffer_index == 0) {
          break;
        }
        cmdbuffer[cmdbuffer_index] = '\0';
        cmdwords[0] = cmdbuffer;
        cmdwords_count = 1;
        for(int i = 0; i < cmdbuffer_index && cmdwords_count < CMDMAXWORDS; i++) {
          switch(cmdbuffer[i]) {
            case ' ':
              cmdbuffer[i] = '\0';
              cmdwords[cmdwords_count++] = cmdbuffer + i + 1;
            default:
              break;
          }
        }

        processCommand();
        cmdbuffer_index = 0;
        break;
      default:
        cmdbuffer[cmdbuffer_index++] = c;
        break;
    }
  }
}

void processCommand() {
  Serial.print(F("ECHO"));
  for(int i=0; i < cmdwords_count; i++) {
    Serial.print(F(" "));
    Serial.print(cmdwords[i]);
  }
  Serial.println(F(""));

  switch(*cmdwords[0]) {
    case 'Q': {
      Serial.print(F("OK "));
      Serial.print(NUMLIGHTS);
      Serial.print(F(" "));
      Serial.println(NUMELECTRODES);
      for(int numLight = 0; numLight < NUMLIGHTS; numLight++) {
        for(int numElectrode = 0; numElectrode < NUMELECTRODES; numElectrode++) {
          Serial.print(numLight);
          Serial.print(F(" "));

          Serial.print(numElectrode);
          Serial.print(F(" "));

          uint32_t targetColor = config.targetColors[numLight][numElectrode];

          uint32_t w = (uint8_t)(targetColor >> 24),
                   r = (uint8_t)(targetColor >> 16),
                   g = (uint8_t)(targetColor >>  8),
                   b = (uint8_t)targetColor;

          Serial.print(r);
          Serial.print(F(" "));
          Serial.print(g);
          Serial.print(F(" "));
          Serial.print(b);
          Serial.print(F(" "));
          Serial.println(w);
        }
      }
      break;
    }

    case 'C': {
      if(cmdwords_count != 7) {
        Serial.println(F("ERR usage: C light electrode r g b w"));
        break;
      }

      int numLight = atoi(cmdwords[1]);
      int numElectrode = atoi(cmdwords[2]);
      int red = atoi(cmdwords[3]);
      int green = atoi(cmdwords[4]);
      int blue = atoi(cmdwords[5]);
      int white = atoi(cmdwords[6]);

      if(numLight < 0 || numLight >= NUMLIGHTS) {
        Serial.print(F("ERR invalid light "));
        Serial.println(numLight);
        break;
      }
      if(numElectrode < 0 || numElectrode >= NUMELECTRODES) {
        Serial.print(F("ERR invalid electrode "));
        Serial.println(numElectrode);
        break;
      }
      Serial.print(F("OK Setting light "));
      Serial.print(numLight);
      Serial.print(F(" for electrode "));
      Serial.print(numElectrode);
      Serial.print(F(" to ("));
      Serial.print(red);
      Serial.print(F(","));
      Serial.print(green);
      Serial.print(F(","));
      Serial.print(blue);
      Serial.print(F(","));
      Serial.print(white);
      Serial.println(F(")"));
      config.targetColors[numLight][numElectrode] = Adafruit_NeoPixel::Color(red, green, blue, white);
      configDirty = true;
      break;
    }
    default:
      Serial.print("ERR Unknown command ");
      Serial.println(cmdwords[0]);
      break;
  }
}

void loop() {
#if HAS_MPR121
  // Get the currently touched pads
  currtouched = cap.touched();

  for (uint8_t i=0; i<NUMELECTRODES; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      //Serial.print(i); Serial.println(" touched");
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      //Serial.print(i); Serial.println(" released");
    }
  }

  for (uint8_t electrodeIndex=0; electrodeIndex<NUMELECTRODES-1; electrodeIndex++) {
    change(electrodeIndex, currtouched & _BV(electrodeIndex));

    //Serial.print(saturation[electrodeIndex]);
    //Serial.print("\t");
  }

//  change(NUMELECTRODES - 1, radio_read());
  //Serial.println();
#endif

  show(pixels1, config.targetColors[0]);
  show(pixels2, config.targetColors[1]);

  processSerialInput();

  if (configDirty && (millis() - lastConfigSave) > 1000) {
    saveConfig();
  }


  // reset our state
  lasttouched = currtouched;

  // comment out this line for detailed data from the sensor!
#if 0
  // debugging info, what
  Serial.print(F("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x")); Serial.println(cap.touched(), HEX);
  Serial.print(F("Filt: "));
  for (uint8_t i=0; i<NUMELECTRODES; i++) {
    Serial.print(cap.filteredData(i)); Serial.print(F("\t"));
  }
  Serial.println();
  Serial.print(F("Base: "));
  for (uint8_t i=0; i<NUMELECTRODES; i++) {
    Serial.print(cap.baselineData(i)); Serial.print(F("\t"));
  }
  Serial.println();

  // put a delay so it isn't overwhelming
  delay(100);
#endif
}
