/* 
 *  Questo Sketch serve a testare le funzionalità
 *  dello shield di controllo moroti DC della  
 *  MH Electronics.
 *  
 *  Al pin  4 è connesso DIR_CLK
 *  Al pin  8 è connesso DIR_SER
 *  Al pin 12 è connesso DIR_LATCH
 *  
 *  Sul fronte positivo di DIR_CLK viene caricato 
 *  DIR_SER sul registro a scorrimento. 
 *  
 *  Sul fronte positivo di DIR_LATCH viene caricato 
 *  in uscita il byte intero del registro a scorrimento. 
 *  
 *  Il primo bit che carico va su M4B e di conseguenza 
 *  la successione dei segnali è la seguente:
 *  
 *  primo M4B, M3B, M4A, M2B, M1B, M1A, M2A, ultimo M3A
 *  
 *  Mentre i PWM che regolano le velocità dei motori sono 4:
 *  PWM0A - MOTORE 4 - Pin  6
 *  PWM0B - MOTORE 3 - Pin  5
 *  PWM1A - SERVO 2  - Pin  9
 *  PWM1B - SERVO 1  - Pin 10
 *  PWM2A - MOTORE 1 - Pin 11
 *  PWM2B - MOTORE 2 - Pin  3
 *  
 *  
 *  Visualizza l'esperienza di laboratorio completa:  
 *  https://youtube.com/live/7llBNC1qa5o
 *  
 */

#include <Servo.h>
 
Servo servo1;
Servo servo2;

int velocita;

void setup() {

  /*
  servo1.attach( SERVOMOTORE1 );
  servo2.attach( SERVOMOTORE2 );
  */
  
  initMotori();

  setVelocitaMotore1( 180 );
  setVelocitaMotore2( 180 );
  setVelocitaMotore3( 180 );
  setVelocitaMotore4( 180 );
  
  velocita = 0;
  
}

   
void loop() {

  /* velocita = velocita + 10;
   if ( velocita > 255 ) { velocita = -255; }

   setVelocitaMotore1( velocita );
   
   delay ( 100 );*/
   
}
