// Programma di test generale per scheda MAEDC v1.1
//
// guarda la lezione integrale:
// https://www.youtube.com/live/YLyMxRAc3R0

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

void setup() {

  Serial.begin(115200);

  Serial2.begin(19200, SERIAL_8N1, 16, 17);
  Serial.println("Serial2 configurata su RX 16 e TX 17");

  pinMode     ( PIN_LED, OUTPUT );   
  digitalWrite( PIN_LED, LOW );

  pinMode     ( INPUT_REFILL, INPUT );   

  setupBilancia();
  setupTFT();
  setupEncoder();
  setupServo();
  
  delay(100);

  tft.fillSmoothRoundRect(20,  30, 280, 20,  5, TFT_RED);
  tft.fillSmoothRoundRect(80,  80, 140, 60, 20, TFT_BLACK);
  tft.fillSmoothRoundRect(80, 145, 140, 60, 20, TFT_BLACK);

}

// the loop function runs over and over again forever
void loop() {
  uint16_t x, y;
  char str[16];

  if (getTouchXY(&x, &y)) {
    if ((y<30) || (y>50)) {
      tft.fillRect( x, y, 5, 5, TFT_YELLOW );
    }
    else {
      servoSetPos( map(x, 0, 320, 0, 180) );
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

  digitalWrite(PIN_LED, getSwitchEncoder());

  
  if (digitalRead(INPUT_REFILL)) {
    tft.fillSmoothCircle(260, 110, 20, TFT_GREEN);
  }
  else {
    tft.fillSmoothCircle(260, 110, 20, TFT_RED);
  }

  delay(25);
}
