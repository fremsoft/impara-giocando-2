/*
* Programma per la movimentazione di tre motori passo passo
 * 23HS2430 con controller TB6600 interpolato mediante spline.
 *  "Azionamento TB6600"     - https://amzn.to/46pkFEi
 *  "Motore stepper NEMA 23" - https://amzn.to/3Jr2Evq
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
 * All'accensione viene eseguita una procedura di homing
 * per azzerare le coordinate
 * 
 * Visualizza il progetto integrale su YouTube: 
 * https://youtube.com/live/uFQ9prJWlKU
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

#define HOMING_X     14 // A0
#define HOMING_Y     15 // A1
#define HOMING_Z     16 // A2

#define DEBUG_MODE false

long int pos_mot1_presente, pos_mot2_presente, pos_mot3_presente;
long int pos_mot1_futura,   pos_mot2_futura,   pos_mot3_futura;

#define MAX_POINTS 50
long int percorso[MAX_POINTS][3];
int numero_passi, passo_in_esecuzione, passo_precedente;

unsigned long t0, t1;
float vel0[3], vel1[3]; /* velocità di partenza e di arrivo del singolo tratto della spline*/
float coefX[4], coefY[4], coefZ[4];

#define MAX_PASSI_HOMING 10000

#define PIN_SD_CS  10

File pathFile;

#define DELTA_T_MS           2000
#define RISOLUZIONE_SPLINE    100.0

const float  T = DELTA_T_MS / RISOLUZIONE_SPLINE;


void calcolaCoefficientiSpline(float p0, float p1, float v0, float v1,
                               float *a, float *b, float *c, float *d ) {
#if false
       /* 
        *  Hermite  "naturale"
        */

       *a = -2.0 * (p1 - p0) / (T * T * T);
       *b =  3.0 * (p1 - p0) / (T * T);
       *c =  0;
       *d =  p0;
#else
       /*       
        *  Catmull-Rom
        */
       *a = (v1 - v0 - 2.0 * (p1 - v0*T - p0) / T) / (T * T);
       *b = (p1 - v0*T - p0) / (T * T) - (*a)*T;
       *c =  v0;
       *d =  p0;
#endif
}

float calcolaSpline(float t, float a, float b, float c, float d) {
  return a * t * t * t + b * t * t + c * t + d;
}

void homing() {
  int passi;

  /* porta il motore BLU a toccare il finecorsa */
  passi = 0;
  while ((passi < MAX_PASSI_HOMING) && (digitalRead(HOMING_X) != 0)) {
    // GIRA BLU   CCW
    digitalWrite( PIN_MOT1_DIR,  HIGH );
    delay(1);
    digitalWrite( PIN_MOT1_PUL, HIGH );
    delay(1);
    digitalWrite( PIN_MOT1_PUL, LOW );
    passi ++;
  }

  /* porta il motore VERDE a toccare il finecorsa */
  passi = 0;
  while ((passi < MAX_PASSI_HOMING) && (digitalRead(HOMING_Y) != 0)) {
    // GIRA VERDE  CCW
    digitalWrite( PIN_MOT2_DIR,  HIGH );
    delay(1);
    digitalWrite( PIN_MOT2_PUL, HIGH );
    delay(1);
    digitalWrite( PIN_MOT2_PUL, LOW );
    passi ++;
  }

  /* porta il motore FUCSIA a toccare il finecorsa */
  passi = 0;
  while ((passi < MAX_PASSI_HOMING) && (digitalRead(HOMING_Z) != 0)) {
    // GIRA FUCSIA  CCW
    digitalWrite( PIN_MOT3_DIR,  HIGH );
    delay(1);
    digitalWrite( PIN_MOT3_PUL, HIGH );
    delay(1);
    digitalWrite( PIN_MOT3_PUL, LOW );
    passi ++;
  }
}

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
  
  pinMode( HOMING_X,    INPUT_PULLUP);
  pinMode( HOMING_Y,    INPUT_PULLUP);
  pinMode( HOMING_Z,    INPUT_PULLUP);

  pos_mot1_presente = 0;
  pos_mot2_presente = 0;
  pos_mot3_presente = 0;
  pos_mot1_futura = 0;
  pos_mot2_futura = 0;
  pos_mot3_futura = 0;


  /* LETTURA DEL PERCORSO DA MICRO SD */
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

  pathFile.close();

  numero_passi = i;
  passo_in_esecuzione = 0;
  passo_precedente = i-1;

  /* inizializzazione parametri spline */
  t0 = millis();
  
  vel0[0] = 0;                                             // velocita X di partenza
  vel1[0] = ( percorso[2][0] - percorso[0][0] ) / (2*T);   // velocita X di arrivo secondo 
                                                           //    la regola di Catmull-Rom
  calcolaCoefficientiSpline( percorso[0][0], percorso[1][0], vel0[0], vel1[0],
                             &coefX[0], &coefX[1], &coefX[2], &coefX[3] );

  vel0[1] = 0;                                             // velocita Y di partenza
  vel1[1] = ( percorso[2][1] - percorso[0][1] ) / (2*T);   // velocita Y di arrivo secondo 
                                                           //    la regola di Catmull-Rom
  calcolaCoefficientiSpline( percorso[0][1], percorso[1][1], vel0[1], vel1[1], 
                             &coefY[0], &coefY[1], &coefY[2], &coefY[3] );

  vel0[2] = 0;                                             // velocita Z di partenza
  vel1[2] = ( percorso[2][2] - percorso[0][2] ) / (2*T);   // velocita Z di arrivo secondo 
                                                           //    la regola di Catmull-Rom
  calcolaCoefficientiSpline( percorso[0][2], percorso[1][2], vel0[2], vel1[2],
                             &coefZ[0], &coefZ[1], &coefZ[2], &coefZ[3] );
  

  

  /* va alla ricerca dei finecorsa per stabilire il punto iniziale */
  homing();
}

void loop() {
  float t;

  if (DEBUG_MODE) {
    Serial.print(pos_mot1_presente);
    Serial.print(", ");
    Serial.print(pos_mot2_presente);
    Serial.print(", ");
    Serial.println(pos_mot3_presente);
  }

  /* CALCOLO DELLA POSIZIONE DEI MOTORI */
  t = (millis() - t0) / RISOLUZIONE_SPLINE;

  /* devo andare da percorso [passo_precedente] a percorso[passo_in_esecuzione] */
  pos_mot1_futura = calcolaSpline(t, coefX[0], coefX[1], coefX[2], coefX[3]);
  pos_mot2_futura = calcolaSpline(t, coefY[0], coefY[1], coefY[2], coefY[3]);
  pos_mot3_futura = calcolaSpline(t, coefZ[0], coefZ[1], coefZ[2], coefZ[3]);
  
  if ( (millis() - t0) >= DELTA_T_MS ) {
    t0 = millis();
        
    vel0[0] = vel1[0];                                         // velocita X di partenza
    vel1[0] = ( percorso[(passo_in_esecuzione + 2)%numero_passi][0] - 
                 percorso[passo_in_esecuzione][0] ) / (2*T);   // velocita X di arrivo secondo 
                                                               //    la regola di Catmull-Rom
    vel0[1] = vel1[1];                                         // velocita Y di partenza
    vel1[1] = ( percorso[(passo_in_esecuzione + 2)%numero_passi][1] - 
                 percorso[passo_in_esecuzione][1] ) / (2*T);   // velocita Y di arrivo secondo 
                                                               //    la regola di Catmull-Rom
    vel0[2] = vel1[2];                                         // velocita Z di partenza
    vel1[2] = ( percorso[(passo_in_esecuzione + 2)%numero_passi][2] - 
                 percorso[passo_in_esecuzione][2] ) / (2*T);   // velocita di arrivo secondo 
                                                               //    la regola di Catmull-Rom
                                                       
    calcolaCoefficientiSpline( 
      percorso[passo_in_esecuzione][0], percorso[(passo_in_esecuzione + 1)%numero_passi][0], 
      vel0[0], vel1[0], &coefX[0], &coefX[1], &coefX[2], &coefX[3] );
    calcolaCoefficientiSpline( 
      percorso[passo_in_esecuzione][1], percorso[(passo_in_esecuzione + 1)%numero_passi][1], 
      vel0[1], vel1[1], &coefY[0], &coefY[1], &coefY[2], &coefY[3] );
    calcolaCoefficientiSpline( 
      percorso[passo_in_esecuzione][2], percorso[(passo_in_esecuzione + 1)%numero_passi][2], 
      vel0[2], vel1[2], &coefZ[0], &coefZ[1], &coefZ[2], &coefZ[3] );
      
    passo_in_esecuzione = (passo_in_esecuzione + 1)%numero_passi;
  }

  /* MOVIMENTO DEI MOTORI */
  if (pos_mot1_futura > pos_mot1_presente) {
    // GIRA BLU   CW
    digitalWrite( PIN_MOT1_DIR,  LOW );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT1_PUL, HIGH );
    pos_mot1_presente ++;
  }

  if (pos_mot1_futura < pos_mot1_presente) {
    // GIRA BLU   CCW
    digitalWrite( PIN_MOT1_DIR,  HIGH );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT1_PUL, HIGH );
    pos_mot1_presente --;
  }

  if (pos_mot2_futura > pos_mot2_presente) {
    // GIRA VERDE CW
    digitalWrite( PIN_MOT2_DIR,  LOW );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT2_PUL, HIGH );
    pos_mot2_presente ++;
  }

  if (pos_mot2_futura < pos_mot2_presente) {
    // GIRA VERDE CCW
    digitalWrite( PIN_MOT2_DIR,  HIGH );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT2_PUL, HIGH );
    pos_mot2_presente --;
  }

  if (pos_mot3_futura > pos_mot3_presente) {
    // GIRA FUCSIA   CW
    digitalWrite( PIN_MOT3_DIR,  LOW );
    delayMicroseconds(10);
    digitalWrite( PIN_MOT3_PUL, HIGH );
    pos_mot3_presente ++;
  }

  if (pos_mot3_futura < pos_mot3_presente) {
    // GIRA FUCSIA   CCW
    digitalWrite( PIN_MOT3_DIR,  HIGH );
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
