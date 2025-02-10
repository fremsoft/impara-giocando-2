// Programma di gestione della scheda MAEDC v1.1
// per erogare fino a 5 pasti al giorno.
//
// Guarda la lezione integrale:
// https://youtube.com/live/gxMmNqdsiB8

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
extern TFT_eSprite tftBuff;

extern int actualScreen, nextScreen;
extern int16_t yOffset;
  
void setup() {

  Serial.begin( 115200 );
  //printFreeRAM();

  pinMode     ( PIN_BUZZER, OUTPUT );   
  digitalWrite( PIN_BUZZER, LOW );

  pinMode     ( INPUT_REFILL, INPUT );   

  setupDatiNV();
  startWifi();

  setupBilancia();
  setupTFT();
  setupEncoder();
  setupServo();
  setupRS485( 19200 );

  delay( 3000 );

  //printFreeRAM();
}

int c=0;
void loop() {
  char str[16];
  int32_t grammi;

  // YOFFSET
  // serve per disegnare mezza schermata alla volta poiche' 
  // non c'è abbastanza RAM per creare uno sprite grande quanto
  // tutto lo schermo
  if (yOffset == 0) { yOffset = SCREEN_H/2; } else { yOffset = 0; }

  touchManageClick();

  // GESTIONE DELLE SCHERMATE
  switch ( actualScreen ) {

    case 1 : // schermata principale
      tftBuffStartScreen();

      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_BIG );
      sprintf(str, "Status");
      tftBuff.drawString(str, 160, 90-yOffset);

      tftBuffEndScreen();

      /* salva i dati non volatili 
         solo nella schermata principale 
         per evitare di scrivere troppe volte
         e solo se sono cambiati i dati */
      salvataggioDatiNV();   
      break;

    case 2 : // schermata erogazione manuale
      grammi = getPosEncoder();
      if (grammi < 1) { grammi = 1; setPosEncoder(grammi); }
      if (grammi > MAX_PORZIONE_GRAMMI) { grammi = MAX_PORZIONE_GRAMMI; setPosEncoder(grammi); }

      tftBuffStartScreen();
      
      // CASELLA SELEZIONE GRAMMI
      tftBuff.fillSmoothRoundRect(30, 65-yOffset, 160, 50,  5, TFT_NAVY);
      tftBuff.fillSmoothRoundRect(31, 66-yOffset, 158, 48,  3, TFT_WHITE);
      
      tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_LARGE );
      sprintf(str, " %dg ", grammi);
      tftBuff.drawString(str, 110, 90-yOffset);

      // PULSANTE GO
      tftBuff.fillSmoothRoundRect(210, 65-yOffset, 80, 50,  5, TFT_NAVY);

      tftBuff.setTextColor(TFT_WHITE, TFT_NAVY);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_LARGE );
      tftBuff.drawString("GO", 250, 90-yOffset);

      tftBuffEndScreen();
      break;

    case 3 : // erogazione in corso
      delay(1000); actualScreen = nextScreen = 1;
      break;

    case 4 : // finito di erogare le crocchette
      break;

    case 5 : // programmazione dei pasti
      /*grammi = getPosEncoder();
      if (grammi < 1) { grammi = 1; setPosEncoder(grammi); }
      if (grammi > MAX_PORZIONE_GRAMMI) { grammi = MAX_PORZIONE_GRAMMI; setPosEncoder(grammi); }
      */
      
      tftBuffStartScreen();
      
      for (int i=0; i<5; i++) {
        struct Pasto p = getPasto(i);

        // NUMERO DELLA RICETTA
        tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        sprintf(str, "%d", i+1);
        tftBuff.drawString(str, 3+(30/2), 45+(34/2) + (38*i) - yOffset);

        tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        tftBuff.drawString(":", 83+(14/2), 45+(34/2) + (38*i) - yOffset);

        // CASELLA DELLE ORE 
        tftBuff.fillSmoothRoundRect(35, 46 + (38*i) - yOffset, 50, 34,  5, TFT_NAVY);
        tftBuff.fillSmoothRoundRect(36, 47 + (38*i) - yOffset, 48, 32,  3, TFT_WHITE);
      
        tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        sprintf(str, "%2d", p.ora);
        tftBuff.drawString(str, 35 + (50/2), 45 + (38*i) + (34/2) - yOffset);

        // CASELLA DEI MINUTI
        tftBuff.fillSmoothRoundRect(96, 46 + (38*i) - yOffset, 50, 34,  5, TFT_NAVY);
        tftBuff.fillSmoothRoundRect(97, 47 + (38*i) - yOffset, 48, 32,  3, TFT_WHITE);
      
        tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        sprintf(str, "%02d", p.minuti);
        tftBuff.drawString(str, 96 + (50/2), 45 + (38*i) + (34/2) - yOffset);

        // CASELLA DEI GRAMMI
        tftBuff.fillSmoothRoundRect(160, 46 + (38*i) - yOffset, 94, 34,  5, TFT_NAVY);
        tftBuff.fillSmoothRoundRect(161, 47 + (38*i) - yOffset, 92, 32,  3, TFT_WHITE);
      
        tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        sprintf(str, "%dg", p.grammi);
        tftBuff.drawString(str, 160 + (94/2), 45 + (38*i) + (34/2) - yOffset);

        // PULSANTE ACTIVE
        if (p.attivo) {
          tftBuff.pushImage(264, 47 + (38*i) - yOffset, 32, 32, (uint16_t *)_check_32x32_map);
        }
        else {
          tftBuff.pushImage(264, 47 + (38*i) - yOffset, 32, 32, (uint16_t *)_not_check_32x32_map);
        }
      }

      tftBuffEndScreen();
      break;

    case 6 : // menu impostazioni
      tftBuff.fillSprite(TFT_WHITE);

      tftBuff.setTextColor(TFT_BLACK, TFT_WHITE);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_BIG );
      sprintf(str, "Settings");
      tftBuff.drawString(str, 160, 90-yOffset);
      
      tftBuff.pushSprite(0, yOffset);
      break;

    case 7 : // impostazione orologio
      break;

    case 8 : // impostazione WiFi
      break;

    case 9 : // manutenzione
      break;

    default : 
      actualScreen = nextScreen = 1; 
      break;

  }

  /*
  tft.fillRect( x, y, 5, 5, TFT_YELLOW );
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
