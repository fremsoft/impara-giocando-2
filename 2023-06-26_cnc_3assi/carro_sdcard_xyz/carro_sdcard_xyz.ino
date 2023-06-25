/*
 * Programma per la movimentazione di tre motori passo passo
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
 * Visualizza il progetto integrale su YouTube: 
 * https://www.youtube.com/c/fremsoft
 *  
 */

#include <SD.h>

//#define PIN_MOT1_ENA  8  // LOGICA NEGATIVA
#define PIN_MOT1_DIR  3  // LOW = CW, HIGH = CCW
#define PIN_MOT1_PUL  2

//#define PIN_MOT2_ENA  8  // LOGICA NEGATIVA
#define PIN_MOT2_DIR  5  // LOW = CW, HIGH = CCW
#define PIN_MOT2_PUL  4

//#define PIN_MOT3_ENA  8  // LOGICA NEGATIVA
#define PIN_MOT3_DIR  7  // LOW = CW, HIGH = CCW
#define PIN_MOT3_PUL  6

#define DEBUG_MODE false

int contatore;

long int pos_mot1_presente, pos_mot2_presente, pos_mot3_presente;
long int pos_mot1_futura,   pos_mot2_futura,   pos_mot3_futura;

#define MAX_POINTS 50
long int percorso[MAX_POINTS][3];
int numero_passi, passo_in_esecuzione, passo_precedente;


#define PIN_SD_CS  10

File pathFile;

void setup() {

  Serial.begin ( 115200 );

  pinMode ( PIN_MOT1_DIR, OUTPUT );
  pinMode ( PIN_MOT1_PUL, OUTPUT );

  pinMode ( PIN_MOT2_DIR, OUTPUT );
  pinMode ( PIN_MOT2_PUL, OUTPUT );

  pinMode ( PIN_MOT3_DIR, OUTPUT );
  pinMode ( PIN_MOT3_PUL, OUTPUT );

  digitalWrite ( PIN_MOT1_DIR, LOW );
  digitalWrite ( PIN_MOT1_PUL, LOW );
  
  digitalWrite ( PIN_MOT2_DIR, LOW );
  digitalWrite ( PIN_MOT2_PUL, LOW );
  
  digitalWrite ( PIN_MOT3_DIR, LOW );
  digitalWrite ( PIN_MOT3_PUL, LOW );
  
  contatore = 0;

  pos_mot1_presente = 0;
  pos_mot2_presente = 0;
  pos_mot3_presente = 0;
  pos_mot1_futura = 0;
  pos_mot2_futura = 0;
  pos_mot3_futura = 0;
  
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
    while ((c != ',') && (c != '\n')) {
      if ((c >= '0') && (c <= '9')) { buffer[j++] = c; }
      if (pathFile.available()) { c = pathFile.read(); } else { c = '\n'; }
    }
    
    buffer[j] = '\0';
    
    percorso[i][1] = atoi(buffer);
    
    j = 0;
    if (c == '\n') {
      percorso[i][2] = 0;
    }
    else {
      c = pathFile.read();
      
      while (c != '\n') { 
        if ((c >= '0') && (c <= '9')) { buffer[j++] = c; }
        if (pathFile.available()) { c = pathFile.read(); } else { c = '\n'; }
      }
        
      buffer[j] = '\0';
    
      percorso[i][2] = atoi(buffer);
    }

    
    Serial.print(i+1);
    Serial.print(", ");
    Serial.print(percorso[i][0]);
    Serial.print(", ");
    Serial.print(percorso[i][1]);
    Serial.print(", ");
    Serial.println(percorso[i][2]);

    i++;
  }  

  numero_passi = i;
  passo_in_esecuzione = 0;
  passo_precedente = i-1;
  
  
  pathFile.close();
}

void loop() {
  int delta_x, delta_y, delta_z;

  if (DEBUG_MODE) {
    Serial.print(pos_mot1_presente);
    Serial.print(", ");
    Serial.print(pos_mot2_presente);
    Serial.print(", ");
    Serial.println(pos_mot3_presente);
  }
   
  if (
       ( pos_mot1_presente == percorso[passo_in_esecuzione][0] )
       &&
       ( pos_mot2_presente == percorso[passo_in_esecuzione][1] )
       &&
       ( pos_mot3_presente == percorso[passo_in_esecuzione][2] )
     )
  {
    /* sono arrivato a destinazione, procedo verso il prossimopasso */
    passo_precedente = passo_in_esecuzione;
    passo_in_esecuzione ++;
    if ( passo_in_esecuzione >= numero_passi ) {
      passo_in_esecuzione = 0;
    } 
  }
  else
  {
    /* sto andando al punto percorso[passo_in_esecuzione] */
  
    // determino il delta maggiore
    delta_x = percorso[passo_in_esecuzione][0] - percorso[passo_precedente][0];
    delta_y = percorso[passo_in_esecuzione][1] - percorso[passo_precedente][1];
    delta_z = percorso[passo_in_esecuzione][2] - percorso[passo_precedente][2];
      
    if (((abs(delta_x) >= abs(delta_y))) && ((abs(delta_x) >= abs(delta_z))))
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

      // interpolazione motore 3
      pos_mot3_futura = map(pos_mot1_futura, 
                            percorso[passo_precedente][0],
                            percorso[passo_in_esecuzione][0],
                            percorso[passo_precedente][2],
                            percorso[passo_in_esecuzione][2]);
    }
    else
    if (((abs(delta_y) >= abs(delta_x))) && ((abs(delta_y) >= abs(delta_z))))
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
                              
      // interpolazione motore 3
      pos_mot3_futura = map(pos_mot2_futura, 
                            percorso[passo_precedente][1],
                            percorso[passo_in_esecuzione][1],
                            percorso[passo_precedente][2],
                            percorso[passo_in_esecuzione][2]);
    }
    else
    /* if (((abs(delta_z) >= abs(delta_x))) && ((abs(delta_z) >= abs(delta_y)))) */
    {
      // comanda l'asse z (mot3)
      if (percorso[passo_in_esecuzione][2] > pos_mot3_presente) { pos_mot3_futura ++; }
      if (percorso[passo_in_esecuzione][2] < pos_mot3_presente) { pos_mot3_futura --; }

      // interpolazione motore 1
      pos_mot1_futura = map(pos_mot3_futura, 
                            percorso[passo_precedente][2],
                            percorso[passo_in_esecuzione][2],
                            percorso[passo_precedente][0],
                            percorso[passo_in_esecuzione][0]);
                              
      // interpolazione motore 2
      pos_mot2_futura = map(pos_mot3_futura, 
                            percorso[passo_precedente][2],
                            percorso[passo_in_esecuzione][2],
                            percorso[passo_precedente][1],
                            percorso[passo_in_esecuzione][1]);
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

  if (pos_mot3_futura > pos_mot3_presente) {
    // GIRA FUCSIA   CW
    digitalWrite( PIN_MOT3_DIR,  HIGH );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT3_PUL, HIGH );
    pos_mot3_presente ++;
  }

  if (pos_mot3_futura < pos_mot3_presente) {
    // GIRA FUCSIA   CCW
    digitalWrite( PIN_MOT3_DIR,  LOW );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT3_PUL, HIGH );
    pos_mot3_presente --;
  }

  delayMicroseconds(10);
  digitalWrite( PIN_MOT1_PUL, LOW );
  digitalWrite( PIN_MOT2_PUL, LOW );
  digitalWrite( PIN_MOT3_PUL, LOW );
  delayMicroseconds(10);

}
