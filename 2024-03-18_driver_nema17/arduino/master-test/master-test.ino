/*
Sketch per Arduino UNO
con shield RS485 (Half Duplex) 
https://amzn.to/3MBjLMb
connesso mediante software serial
sulle porte:

   *  2 ( TX ) 
   *  3 ( RX ) 
   *  4 ( EN )

Trasmette i comandi ai motori in seguito a 
comandi inviati su monitor seriale.

Vedi la sessione di laboratorio completa:
https://youtube.com/live/J3OgzIutEWA

*/

#include <SoftwareSerial.h>

#define ID_SERVO    1

// Pin definitions
#define RS485_RX 3 // RX pin
#define RS485_TX 2 // TX pin
#define RS485_DE 4 // DE/DI pin (Data Enable)

SoftwareSerial rs485Serial(RS485_RX, RS485_TX); // RX, TX

void setup() {
  
  // Initialize the control pin for RS485
  pinMode(RS485_DE, OUTPUT);
  digitalWrite(RS485_DE, LOW); // Initialize to receive mode

  // Begin the serial communication at 19200 baud rate
  rs485Serial.begin(19200);

  // Optional: Begin the hardware serial communication for debugging
  Serial.begin(19200);
  while (!Serial) {
    ; // Wait for the serial port to connect. Needed for native USB port only
  }

  Serial.println("RS485 Communication Initialized");
}

void loop() {
  // Example: Send a message every 2 seconds
  char c;

  if (Serial.available() != 0) {
    c = Serial.read();

    if (c == '1') {
      int id=1;
      int dir=1;
      int step=1600;
      int ppr=1600;  // pulses per revolution
      int rnd=millis()&0xFFF;
      int csum=id+dir+step+ppr+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,dir:%d,step:%d,ppr:%d,rnd:%d,csum:%d}", id, dir, step, ppr, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == '2') {
      int id=2;
      int dir=1;
      int step=1600;
      int ppr=1600;  // pulses per revolution
      int rnd=millis()&0xFFF;
      int csum=id+dir+step+ppr+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,dir:%d,step:%d,ppr:%d,rnd:%d,csum:%d}", id, dir, step, ppr, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == '3') {
      int id=3;
      int dir=1;
      int step=1600;
      int ppr=1600;  // pulses per revolution
      int rnd=millis()&0xFFF;
      int csum=id+dir+step+ppr+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,dir:%d,step:%d,ppr:%d,rnd:%d,csum:%d}", id, dir, step, ppr, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == '0') {
      int id=ID_SERVO;
      int dir=1;
      int step=1600;
      int ppr=400;  // pulses per revolution
      int rnd=millis()&0xFFF;
      int csum=id+dir+step+ppr+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,dir:%d,step:%d,ppr:%d,rnd:%d,csum:%d}", id, dir, step, ppr, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'g') {
      int id=0x100;     // broadcasting message
      int go=1;
      int rnd=millis()&0xFFF;
      int csum=id+go+rnd;
      char s[256];
      
      sprintf(s, "  {id:%d,go:%d,rnd:%d,csum:%d}", id, go, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'q') {
      int id=ID_SERVO;
      int deg=4095/8 * 0;  
      int rnd=millis()&0xFFF;
      int csum=id+deg+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,deg:%d,rnd:%d,csum:%d}", id, deg, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'w') {
      int id=ID_SERVO;
      int deg=4095/8 * 1;  
      int rnd=millis()&0xFFF;
      int csum=id+deg+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,deg:%d,rnd:%d,csum:%d}", id, deg, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'e') {
      int id=ID_SERVO;
      int deg=4095/8 * 2;  
      int rnd=millis()&0xFFF;
      int csum=id+deg+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,deg:%d,rnd:%d,csum:%d}", id, deg, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'd') {
      int id=ID_SERVO;
      int deg=4095/8 * 3;  
      int rnd=millis()&0xFFF;
      int csum=id+deg+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,deg:%d,rnd:%d,csum:%d}", id, deg, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'c') {
      int id=ID_SERVO;
      int deg=4095/8 * 4;  
      int rnd=millis()&0xFFF;
      int csum=id+deg+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,deg:%d,rnd:%d,csum:%d}", id, deg, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'x') {
      int id=ID_SERVO;
      int deg=4095/8 * 5;  
      int rnd=millis()&0xFFF;
      int csum=id+deg+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,deg:%d,rnd:%d,csum:%d}", id, deg, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'z') {
      int id=ID_SERVO;
      int deg=4095/8 * 6;  
      int rnd=millis()&0xFFF;
      int csum=id+deg+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,deg:%d,rnd:%d,csum:%d}", id, deg, rnd, csum);
      sendRS485Message(s);
    }
    else
    if (c == 'a') {
      int id=ID_SERVO;
      int deg=4095/8 * 7;  
      int rnd=millis()&0xFFF;
      int csum=id+deg+rnd;
      char s[256];
      
      sprintf(s, "      {id:%d,deg:%d,rnd:%d,csum:%d}", id, deg, rnd, csum);
      sendRS485Message(s);
    }
    
  }
    
  if (rs485Serial.available()) {
    String receivedMessage = readRS485Message();
    Serial.println("Received: " + receivedMessage);
  }

}

void sendRS485Message(String message) {

  // Set the DE/DI pin HIGH to enable transmission mode
  digitalWrite(RS485_DE, HIGH);

  Serial.println("TX: " + message);
  
  // Send the message via RS485
  rs485Serial.print(message); // trasmissione blocking  
  
  // Set the DE/DI pin LOW to enable receive mode
  digitalWrite(RS485_DE, LOW);
}

unsigned long calculateTransmissionTime(unsigned int numBytes) {
  // Calculate the time to send the specified number of bytes at 19200 baud
  // Transmission time per byte = 10 bits / 19200 baud = 0.00052 seconds per byte
  // Total transmission time = numBytes * 0.00052 seconds = numBytes * 520 microseconds
  return (5000L * numBytes) / 19000; // in millise
}

String readRS485Message() {
  String message = "";
  while (rs485Serial.available()) {
    char c = rs485Serial.read();
    message += c;
  }
  return message;
}
