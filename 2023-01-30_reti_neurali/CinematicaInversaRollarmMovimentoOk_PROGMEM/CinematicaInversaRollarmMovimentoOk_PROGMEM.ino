/* 
 *  Determinazione dei due angoli Alfa Beta e Gamma mediante machine learning
 *  Beta è determinato analiticamente come arcotangente di Y/X e ridotto 
 *  nel range -PI/2 e PI/2.
 *
 *  VEDI LA LEZIONE COMPLETA : https://youtube.com/live/5trqQZzrbJU
 */

/* Neural Network */
#define PI            3.1415927 

#define NumberOf(arg) ((unsigned int)(sizeof(arg) / sizeof(arg[0])))

#define NO_TRAIN

#ifdef YES_TRAIN
#define _1_OPTIMIZE B00010000  //  https://github.com/GiorgosXou/NeuralNetworks#macro-properties
#else
#define _1_OPTIMIZE B11010010 // https://github.com/GiorgosXou/NeuralNetworks#macro-properties
#endif

// ACTIVATION FUNCTION
//#define SELU                 
#define Tanh                 
//#define ELU                  
//#define RELU
//#define LeakyELU

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

#define NORMALIZATION_FACTOR_ABG 2000.0
#define NORMALIZATION_FACTOR_MM  200.0

const unsigned int layers[] = { 2, 18, 9, 2 };  // 4 layers (1st)layer with 2 input neurons (xy, z) (2nd, 3rd)layer with 18,9 hidden neurons and (4th)layer with 2 output neuron (alfa, gamma)
float *output;                                  // 4th layer's output(s)

#ifndef YES_TRAIN
// It is 1 for each layer [Pretrained Biases ]
const PROGMEM float biases[] = { 1.0000000000, 1.0000000000, 0.1088151216 };

// It is 2*18 + 18*9 + 9*2  [Pretrained weights]
const PROGMEM float weights[] = { 
-0.6860645294, -0.3749949693, 
 0.3914447784,  0.7889215946, 
-0.7039104461, -0.6927099227, 
-0.5241856575, -0.7933715820, 
-0.3838203430, -0.4641043663, 
-0.2249741077,  0.7241969108, 
 0.5858575820,  0.1284338951, 
 0.4187087059,  0.0710988759, 
-1.6099979877,  0.8499732971, 
-0.4333911895, -0.1684107875, 
 0.5437389373, -0.4002852916, 
 0.6091814517, -0.2785488128, 
-0.5476961612, -0.2345921516, 
-0.4423710823, -0.0154347109, 
-0.8880438804, -0.0287045478, 
 0.1766935110, -0.1036068201, 
-0.1053358435, -0.5731315135, 
 0.6798972129,  1.0644047260
, 
 0.3086471796, -0.3109565973,  0.0648807048,  0.2874043273, -0.3816649913, -0.3955187606,  0.3118038177,  0.6911504745,  0.7527854442, -0.5841815471,  0.3501777172,  0.8388099670,  0.1004115939,  0.1618442440,  0.2485745191,  0.1097084407,  0.4659520149,  0.4768974304, 
-0.6257753372,  0.2440698385, -0.3977744865, -0.2348012208, -0.4911894798,  0.2527439117,  0.9302192687, -0.4005730152,  0.1825352287, -0.5461310863, -0.6077417850, -0.1092561483, -0.5309916019, -0.0318142747, -0.4886993408,  0.7103791236,  0.2250561237,  1.1712387800, 
 0.3861493587, -0.3881358909, -0.6172375202, -0.7486250400, -0.1908398246,  0.0576294374, -0.3694759607,  0.8390612602, -0.5832667350,  0.3866434097, -0.2167572736,  0.2904973983,  0.2034901618,  0.4989038467,  0.0135941088,  0.1374692201,  0.9518417358, -0.7591569900, 
-0.6948287487,  0.0732175445,  0.5264945507,  0.2208582878, -0.3914394855,  0.7598209381, -0.3210011720,  0.2055653572, -0.9400753021,  0.6300000667, -0.3781172752, -0.0443834972,  0.1141472578,  0.0345332050,  0.7594641685, -0.6815858840, -0.7170318603, -0.3267737627, 
-0.4909605979, -0.7426207065,  0.7080261707, -0.0472348737, -0.1222160339,  0.7201554775,  0.4062203884,  0.7355307579, -0.7408422470, -0.5174956321,  0.5552189350, -0.8212275505,  0.0855430603,  0.8154432296,  0.4972657680,  0.3525476455,  0.2515549898, -0.0599063968, 
 0.1551790809, -0.4115454196, -0.4727065563, -0.1997239875, -0.4860182762, -0.2149386167,  0.0549816322,  0.4696039676,  0.7365360260,  0.4712077140, -0.7334231853, -0.7427972793, -0.0808046722, -0.6239738941, -0.2957884025, -0.1248321294, -0.7856171131,  0.0796654319, 
-0.3038415908,  0.5134655952,  0.8496555328,  0.0089497861,  0.8515346527, -0.4449630737, -0.7231344699, -0.3416546583,  0.5593485355, -0.5110959529,  0.5632272720, -0.0193689556,  0.4932916641,  0.5295286655, -0.1017368555, -0.8271635055,  0.6694114208, -0.7866168498, 
 0.2885865020,  0.6228966236,  0.0872350406, -0.1664623641, -0.2837353706, -0.3135255813,  0.8344362258, -0.4896384239, -1.0012897253, -0.5609952926,  0.3166217803, -0.7099394798,  0.5236249446,  0.2475618362, -0.8384284973,  0.7673512458, -0.1695421314, -0.9127489089, 
 0.8519000053, -0.6679534435, -0.6927726745, -0.3048113822, -0.0593425703, -0.9324775695,  0.6305344104,  0.6473044872, -0.8948010444, -0.0683952045, -0.2069406509, -0.3706054449,  0.8561906814,  0.6952116489,  0.0892434978,  0.5397724151, -0.4091213226,  0.6014554977
, 
 0.2643215417, -0.7811936855,  0.5641304969,  0.1137965440, -0.2659602165,  0.0603535652, -0.1438084697,  0.1949425792,  0.6717787742, 
-0.1510553121, -0.3355524778, -0.0954658412,  0.1002910852,  0.5658152580,  0.4892635822, -0.3566356182,  0.2192324161,  0.7625902175
 };

#endif

float inputs[2];

#ifdef YES_TRAIN
float expectedOutput[2];
float mse;
NeuralNetwork NN(layers, NumberOf(layers));  // Creating a NeuralNetwork with default learning-rates
#else
NeuralNetwork NN(layers, weights, biases, NumberOf(layers)); // Creating a NeuralNetwork with pretrained Weights and Biases
#endif

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

#define DELTA_T_MS    5000


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
  long j = 0, n;
  Serial.begin(115200);

  NN.LearningRateOfWeights = 0.1;
#ifdef YES_TRAIN
  do {
    for (n = 0; (n < 100); n++) { // Epoch
      /* posizione corrente: partenza a caso */
      alfa1024 = random(100, 1000);
      beta1024 = random(100, 1000);
      gamma1024 = random(100, 1000);

      /* normalizzazione */
      expectedOutput[0] = alfa1024 / NORMALIZATION_FACTOR_ABG;
      //expectedOutput[1] = beta1024 / NORMALIZATION_FACTOR_ABG;
      expectedOutput[1] = gamma1024 / NORMALIZATION_FACTOR_ABG;

      /* cinematica diretta */
      polarToRect(round(alfa1024), round(beta1024), round(gamma1024), &xmm, &ymm, &zmm);
   
      /* normalizzazione */
      inputs[0] = sqrt((xmm*xmm) + (ymm*ymm))/NORMALIZATION_FACTOR_MM; // meta coordinata
      inputs[1] = zmm/NORMALIZATION_FACTOR_MM;

      output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally
 
      NN.BackProp(expectedOutput);  // "Tells" to the NN if the output was the-expected-correct one | then, "teaches" it
    }
    mse =                NN.getMeanSqrdError(n);
    //mse = (mse * 0.9) + (NN.getMeanSqrdError(n) * 0.1);
    
    j = j + n;
    Serial.print(j);
    Serial.print(": alfa ");
    Serial.print(output[0] * NORMALIZATION_FACTOR_ABG, 0);  
    Serial.print(" (");
    Serial.print(alfa1024, 0);  
    Serial.print("), beta ");
    /*Serial.print(output[1] * NORMALIZATION_FACTOR_ABG, 0); */
    Serial.print(" (");
    Serial.print(beta1024, 0);  
    Serial.print("), gamma ");
    Serial.print(output[1] * NORMALIZATION_FACTOR_ABG, 0); 
    Serial.print(" (");
    Serial.print(gamma1024, 0);  
    Serial.print(")");  

    Serial.print("\tx: ");
    Serial.print(xmm, 2); /* 7 cifre dopo la virgola */
    Serial.print(", y: ");
    Serial.print(ymm, 2); /* 7 cifre dopo la virgola */
    Serial.print(", z: ");
    Serial.print(zmm, 2); /* 7 cifre dopo la virgola */
    Serial.print("  \tMSE: ");
    Serial.println(mse, 7); /* 7 cifre dopo la virgola */
    
  } while ((j < 1000000) && (mse > 0.001));

  NN.print();
  
  Serial.println("\n LEARNING COMPLETE, PRESS ANY KEY TO CONTINUE");

#else

  Serial.println("\n PRE-TRAINED NETWORK, PRESS ANY KEY TO CONTINUE");

#endif

  while ( Serial.available() == 0 ) /* do nothing */;
  while ( Serial.available() != 0 ) Serial.read();

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
  

  /* normalizzazione */
  inputs[0] = sqrt((x2mm*x2mm) + (y2mm*y2mm))/NORMALIZATION_FACTOR_MM; // meta coordinata
  inputs[1] = z2mm/NORMALIZATION_FACTOR_MM;

  output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally

  alfa1024 =  output[0] * NORMALIZATION_FACTOR_ABG;
  //beta1024 =  output[n] * NORMALIZATION_FACTOR_ABG;
  gamma1024 = output[1] * NORMALIZATION_FACTOR_ABG; 
 
  beta1024 = map((atan2(y2mm, x2mm) * 18000)/ PI, -9000, 9000, 0, 1023);
  
  Serial.print(alfa1024);
  Serial.print(",\t");
  Serial.print(beta1024);
  Serial.print(",\t");
  Serial.println(gamma1024);
  
  writeServo();
}
