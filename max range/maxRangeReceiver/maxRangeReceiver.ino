/*
Pozyx tests for max range
Juan L. Pérez Díez
*/
#include <Pozyx.h>
#include <Pozyx_definitions.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>
#include <Adafruit_NeoPixel.h>

//SPI Flash 2MB memory
#define FLASH_TYPE	SPIFLASHTYPE_W25Q16BV
#define FLASH_SS       SS1                    // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1                   // What SPI port is Flash on?
Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);     // Use hardware SPI
Adafruit_W25Q16BV_FatFs fatfs(flash);

//NeoPixel
const uint8_t neoPixelPin = 40;
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, neoPixelPin, NEO_GRB + NEO_KHZ800);

//Pozyx ID's
uint16_t myID = 0; 
const uint16_t remoteID = 0x6717;

String inputString = "";
boolean stringComplete = false;

void setup() {
	boolean failInit = false;

	Serial.begin(115200);
	//Waits for usb connection before continuing
	while (!Serial);
	Serial.println();
	Serial.println(F(".::[ Pozyx max range experiment - Juan L. Pérez Díez ]::."));

	// Initialize flash library and check its chip ID.
	if (!flash.begin(FLASH_TYPE)) {
		Serial.println(F("ERROR:, failed to initialize flash chip!"));
		failInit = true;
	} else {
		Serial.print(F("Flash chip JEDEC ID: 0x")); 
		Serial.println(flash.GetJEDECID(), HEX);
	}

	//Init filesystem
	if (!fatfs.begin()) {
		Serial.println(F("ERROR: failed to mount newly formatted filesystem!"));
		Serial.println(F("Was the flash chip formatted with the fatfs_format example?"));
		failInit = true;
	} else {
	  Serial.println(F("Mounted filesystem!"));
  }

  initPozyx(&failInit);

  //Init LED
  pixel.begin();
  //Reserve memory for input string
  inputString.reserve(100);

	if (failInit) {
		//Purple led
		pixel.setPixelColor(0, pixel.Color(255,0,255));
    	pixel.show();
		while(1);
	} else {
		//Green led
		pixel.setPixelColor(0, pixel.Color(0,255,0));
    	pixel.show();
    	Serial.println("Setup went OK");
    }
}

//Init pozyx and display it's settings
void initPozyx(bool failInit) {
  //Init pozyx
  if(Pozyx.begin() == POZYX_FAILURE) {
    Serial.println(F("ERROR: Unable to connect to pozyx shield"));
    failInit = true;
  } else {
    Pozyx.setTxPower(33.0);
    //Read the network id of this device
    Pozyx.regRead(POZYX_NETWORK_ID, (uint8_t*)&myID, 2);
    Serial.print("Pozyx ID: ");
    Serial.print(myID, HEX);
    //Read and print configuration
    UWB_settings_t settings;
    if (Pozyx.getUWBSettings(&settings) == POZYX_SUCCESS) {
      //Channel
      Serial.print(" | channel: ");
      Serial.print(settings.channel);
      //Gain
      Serial.print(" | ");
      Serial.print(settings.gain_db);
      Serial.print("dB");
      //Bitrate
      Serial.print(" | ");
      switch(settings.bitrate) {
        case 0: 
          Serial.print("110kbits/s");
          break;
        case 1:
          Serial.print("850kbits/s");
          break;
        case 2:
          Serial.print("6.8Mbits/s");
          break;
      }
      //Pulse repetition frequency
      Serial.print(" | ");
      switch (settings.prf) {
          case 1:
            Serial.print("16MHz");
            break;
          case 2:
            Serial.print("64MHz");
            break;
      }
      //Preamble length
      Serial.print(" | ");
      switch (settings.plen) {
          case 0x0C:
            Serial.print("4096 symbols.");
            break;
          case 0x28:
            Serial.print("1024 symbols");
            break;
          case 0x18: 
            Serial.print("1536 symbols.");
            break;
          case 0x08: 
            Serial.print("1024 symbols.");
            break;
          case 0x34: 
            Serial.print("512 symbols.");
            break;
          case 0x24: 
            Serial.print("256 symbols.");
            break;
          case 0x14: 
            Serial.print("128 symbols.");
            break;
          case 0x04: 
            Serial.print("64 symbols.");
            break;
      }
      Serial.println();
    }
  }
}

void loop() {
  //readUWBdata();
  calcRange();
	//rainbow(10);
	//Serial.print("Free RAM: ");
	//Serial.print(freeRam());
	//Serial.println(" Bytes");
  delay(500);
}

//Reads incoming UWB data - 300ms timeout
void readUWBdata() {
  // we wait up to 300ms to see if we have received an incoming message (if so we receive an RX_DATA interrupt)
  if(Pozyx.waitForFlag(POZYX_INT_STATUS_RX_DATA, 500)) {
    // we have received a message!
    uint8_t length = 0;
    uint16_t messenger = 0x00;
    delay(1);
    // Let's read out some information about the message (i.e., how many bytes did we receive and who sent the message)
    Pozyx.getLastDataLength(&length);
    Pozyx.getLastNetworkId(&messenger);

    char data[length];

    // read the contents of the receive (RX) buffer, this is the message that was sent to this device
    Pozyx.readRXBufferData((uint8_t *) data, length);
    Serial.print("Ox");
    Serial.print(messenger, HEX);
    //Serial.print(" - ");
    //Serial.print(length);
    //Serial.print("B:");
    Serial.print(": ");
    Serial.println(data);
  } else {
    Serial.println("Timeout waiting for data");
  }
}

//Needs remoteID
void calcRange() {
  device_range_t rangeInfo;
  if (Pozyx.doRanging(remoteID, &rangeInfo) == POZYX_SUCCESS) {
    Serial.print(rangeInfo.timestamp);
    Serial.print("ms | ");
    Serial.print(rangeInfo.distance);
    Serial.print("mm | ");
    Serial.print(rangeInfo.RSS);
    Serial.println("dBm");
  } else {
    Serial.println("Ranging failed");
  }
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


//Check memory available - MAX 32K
extern "C" char *sbrk(int i);
 
int freeRam() {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}