#define PWM1A              9
#define PWM1B             10

#define SERVOMOTORE1      PWM1B
#define SERVOMOTORE2      PWM1A

#define SONAR_TRIG        18  /* A4 */ 
#define SONAR_ECHO        19  /* A5 */ 

#include <Servo.h>

Servo servo1;
Servo servo2;

int angolo_sonar;
unsigned long distanza_sonar; 
bool direzione_sonar;           // true = avanti, false = indietro;

unsigned long sonar_t0, sonar_t1; 

void initSonar() {
  // i pwm non serve definirli

  servo1.attach( SERVOMOTORE1 );
  // servo2.attach( SERVOMOTORE2 );

  angolo_sonar = 90;
  servo1.write( angolo_sonar );
  
  pinMode ( SONAR_TRIG , OUTPUT );
  pinMode ( SONAR_ECHO , INPUT );
  digitalWrite ( SONAR_TRIG , LOW );
  distanza_sonar = 0;
  sonar_t0 = 0;
  sonar_t1 = 0; 
  direzione_sonar = true;
  
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(SONAR_ECHO), misuraSonar, CHANGE);
  
  delay(100);

  /* Fai la prima scansione */
  digitalWrite( SONAR_TRIG, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( SONAR_TRIG, LOW );
  
}

void misuraSonar() {

  if (digitalRead(SONAR_ECHO) == HIGH) { 
    sonar_t0 = micros(); 
  }
  else /* if (digitalRead(SONAR_ECHO) == LOW) */ {
    sonar_t1 = micros();

    /* calcolo distanza */
    distanza_sonar = 1 + (sonar_t1 - sonar_t0) / 6;  /* approssimata ed espressa in millimetri */  
  }  
}

void sonarMisuraDistanza(int angolo) {
  
  servo1.write( angolo );
  angolo_sonar = angolo;

  distanza_sonar = 0;
  sonar_t0 = 0;
  sonar_t1 = 0; 
  
  /* Fai la prima scansione */
  digitalWrite( SONAR_TRIG, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( SONAR_TRIG, LOW );
  
}

void sonarScansione(int incremento_angolo, int apertura_gradi) {

  if (direzione_sonar) { // avanti
    angolo_sonar += incremento_angolo;

    if (angolo_sonar >= (90 + apertura_gradi)) {
      direzione_sonar = false;
      angolo_sonar = (90 + apertura_gradi);
    }
  }
  else { // indietro
    angolo_sonar -= incremento_angolo;

    if (angolo_sonar <= (90 - apertura_gradi)) {
      direzione_sonar = true;
      angolo_sonar = (90 - apertura_gradi);
    }
  }

  sonarMisuraDistanza( angolo_sonar );
}
