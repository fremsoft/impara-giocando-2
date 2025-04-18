#include "settings.h"

#define RX_BUF_SIZE 128 
char rx_buf[RX_BUF_SIZE];

void setupRS485 (int baudrate) {

  pinMode     ( RS485_TXEN, OUTPUT );   
  digitalWrite( RS485_TXEN, HIGH );    // questo è il master è pronto a spedire pacchetti

  Serial2.begin( baudrate, SERIAL_8N1, RS485_RX, RS485_TX );
  Serial.println( "RS485 configurata su RX " + String(RS485_RX) + " e TX " + String(RS485_TX) );
  Serial.println( "Baud rate " + String(baudrate) );

  rx_buf[0] = 0;

}

void sendRS485Message(String message, bool waitAck) {
  uint32_t t0;
  char c;
  int rx_index;

  // Set the DE/DI pin HIGH to enable transmission mode
  digitalWrite(RS485_TXEN, HIGH);

  Serial.println("TX: " + message);

  // Send the message via RS485
  Serial2.print(message); // trasmissione NON blocking 
  Serial2.flush();        // block trnsmission until flushed
  
  // Set the DE/DI pin LOW to enable receive mode
  digitalWrite(RS485_TXEN, LOW);

  if ( waitAck ) {
    t0 = millis();
    rx_index = 0;

    do {
      if ( Serial2.available() != 0 ) {
        c = Serial2.read();
        rx_buf[rx_index++] = c;
        if (rx_index >= RX_BUF_SIZE) { rx_index--; }
        rx_buf[rx_index] = 0;
      }
      else { 
        c = 0; 
      }
    } while ((c != '}') && ((millis() - t0) < 1000) ); 
    // timeout 1000ms

    // verifica che sia ACK=1
    // e gestire eventualmente la ritrasmissione 
    // o la segnalazione di un errore
  } 
  
  // alla fine introduce sempre un leggero ritardo
  delay(10);
}

char * RS485Buffer() {
  return rx_buf;
}

void speedCoclea(int speed) {
  int id=1;
  int dir=1;       // avanti
  int ppr=1600;    // pulses per revolution
  int pps=speed;   // pulses per second
  int rnd=millis()&0xFFF;
  int csum=id+dir+ppr+pps+rnd;
  char s[256];
  
  sprintf(s, "   {id:%d,dir:%d,ppr:%d,pps:%d,rnd:%d,csum:%d}", id, dir, ppr, pps, rnd, csum);
  sendRS485Message(s, true);
  delay(500);      
}

