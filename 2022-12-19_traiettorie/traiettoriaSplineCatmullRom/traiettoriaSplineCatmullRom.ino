/* 
 *  Spline Cubica Catmull-Rom
 *  VEDI LA LEZIONE COMPLETA :  https://youtu.be/f8f3Ch6ZPG0
 *  
 *  Appunti: http://www.mat.unimi.it/users/alzati/Geometria_Computazionale_98-99/apps/interpolanti/teoria.html
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

#define DELTA_T_MS          10000
#define RISOLUZIONE_SPLINE    100.0

const float  T = DELTA_T_MS / RISOLUZIONE_SPLINE;

int potenziometro[4];
int alfa1024, beta1024, gamma1024;
float xmm, ymm, zmm;
int tempoPressionePulsanteDS;

float punto0[3], punto1[3], punto2[3], punto3[3];
unsigned long t0, t1;
float vel0[3], vel1[3]; /* velocità di partenza e di arrivo del singolo tratto della spline*/
int   stato_movimento;
float coefX[4], coefY[4], coefZ[4];

  
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

  punto0[0] = xmm - 30; // x del punto 0
  punto0[1] = ymm - 30; // y del punto 0
  punto0[2] = zmm; // z del punto 0

  punto1[0] = xmm + 30; // x del punto 1
  punto1[1] = ymm - 30; // y del punto 1
  punto1[2] = zmm; // z del punto 1

  punto2[0] = xmm + 30; // x del punto 0
  punto2[1] = ymm + 30; // y del punto 0
  punto2[2] = zmm; // z del punto 0

  punto3[0] = xmm - 30; // x del punto 1
  punto3[1] = ymm + 30; // y del punto 1
  punto3[2] = zmm; // z del punto 1

  t0 = millis();
  
  vel0[0] = 0;                                   // velocita X di partenza
  vel1[0] = ( punto2[0] - punto0[0] ) / (2*T);   // velocita X di arrivo secondo 
                                                 //    la regola di Catmull-Rom
  calcolaCoefficientiSpline( punto0[0], punto1[0], vel0[0], vel1[0],
                             &coefX[0], &coefX[1], &coefX[2], &coefX[3] );

  vel0[1] = 0;                                   // velocita Y di partenza
  vel1[1] = ( punto2[1] - punto0[1] ) / (2*T);   // velocita Y di arrivo secondo 
                                                 //    la regola di Catmull-Rom
  calcolaCoefficientiSpline( punto0[1], punto1[1], vel0[1], vel1[1], 
                             &coefY[0], &coefY[1], &coefY[2], &coefY[3] );

  vel0[2] = 0;                                   // velocita Z di partenza
  vel1[2] = ( punto2[2] - punto0[2] ) / (2*T);   // velocita Z di arrivo secondo 
                                                 //    la regola di Catmull-Rom
  calcolaCoefficientiSpline( punto0[2], punto1[2], vel0[2], vel1[2],
                             &coefZ[0], &coefZ[1], &coefZ[2], &coefZ[3] );
  stato_movimento = 1;
  
  Serial.println( "Tutti i coefficienti X" );
  Serial.println( punto0[0] );
  Serial.println( punto1[0] );
  Serial.println( punto2[0] );
  Serial.println( vel0[0] );
  Serial.println( vel1[0] );
  Serial.println( coefX[0],15 );
  Serial.println( coefX[1],15 );
  Serial.println( coefX[2],15 );
  Serial.println( coefX[3],15 );
  
}

/*
 - loop function
 * ---------------------------------------------------------------------------*/
void loop() 
{
  float t, x2mm, y2mm, z2mm;

  readPot();   // leggo il potenziometro solo per la pinza
  
  /*        
    x2mm = map(potenziometro[0], 0, 1023,   30, 100);
    y2mm = map(potenziometro[1], 0, 1023, -100, 100);
    z2mm = map(potenziometro[2], 0, 1023,    0, 150);
  */
  t = (millis() - t0) / RISOLUZIONE_SPLINE;
  
  switch (stato_movimento) {
    case 1:
      /* devo andare da Punto0 a Punto1 */
      x2mm = calcolaSpline(t, coefX[0], coefX[1], coefX[2], coefX[3]);
      y2mm = calcolaSpline(t, coefY[0], coefY[1], coefY[2], coefY[3]);
      z2mm = calcolaSpline(t, coefZ[0], coefZ[1], coefZ[2], coefZ[3]);
  
      if ( (millis() - t0) >= DELTA_T_MS ) {
        t0 = millis();
        
        vel0[0] = vel1[0];                             // velocita X di partenza
        vel1[0] = ( punto3[0] - punto1[0] ) / (2*T);   // velocita X di arrivo secondo 
                                                       //    la regola di Catmull-Rom
        vel0[1] = vel1[1];                             // velocita Y di partenza
        vel1[1] = ( punto3[1] - punto1[1] ) / (2*T);   // velocita Y di arrivo secondo 
                                                       //    la regola di Catmull-Rom
        vel0[2] = vel1[2];                             // velocita Z di partenza
        vel1[2] = ( punto3[2] - punto1[2] ) / (2*T);   // velocita di arrivo secondo 
                                                       //    la regola di Catmull-Rom
                                                       
        calcolaCoefficientiSpline( punto1[0], punto2[0], vel0[0], vel1[0],
             &coefX[0], &coefX[1], &coefX[2], &coefX[3] );
        calcolaCoefficientiSpline( punto1[1], punto2[1], vel0[1], vel1[1],
             &coefY[0], &coefY[1], &coefY[2], &coefY[3] );
        calcolaCoefficientiSpline( punto1[2], punto2[2], vel0[2], vel1[2],
             &coefZ[0], &coefZ[1], &coefZ[2], &coefZ[3] );
        stato_movimento = 2;
      }
      break;

    case 2:
      /* devo andare da Punto1 a Punto2 */
      x2mm = calcolaSpline(t, coefX[0], coefX[1], coefX[2], coefX[3]);
      y2mm = calcolaSpline(t, coefY[0], coefY[1], coefY[2], coefY[3]);
      z2mm = calcolaSpline(t, coefZ[0], coefZ[1], coefZ[2], coefZ[3]);
  
      if ( (millis() - t0) >= DELTA_T_MS ) {
        t0 = millis();
        
        vel0[0] = vel1[0];                             // velocita X di partenza
        vel1[0] = ( punto0[0] - punto2[0] ) / (2*T);   // velocita X di arrivo secondo 
                                                       //    la regola di Catmull-Rom
        vel0[1] = vel1[1];                             // velocita Y di partenza
        vel1[1] = ( punto0[1] - punto2[1] ) / (2*T);   // velocita Y di arrivo secondo 
                                                       //    la regola di Catmull-Rom
        vel0[2] = vel1[2];                             // velocita Z di partenza
        vel1[2] = ( punto0[2] - punto2[2] ) / (2*T);   // velocita di arrivo secondo 
                                                       //    la regola di Catmull-Rom
                                                       
        calcolaCoefficientiSpline( punto2[0], punto3[0], vel0[0], vel1[0],
             &coefX[0], &coefX[1], &coefX[2], &coefX[3] );
        calcolaCoefficientiSpline( punto2[1], punto3[1], vel0[1], vel1[1],
             &coefY[0], &coefY[1], &coefY[2], &coefY[3] );
        calcolaCoefficientiSpline( punto2[2], punto3[2], vel0[2], vel1[2],
             &coefZ[0], &coefZ[1], &coefZ[2], &coefZ[3] );

        stato_movimento = 3;
      }
      break;

    case 3:
      /* devo andare da Punto2 a Punto3 */
      x2mm = calcolaSpline(t, coefX[0], coefX[1], coefX[2], coefX[3]);
      y2mm = calcolaSpline(t, coefY[0], coefY[1], coefY[2], coefY[3]);
      z2mm = calcolaSpline(t, coefZ[0], coefZ[1], coefZ[2], coefZ[3]);
  
      if ( (millis() - t0) >= DELTA_T_MS ) {
        t0 = millis();
        
        vel0[0] = vel1[0];                             // velocita X di partenza
        vel1[0] = 0;                                   // velocita X di arrivo  fermo
        vel0[1] = vel1[1];                             // velocita Y di partenza
        vel1[1] = 0;                                   // velocita Y di arrivo secondo 
        vel0[2] = vel1[2];                             // velocita Z di partenza
        vel1[2] = 0;                                   // velocita di arrivo secondo 
                                                       
        calcolaCoefficientiSpline( punto3[0], punto0[0], vel0[0], vel1[0],
             &coefX[0], &coefX[1], &coefX[2], &coefX[3] );
        calcolaCoefficientiSpline( punto3[1], punto0[1], vel0[1], vel1[1],
             &coefY[0], &coefY[1], &coefY[2], &coefY[3] );
        calcolaCoefficientiSpline( punto3[2], punto0[2], vel0[2], vel1[2],
             &coefZ[0], &coefZ[1], &coefZ[2], &coefZ[3] );

        stato_movimento = 4;
      }
      break;

    default:
      /* devo andare da Punto3 a Punto0 */
      x2mm = calcolaSpline(t, coefX[0], coefX[1], coefX[2], coefX[3]);
      y2mm = calcolaSpline(t, coefY[0], coefY[1], coefY[2], coefY[3]);
      z2mm = calcolaSpline(t, coefZ[0], coefZ[1], coefZ[2], coefZ[3]);
  
      if ( (millis() - t0) >= DELTA_T_MS ) {
        t0 = millis();
        
        vel0[0] = vel1[0];                             // velocita X di partenza
        vel1[0] = ( punto2[0] - punto0[0] ) / (2*T);   // velocita X di arrivo secondo 
                                                       //    la regola di Catmull-Rom
        vel0[1] = vel1[1];                             // velocita Y di partenza
        vel1[1] = ( punto2[1] - punto0[1] ) / (2*T);   // velocita Y di arrivo secondo 
                                                       //    la regola di Catmull-Rom
        vel0[2] = vel1[2];                             // velocita Z di partenza
        vel1[2] = ( punto2[2] - punto0[2] ) / (2*T);   // velocita di arrivo secondo 
                                                       //    la regola di Catmull-Rom
                                                       
        calcolaCoefficientiSpline( punto0[0], punto1[0], vel0[0], vel1[0],
             &coefX[0], &coefX[1], &coefX[2], &coefX[3] );
        calcolaCoefficientiSpline( punto0[1], punto1[1], vel0[1], vel1[1],
             &coefY[0], &coefY[1], &coefY[2], &coefY[3] );
        calcolaCoefficientiSpline( punto0[2], punto1[2], vel0[2], vel1[2],
             &coefZ[0], &coefZ[1], &coefZ[2], &coefZ[3] );

        stato_movimento = 1;
      }
      break;
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
