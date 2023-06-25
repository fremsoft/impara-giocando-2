/*
 * Programma per la movimentazione di due motori passo passo
 * 23HS2430 con controller TB6600 ( https://amzn.to/41r0FO4 )
 * in modo interpolato.
 * 
 * IL TB6600 è configurato con i DIP-SW S1-OFF, S2-OFF, S3-ON
 * per avere 16 microstep (200 * 16 = 3200 impulsi/giro)
 * 
 * Le coordinate del percorso sono registrate nel file "PATH.CSV"
 * situato nella microSD Card, connessa al modulo per Arduino 
 * nel seguente modo: 
 * 
 * 1 (GND)   - GND
 * 2 (VCC)   - +5V
 * 3 (MISO)  - 12
 * 4 (MOSI)  - 11
 * 5 (SCK)   - 13
 * 6 (CS)    - 10 
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

#include <SD.h>


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

#define MAX_POINTS 50
long int percorso[MAX_POINTS][2];
int numero_passi, passo_in_esecuzione, passo_precedente;


#define PIN_SD_CS  10

File pathFile;

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

  if (!SD.begin( PIN_SD_CS )) {
    Serial.println("Errore inizializzazione scheda SD");
    return;
  }

  if (!SD.exists("path.csv")) {
    Serial.println("File non trovato");
    return;
  }

  pathFile = SD.open("path.csv");
  
  if (!pathFile.available()) {
    Serial.println("Errore apertura file");
    return;
  }

  int i = 0;
  char buffer[32];
  
  while ((pathFile.available()) && (i < MAX_POINTS)) {
    int j = 0;
    char c = pathFile.read();
    
    while (c != ',') {
      if ((c >= '0') && (c <= '9')) { buffer[j++] = c; }
      if (pathFile.available()) { c = pathFile.read(); } else { c = ','; }
    }
    
    buffer[j] = '\0';
    
    percorso[i][0] = atoi(buffer);
    
    j = 0;
    c = pathFile.read();
    
    while (c != '\n') {
      if ((c >= '0') && (c <= '9')) { buffer[j++] = c; }
      if (pathFile.available()) { c = pathFile.read(); } else { c = '\n'; }
    }
        
    buffer[j] = '\0';
    
    percorso[i][1] = atoi(buffer);

    i++;

    Serial.print(i);
    Serial.print(", ");
    Serial.print(percorso[i-1][0]);
    Serial.print(", ");
    Serial.println(percorso[i-1][1]);
    
  }  

  numero_passi = i;
  apprendimento = false;

  pathFile.close();
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
      if (numero_passi < (MAX_POINTS - 1)) {
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
