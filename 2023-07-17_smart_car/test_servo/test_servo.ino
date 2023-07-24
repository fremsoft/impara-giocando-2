/* 
 *  Questo Sketch serve a testare le funzionalità
 *  del servomotore SG90 con shield MH Electronics.
 *  
 *  Al pin 10 è connesso 
 *  
 *  Visualizza l'esperienza di laboratorio completa:  
 *  https://youtube.com/live/7llBNC1qa5o
 *  
 */

#include <Servo.h>
Servo servo1;
  
#define PWM1B              10
#define SERVOMOTORE        PWM1B

unsigned int angolo;

void setup() {
  
  servo1.attach( SERVOMOTORE );
  angolo = 30;

}

   
void loop() {
   
   angolo++;
   if (angolo > 150) { angolo = 0; }
   servo1.write( angolo );

   delay(50);

}
