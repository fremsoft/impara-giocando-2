#include "settings.h"
#include <SPI.h>
#include <TFT_eSPI.h>

// Inizializzazione del display TFT
TFT_eSPI tft = TFT_eSPI();

void setupTFT() {

  // Inizializzazione TFT
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(1);
  tft.pushImage(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)_bg_map);
  Serial.println("TFT Display configurato");

}

/*             x     y
  alto  sx = 3800, 3700
  basso sx =  320, 3700
  alto  dx = 3700,  390
  basso dx =  320,  270

  praticamente la x e la y si scambiano di posto!
*/ 
bool getTouchXY(uint16_t *retX, uint16_t *retY) {
  uint16_t x, y, z;
  
  tft.getTouchRaw(&x, &y);
  z = tft.getTouchRawZ();

/*
  Serial.printf("x:%i\t,", x);
  Serial.printf("y:%i\t,", y);
  Serial.printf("z:%i\n" , z);
*/
  if (z > 400) {
    *retX = constrain(map(y, 3700, 300, 0, 320), 0, 319);
    *retY = constrain(map(x, 3800, 320, 0, 240), 0, 239);
    return true;
  }
  /* else */
  return false;
}
