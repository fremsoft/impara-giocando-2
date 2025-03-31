#include "settings.h"
#include <SPI.h>
#include <TFT_eSPI.h>

// Inizializzazione del display TFT
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite tftBuff = TFT_eSprite(&tft);  // Creazione del buffer

uint32_t t0_buzzer;
int actualScreen, nextScreen, currentElement, currentChar;
int16_t yOffset, elementValues[15];
int16_t touchX=-1, touchY=-1;
String  nextString;
  
extern int ora_ore, ora_minuti;
extern int statoCoclea;   // 0 = ferma, 1 = in movimento
extern int statoServo;    // 0 = chiuso, 1 = aperto
extern int degServo; 
extern int statoErogazioneCibo; // 0 = riposo; 
                                // 1 = apertura sportellino (chiude diaframma)
                                // 2 = erogazione cibo
                                // 3 = scarico cibo (chiusura sportellino)
extern uint16_t grammiDaErogare; 

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
  nextString = "";

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
  else if ( ( actualScreen == 6 ) || ( actualScreen == 7 ) || ( actualScreen == 8 ) ) {
    // PULSANTI TIME, WiFi, MAINTENANCE
    tftBuff.pushImage(0, 156 - yOffset, SCREEN_W, SCREEN_H, (uint16_t *)_screen6_320x84_map);
  }

  if ( (actualScreen == 2) || (actualScreen == 5) || (actualScreen == 6) || (actualScreen == 7) || (actualScreen == 8)  || (actualScreen == 9) ) {
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
  if ( ( (actualScreen == 7) || (actualScreen == 8) || (actualScreen == 9) )
      && ((touchX > 46) && (touchX < 46+32) && (touchY > 11) && (touchY < 11+32)) ) {
    // HO PREMUTO BACK 
    nextScreen = 6;
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
    currentElement = -1;
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
    setGrammiPastoNow(getPosEncoder());
    
    statoErogazioneCibo = 0;  // 0 = riposo; 
    grammiDaErogare = getGrammiPastoNow();

    nextScreen = 3;
  } 
  else if ( ( (actualScreen == 5) )
      && ((touchX > 35) && (touchX < 35+50) && (touchY > 46) && (touchY < 46+(38*5))) ) {
    // HO PREMUTO UNA CASELLA ORE NELLA PAGINA PLAN
    int el = constrain((touchY - 46) / (38), 0, 4) * 3;
    if (el != currentElement) { 
      currentElement = el;
      setPosEncoder(elementValues[currentElement]);
    }
  } 
  else if ( ( (actualScreen == 5) )
      && ((touchX > 96) && (touchX < 96+50) && (touchY > 46) && (touchY < 46+(38*5))) ) {
    // HO PREMUTO UNA CASELLA MINUTI NELLA PAGINA PLAN
    int el = constrain((touchY - 46) / (38), 0, 4) * 3 + 1;
    if (el != currentElement) { 
      currentElement = el;
      setPosEncoder(elementValues[currentElement]);
    }
  } 
  else if ( ( (actualScreen == 5) )
      && ((touchX > 160) && (touchX < 160+94) && (touchY > 46) && (touchY < 46+(38*5))) ) {
    // HO PREMUTO UNA CASELLA GRAMMI NELLA PAGINA PLAN
    int el = constrain((touchY - 46) / (38), 0, 4) * 3 + 2;
    if (el != currentElement) { 
      currentElement = el;
      setPosEncoder(elementValues[currentElement]);
    }
  } 
  else if ( ( (actualScreen == 5) )
      && ((touchX > 264) && (touchX < 264+32) && (touchY > 46) && (touchY < 46+(38*5))) ) {
    // HO PREMUTO UNA CASELLA ACTIVE NELLA PAGINA PLAN
    int n = constrain((touchY - 46) / (38), 0, 4);
    if (( -100 - n ) != currentElement) { 
      struct Pasto p = getPasto(n);
      p.attivo = !p.attivo;
      setPasto(n, p);

      // disattivo il currentElement ed evito che si inneschi un loop
      currentElement = -100 - n;
    }
  } 
  else if ( ( (actualScreen == 6) || (actualScreen == 7) || (actualScreen == 8) )
      && ((touchX > 5) && (touchX < 5 + 93) && (touchY > 160) && (touchY < 160 + 75)) ) {
    // HO PREMUTO SET TIME
    nextScreen = 7;
    nextString = getTimeURL();
    setPosEncoder((int32_t)' ');
  } 
  else if ( ( (actualScreen == 6) || (actualScreen == 7) || (actualScreen == 8) )
      && ((touchX > 110) && (touchX < 110 + 93) && (touchY > 160) && (touchY < 160 + 75)) ) {
    // HO PREMUTO SET WiFi
    nextScreen = 8;
    currentElement = 1;
    nextString = getSSID();
    setPosEncoder((int32_t)' ');
  } 
  else if ( ( (actualScreen == 6) || (actualScreen == 7) || (actualScreen == 8) )
      && ((touchX > 205) && (touchX < 205 + 112) && (touchY > 160) && (touchY < 160 + 75)) ) {
    // HO PREMUTO MANUAL MODE
    nextScreen = 9;
  } 
  else if ( ( actualScreen == 7 )
      && getSwitchEncoder() ) {
    // HO PREMUTO IL PULSANTE DELL'ENCODER
    String url = getTimeURL();
    nextString = url + (char)elementValues[0];
  } 
  else if ( ( actualScreen == 7 )
      && ((touchX > 286) && (touchX < 286 + 32) && (touchY > 55) && (touchY < 55 + 32)) ) {
    // HO PREMUTO CANC
    String url = getTimeURL();
    if (url.length() > 0) { nextString = url; nextString.remove(url.length() - 1); }
  } 
  else if ( (actualScreen == 8)
      && ((touchX > 80) && (touchX < 80 + 204) && (touchY > 55 - 5) && (touchY < 55 + 37)) ) {
    // PREMUTO CAMPO SSID
    currentElement = 1;
    nextString = getSSID();
    setPosEncoder((int32_t)' ');
  }
  else if ( (actualScreen == 8)
      && ((touchX > 80) && (touchX < 80 + 204) && (touchY > 100 - 5) && (touchY < 100 + 37)) ) {
    // PREMUTO CAMPO PASSWD
    currentElement = 2;
    nextString = getPasswd();
    setPosEncoder((int32_t)' ');
  }
  else if ( ( actualScreen == 8 )
      && getSwitchEncoder() ) {
    // HO PREMUTO IL PULSANTE DELL'ENCODER
    if (currentElement == 1) {
      String ssid = getSSID();
      nextString = ssid + (char)elementValues[0];
    }
    else
    if (currentElement == 2) {
      String passwd = getPasswd();
      nextString = passwd + (char)elementValues[0];
    }
  } 
  else if ( ( actualScreen == 8 )
      && ((touchX > 286) && (touchX < 286 + 32) && (touchY > 55) && (touchY < 55 + 32)) ) {
    // HO PREMUTO CANC 1
    if (currentElement == 1) {
      String ssid = getSSID();
      if (ssid.length() > 0) { nextString = ssid; nextString.remove(ssid.length() - 1); }
    }
  } 
  else if ( ( actualScreen == 8 )
      && ((touchX > 286) && (touchX < 286 + 32) && (touchY > 100) && (touchY < 100 + 32)) ) {
    // HO PREMUTO CANC 2
    if (currentElement == 2) {
      String passwd = getPasswd();
      if (passwd.length() > 0) { nextString = passwd; nextString.remove(passwd.length() - 1); }
    }
  }
  else if ( ( actualScreen == 9 )
      && ((touchX > (55-(90/2))) && (touchX < (55+(90/2))) && (touchY > (175-(90/2))) && (touchY < (175+(90/2)))) ) {
    // HO PREMUTO IL PULSANTE START/STOP
    //statoCoclea = !statoCoclea;
    if(statoCoclea == 0) { statoCoclea = SPEED_COCLEA_VELOCE; } else { statoCoclea = 0; }
    
    speedCoclea( statoCoclea );
  } 
  else if ( ( actualScreen == 9 )
      && ((touchX > (155-(90/2))) && (touchX < (155+(90/2))) && (touchY > (175-(90/2))) && (touchY < (175+(90/2)))) ) {
    // HO PREMUTO IL PULSANTE APRE/CHIUDE
    int degSetPoint;
    //statoServo = !statoServo;
    if(statoServo == 0) { 
      statoServo = 1; 
      degSetPoint = DEG_SERVO_APERTO;
    } else { 
      statoServo = 0; 
      degSetPoint = DEG_SERVO_CHIUSO;
    }

    servoON();
    do {
      servoSetPos(degServo);
      delay(10);
      if (degSetPoint > degServo) { degServo ++; }
      if (degSetPoint < degServo) { degServo --; }
    } while ( degSetPoint != degServo );
    delay(100);
    servoOFF();
  }
  else if ( ( actualScreen == 9 )
      && ((touchX > (255-(90/2))) && (touchX < (255-(90/2)) + 90) && (touchY > (175-(60/2))) && (touchY < (175-(60/2)) + 60)) ) {
    // HO PREMUTO IL PESO (TARA)
    setTara();
  }     
  else {
    // cambio schermata solo al rilascio del touch :)
    actualScreen = nextScreen;

    if ( actualScreen == 5 ) {
      if (currentElement < 0) { currentElement = -1; }
    }
    if ( actualScreen == 7 ) { 
      setTimeURL(nextString); 
    }
    if ( actualScreen == 8 ) { 
      if (currentElement == 1) { setSSID(nextString); }
      if (currentElement == 2) { setPasswd(nextString); }
    }
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