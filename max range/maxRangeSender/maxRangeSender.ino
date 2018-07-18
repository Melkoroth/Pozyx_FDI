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
//0x0 broadcasts to all devices
const uint16_t DESTINATIONID = 0x00;
//Data to send
const String TXSTRING = "ping";
const uint8_t TXBUFFERSIZE = TXSTRING.length() + 1;


void setup() {
	Serial.begin(115200);
	Serial.println();
	Serial.println(F(".::[ Pozyx max range experiment - Juan L. Pérez Díez ]::."));
  Serial.println();

  //Init LED
  pixel.begin();

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
  //Turn off LED
  pixel.setBrightness(0);
  pixel.show();
  pixel.setBrightness(150);
  delay(125);

  //Create send buffer
  uint8_t txBuffer[TXBUFFERSIZE];
  //Fill with data
  TXSTRING.getBytes(txBuffer, TXBUFFERSIZE);
  //Write to Pozyx buffer & send data
  if ((Pozyx.writeTXBufferData(txBuffer, TXBUFFERSIZE)  == POZYX_SUCCESS) && 
      Pozyx.sendTXBufferData(DESTINATIONID) == POZYX_SUCCESS) {
    pixel.setPixelColor(0, pixel.Color(0, 255, 0));

  } else {
    pixel.setPixelColor(0, pixel.Color(255, 150, 0));
  }
  
  pixel.show();
  delay(125);
}