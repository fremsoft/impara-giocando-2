/* 
 *  Questo Sketch serve a testare le funzionalità
 *  dello shield di controllo moroti DC della  
 *  MH Electronics.
 *  
 *  Al pin  4 è connesso DIR_CLK
 *  Al pin  7 è connesso DIR_EN (attivo basso)
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
 *  Negli ingressi analogici A0..A3 (corrispondenti a 14..17)
 *  sono collegati dei sensori che rilevano 20 impulsi per giro.
 *  Questo programma li gestisce a cadenza regolare, usando un 
 *  interrupt agganciato al timer2 che produce un interrupt ogni 
 *  2 millisec (490 Hz), perciò è completamente autonomo anche 
 *  se il loop impegna molte risorse, ma non è troppo preciso 
 *  sugli alti valori di RPM. 
 *  
 *  Visualizza l'esperienza di laboratorio completa:  
 *  https://youtube.com/live/3C4hxCGg-NI
 *  
 */

#include <Servo.h>
 
Servo servo1;
Servo servo2;

int velocita;
extern int velocitaRPM[4];

void setup() {

  Serial.begin( 9600 );
  
  /*
  servo1.attach( SERVOMOTORE1 );
  servo2.attach( SERVOMOTORE2 );
  */
  
  initMotori();
  initEncoder();
  
  setVelocitaMotore1( 0 );
  setVelocitaMotore2( 0 );
  setVelocitaMotore3( 0 );
  setVelocitaMotore4( 0 );
  
  velocita = 0;
  
}

   
void loop() {
  
   noInterrupts();
   int v[4] = { velocitaRPM[0], velocitaRPM[1], velocitaRPM[2], velocitaRPM[3] };
   interrupts();
   
   Serial.print(v[0]);
   Serial.print(",");
   Serial.print(v[1]);
   Serial.print(",");
   Serial.print(v[2]);
   Serial.print(",");
   Serial.println(v[3]);
   
}
