/*
    Questo Sketch serve a far andare la Smart Car a guida
    autonoma in giro per casa senza andare a sbattere.
    
    Il controllo della velocità e della posizione delle 
    ruote della Smart Car avviene mediante lo shield di
    controllo moroti DC della MH Electronics.

    Al pin  4 è connesso DIR_CLK
    Al pin  7 è connesso DIR_EN (attivo basso)
    Al pin  8 è connesso DIR_SER
    Al pin 12 è connesso DIR_LATCH

    Sul fronte positivo di DIR_CLK viene caricato
    DIR_SER sul registro a scorrimento.

    Sul fronte positivo di DIR_LATCH viene caricato
    in uscita il byte intero del registro a scorrimento.

    Il primo bit che carico va su M4B e di conseguenza
    la successione dei segnali è la seguente:

    primo M4B, M3B, M4A, M2B, M1B, M1A, M2A, ultimo M3A

    Mentre i PWM che regolano le velocità dei motori sono 4:
    PWM0A - MOTORE 4 - Pin  6
    PWM0B - MOTORE 3 - Pin  5
    PWM1A - SERVO 2  - Pin  9
    PWM1B - SERVO 1  - Pin 10
    PWM2A - MOTORE 1 - Pin 11
    PWM2B - MOTORE 2 - Pin  3

    Negli ingressi analogici A0..A3 (corrispondenti a 14..17)
    sono collegati dei sensori che rilevano 20 impulsi per giro.

    Questo programma li gestisce a interrupt, usando la libreria
    <PinChangeInterrupt.h> di NicoHood, perciò la precisione è
    massima anche se il loop impegna molte risorse.

    Inoltre è collegato a SERVO1 (PWM1B - 10) un 
    servomotore SG90 per far ruotare il sonar da sinistra
    a destra.

    Il sonar HC-SR04 è collegato ai pin:

    A4 - Trigger
    A5 - Echo

    Visualizza l'esperienza di laboratorio completa:
    https://youtube.com/live/xZ33SiJyQp4

*/

#include <PinChangeInterrupt.h>

int velocita;
extern int velocitaRPM[4];
int setPointVelocitaRPM[4];
int pwmRuota[4];
extern long posizione[4];

unsigned long tempoRitardoControllo;
#define RITARDO_CONTROLLO_MS 20

extern int angolo_sonar;
extern unsigned long distanza_sonar; 
unsigned long tempoRitardoSonar;
#define RITARDO_SONAR_MS          500
#define INCREMENTO_GRADI_SONAR     10
#define APERTURA_GRADI_SONAR       30
#define N_PUNTI_MISURA_LATERALI_SONAR  (APERTURA_GRADI_SONAR/INCREMENTO_GRADI_SONAR)
#define N_PUNTI_MISURA_SONAR  (1+((APERTURA_GRADI_SONAR * 2)/INCREMENTO_GRADI_SONAR))

unsigned long distanze_sonar[N_PUNTI_MISURA_SONAR]; 

enum { STAI_FERMO, VAI_A_SX, VAI_A_DX, VAI_DRITTO, GIRA_90_GRADI } cosa_fare;
#define SOGLIA_ARRIVO 2

/* definizione delle routine del ciclo principale */

void macchinaFerma() {

  cosa_fare = STAI_FERMO;
  
  pwmRuota[0] = 0;
  pwmRuota[1] = 0;
  pwmRuota[2] = 0;
  pwmRuota[3] = 0;

  setVelocitaMotore1( pwmRuota[0] );
  setVelocitaMotore2( pwmRuota[1] );
  setVelocitaMotore3( pwmRuota[2] );
  setVelocitaMotore4( pwmRuota[3] );

  setPointVelocitaRPM[0] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[1] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[2] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[3] = 0;  /* espresso in RPM */

}

void scansioneSonar() {
  int angolo, i;
  unsigned long d, t;
  
  /* prima andiamo a APERTURA_GRADI_SONAR gradi a SX */
  angolo = 90 + APERTURA_GRADI_SONAR;
  sonarMisuraDistanza( angolo );
  
  /* poi andiamo a APERTURA_GRADI_SONAR gradi a DX */
  i = 0;
  while ( angolo >= (90 - APERTURA_GRADI_SONAR) ) {
    delay(RITARDO_SONAR_MS);
    sonarMisuraDistanza( angolo );
    d = 0;

    t = millis();  /* tempo iniziale per timeout 1000ms */
    while ( (d == 0) && ((millis() - t) < 1000) ) {
      noInterrupts();
      d = distanza_sonar; 
      interrupts();
    }    
    distanze_sonar[i] = d;
    i++;

    /* ASSERT */ 
    if (i >= N_PUNTI_MISURA_SONAR) { i = N_PUNTI_MISURA_SONAR - 1; } 
       
    angolo -= INCREMENTO_GRADI_SONAR;
  }
    
  /* poi torniamo al centro */
  sonarMisuraDistanza( 90 );
}

void decisioneCosaFare() {
  int i, n;
  unsigned long sx, dx;

  /* faccio la somma delle misure di sinistra */
  for (i=0, n=0, sx=0; n<N_PUNTI_MISURA_LATERALI_SONAR; n++) {
    sx += distanze_sonar[i];
    i++;   
  }
  
  /* faccio la somma delle misure di destra */
  for (i=N_PUNTI_MISURA_LATERALI_SONAR+1, n=0, dx=0; n<N_PUNTI_MISURA_LATERALI_SONAR; n++) {
    dx += distanze_sonar[i];
    i++;   
  }

  if ( ((sx+dx+distanze_sonar[N_PUNTI_MISURA_LATERALI_SONAR]) / N_PUNTI_MISURA_SONAR) < 500 ) {
    cosa_fare = GIRA_90_GRADI;
  }
  else if ( ((100*sx)/dx) > 125 ) { 
    // SX è almeno il 25% maggiore di DX
    cosa_fare = VAI_A_SX;
  }
  else if ( ((100*dx)/sx) > 125 ) { 
    // DX è almeno il 25% maggiore di SX
    cosa_fare = VAI_A_DX;
  }
  else {
    // la distanza del prossimo ostacolo è almeno 500 mm 
    cosa_fare = VAI_DRITTO;
  }
  
}

void eseguiAzione() {

  noInterrupts();
  int v[4] = { velocitaRPM[0], velocitaRPM[1], velocitaRPM[2], velocitaRPM[3] };
  long p[4] = { posizione[0],  posizione[1],   posizione[2],   posizione[3] };
  interrupts();

  long p_start[4] = { p[0],  p[1],   p[2],   p[3] };
  long p_target[4] = { p[0],  p[1],   p[2],   p[3] };

  int n_target_ok;

  switch (cosa_fare) {
     case VAI_A_SX : 
       p_target[0] += 100;
       p_target[1] += 200;
       p_target[2] += 100;
       p_target[3] += 200;
       break;
       
     case VAI_A_DX : 
       p_target[0] += 200;
       p_target[1] += 100;
       p_target[2] += 200;
       p_target[3] += 100;
       break;

     case VAI_DRITTO :
       p_target[0] += 200;
       p_target[1] += 200;
       p_target[2] += 200;
       p_target[3] += 200;
       break;

     case GIRA_90_GRADI :  
       p_target[0] += 30;
       p_target[1] -= 30;
       p_target[2] += 30;
       p_target[3] -= 30;
       break;
  }

  n_target_ok = 0;
  while ( n_target_ok < 4 ) {

    /* VERIFICARE A POLLING GLI INGRESSI ENCODER */
    checkEncoder();

    noInterrupts();
    v[0] = velocitaRPM[0];
    v[1] = velocitaRPM[1];
    v[2] = velocitaRPM[2];
    v[3] = velocitaRPM[3];

    p[0] = posizione[0];
    p[1] = posizione[1];
    p[2] = posizione[2];
    p[3] = posizione[3];
    interrupts();

    /* controllo velocita ruote */
    if ((millis() - tempoRitardoControllo) > RITARDO_CONTROLLO_MS) {
      tempoRitardoControllo = millis();

      n_target_ok = 0;
      for (int nm = 0; nm < 4; nm++) {

        if (p[nm]-p_target[nm] > SOGLIA_ARRIVO) { setPointVelocitaRPM[nm] = -20 + (p_target[nm] - p[nm]); }
        else if (p[nm]-p_target[nm] < -SOGLIA_ARRIVO) { setPointVelocitaRPM[nm] = 20 + (p_target[nm] - p[nm]); }
        else /* (p[nm] == p_target[nm]) */ { 
          setPointVelocitaRPM[nm] = 0; 
          n_target_ok ++;
        }
        
        if (setPointVelocitaRPM[nm] > 0) {
          if (v[nm] < setPointVelocitaRPM[nm]) {
            if (pwmRuota[nm] < 255) {
              pwmRuota[nm] ++;
            }
          }
          else if (v[nm] > setPointVelocitaRPM[nm]) {
            if (pwmRuota[nm] > 0)   {
              pwmRuota[nm] --;
            }
          }
        }
        else if (setPointVelocitaRPM[nm] < 0) {
          if (v[nm] > setPointVelocitaRPM[nm]) {
            if (pwmRuota[nm] > -255) {
              pwmRuota[nm] --;
            }
          }
          else if (v[nm] < setPointVelocitaRPM[nm]) {
            if (pwmRuota[nm] < 0)   {
              pwmRuota[nm] ++;
            }
          }
        }
        else {
          pwmRuota[nm] = 0;
        }
      }
      setVelocitaMotore1( pwmRuota[0] ); setDirezione(0, pwmRuota[0]);
      setVelocitaMotore2( pwmRuota[1] ); setDirezione(1, pwmRuota[1]);
      setVelocitaMotore3( pwmRuota[2] ); setDirezione(2, pwmRuota[2]);
      setVelocitaMotore4( pwmRuota[3] ); setDirezione(3, pwmRuota[3]);
    }
  
    
    Serial.print(p[0]);
    Serial.print(",");
    //Serial.println(pwmRuota[0]);
    Serial.print(p[1]);
    Serial.print(",");
    Serial.print(p[2]);
    Serial.print(",");
    Serial.println(p[3]);
    
  }
}

void setup() {
  int i;
  
  Serial.begin( 115200 );

  initMotori();
  initEncoder();
  initSonar();

  pwmRuota[0] = 0;
  pwmRuota[1] = 0;
  pwmRuota[2] = 0;
  pwmRuota[3] = 0;

  setVelocitaMotore1( pwmRuota[0] );
  setVelocitaMotore2( pwmRuota[1] );
  setVelocitaMotore3( pwmRuota[2] );
  setVelocitaMotore4( pwmRuota[3] );

  setPointVelocitaRPM[0] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[1] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[2] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[3] = 0;  /* espresso in RPM */

  for (i=0; i<N_PUNTI_MISURA_SONAR; i++) {
    distanze_sonar[i] = 0;
  }
  
  tempoRitardoControllo = millis();

  velocita = 0;
}

void loop() {

  macchinaFerma();

  scansioneSonar();

  decisioneCosaFare();

  Serial.print(N_PUNTI_MISURA_SONAR);
  Serial.print(" - ");

  for (int i=0; i<N_PUNTI_MISURA_SONAR; i++) {
    Serial.print(distanze_sonar[i]);
    Serial.print(", ");
  }
  
  const String s[] = {"STAI_FERMO", "VAI_A_SX", "VAI_A_DX", "VAI_DRITTO", "GIRA_90_GRADI"};
  Serial.println( s[cosa_fare] );

  eseguiAzione();
  /*delay(5000);*/
 
}
