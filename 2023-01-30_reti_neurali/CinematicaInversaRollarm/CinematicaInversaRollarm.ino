/* 
 *  Determinazione dei due angoli Alfa e Gamma mediante machine learning
 *  Beta è determinato analiticamente come arcotangente di Y/X e ridotto 
 *  nel range -PI/2 e PI/2.
 *
 *  VEDI LA LEZIONE COMPLETA : https://youtube.com/live/dJgejDh9f24
 */

#define PI            3.1415927 

#define NumberOf(arg) ((unsigned int)(sizeof(arg) / sizeof(arg[0])))

#define _1_OPTIMIZE B00010000  //  https://github.com/GiorgosXou/NeuralNetworks#macro-properties
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

float inputs[4];
float expectedOutput[2];
float mse;
float alfa1024, beta1024, gamma1024, xmm, ymm, zmm;

NeuralNetwork NN(layers, NumberOf(layers));  // Creating a NeuralNetwork with default learning-rates
  
void setup() {
  int skip = 1000;

  Serial.begin(115200);

  mse = 1;
  for (long j = 0; (j < 1000000L) && (mse > 0.000005); j++)  // Epoch
  {
    /* posizione corrente: partenza a caso */
    alfa1024 = random(1024);
    beta1024 = random(1024);
    gamma1024 = random(1024);

    /* normalizzazione */
    inputs[0] = alfa1024 / NORMALIZATION_FACTOR;
    inputs[1] = gamma1024 / NORMALIZATION_FACTOR;

    /* piccolo spostamento */
    alfa1024  = constrain(alfa1024 + random(21) - 10,  0, 1023);
    beta1024  = constrain(beta1024 + random(21) - 10,  0, 1023);
    gamma1024 = constrain(gamma1024 + random(21) - 10, 0, 1023);
    
    /* cinematica diretta */
    polarToRect(round(alfa1024), round(beta1024), round(gamma1024), &xmm, &ymm, &zmm);

    /* normalizzazione */
    xmm = xmm/NORMALIZATION_FACTOR; 
    ymm = ymm/NORMALIZATION_FACTOR; 
    zmm = zmm/NORMALIZATION_FACTOR;
    inputs[2] = (xmm*xmm) + (ymm*ymm); /* non calcolo la radice quadrata per snellire i calcoli */
    inputs[3] = zmm;

    expectedOutput[0] = alfa1024 / NORMALIZATION_FACTOR;
    expectedOutput[1] = gamma1024 / NORMALIZATION_FACTOR;

    output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally

    skip++;
    if (skip > 100) {
      Serial.print(j);
      Serial.print(": alfa ");
      Serial.print(output[0] * NORMALIZATION_FACTOR, 0);  
      Serial.print("\t(");
      Serial.print(alfa1024, 0);  
      Serial.print("),\tgamma ");
      Serial.print(output[1] * NORMALIZATION_FACTOR, 0); 
      Serial.print("\t(");
      Serial.print(gamma1024, 0);  
      Serial.print(")\t");  
    }

    NN.BackProp(expectedOutput);  // "Tells" to the NN if the output was the-expected-correct one | then, "teaches" it
    //mse =                NN.getMeanSqrdError(NumberOf(inputs));
    mse = (mse * 0.9) + (NN.getMeanSqrdError(NumberOf(inputs)) * 0.1);
      
    if (skip > 100) {
      skip = 0;
      Serial.print("  \tMSE: ");
      Serial.println(mse, 7); /* 7 cifre dopo la virgola */
    }
  }

  Serial.println("\n LEARNING COMPLETE, PRESS ANY KEY TO CONTINUE");
}


void loop() {
  
  if (Serial.available() > 0) {
    while (Serial.available() > 0) { /* flush */ Serial.read(); }
    
    Serial.println("\n =-[OUTPUTS]-=");

    for (int i = 0; i < 100; i++) {
      /* posizione corrente: partenza a caso */
      alfa1024 = random(1024);
      beta1024 = random(1024);
      gamma1024 = random(1024);
    
      /* normalizzazione */
      inputs[0] = alfa1024 / NORMALIZATION_FACTOR;
      inputs[1] = gamma1024 / NORMALIZATION_FACTOR;

      /* piccolo spostamento */
      alfa1024  = constrain(alfa1024 + random(21) - 10,  0, 1023);
      beta1024  = constrain(beta1024 + random(20) - 10,  0, 1023);
      gamma1024 = constrain(gamma1024 + random(20) - 10, 0, 1023);
    
      /* cinematica diretta */
      polarToRect(round(alfa1024), round(beta1024), round(gamma1024), &xmm, &ymm, &zmm);

      /* normalizzazione */
      xmm = xmm/NORMALIZATION_FACTOR; ymm = ymm/NORMALIZATION_FACTOR; zmm = zmm/NORMALIZATION_FACTOR;
      inputs[2] = (xmm*xmm) + (ymm*ymm); /* non calcolo la radice quadrata per snellire i calcoli */
      inputs[3] = zmm;

      output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally

      Serial.print("alfa ");
      Serial.print(output[0] * NORMALIZATION_FACTOR, 0);  
      Serial.print("\t(");
      Serial.print(alfa1024, 0);  
      Serial.print("),\tgamma ");
      Serial.print(output[1] * NORMALIZATION_FACTOR, 0); 
      Serial.print("\t(");
      Serial.print(gamma1024, 0);  
      Serial.println(")");  

    }

    NN.print();  // Prints the weights and biases of each layer
  }
}
