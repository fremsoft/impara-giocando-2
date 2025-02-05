// Programma di gestione della scheda MAEDC v1.1
// per erogare fino a 5 pasti al giorno.
//
// Guarda la lezione integrale:
// https://youtube.com/live/JDVK9Ja8ZW0

// Questo sketch usa le seguenti librerie:
//  - TFT_eSPI by Bodmer                 v2.5.43 (per TFT e TOUCH)
//  - ESP32Servo by Kevin H., John K.B.  v3.0.5  (per Servo SG90)

// Bisogna caricare l'immagine 'bg.png' a tutto schermo
//
// Definizione dei pin in Users\...\Documenti\Arduino\libraries\TFT_eSPI\User_Setup.h

#include "settings.h"
#include "images.h"
#include <SPI.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

// variabili globali
uint32_t t0_buzzer;
int screen;

void setup() {

  Serial.begin( 115200 );
  printFreeRAM();

  pinMode     ( PIN_BUZZER, OUTPUT );   
  digitalWrite( PIN_BUZZER, LOW );

  pinMode     ( INPUT_REFILL, INPUT );   

  setupBilancia();
  setupTFT();
  setupEncoder();
  setupServo();
  setupRS485( 19200 );
  
  printFreeRAM();
  delay( 3000 );

  t0_buzzer = 0;
  screen = 0;

}


void loop() {
  int16_t x=-1, y=-1;
  char str[16];

  if (getTouchXY(&x, &y)) {
    // e' stato toccato il touch screen alle coordinate  x, y
    digitalWrite( PIN_BUZZER, HIGH ); 
    t0_buzzer = millis();
  } else if ( getSwitchEncoder() ) {
    // e' stato premuto il tasto dell'encoder
    digitalWrite( PIN_BUZZER, HIGH ); 
    t0_buzzer = millis();
  }
  else {
    if ( (millis() - t0_buzzer) > T_BUZZER_MS ) {
      digitalWrite( PIN_BUZZER, LOW );
    }
  }

  // GESTIONE DELLE SCHERMATE
  switch ( screen ) {
    case 1 : // schermata principale
      tft.pushImage(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)_screen1_map);

      tft.pushImage(11, 11, 32, 32, (uint16_t*)_wifi_32x32_map);

      tft.setTextColor(TFT_BLACK, TFT_WHITE);
      tft.setTextDatum(CC_DATUM);
      tft.setTextSize(3);
      sprintf(str, "%2d:%2d", 12, 34);
      tft.drawString(str, 265, 27);

      tft.setTextSize(5);
      sprintf(str, "Status");
      tft.print(str);
      tft.drawString(str, 160, 90);

      if ((x > 5) && (x < 5+93) && (y > 160) && (y < 160+75)) {
        // HO PREMUTO RUN
        screen = 2;
      }
      if ((x > 110) && (x < 110+93) && (y > 160) && (y < 160+75)) {
        // HO PREMUTO PLAN
        screen = 5;
      }
      if ((x > 205) && (x < 205+112) && (y > 160) && (y < 160+75)) {
        // HO PREMUTO SETTINGS
        screen = 6;
      }
      break;
    case 2 : // schermata erogazione manuale
      tft.fillScreen(TFT_WHITE);

      tft.setTextColor(TFT_BLACK, TFT_WHITE);
      tft.setTextDatum(CC_DATUM);
      tft.setTextSize(5);
      sprintf(str, "Run");
      tft.print(str);
      tft.drawString(str, 160, 90);
      break;
    case 3 : // erogazione in corso
      break;
    case 4 : // finito di erogare le crocchette
      break;
    case 5 : // programmazione dei pasti
      tft.fillScreen(TFT_WHITE);

      tft.setTextColor(TFT_BLACK, TFT_WHITE);
      tft.setTextDatum(CC_DATUM);
      tft.setTextSize(5);
      sprintf(str, "Plan");
      tft.print(str);
      tft.drawString(str, 160, 90);
      break;
    case 6 : // menu impostazioni
      tft.fillScreen(TFT_WHITE);

      tft.setTextColor(TFT_BLACK, TFT_WHITE);
      tft.setTextDatum(CC_DATUM);
      tft.setTextSize(5);
      sprintf(str, "Settings");
      tft.print(str);
      tft.drawString(str, 160, 90);
      break;
    case 7 : // impostazione orologio
      break;
    case 8 : // impostazione WiFi
      break;
    case 9 : // manutenzione
      break;
    default : 
      screen = 1; 
      break;
  }

  /*
  tft.fillRect( x, y, 5, 5, TFT_YELLOW );
  tft.fillSmoothRoundRect(20,  30, 280, 20,  5, TFT_RED);
  tft.fillSmoothRoundRect(80,  80, 140, 60, 20, TFT_BLACK);
  tft.fillSmoothRoundRect(80, 145, 140, 60, 20, TFT_BLACK);
  tft.fillSmoothCircle(260, 110, 20, TFT_GREEN);

  servoSetPos( map(x, 0, 320, 180, 0) );  delay(50); servoON();
  servoOFF();   
  */

  /*
  if (getPeso()) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(100, 100);
    sprintf(str, "%6.1f", getPesoNettoFiltrato());
    tft.print(str);
  }  
 */
 /*
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(100, 165);
  sprintf(str, "%5d", getPosEncoder());
  tft.print(str);
 */
  /*
  if ( getSwitchEncoder() ) {
    
    if ( getPosEncoder() != 0 ) {
      int id=1;
      int dir=(getPosEncoder() >= 0)?(1):(0);
      int step=100*abs(getPosEncoder());
      int ppr=1600;  // pulses per revolution
      int rnd=millis()&0xFFF;
      int csum=id+dir+step+ppr+rnd;
      int go=1;
        
      char s[256];
      sprintf(s, "   {id:%d,dir:%d,step:%d,ppr:%d,rnd:%d,csum:%d}", id, dir, step, ppr, rnd, csum);
      sendRS485Message(s, true);

      id=0x100;     // broadcasting message
      csum=id+go+rnd;
      sprintf(s, "   {id:%d,go:%d,rnd:%d,csum:%d}", id, go, rnd, csum);
      sendRS485Message(s, false);

      setPosEncoder(0);
    }
  }
  */
  
  //if ( digitalRead(INPUT_REFILL) ) 
  delay(25);
}
