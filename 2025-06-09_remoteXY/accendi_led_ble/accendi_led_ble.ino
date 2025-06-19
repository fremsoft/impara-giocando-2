/*
   -- New project --
   
   This source code of graphical user interface 
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 3.1.13 or later version 
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/                   
     - for ANDROID 4.15.01 or later version;
     - for iOS 1.12.1 or later version;
    
   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.  

   Per ESP32-S3 N16R8 : https://www.amazon.it/dp/B0D6B2CYNW  
   attivare libreria Adafruit_NeoPixel.h

   Guarda la lezione integrale: https://youtube.com/live/KjdNHTT95hw
   e ADDENDUM: https://youtu.be/rJdvqBekENA
*/

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_BLE

#include <BLEDevice.h>

// RemoteXY connection settings 
#define REMOTEXY_BLUETOOTH_NAME "RemoteXY"


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 29 bytes
  { 255,1,0,0,0,22,0,19,0,0,0,0,31,1,106,200,1,1,1,0,
  1,24,64,63,63,0,2,31,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t button_01; // =1 if button pressed, else =0

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

/*
#include <Adafruit_NeoPixel.h>
#define PIN_LED 48
#define NUMPIXELS 1
Adafruit_NeoPixel pixel(NUMPIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);
*/

void setup() 
{
  RemoteXY_Init (); 
  
  
  // TODO you setup code

  pinMode( 2, OUTPUT );
  digitalWrite( 2, LOW );
  /*
  pixel.begin();
  pixel.setBrightness(50);  // regola la luminosità da 0 a 255
  pixel.setPixelColor(0, pixel.Color(255, 0, 0));  // Rosso
  pixel.show();
  */
}

void loop() 
{ 
  RemoteXY_Handler ();
  
  
  // TODO you loop code
  // use the RemoteXY structure for data transfer
  // do not call delay(), use instead RemoteXY_delay() 

  if (RemoteXY.button_01) {
    digitalWrite( 2, HIGH );
    /*
    pixel.setPixelColor(0, pixel.Color(255, 0, 0));  // Rosso
    pixel.show();
    */
  }
  else {
    digitalWrite( 2, LOW );
    /*
    pixel.setPixelColor(0, pixel.Color(0, 255, 0));  // Verde
    pixel.show();
    */
  }
}