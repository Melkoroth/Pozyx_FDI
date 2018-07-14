/*
Pozyx tests for max range
Juan L. Pérez Díez

*/

#include <Pozyx.h>
#include <Pozyx_definitions.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

const uint8_t neoPixelPin = 40;

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, neoPixelPin, NEO_GRB + NEO_KHZ800);

void setup() {
	Serial.begin(115200);
	pixel.begin();

	if(Pozyx.begin() == POZYX_FAILURE) {
		Serial.println(F("ERROR: Unable to connect to shield"));
		//Red led
		pixel.setPixelColor(0, pixel.Color(255,0,255));
    	pixel.show();
	}
	//Green led
	pixel.setPixelColor(0, pixel.Color(0,255,0));
    pixel.show();
}

void loop() {
	
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    pixel.setPixelColor(0, Wheel((i+j) & 255));
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