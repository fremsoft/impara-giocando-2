#include "settings.h"


volatile int32_t encoderPos = 0;      
volatile int     statoEncoder = 0;    

void encStateMachine() {
  
  switch( statoEncoder ) {
    case 0 : 
      if ((digitalRead(ENCODER_A) == 0) && (digitalRead(ENCODER_B) == 0)) { 
        statoEncoder = 1; 
      }
      break;
    case 1 : 
      if (digitalRead(ENCODER_A) == 1) { statoEncoder = 2; }
      if (digitalRead(ENCODER_B) == 1) { statoEncoder = 4; encoderPos ++; }
      break;
    case 2 : 
      if (digitalRead(ENCODER_A) == 0) { statoEncoder = 1; }
      if (digitalRead(ENCODER_B) == 1) { statoEncoder = 3; }
      break;
    case 3 : 
      if (digitalRead(ENCODER_A) == 0) { statoEncoder = 4; }
      if (digitalRead(ENCODER_B) == 0) { statoEncoder = 2; }
      break;
    case 4 : 
      if (digitalRead(ENCODER_A) == 1) { statoEncoder = 3; }
      if (digitalRead(ENCODER_B) == 0) { statoEncoder = 1; encoderPos --; }
      break;
    default : 
      Serial.println( "STATO SCONOSCIUTO" );
      statoEncoder = 0;
      break;
  }
}

void IRAM_ATTR readEncoderA() { encStateMachine(); }

void IRAM_ATTR readEncoderB() { encStateMachine(); }

void setupEncoder() {
  
  pinMode( ENCODER_A,  INPUT );
  pinMode( ENCODER_B,  INPUT );
  pinMode( ENCODER_SW, INPUT );

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), readEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_B), readEncoderB, CHANGE);

  Serial.println("Encoder configurato");

}

int32_t getPosEncoder() {
  return encoderPos;
}

bool getSwitchEncoder() {
  return !digitalRead(ENCODER_SW);
}
