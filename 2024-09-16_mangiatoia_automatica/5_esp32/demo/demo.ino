// Programma di test per TFT 2.8"
// da utilizzare sulla scheda MAEDC v1.1
//
// guarda la lezione integrale:
// https://www.youtube.com/live/YLyMxRAc3R0

// Questo sketch usa la libreria:
//  - TFT_eSPI by Bodmer        v2.5.43 (per TFT e TOUCH)

// Bisogna caricare l'immagine 'bg.png' a tutto schermo
//
// Definizione dei pin in Users\...\Documenti\Arduino\libraries\TFT_eSPI\User_Setup.h


#define TFT_CS    5
#define TFT_DC    21
#define TFT_RST   22

#define TOUCH_CS  15
#define TOUCH_IRQ 14

/*
  non c'è SD CARD
#define SD_CS     13
*/

#include <SPI.h>
#include <TFT_eSPI.h>

#include "bg.h"

#define SCREEN_W 320
#define SCREEN_H 240

#define PIN_LED  32

// Inizializzazione del display TFT
TFT_eSPI tft = TFT_eSPI(); 

//int16_t calData[5];

void setup() {

  Serial.begin(115200);

  Serial2.begin(19200, SERIAL_8N1, 16, 17);
  Serial.println("Serial2 configurata su RX 16 e TX 17");

  pinMode(PIN_LED, OUTPUT);   // LED 
  digitalWrite(PIN_LED, LOW);

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

  uint16_t x, y, z;

  tft.getTouchRaw(&x, &y);
  z = tft.getTouchRawZ();

  Serial.printf("x:%i\t,", x);
  Serial.printf("y:%i\t,", y);
  Serial.printf("z:%i\n" , z);

  /*              x     y
     alto  sx = 3800, 3700
     basso sx =  320, 3700
     alto  dx = 3700,  390
     basso dx =  320,  270

     praticamente la x e la y si scambiano di posto!
   */
  if (z > 400) {
    digitalWrite(PIN_LED, HIGH);

    tft.fillRect(
          constrain(map(y, 3700, 300, 0, 320), 0, 314),
          constrain(map(x, 3800, 320, 0, 240), 0, 234),
          5, 5, TFT_YELLOW /* 0b11111 111111 00000 */
    );
  }
  else {
    digitalWrite(PIN_LED, LOW);
  }
  
  delay(25);
}
