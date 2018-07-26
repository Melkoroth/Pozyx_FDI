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
#define FLASH_TYPE     SPIFLASHTYPE_W25Q16BV
#define FLASH_SS       SS1                    // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1                   // What SPI port is Flash on?
Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT);     // Use hardware SPI
Adafruit_W25Q16BV_FatFs fatfs(flash);

//NeoPixel
const uint8_t neoPixelPin = 40;
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, neoPixelPin, NEO_GRB + NEO_KHZ800);

//Pozyx ID's
uint16_t myID = 0; 
//const uint16_t remoteID = 0x6717;
const uint16_t remoteID = 0x677D;

//Structure that holds distance info
device_range_t rangeInfo;
uint32_t maxDistance = 0;
uint32_t maxDistanceRemote = 0;

//File where maxRanges are stored
const String fileName = "maxRange.txt";
//Flag to signal that txt file needs updating
bool fileNeedsUpdate = false;
//Flag that controls reporting of sensor to serial
boolean realtimeReport = false;

/******************
-> SETUP
******************/
void setup() {
  boolean failInit = false;

  Serial.begin(115200);
  //Waits for usb connection before continuing
  //while (!Serial);
  Serial.println();
  Serial.println(F(".::[ Pozyx max range experiment - Juan L. Pérez Díez ]::."));
  Serial.println(F(".::[ Universidad Complutense de Madrid - Facultad de Informática ]::."));
  
  initFS(&failInit);
  initPozyx(&failInit);

  //Init LED
  pixel.begin();
  pixel.setBrightness(10);

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

//Inits file system and reads maxRange variables from fileName
void initFS(bool failInit) {
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
    Serial.println(F("Mounted filesystem."));
  }
  //Read variables from file
  readFileMaxRange();
}

//Opens fileName and reads maxDistance variables
//Recreates file if not found
void readFileMaxRange() {
  File readFile = fatfs.open(fileName, FILE_READ);
  if (readFile) {
    //Discard "maxDistance: "
    readFile.readStringUntil(' ');
    String line = readFile.readStringUntil('\n');
    maxDistance = line.toInt();

    //Discard "remoteMaxDistance: "
    readFile.readStringUntil(' ');
    line = readFile.readStringUntil('\n');
    maxDistanceRemote = line.toInt();

    readFile.close();
    //Serial.println("File read successfully. Contents:");
    //printEntireFile();
    //Serial.println("Data in memory:");
    printVarsInMem();

  //File not found
  } else {
    File createFile = fatfs.open(fileName, FILE_WRITE);
    createFile.println(F("maxDistance: 0"));
    createFile.println(F("remoteMaxDistance: 0"));
    createFile.close();
    Serial.println(F("WARNING: Couldn't locate file, recreated."));
    readFileMaxRange();
  }
}

/******************
-> LOOP
******************/
void loop() {
  calcRange();
  delay(100);
  calcRemoteRange();
  delay(100);
  serviceSerialEvents();
  //delay(10);
  writeFileRangeInfo();
}

/******************
-> POZYX FUNCS
******************/
//Needs remoteID
void calcRange() {
  int result = Pozyx.doRanging(remoteID, &rangeInfo);
  if (result == POZYX_SUCCESS) {
    testMax(false);
    printSerialRangeInfo(false);
  } else if (result == POZYX_TIMEOUT) {
    Serial.println("Timeout");
  } else {
    //Serial.println("Fail");
  }
}

//Uses remoteID & self ID
void calcRemoteRange() {
  int result = Pozyx.doRemoteRanging(remoteID, myID, &rangeInfo);
  if (result == POZYX_SUCCESS) {
    testMax(true);
    printSerialRangeInfo(true);
  } else if (result == POZYX_TIMEOUT) {
    Serial.println("Timeout");   
  } else {
    //Serial.println("Fail");
  }
}

//Tests max and stores in memory & file
void testMax(bool remote) {
  if (!remote) {
    if (rangeInfo.distance > maxDistance) {
      maxDistance = rangeInfo.distance;
      fileNeedsUpdate = true;
      Serial.print("New max: ");
      Serial.println(maxDistance);
    }
  } else {
    if (rangeInfo.distance > maxDistanceRemote) {
      maxDistanceRemote = rangeInfo.distance;
      fileNeedsUpdate = true;
      Serial.print("New remote max: ");
      Serial.println(maxDistanceRemote);
    }
  }
}

/******************
-> SERIAL & FILES
******************/
//Prints data from pozyx range
void printSerialRangeInfo(bool remote) {
  if (realtimeReport) {
    (remote) ? Serial.print("R: ") : 0;
    Serial.print(rangeInfo.timestamp);
    Serial.print("ms | ");
    Serial.print(rangeInfo.distance);
    Serial.print("mm | ");
    Serial.print(rangeInfo.RSS);
    Serial.println("dBm");
  }
}

//Reads incoming chars until NL or CR & activates flag
void serviceSerialEvents() {
  if (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    //Show help
    if (inChar == 'h') {
      printSerialHelp();
    //Toggle real-time serial reports
    } else if (inChar == 'r') {
      realtimeReport = !realtimeReport;
    //Show maxes
    } else if (inChar == 'm') {
      printVarsInMem();
    //Delete maxes
    } else if (inChar == 'd') {
      deleteMaxValues();
    //Print internal file
    } else if (inChar == 'p') {
      printEntireFile();
    }
    //Empty buffer
    while (Serial.available() > 0) {
      Serial.read();
    }
  }
}

//Prints command info to serial
void printSerialHelp() {
  Serial.println("Available commands are: ");
  Serial.println("h - show this help");
  Serial.println("r - toggle real-time reports");
  Serial.println("m - print max range stored values");
  Serial.println("d - delete stored maximum variables");
  Serial.println("p - print contents of internal file");
}

//Prints the max value variables
void printVarsInMem() {
  Serial.print("* maxDistance: ");
  Serial.println(maxDistance);
  Serial.print("* maxDistanceRemote: ");
  Serial.println(maxDistanceRemote);
}

//Resets max distance variables both in memory & in filesystem
void deleteMaxValues() {
  maxDistance = 0;
  maxDistanceRemote = 0;
  fatfs.remove(fileName);
  fatfs.open(fileName, FILE_WRITE);
  fileNeedsUpdate = true;
  writeFileRangeInfo();
}

//Writes contents of vars to FS if flag is true
void writeFileRangeInfo() {
  if (fileNeedsUpdate) {
    //Open to write from the beginning
    File writeFile = fatfs.open(fileName, FA_WRITE);
    if (writeFile) {
      writeFile.print(F("maxDistance: "));
      writeFile.println(maxDistance);
      writeFile.print(F("remoteMaxDistance: "));
      writeFile.println(maxDistanceRemote);
      writeFile.close();
    } else {
      Serial.println(F("ERROR attempting to write range info into FS"));
    }
    fileNeedsUpdate = false;
  }
}

//Prints contents of fileName to Serial port
void printEntireFile() {
  File readFile = fatfs.open(fileName, FILE_READ);
  if (readFile) {
    while (readFile.available()) {
      char c = readFile.read();
      Serial.print(c);
    }
    readFile.close();
  } else {
    Serial.println(F("ERROR: Can't find file to print."));
  }
}

/******************
-> AUX FUNCS
******************/
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