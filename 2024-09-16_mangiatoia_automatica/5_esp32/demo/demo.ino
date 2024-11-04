// Programma di test per TFT 2.8"
// da utilizzare sulla scheda MAEDC v1.0
//
// guarda la lezione integrale:
// https://www.youtube.com/live/ObEYt9QQGqc

// Questo sketch usa la libreria:
//  - TFT_eSPI by Bodmer        v2.5.43 (per TFT e TOUCH)

// Bisogna caricare l'immagine 'bg.png' a tutto schermo
//
// Definizione dei pin in Users\...\Documenti\Arduino\libraries\TFT_eSPI\User_Setup.h

/*
    Non c'è touch screen

#define TFT_CS    5
#define TFT_DC    21
#define TFT_RST   22
#define TOUCH_CS  15
#define TOUCH_IRQ 14

    E nemmeno SD CARD
#define SD_CS     13
*/

#include <SPI.h>
#include <TFT_eSPI.h>

#include "bg.h"

#define SCREEN_W 320
#define SCREEN_H 240

// Inizializzazione del display TFT
TFT_eSPI tft = TFT_eSPI(); 

#ifdef USE_TOUCH
int16_t calData[5];
#endif

void setup() {

  Serial.begin(115200);

  Serial2.begin(19200, SERIAL_8N1, 16, 17);
  Serial.println("Serial2 configurata su RX 16 e TX 17");

  pinMode(12, OUTPUT);   // LED 

  // Inizializzazione TFT
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  //tft.println("TFT Display Initialized!");
  tft.setSwapBytes(1);
  tft.pushImage(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)_bg_map);

  delay(100);
}

// the loop function runs over and over again forever
void loop() {

  /* do nothing */

}
