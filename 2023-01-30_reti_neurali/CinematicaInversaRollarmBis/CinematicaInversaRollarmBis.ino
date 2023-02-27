/* 
 *  Determinazione dei due angoli Alfa e Gamma mediante machine learning
 *  Beta è determinato analiticamente come arcotangente di Y/X e ridotto 
 *  nel range -PI/2 e PI/2.
 *
 *  VEDI LA LEZIONE COMPLETA : https://youtube.com/live/5trqQZzrbJU
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

//const unsigned int layers[] = { 3, 100, 3 };  // 3 layers (1st)layer with 3 input neurons (x,y,z) (2nd)layer with 100 hidden neurons and (3th)layer with 3 output neuron (alfa, beta, gamma)
const unsigned int layers[] = { 3, 15, 3 };  // 3 layers (1st)layer with 3 input neurons (x,y,z) (2nd)layer with 100 hidden neurons and (3th)layer with 3 output neuron (alfa, beta, gamma)
float *output;                                // 3th layer's output(s)

float inputs[3];
float expectedOutput[3];
float mse;
float alfa1024, beta1024, gamma1024, xmm, ymm, zmm;

NeuralNetwork NN(layers, NumberOf(layers));  // Creating a NeuralNetwork with default learning-rates
  
void setup() {
  long j = 0, n;
  float xn, yn, zn;
  
  Serial.begin(115200);

  mse = 1;
  while ((j < 1000000L) && (mse > 0.00005)) {

    for (n=0; n<1000; n++) {  // Epoch
  
      /* posizione da imparare a caso */
      alfa1024 = random(1024);
      beta1024 = random(1024);
      gamma1024 = random(1024);

      /* normalizzazione */
      expectedOutput[0] = alfa1024 / NORMALIZATION_FACTOR;
      expectedOutput[1] = beta1024 / NORMALIZATION_FACTOR;
      expectedOutput[2] = gamma1024 / NORMALIZATION_FACTOR;

      /* cinematica diretta */
      polarToRect(round(alfa1024), round(beta1024), round(gamma1024), &xmm, &ymm, &zmm);

      /* normalizzazione */
      xn = xmm/NORMALIZATION_FACTOR; 
      yn = ymm/NORMALIZATION_FACTOR; 
      zn = zmm/NORMALIZATION_FACTOR;
      inputs[0] = xn;
      inputs[1] = yn;
      inputs[2] = zn;

      output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally
      NN.BackProp(expectedOutput);  // "Tells" to the NN if the output was the-expected-correct one | then, "teaches" it
    }
    mse = NN.getMeanSqrdError( n );
    j = j + n;
    
    Serial.print(j);
    Serial.print(": alfa ");
    Serial.print(output[0] * NORMALIZATION_FACTOR, 0);  
    Serial.print(" (");
    Serial.print(alfa1024, 0);  
    Serial.print("), beta ");
    Serial.print(output[1] * NORMALIZATION_FACTOR, 0); 
    Serial.print(" (");
    Serial.print(beta1024, 0);  
    Serial.print("), gamma ");
    Serial.print(output[2] * NORMALIZATION_FACTOR, 0); 
    Serial.print(" (");
    Serial.print(gamma1024, 0);  
    Serial.print(")\t");  
    Serial.print("\tMSE: ");
    Serial.println(mse, 7); /* 7 cifre dopo la virgola */
  }

  Serial.println("\n LEARNING COMPLETE, PRESS ANY KEY TO CONTINUE");
}


void loop() {
  float xn, yn, zn;
  
  if (Serial.available() > 0) {
    while (Serial.available() > 0) { /* flush */ Serial.read(); delay(100);}
    
    Serial.println("\n =-[OUTPUTS]-=");

    for (int i = 0; i < 100; i++) {
      /* posizione corrente: partenza a caso */
      alfa1024 = random(1024);
      beta1024 = random(1024);
      gamma1024 = random(1024);
    
      /* cinematica diretta */
      polarToRect(round(alfa1024), round(beta1024), round(gamma1024), &xmm, &ymm, &zmm);

      /* normalizzazione */
      xn = xmm/NORMALIZATION_FACTOR;
      yn = ymm/NORMALIZATION_FACTOR;
      zn = zmm/NORMALIZATION_FACTOR;
      inputs[0] = xn;
      inputs[1] = yn;
      inputs[2] = zn;

      output = NN.FeedForward(inputs);  // FeedForwards the input arrays through the NN | stores the output array internally

      Serial.print("alfa ");
      Serial.print(output[0] * NORMALIZATION_FACTOR, 0);  
      Serial.print(" (");
      Serial.print(alfa1024, 0);  
      Serial.print("), beta ");
      Serial.print(output[1] * NORMALIZATION_FACTOR, 0); 
      Serial.print(" (");
      Serial.print(beta1024, 0);  
      Serial.print("), gamma ");
      Serial.print(output[2] * NORMALIZATION_FACTOR, 0); 
      Serial.print(" (");
      Serial.print(gamma1024, 0);  
      Serial.println(")");
    }

    NN.print();  // Prints the weights and biases of each layer
  }
}
