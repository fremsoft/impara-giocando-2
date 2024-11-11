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

#define HX711_DT   35
#define HX711_SCK  25
#define HX711_GAIN 128   // 25 ck
//#define HX711_GAIN 64    // 27 ck

static const float k_taratura = 500.0/552500.0;  /*  g/unit  558800 per 500g */  
static const float k_filtro = 0.35;  /*  deve essere un numero <= 1.0 */  
 
float tara, peso_netto, peso_netto_filtrato;
signed long tara_raw;

// Inizializzazione del display TFT
TFT_eSPI tft = TFT_eSPI(); 

//int16_t calData[5];

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
    *retY = constrain(map(x, 3800, 320, 0, 240), 0, 234);
    return true;
  }
  /* else */
  return false;
}

bool getPeso() {
  int n_ck;
  signed long raw;

  // Attendo che il DT vada basso per iniziare la lettura
  if ( digitalRead ( HX711_DT ) == LOW ) {

    // scarico i 24 bit dalla seriale
    raw = 0;
    for (n_ck=0; n_ck<24; n_ck++) {
      digitalWrite ( HX711_SCK, HIGH );
      delayMicroseconds(1);
      digitalWrite ( HX711_SCK, LOW );
      //delayMicroseconds(1);
      
      raw = raw << 1;
      raw |= (digitalRead ( HX711_DT ) & 1);
      
      //if ( digitalRead ( HX711_DT ) != 0 ) { raw = raw | 1; }      
    }
    
    if ((raw & 0x800000) != 0) { raw = raw | 0xFF000000; }
    
    // poi conto altri n-24 ck
    if (HX711_GAIN == 128) {
      n_ck = 25 - 24;
    } else /* if (HX711_GAIN == 64) */ {
      n_ck = 27 - 24;
    }

    while (n_ck > 0) { 
      digitalWrite ( HX711_SCK, HIGH );
      delayMicroseconds(1);
      digitalWrite ( HX711_SCK, LOW );
      delayMicroseconds(1);
      
      n_ck--;
    }

    //Serial.print ( raw, DEC );
    //Serial.print ( ",\t " );

    peso_netto = (float)(raw - tara_raw) * k_taratura;  // [g] 
     
    if ((tara < 0) || (peso_netto < tara)) { 
      tara_raw = raw;
      tara = peso_netto;
    }

    peso_netto_filtrato = (peso_netto_filtrato * (1-k_filtro)) + (peso_netto * k_filtro);

/*
    Serial.print ( "0,1500," );
    Serial.println( peso_netto_filtrato, 2 );
*/
    return true;
  }
  /* else */
  
  return false;
}

void setup() {

  Serial.begin(115200);

  Serial2.begin(19200, SERIAL_8N1, 16, 17);
  Serial.println("Serial2 configurata su RX 16 e TX 17");

  pinMode(PIN_LED, OUTPUT);   // LED 
  digitalWrite(PIN_LED, LOW);

  pinMode ( HX711_DT,  INPUT );
  pinMode ( HX711_SCK, OUTPUT );
  digitalWrite ( HX711_SCK, LOW );
  Serial.println("HX711 configurato");

  // Inizializzazione TFT
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(1);
  tft.pushImage(0, 0, SCREEN_W, SCREEN_H, (uint16_t*)_bg_map);
  Serial.println("TFT Display configurato");

  tara  = -1.0;
  tara_raw = 0;

  delay(100);

  tft.fillSmoothRoundRect(80, 80, 140, 60, 20, TFT_BLACK);

}

// the loop function runs over and over again forever
void loop() {
  uint16_t x, y;
  char str[16];

  if (getTouchXY(&x, &y)) {
    digitalWrite(PIN_LED, HIGH);

    tft.fillRect( x, y, 5, 5, TFT_YELLOW );
  }
  else {
    digitalWrite(PIN_LED, LOW);
  }

  if (getPeso()) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(100, 100);
    sprintf(str, "%6.1f", peso_netto_filtrato);
    tft.print(str);
  }
  
  delay(25);
}
