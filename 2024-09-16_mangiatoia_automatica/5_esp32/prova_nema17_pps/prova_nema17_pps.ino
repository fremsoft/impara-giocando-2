// Programma di test generale per scheda MAEDC v1.1
//
// guarda la lezione integrale:
// https://youtube.com/live/cn3IcIu6waY

// Questo sketch usa le seguenti librerie:
//  - TFT_eSPI by Bodmer                 v2.5.43 (per TFT e TOUCH)
//  - ESP32Servo by Kevin H., John K.B.  v3.0.5  (per Servo SG90)

// Bisogna caricare l'immagine 'bg.png' a tutto schermo
//
// Definizione dei pin in Users\...\Documenti\Arduino\libraries\TFT_eSPI\User_Setup.h

#include "settings.h"
#include "bg.h"
#include <SPI.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

#include "settings.h"

void setup() {

  Serial.begin(115200);

  pinMode     ( PIN_LED, OUTPUT );   
  digitalWrite( PIN_LED, LOW );

  pinMode     ( INPUT_REFILL, INPUT );   

  setupBilancia();
  setupTFT();
  setupEncoder();
  setupServo();
  setupRS485( 19200 );
  
  delay(100);

  tft.fillSmoothRoundRect(20,  30, 280, 20,  5, TFT_RED);
  tft.fillSmoothRoundRect(80,  80, 140, 60, 20, TFT_BLACK);
  tft.fillSmoothRoundRect(80, 145, 140, 60, 20, TFT_BLACK);

}


void loop() {
  uint16_t x, y;
  char str[16];

  if (getTouchXY(&x, &y)) {
    if ((y<30) || (y>50)) {
      tft.fillRect( x, y, 5, 5, TFT_YELLOW );
    }
    else {
      servoSetPos( map(x, 0, 320, 180, 0) );
      delay(50);
      servoON();
    }
  }
  else {
    servoOFF();
  }

  if (getPeso()) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(100, 100);
    sprintf(str, "%6.1f", getPesoNettoFiltrato());
    tft.print(str);
  }  

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(100, 165);
  sprintf(str, "%5d", getPosEncoder());
  tft.print(str);

  if ( getSwitchEncoder() ) {
    digitalWrite(PIN_LED, HIGH);

    if ( true /* getPosEncoder() != 0 */ ) {
      int id=1;
      int dir=(getPosEncoder() >= 0)?(1):(0);
      int pps=10*abs(getPosEncoder());
      int ppr=800;  // pulses per revolution
      int rnd=millis()&0xFFF;
      int csum=id+dir+pps+ppr+rnd;
      char s[256];
      sprintf(s, "   {id:%d,dir:%d,ppr:%d,pps:%d,rnd:%d,csum:%d}", id, dir, ppr, pps, rnd, csum);
      sendRS485Message(s, true);

      setPosEncoder(0);
    }

    delay(200); //antirimbalzo
  }
  else {
    digitalWrite(PIN_LED, LOW);
  }
  
  if ( digitalRead(INPUT_REFILL) ) {
    tft.fillSmoothCircle(260, 110, 20, TFT_GREEN);
  }
  else {
    tft.fillSmoothCircle(260, 110, 20, TFT_RED);
  }

  delay(25);
}
