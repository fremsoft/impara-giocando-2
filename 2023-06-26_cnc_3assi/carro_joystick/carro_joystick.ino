/*
 * Programma per la movimentazione di due motori passo passo
 * 23HS2430 con controller TB6600 ( https://amzn.to/41r0FO4 )
 * in modo interpolato.
 * 
 * IL TB6600 è configurato con i DIP-SW S1-OFF, S2-OFF, S3-ON
 * per avere 16 microstep (200 * 16 = 3200 impulsi/giro)
 * 
 * Un Joystick collegato sugli ingressi analogici A0 e A1 
 * controllano i movimenti dei due motori in fase di apprendimento.
 * 
 * Un pulsante collegato all'ingresso A3 (con pull-up interna)
 * permette di registrare le posizioni (con pressione veloce) 
 * e di avviare la movimentazione (con pressione prolungata)
 * 
 * Visualizza il progetto integrale su YouTube: 
 * https://www.youtube.com/c/fremsoft
 *  
 */

#define PIN_MOT1_ENA  8  // LOGICA NEGATIVA
#define PIN_MOT1_DIR  3  // LOW = CW, HIGH = CCW
#define PIN_MOT1_PUL  2

#define PIN_MOT2_ENA  9  // LOGICA NEGATIVA
#define PIN_MOT2_DIR  5  // LOW = CW, HIGH = CCW
#define PIN_MOT2_PUL  4

#define JOYSTICK_X   A0
#define JOYSTICK_Y   A1
#define BUTTON       A3


/*  0 = GIRA BLU   CW
    1 = GIRA BLU   CCW
    2 = GIRA VERDE CW
    3 = GIRA VERDE CCW
*/
int stato_sequenza;
int contatore;

bool apprendimento;


long int pos_mot1_presente, pos_mot2_presente;
long int pos_mot1_futura,   pos_mot2_futura;

#define NUMERO_MASSIMO_PASSI 100
long int percorso[NUMERO_MASSIMO_PASSI][3];
int numero_passi, passo_in_esecuzione, passo_precedente;


void setup() {

  Serial.begin ( 115200 );

  pinMode ( PIN_MOT1_ENA, OUTPUT );
  pinMode ( PIN_MOT1_DIR, OUTPUT );
  pinMode ( PIN_MOT1_PUL, OUTPUT );

  pinMode ( PIN_MOT2_ENA, OUTPUT );
  pinMode ( PIN_MOT2_DIR, OUTPUT );
  pinMode ( PIN_MOT2_PUL, OUTPUT );

  digitalWrite ( PIN_MOT1_ENA, LOW );
  digitalWrite ( PIN_MOT1_DIR, LOW );
  digitalWrite ( PIN_MOT1_PUL, LOW );
  
  digitalWrite ( PIN_MOT2_ENA, LOW );
  digitalWrite ( PIN_MOT2_DIR, LOW );
  digitalWrite ( PIN_MOT2_PUL, LOW );
  
  pinMode( BUTTON,    INPUT_PULLUP);

  stato_sequenza = 0;
  contatore = 0;

  pos_mot1_presente = 0;
  pos_mot2_presente = 0;
  pos_mot1_futura = 0;
  pos_mot2_futura = 0;
  
  numero_passi = 0;

  apprendimento = true;
}

void loop() {
  int x, y;
  int delta_x, delta_y;
  

  if ( apprendimento ) {
    /* modalita' di apprendimento */
    x = analogRead( JOYSTICK_X );
    
    if ( x > 700) {
      pos_mot1_futura ++;
    }
    else if ( x < 300) {
      pos_mot1_futura --;
    }

    y = analogRead( JOYSTICK_Y );
    if ( y > 700) {
      pos_mot2_futura ++;
    }
    else if ( y < 300) {
      pos_mot2_futura --;
    }
    
    if ( analogRead( BUTTON ) < 500 ) {
      /* ho premuto il tasto */
      /* memorizzo la posizione */

      Serial.print(pos_mot1_presente);
      Serial.print(", ");
      Serial.println(pos_mot2_presente);

      percorso[numero_passi][0] = pos_mot1_presente;
      percorso[numero_passi][1] = pos_mot2_presente;
      if (numero_passi < (NUMERO_MASSIMO_PASSI - 1)) {
        numero_passi ++;
      }

      delay( 1000 );

      if ( analogRead( BUTTON ) < 500 ) {
        apprendimento = false;
        passo_in_esecuzione = 0;
        passo_precedente = numero_passi - 1;
      }
    }
  }
  else {
    /* modalita' esecuzione */

    Serial.print(pos_mot1_presente);
    Serial.print(", ");
    Serial.println(pos_mot2_presente);
    
    /* sto andando al punto percorso[passo_in_esecuzione] */

    if (
         ( pos_mot1_presente == percorso[passo_in_esecuzione][0] )
         &&
         ( pos_mot2_presente == percorso[passo_in_esecuzione][1] )
       )
    {
      passo_precedente = passo_in_esecuzione;
      passo_in_esecuzione ++;
      if ( passo_in_esecuzione >= numero_passi ) {
        passo_in_esecuzione = 0;
      } 
    }
    else
    {
      // determino il delta maggiore
      delta_x = percorso[passo_in_esecuzione][0] - percorso[passo_precedente][0];
      delta_y = percorso[passo_in_esecuzione][1] - percorso[passo_precedente][1];
      
      if ((abs(delta_x) >= abs(delta_y)))
      {
        // comanda l'asse x (mot1)
        if (percorso[passo_in_esecuzione][0] > pos_mot1_presente) { pos_mot1_futura ++; }
        if (percorso[passo_in_esecuzione][0] < pos_mot1_presente) { pos_mot1_futura --; }

        // interpolazione motore 2
        pos_mot2_futura = map(pos_mot1_futura, 
                              percorso[passo_precedente][0],
                              percorso[passo_in_esecuzione][0],
                              percorso[passo_precedente][1],
                              percorso[passo_in_esecuzione][1]);

      }
      else
      {
        // comanda l'asse y (mot2)
        if (percorso[passo_in_esecuzione][1] > pos_mot2_presente) { pos_mot2_futura ++; }
        if (percorso[passo_in_esecuzione][1] < pos_mot2_presente) { pos_mot2_futura --; }

        // interpolazione motore 1
        pos_mot1_futura = map(pos_mot2_futura, 
                              percorso[passo_precedente][1],
                              percorso[passo_in_esecuzione][1],
                              percorso[passo_precedente][0],
                              percorso[passo_in_esecuzione][0]);
                              
      }
    }
  }

  if (pos_mot1_futura > pos_mot1_presente) {
    // GIRA BLU   CW
    digitalWrite( PIN_MOT1_DIR,  HIGH );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT1_PUL, HIGH );
    pos_mot1_presente ++;
  }

  if (pos_mot1_futura < pos_mot1_presente) {
    // GIRA BLU   CCW
    digitalWrite( PIN_MOT1_DIR,  LOW );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT1_PUL, HIGH );
    pos_mot1_presente --;
  }

  if (pos_mot2_futura > pos_mot2_presente) {
    // GIRA VERDE CW
    digitalWrite( PIN_MOT2_DIR,  HIGH );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT2_PUL, HIGH );
    pos_mot2_presente ++;
  }

  if (pos_mot2_futura < pos_mot2_presente) {
    // GIRA VERDE CCW
    digitalWrite( PIN_MOT2_DIR,  LOW );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT2_PUL, HIGH );
    pos_mot2_presente --;
  }

  delay(1);
  digitalWrite( PIN_MOT1_PUL, LOW );
  digitalWrite( PIN_MOT2_PUL, LOW );
  delay(1);

}
