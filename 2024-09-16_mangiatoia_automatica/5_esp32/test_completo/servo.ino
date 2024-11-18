#include "settings.h"
#include <ESP32Servo.h>

Servo myServo;

void setupServo() {
  pinMode      ( SERVO_POWER, OUTPUT);
  digitalWrite ( SERVO_POWER, LOW );

  myServo.attach( SERVO_PIN );  

  Serial.println("Servo collegato e pronto.");
}

void servoOFF() {
  digitalWrite( SERVO_POWER, LOW );
}

void servoON() {
  digitalWrite( SERVO_POWER, HIGH );
}

void servoSetPos(int pos) {
  myServo.write(pos);
}
