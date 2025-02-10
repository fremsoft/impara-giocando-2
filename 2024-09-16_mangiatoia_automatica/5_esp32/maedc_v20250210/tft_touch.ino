#include "settings.h"
#include <SPI.h>
#include <TFT_eSPI.h>

// Inizializzazione del display TFT
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tftBuff = TFT_eSprite(&tft);  // Creazione del buffer

uint32_t t0_buzzer;
int actualScreen, nextScreen;
int16_t yOffset;
int16_t touchX=-1, touchY=-1;
  

extern int ora_ore, ora_minuti;

void setupTFT() {
  // Inizializzazione TFT
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_GREEN);

  tft.setSwapBytes(1);
  // attenzione: sistemare anche _swapBytes = true; o false;
  // nella libreria: TFT_eSPI/Extensions/Sprite.cpp linea #24

  tftBuff.createSprite(SCREEN_W, SCREEN_H / 2);

  tftBuff.setTextColor(TFT_BLACK, TFT_WHITE);
  //tftBuff.setTextWrap(true, true);

  tft.pushImage(0, 0, SCREEN_W, SCREEN_H, (uint16_t *)_bg_map);

  t0_buzzer = 0;
  actualScreen = nextScreen = 0;
  yOffset = 0;

  Serial.println("TFT Display configurato");
}

/*             x     y
  alto  sx = 3800, 3700
  basso sx =  320, 3700
  alto  dx = 3700,  390
  basso dx =  320,  270

  praticamente la x e la y si scambiano di posto!
*/
bool getTouchXY(int16_t *retX, int16_t *retY) {
  uint16_t x, y, z;

  tft.getTouchRaw(&x, &y);
  z = tft.getTouchRawZ();

  if (z > 400) {
    *retX = constrain(map(y, 3700, 300, 0, 320), 0, 319);
    *retY = constrain(map(x, 3800, 320, 0, 240), 0, 239);
    return true;
  }
  /* else */
  return false;
}

void tftBuffStartScreen(void) {
  char str[16];

  tftBuff.fillSprite(TFT_WHITE);

  if ( (actualScreen == 1) || (actualScreen == 2) || (actualScreen == 3) ) {
    // PULSANTI RUN, PLAN, SETTINGS
    tftBuff.pushImage(0, 156 - yOffset, SCREEN_W, SCREEN_H, (uint16_t *)_screen1_320x84_map);
  }

  if ( (actualScreen == 2) || (actualScreen == 5) || (actualScreen == 6) ) {
    // TASTO BACK
    tftBuff.pushImage(46, 11 - yOffset, 32, 32, (uint16_t *)_back_32x32_map);
  }

  if (checkWiFiConnection()) {
    tftBuff.pushImage(11, 11 - yOffset, 32, 32, (uint16_t *)_wifi_32x32_map);
  } else {
    if ((millis() % 2000) > 1000) {
      // lampeggia il simbolo del WiFi sbarrato
      tftBuff.pushImage(11, 11 - yOffset, 32, 32, (uint16_t *)_wifi_no_32x32_map);
    }
  }

  tftBuff.setFreeFont(FONT_MEDIUM);
  tftBuff.setTextColor(TFT_BLACK, TFT_WHITE);
  
  //printFreeRAM();

  tftBuff.setTextDatum(CR_DATUM);
  sprintf(str, "%2d", ora_ore);
  tftBuff.drawString(str, 265, 27 - yOffset);

  tftBuff.setTextDatum(CL_DATUM);
  sprintf(str, " %02d", ora_minuti);
  tftBuff.drawString(str, 265, 27 - yOffset);

  tftBuff.setTextDatum(CL_DATUM);
  if ((millis() % 2000) < 1000) { tftBuff.drawString(":", 265, 25 - yOffset); }
}

void tftBuffEndScreen(void) {

  tftBuff.pushSprite(0, yOffset);

  if ( ( (actualScreen == 2) || (actualScreen == 5) || (actualScreen == 6) )
      && ((touchX > 46) && (touchX < 46+32) && (touchY > 11) && (touchY < 11+32)) ) {
    // HO PREMUTO BACK
    nextScreen = 1;
  } 
  else if ( ( (actualScreen == 1) )
      && ((touchX > 5) && (touchX < 5 + 93) && (touchY > 160) && (touchY < 160 + 75)) ) {
    // HO PREMUTO RUN
    nextScreen = 2;
    setPosEncoder(getGrammiPastoNow());
  } 
  else if ( ( (actualScreen == 1) || (actualScreen == 2) )
      && ((touchX > 110) && (touchX < 110 + 93) && (touchY > 160) && (touchY < 160 + 75)) ) {
    // HO PREMUTO PLAN
    nextScreen = 5;
  } 
  else if ( ( (actualScreen == 1)  || (actualScreen == 2) )
      && ((touchX > 205) && (touchX < 205 + 112) && (touchY > 160) && (touchY < 160 + 75)) ) {
    // HO PREMUTO SETTINGS
    nextScreen = 6;
  } 
  else if ( ( (actualScreen == 2) )
      && ( ((touchX > 210) && (touchX < 210+80) && (touchY > 65) && (touchY < 65+50)) 
           || getSwitchEncoder() ) ) {
    // HO PREMUTO GO o IL TASTO DELL'ENCODER NELLA PAGINA RUN
    nextScreen = 3;
    setGrammiPastoNow(getPosEncoder());
  } 
  else {
    // cambio schermata solo al rilascio del touch :)
    actualScreen = nextScreen;
  }
}

void touchManageClick(void) {
  // GESTIONE DEL TOUCH SCREEN
  if (getTouchXY(&touchX, &touchY)) {
    // e' stato toccato il touch screen alle coordinate  touchX, touchY
    digitalWrite(PIN_BUZZER, HIGH);
    t0_buzzer = millis();
  } else if (getSwitchEncoder()) {
    // e' stato premuto il tasto dell'encoder
    digitalWrite(PIN_BUZZER, HIGH);
    t0_buzzer = millis();
  } else {
    touchX = touchY = -1;
    if ((millis() - t0_buzzer) > T_BUZZER_MS) {
      digitalWrite(PIN_BUZZER, LOW);
    }
  }
}