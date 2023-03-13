/* 
 *  Scansione 3D di uno spazio circostante usando il Robot RollArm
 *  comandato usando i tre potenziometri per muovere in X, Y, Z
 *  
 *  VEDI LA LEZIONE COMPLETA : https://youtube.com/live/WL63nk-wrHw
 *  
 *  Kit Sonar: https://it.aliexpress.com/item/4000284339692.html
 */

/* Neural Network */
#define NO_TRAIN

#define PI            3.1415927 
#define NumberOf(arg) ((unsigned int)(sizeof(arg) / sizeof(arg[0])))
#ifdef YES_TRAIN
#define _1_OPTIMIZE B00010000  //  https://github.com/GiorgosXou/NeuralNetworks#macro-properties
#else
#define _1_OPTIMIZE B11010010 // https://github.com/GiorgosXou/NeuralNetworks#macro-properties
#endif
// ACTIVATION FUNCTION
#define Tanh                 
#include <NeuralNetwork.h>

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


// Configurazione dei Servomotori
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

int potenziometro[4];

/* variabili globali della movimentazione */
float alfa1024, beta1024, gamma1024;
float xmm, ymm, zmm;
int   tempoPressionePulsanteDS;


// Configurazione del SONAR HC-SR04
#define  SONAR_ECHO    2
#define  SONAR_TRIG    3

float dist_sonar_mm;

void setup() {
  
  Serial.begin(115200);

  NN.LearningRateOfWeights = 0.1;
#ifdef YES_TRAIN
  long j = 0, n;
  do {
    for (n = 0; (n < 100); n++) { // Epoch
      /* posizione corrente: partenza a caso */
      alfa1024 = random(100, 1000);
      beta1024 = random(100, 1000);
      gamma1024 = random(100, 1000);

      /* normalizzazione */
      expectedOutput[0] = alfa1024 / NORMALIZATION_FACTOR_ABG;
      expectedOutput[1] = gamma1024 / NORMALIZATION_FACTOR_ABG;

      /* cinematica diretta */
      polarToRect(round(alfa1024), round(beta1024), round(gamma1024), &xmm, &ymm, &zmm);
   
      /* normalizzazione */
      inputs[0] = sqrt((xmm*xmm) + (ymm*ymm))/NORMALIZATION_FACTOR_MM; // meta coordinata
      inputs[1] = zmm/NORMALIZATION_FACTOR_MM;

      output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally
 
      NN.BackProp(expectedOutput);  // "Tells" to the NN if the output was the-expected-correct one | then, "teaches" it
    }
    mse = NN.getMeanSqrdError(n);
    
    j = j + n;
    Serial.print(j);
    Serial.print(": alfa ");
    Serial.print(output[0] * NORMALIZATION_FACTOR_ABG, 0);  
    Serial.print(" (");
    Serial.print(alfa1024, 0);  
    Serial.print("), beta ");
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

  // CONFIGURAZIONE SERVO MOTORI
  Servo_0.attach( SERVO_ASSE_1 );
  Servo_1.attach( SERVO_ASSE_2 );
  Servo_2.attach( SERVO_ASSE_3 );
  Servo_3.attach( SERVO_PINZA );

  // CONFIGURAZIONE PULSANTE 
  pinMode(PULSANTE, INPUT);

  tempoPressionePulsanteDS = 0;

  // CONFIGURAZIONE DEL SONAR
  pinMode( SONAR_ECHO, INPUT );
  pinMode( SONAR_TRIG, OUTPUT );
}


void loop() {
  
  readPot();   // leggo il potenziometro solo per la pinza
  readSonar(); // leggo la distanza con il sonar
  
  /* normalizzazione */
  inputs[0] = sqrt((xmm*xmm) + (ymm*ymm))/NORMALIZATION_FACTOR_MM; // meta coordinata
  inputs[1] = zmm/NORMALIZATION_FACTOR_MM;

  output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally

  alfa1024 =  output[0] * NORMALIZATION_FACTOR_ABG;
  gamma1024 = output[1] * NORMALIZATION_FACTOR_ABG; 
 
  beta1024 = map((atan2(ymm, xmm) * 18000)/ PI, -9000, 9000, 0, 1023);
  
  writeServo();

  // DETERMINA x,y,z,teta, beta del sonar
  float xSmm, ySmm, zSmm, betaRad, tetaRad;
  polarToRectSonar(alfa1024, beta1024, gamma1024, &xSmm, &ySmm, &zSmm, &tetaRad, &betaRad);


  // CALCOLA LE COORDINATE DEL TARGET PUNTATO DAL SONAR
  float xTmm, yTmm, zTmm;
  xTmm = xSmm + dist_sonar_mm * cos(betaRad);
  yTmm = ySmm + dist_sonar_mm * sin(betaRad);
  zTmm = zSmm + dist_sonar_mm * sin(tetaRad);
  
  /*
  Serial.print(x); 
  Serial.print(","); 
  Serial.print(y); 
  Serial.print(","); 
  Serial.print(z);
  Serial.print(","); 
  Serial.print(t);
  Serial.print(","); 
  Serial.print(b);
  Serial.print(","); 
  Serial.println( dist_sonar_mm );
  */
  Serial.print(xTmm); 
  Serial.print(",\t"); 
  Serial.print(yTmm); 
  Serial.print(",\t"); 
  Serial.println(zTmm);
  
  delay(100);
}
