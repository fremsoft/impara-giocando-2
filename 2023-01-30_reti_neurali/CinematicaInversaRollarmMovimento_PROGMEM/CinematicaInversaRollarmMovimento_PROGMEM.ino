/* 
 *  Determinazione dei due angoli Alfa e Gamma mediante machine learning
 *  Beta è determinato analiticamente come arcotangente di Y/X e ridotto 
 *  nel range -PI/2 e PI/2.
 *
 *  VEDI LA LEZIONE COMPLETA : https://youtube.com/live/dJgejDh9f24
 */

/* Neural Network */
#define PI            3.1415927 

#define NumberOf(arg) ((unsigned int)(sizeof(arg) / sizeof(arg[0])))

#define _1_OPTIMIZE B11010010 // https://github.com/GiorgosXou/NeuralNetworks#macro-properties

//#define SELU                 // Comment   this line and ...
#define Tanh                   // Comment   this line and ...
//#define ELU                  // Uncomment this line use ELU Activation Function (way faster!)


// https://learn.sparkfun.com/tutorials/efficient-arduino-programming-with-arduino-cli-and-visual-studio-code/all
// https://www.arduino.cc/en/Hacking/libraryTutorial

#include <NeuralNetwork.h>

/*
#2 https://stackoverflow.com/questions/22318677/is-it-faster-to-have-the-compiler-initialize-an-array-or-to-manually-iterate-over  [I am slightly confused. Anyways...]
#4 https://stackoverflow.com/questions/65860934/undefined-reference-to-outterclassfunction-ptrs-variable-issue

#5 (Most Probably) It will be Optimised by the compiler if no needed
#6 https://stackoverflow.com/questions/68689135/unusual-behavior-unnecessary-variables-inside-a-class-result-in-extra-bytes-of

In Arduino log() = ln = natural logarithm = logarithm with base e 
*/

#define NORMALIZATION_FACTOR 2000.0

const unsigned int layers[] = { 4, 9, 9, 9, 2 };  // 5 layers (1st)layer with 4 input neurons (2nd, 3rd e 4th)layer with 5,4,3 hidden neurons and (5th)layer with 2 output neuron
float *output;                                    // 5th layer's output(s)

// It is 1 for each layer [Pretrained Biases ]
const PROGMEM float biases[] = { 1.00, 1.00, 1.00, 0.91 };

// It is 4*9 + 9*9 + 9*9 + 9*2  [Pretrained weights]
const PROGMEM float weights[] = {

  -0.9578217,  -0.5073918,   0.4170873,   0.4659361, 
  -0.4961844,  -1.0118292,  -0.4278767,  -0.7611830, 
  -0.1947305,  -0.7527140,  -0.2677412,   0.7040981, 
   0.5590309,  -0.5542565,   0.5713985,   0.1019635, 
  -0.8250143,   0.5208229,  -0.3040881,  -0.2106154, 
   0.6155302,  -0.1352160,   0.4667049,   0.1666334, 
  -0.6450900,  -0.0008336,  -0.5635149,   0.0131246, 
  -0.8559253,   0.2769034,   0.0948386,  -0.2029304, 
  -0.7459470,  -0.3611213,   0.5539211,   0.6652194, 

   0.3069395,  -0.2987047,   0.1037323,   0.3549827,  -0.3683701,  -0.3284734,   0.3279930,   0.6954072,   0.7641435, 
  -0.5899501,   0.3434079,   0.8243530,   0.0783571,   0.1540002,   0.2271716,   0.1016015,   0.4605486,   0.4695675, 
  -0.6196782,  -0.1437736,  -0.2696876,  -0.1219127,  -0.4582143,   0.0769519,   0.7206990,  -0.6076213,   0.1347731, 
  -0.5703947,  -0.9213088,  -0.3741615,  -0.6239912,   0.1101099,  -0.4944166,   0.6542152,   0.3627395,   0.8320549, 
   0.3478103,  -0.7012695,  -0.8070938,  -0.9320668,  -0.1454961,  -0.0150446,  -0.4794383,   0.8017751,  -0.4886972, 
   0.2476487,  -0.4186654,   0.0950716,   0.0842533,   0.3849435,  -0.0743717,  -0.0538742,   0.8428293,  -0.8110788, 
  -0.8104154,  -0.3526187,   0.4817098,   0.1647895,  -0.6559054,   0.8159535,  -0.5297725,  -0.0748036,  -0.8309262, 
   0.7840232,  -0.2196732,   0.0622603,   0.2107661,   0.1849653,   0.8811077,  -0.5738915,  -0.6272912,  -0.1405072, 
  -0.5484918,  -0.7986457,   0.6656484,  -0.0736683,  -0.1829200,   0.7092345,   0.3660943,   0.6853108,  -0.7807118, 
  
  -0.7089218,   0.3505632,  -0.9418933,   0.0849916,   0.9840270,   0.3128079,   0.2698024,   0.0377984,  -0.2513903, 
   0.0943922,  -0.4589234,  -0.6320737,  -0.2824728,  -0.6140467,  -0.2594922,   0.0078581,   0.4280114,   0.7612656, 
   0.4251132,  -0.6422326,  -0.6370628,  -0.2306589,  -0.6855820,  -0.4274765,  -0.0333813,  -0.6638794,   0.0196127, 
  -0.4552047,   0.4273589,   0.8199024,   0.0029596,   0.9478427,  -0.5965855,  -0.6938189,  -0.3700539,   0.1274663, 
  -0.6361356,   0.4279202,  -0.1888154,   0.2649397,   0.4194118,  -0.2730010,  -0.9542716,   0.4933559,  -0.9168027, 
   0.1499936,   0.4616910,  -0.0132925,  -0.2167551,  -0.1864863,  -0.4072025,   0.8088179,  -0.6224028,  -0.8101551, 
  -0.3767356,   0.4703036,  -0.7220701,   0.5016443,   0.0950864,  -0.5959211,   0.5648643,  -0.1710261,  -0.7486966, 
   0.8083086,  -0.6768364,  -0.6885372,  -0.3627084,  -0.0591871,  -0.9350825,   0.7158950,   0.7095555,  -0.7198171, 
  -0.1809259,  -0.3170573,  -0.3553135,   0.8699619,   0.7866451,   0.0422317,   0.6699159,  -0.4256679,   0.6454381, 

  -0.0865280,  -0.8389882,  -0.1048066,  -0.0513877,  -0.3513768,   0.1671981,  -0.1248456,   0.1855215,   0.1238820, 
   0.1844898,  -0.9331800,  -0.1883044,   0.1021163,  -0.0064168,  -0.0591175,  -0.0941320,  -0.1468130,   0.4091914
};

float inputs[4];

NeuralNetwork NN(layers, weights, biases, NumberOf(layers)); // Creating a NeuralNetwork with pretrained Weights and Biases


#include <Servo.h> 

Servo Servo_0;   /* asse 1, angolo Beta */
Servo Servo_1;   /* asse 2, angolo Alfa */
Servo Servo_2;   /* asse 3, angolo Gamma */
Servo Servo_3;

#define POTENZIOMETRO_1  0 //A0
#define POTENZIOMETRO_2  1 //A1
#define POTENZIOMETRO_3  2 //A2
#define POTENZIOMETRO_4  3 //A3

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

#define DELTA_T_MS    1000


int potenziometro[4];

/* variabili globali della movimentazione */
float alfa1024, beta1024, gamma1024;
float xmm, ymm, zmm;
int tempoPressionePulsanteDS;


float punto0[3], punto1[3], punto2[3], punto3[3];
unsigned long t0, t1;
int   stato_movimento;
float velX, velY, velZ;

float x2mm, y2mm, z2mm;
float xmmn, ymmn, zmmn;
  
void setup() {
  Serial.begin(115200);
  Serial.println("\n PRE-TRAINED NETWORK, PRESS ANY KEY TO CONTINUE");

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
  polarToRect(alfa1024, beta1024, gamma1024, &xmm, &ymm, &zmm);  
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
  velX = (punto1[0] - punto0[0])/DELTA_T_MS;
  velY = (punto1[1] - punto0[1])/DELTA_T_MS;
  velZ = (punto1[2] - punto0[2])/DELTA_T_MS;
  stato_movimento = 1;
  
  Serial.println( "Le tre velocità" );
  Serial.println( velX );
  Serial.println( velY );
  Serial.println( velZ );
}


void loop() {
  
  
  readPot();   // leggo il potenziometro solo per la pinza
  
  /*        
    x2mm = map(potenziometro[0], 0, 1023,   30, 100);
    y2mm = map(potenziometro[1], 0, 1023, -100, 100);
    z2mm = map(potenziometro[2], 0, 1023,    0, 150);
  */

  switch (stato_movimento) {
    case 1:
      /* devo andare da Punto0 a Punto1 */
      x2mm = punto0[0] + velX*(millis() - t0);
      y2mm = punto0[1] + velY*(millis() - t0);
      z2mm = punto0[2] + velZ*(millis() - t0);
  
      if ( (millis() - t0) >= DELTA_T_MS ) {
        t0 = millis();
        velX = (punto2[0] - punto1[0])/DELTA_T_MS;
        velY = (punto2[1] - punto1[1])/DELTA_T_MS;
        velZ = (punto2[2] - punto1[2])/DELTA_T_MS;
        stato_movimento = 2;
      }
      break;

    case 2:
      /* devo andare da Punto1 a Punto2 */
      x2mm = punto1[0] + velX*(millis() - t0);
      y2mm = punto1[1] + velY*(millis() - t0);
      z2mm = punto1[2] + velZ*(millis() - t0);
  
      if ( (millis() - t0) >= DELTA_T_MS ) {
        t0 = millis();
        velX = (punto3[0] - punto2[0])/DELTA_T_MS;
        velY = (punto3[1] - punto2[1])/DELTA_T_MS;
        velZ = (punto3[2] - punto2[2])/DELTA_T_MS;
        stato_movimento = 3;
      }
      break;

    case 3:
      /* devo andare da Punto2 a Punto3 */
      x2mm = punto2[0] + velX*(millis() - t0);
      y2mm = punto2[1] + velY*(millis() - t0);
      z2mm = punto2[2] + velZ*(millis() - t0);
  
      if ( (millis() - t0) >= DELTA_T_MS ) {
        t0 = millis();
        velX = (punto0[0] - punto3[0])/DELTA_T_MS;
        velY = (punto0[1] - punto3[1])/DELTA_T_MS;
        velZ = (punto0[2] - punto3[2])/DELTA_T_MS;
        stato_movimento = 4;
      }
      break;

    default:
      /* devo andare da Punto3 a Punto0 */
      x2mm = punto3[0] + velX*(millis() - t0);
      y2mm = punto3[1] + velY*(millis() - t0);
      z2mm = punto3[2] + velZ*(millis() - t0);
  
      if ( (millis() - t0) >= DELTA_T_MS ) {
        t0 = millis();
        velX = (punto1[0] - punto0[0])/DELTA_T_MS;
        velY = (punto1[1] - punto0[1])/DELTA_T_MS;
        velZ = (punto1[2] - punto0[2])/DELTA_T_MS;
        stato_movimento = 1;
      }
      break;
  }  

  
  Serial.print(x2mm);
  Serial.print(",\t");
  Serial.print(y2mm);
  Serial.print(",\t");
  Serial.print(z2mm);
  Serial.print("\t-\t");
  

  //Beta è determinato analiticamente come arcotangente di Y/X e ridotto nel range -PI/2 e PI/2.
  beta1024 =  map(18000 *(atan2(y2mm, x2mm)/PI), -9000, 9000, 1, 1023);
  
  /* normalizzazione */
  inputs[0] = alfa1024 / NORMALIZATION_FACTOR;
  inputs[1] = gamma1024 / NORMALIZATION_FACTOR;

  xmmn = x2mm/NORMALIZATION_FACTOR; 
  ymmn = y2mm/NORMALIZATION_FACTOR; 
  zmmn = z2mm/NORMALIZATION_FACTOR;
  inputs[2] = (xmmn*xmmn) + (ymmn*ymmn); /* non calcolo la radice quadrata per snellire i calcoli */
  inputs[3] = zmmn;

  output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally

  alfa1024 =  output[0] * NORMALIZATION_FACTOR;
  gamma1024 = output[1] * NORMALIZATION_FACTOR; 

  Serial.print(alfa1024);
  Serial.print(",\t");
  Serial.print(beta1024);
  Serial.print(",\t");
  Serial.println(gamma1024);
  
  
  writeServo();
}
