// HX711

/* 
 *  fintanto che attendiamo che la conversione sia pronta per essere letta
 *  DT è alto e SCK deve essere mantenuto basso
 *  
 *  Quando DT va basso vuol dire che il dato è pronto
 *  con 25, 26 o 27 colpi di clock uscirà il dato
 *  partendo dal MSB per 24 bit, dopodichè DT andrà alto.
 *  
 *  Sul fronte di salita di SCK viene caricato il bit,
 *  e sul fronte di discesa viene letto.
 *  
 *  Se il totale di ck è 25, la prossima acquisizione sarà 
 *  su CH-A con gain 128.
 *  
 *  Se il totale di ck è 26, la prossima acquisizione sarà 
 *  su CH-B con gain 32.
 *  
 *  Se il totale di ck è 27, la prossima acquisizione sarà 
 *  su CH-A con gain 64.
 */

#define HX711_DT   3
#define HX711_SCK  4
#define HX711_GAIN 128   // 25 ck
//#define HX711_GAIN 64    // 27 ck

static const float k_taratura = 500.0/646000.0;  /*  g/unit  558800 per 500g */  
static const float k_filtro = 0.35;  /*  deve essere un numero <= 1.0 */  
 
float tara, peso_netto, peso_netto_filtrato;
signed long tara_raw;

void setup() {

  Serial.begin ( 9600 );

  pinMode ( LED_BUILTIN, OUTPUT );
 
  pinMode ( HX711_DT,  INPUT );
  pinMode ( HX711_SCK, OUTPUT );
  digitalWrite ( HX711_SCK, LOW );

  tara  = -1.0;
  tara_raw = 0;
}

void loop() {
  int n_ck;
  signed long raw;

  digitalWrite ( LED_BUILTIN, (millis()%600) > 300 );
 
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
    Serial.print ( "0,1500," );
    Serial.println( peso_netto_filtrato, 2 );
  }

  // FAI ALTRO IN MULTITASKING

}
