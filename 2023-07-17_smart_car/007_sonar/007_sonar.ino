/*
    Questo Sketch serve a controllare la velocità
    e la posizione delle ruote della Smart Car
    mediante lo shield di controllo moroti DC della
    MH Electronics.

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
    https://youtube.com/live/9Mn1FpII_dI

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

void setup() {

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

  tempoRitardoControllo = millis();

  velocita = 0;
}

void loop() {

  /* VERIFICARE A POLLING GLI INGRESSI ENCODER */
  checkEncoder();

  noInterrupts();
  int v[4] = { velocitaRPM[0], velocitaRPM[1], velocitaRPM[2], velocitaRPM[3] };
  long p[4] = { posizione[0],  posizione[1],   posizione[2],   posizione[3] };
  int a = angolo_sonar;
  unsigned long d = distanza_sonar; 
  interrupts();

  /* controllo velocita ruote */
  if ((millis() - tempoRitardoControllo) > RITARDO_CONTROLLO_MS) {
    tempoRitardoControllo = millis();

    for (int nm = 0; nm < 4; nm++) {
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

  /*
  Serial.print(p[0]);
  Serial.print(",");
  //Serial.println(pwmRuota[0]);
  Serial.print(p[1]);
  Serial.print(",");
  Serial.print(p[2]);
  Serial.print(",");
  Serial.println(p[3]);
  */

  /* controllo sonar */
  if ((millis() - tempoRitardoSonar) > RITARDO_SONAR_MS) {
    tempoRitardoSonar = millis();

    if (d != 0) {
      Serial.print(a);
      Serial.print(",");
      Serial.println(d);
  
      sonarScansione(INCREMENTO_GRADI_SONAR, APERTURA_GRADI_SONAR);
    }
  }
  

}
