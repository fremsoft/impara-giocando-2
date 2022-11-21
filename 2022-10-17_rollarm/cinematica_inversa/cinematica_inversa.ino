/* 
 *  VEDI LA LEZIONE COMPLETA : https://youtu.be/-GhcSwgHhyY
 */

#include <Servo.h> 

Servo Servo_0;   /* asse 1, angolo Beta */
Servo Servo_1;   /* asse 2, angolo Alfa */
Servo Servo_2;   /* asse 3, angolo Gamma */
Servo Servo_3;

#define SERVO_ASSE_1  4
#define SERVO_ASSE_2  5
#define SERVO_ASSE_3  6
#define SERVO_PINZA   7
#define PULSANTE      3

int potenziometro[4];
int alfa1024, beta1024, gamma1024;
float xmm, ymm, zmm;
int tempoPressionePulsanteDS;

void setup() 
{
  Serial.begin(115200);
  
  Servo_0.attach( SERVO_ASSE_1 );
  Servo_1.attach( SERVO_ASSE_2 );
  Servo_2.attach( SERVO_ASSE_3 );
  Servo_3.attach( SERVO_PINZA );

  // PULSANTE 
  pinMode(PULSANTE, INPUT);

  tempoPressionePulsanteDS = 0;
  alfa1024 = 500;
  beta1024 = 500;
  gamma1024 = 500;  
  calcoloCinematicoDiretto(alfa1024, beta1024, gamma1024, &xmm, &ymm, &zmm);  
  writeServo();
  
}

/*
 - loop function
 * ---------------------------------------------------------------------------*/
void loop() 
{
  readPot();

  float x2mm, y2mm, z2mm;
  x2mm = map(potenziometro[0], 0, 1023,   30, 100);
  y2mm = map(potenziometro[1], 0, 1023, -100, 100);
  z2mm = map(potenziometro[2], 0, 1023,    0, 150);

  Serial.print(x2mm);
  Serial.print(",");
  Serial.print(y2mm);
  Serial.print(",");
  Serial.println(z2mm);


  alfa1024 = potenziometro[1];
  beta1024 = potenziometro[0];
  gamma1024 = potenziometro[2]; 
  calcoloCinematicoDiretto(alfa1024, beta1024, gamma1024, &xmm, &ymm, &zmm);  
  writeServo();
  
  
}
