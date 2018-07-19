/*
Pozyx tests for max range
Juan L. Pérez Díez
*/
#include <Pozyx.h>
#include <Pozyx_definitions.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

//NeoPixel
const uint8_t neoPixelPin = 40;
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, neoPixelPin, NEO_GRB + NEO_KHZ800);

//Pozyx ID's
uint16_t myID = 0; 

void setup() {
	Serial.begin(115200);
	Serial.println();
	Serial.println(F(".::[ Pozyx max range experiment - Juan L. Pérez Díez ]::."));
  Serial.println();

  //Init LED
  pixel.begin();
  pixel.setBrightness(10);

  //Init pozyx
  if(Pozyx.begin() == POZYX_FAILURE) {
    //Purple led
    pixel.setPixelColor(0, pixel.Color(255,0,255));
    pixel.show();
    while(1);

  } else {
    //Set power to MAX
    Pozyx.setTxPower(33.0);
    //Lime led
    pixel.setPixelColor(0, pixel.Color(150,255,0));
    pixel.show();
  }
}

void loop() {
}

//Rainbox effect for NeoPixel
void rainbow(uint8_t wait) {
  uint16_t j;

  for(j=0; j<256; j++) {
    pixel.setPixelColor(0, Wheel((j) & 255));
    pixel.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}