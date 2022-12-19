/* 
 *  VEDI LA LEZIONE COMPLETA :  https://youtu.be/VfE-PyNZjaI
 */

#include <Servo.h> 

Servo Servo_0;   /* asse 1, angolo Beta */
Servo Servo_1;   /* asse 2, angolo Alfa */
Servo Servo_2;   /* asse 3, angolo Gamma */
Servo Servo_3;

#define POTENZIOMETRO_1   A0
#define POTENZIOMETRO_2   A1
#define POTENZIOMETRO_3   A2
#define POTENZIOMETRO_4   A3

#define SERVO_ASSE_1  4
#define SERVO_ASSE_2  5
#define SERVO_ASSE_3  6
#define SERVO_PINZA   7
#define PULSANTE      3

#define DELTA_ALFA    1
#define DELTA_BETA    1
#define DELTA_GAMMA   1

#define OFFSET_ALFA    0
#define OFFSET_BETA    -11
#define OFFSET_GAMMA   -10

#define DELTA_T_MS    3000


int potenziometro[4];
int alfa1024, beta1024, gamma1024;
float xmm, ymm, zmm;
int tempoPressionePulsanteDS;


float punto0[3], punto1[3];
unsigned long t0, t1;
int   stato_movimento;
float velX, velY, velZ;


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

  punto0[0] = xmm; // x del punto 0
  punto0[1] = ymm; // y del punto 0
  punto0[2] = zmm - 30; // z del punto 0

  punto1[0] = xmm; // x del punto 1
  punto1[1] = ymm; // y del punto 1
  punto1[2] = zmm + 30; // z del punto 1

  t0 = millis();
  velX = (punto1[0] - punto0[0])/DELTA_T_MS;
  velY = (punto1[1] - punto0[1])/DELTA_T_MS;
  velZ = (punto1[2] - punto0[2])/DELTA_T_MS;
  stato_movimento = 1;
  
  Serial.println( "Le tre velocità" );
  Serial.println( velX );
  Serial.println( velY );
  Serial.println( velZ );
  
}

/*
 - loop function
 * ---------------------------------------------------------------------------*/
void loop() 
{
  float x2mm, y2mm, z2mm;

  readPot();   // leggo il potenziometro solo per la pinza
  
  /*        
    x2mm = map(potenziometro[0], 0, 1023,   30, 100);
    y2mm = map(potenziometro[1], 0, 1023, -100, 100);
    z2mm = map(potenziometro[2], 0, 1023,    0, 150);
  */

  if (stato_movimento == 1) {
    /* devo andare da Punto0 a Punto1 */
    x2mm = punto0[0] + velX*(millis() - t0);
    y2mm = punto0[1] + velY*(millis() - t0);
    z2mm = punto0[2] + velZ*(millis() - t0);
  
    if ( (millis() - t0) >= DELTA_T_MS ) {
      t0 = millis();
      velX = (punto0[0] - punto1[0])/DELTA_T_MS;
      velY = (punto0[1] - punto1[1])/DELTA_T_MS;
      velZ = (punto0[2] - punto1[2])/DELTA_T_MS;
      stato_movimento = 2;
    }
  } else  /* stato_movimento == 2 */ {
    /* devo andare da Punto1 a Punto0 */
    x2mm = punto1[0] + velX*(millis() - t0);
    y2mm = punto1[1] + velY*(millis() - t0);
    z2mm = punto1[2] + velZ*(millis() - t0);
      
    if ( (millis() - t0) >= DELTA_T_MS ) {
      t0 = millis();
      velX = (punto1[0] - punto0[0])/DELTA_T_MS;
      velY = (punto1[1] - punto0[1])/DELTA_T_MS;
      velZ = (punto1[2] - punto0[2])/DELTA_T_MS;
      stato_movimento = 1;
    }
  }  

  
  Serial.print(x2mm);
  Serial.print(",\t");
  Serial.print(y2mm);
  Serial.print(",\t");
  Serial.println(z2mm);

/***************

  PRESA DIRETTA SUI POTENZIOMETRI:
  
  alfa1024 = potenziometro[1];
  beta1024 = potenziometro[0];
  gamma1024 = potenziometro[2]; 

****************/ 

  /* posizione effettiva : xmm, ymm, zmm */
  /* posizione desiderata : x2mm, y2mm, z2mm */

  int alfa1024_ok,  beta1024_ok,  gamma1024_ok;
  float errore1;

  calcoloCinematicoDiretto(alfa1024, beta1024, gamma1024, &xmm, &ymm, &zmm);
  errore1 = calcoloErrore ((x2mm - xmm), (y2mm - ymm), (z2mm - zmm));

  alfa1024_ok = alfa1024;
  beta1024_ok = beta1024;
  gamma1024_ok = gamma1024;
  
  /* TENTATIVO 1 */
  tentativoMovimento( alfa1024 + DELTA_ALFA, beta1024, gamma1024, &errore1, x2mm, y2mm, z2mm, &alfa1024_ok, &beta1024_ok, &gamma1024_ok);
  
  /* TENTATIVO 2 */
  tentativoMovimento( alfa1024 - DELTA_ALFA, beta1024, gamma1024, &errore1, x2mm, y2mm, z2mm, &alfa1024_ok, &beta1024_ok, &gamma1024_ok);
  
  /* TENTATIVO 3 */
  tentativoMovimento( alfa1024, beta1024 + DELTA_BETA, gamma1024, &errore1, x2mm, y2mm, z2mm, &alfa1024_ok, &beta1024_ok, &gamma1024_ok);
  
  /* TENTATIVO 4 */
  tentativoMovimento( alfa1024, beta1024 - DELTA_BETA, gamma1024, &errore1, x2mm, y2mm, z2mm, &alfa1024_ok, &beta1024_ok, &gamma1024_ok);

  /* TENTATIVO 5 */
  tentativoMovimento( alfa1024, beta1024, gamma1024 + DELTA_GAMMA, &errore1, x2mm, y2mm, z2mm, &alfa1024_ok, &beta1024_ok, &gamma1024_ok);
  
  /* TENTATIVO 6 */
  tentativoMovimento( alfa1024, beta1024, gamma1024 - DELTA_GAMMA, &errore1, x2mm, y2mm, z2mm, &alfa1024_ok, &beta1024_ok, &gamma1024_ok);
  
  alfa1024 =  alfa1024_ok;
  beta1024 =  beta1024_ok;
  gamma1024 = gamma1024_ok; 
  
  writeServo();  
  
}
