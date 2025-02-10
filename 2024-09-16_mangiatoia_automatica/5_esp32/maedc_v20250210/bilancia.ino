#include "settings.h"

static const float k_taratura = 500.0/552500.0;  /*  g/unit  558800 per 500g */  
static const float k_filtro = 0.35;  /*  deve essere un numero <= 1.0 */  
 
float tara, peso_netto, peso_netto_filtrato;
signed long tara_raw;

void setupBilancia() {

  pinMode ( HX711_DT,  INPUT );
  pinMode ( HX711_SCK, OUTPUT );
  digitalWrite ( HX711_SCK, LOW );
  Serial.println("HX711 configurato");

  tara  = -1.0;
  tara_raw = 0;

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

float getPesoNettoFiltrato(void) {
  return peso_netto_filtrato;
}