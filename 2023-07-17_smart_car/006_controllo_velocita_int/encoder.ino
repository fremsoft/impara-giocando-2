#define ENCODER_1         14  /* A0 */ 
#define ENCODER_2         15  /* A1 */
#define ENCODER_3         16  /* A2 */ 
#define ENCODER_4         17  /* A3 */ 


                 
int statoEncoderOld[4] = { 0,  0,  0,  0 };
unsigned long t0Encoder[4] = { 0,  0,  0,  0 };
int velocitaRPM[4] = { 0,  0,  0,  0 };
int direzione[4] = { 0,  0,  0,  0 }; /* 0 = fermo; 1 = avanti; -1 = indietro */
long posizione[4] = { 0,  0,  0,  0 }; 

void initEncoder() {
  
  pinMode ( ENCODER_1, INPUT );
  pinMode ( ENCODER_2, INPUT );
  pinMode ( ENCODER_3, INPUT );
  pinMode ( ENCODER_4, INPUT );

  direzione[0] = 1;
  direzione[1] = 1;
  direzione[2] = 1;
  direzione[3] = 1;

  posizione[0] = 0;
  posizione[1] = 0;
  posizione[2] = 0;
  posizione[3] = 0;
  
  
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(ENCODER_1), checkEncoder, CHANGE);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(ENCODER_2), checkEncoder, CHANGE);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(ENCODER_3), checkEncoder, CHANGE);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(ENCODER_4), checkEncoder, CHANGE);
}

void setDirezione(int n, int velocita) {
  if (velocita > 0) {
    direzione[n] = 1;
  }
  else if (velocita < 0) {
    direzione[n] = -1;
  }
}

void checkEncoder() {
  int encoder;
  unsigned long t1 = micros();

  encoder = digitalRead( ENCODER_1 );
  if (( encoder == HIGH ) && ( statoEncoderOld[0] == LOW )) {
    /* c' e' stato un fronte positivo su encoder 1 */
    if (direzione[0] > 0) { /* sto andando avanti */   posizione[0] ++; }
    if (direzione[0] < 0) { /* sto andando indietro */ posizione[0] --; }
    
    /* calcolo RPM */
    velocitaRPM[0] = direzione[0] * (3000000L / (t1 - t0Encoder[0])); 
    t0Encoder[0] = t1;
  }
  statoEncoderOld[0] = encoder;
  
  encoder = digitalRead( ENCODER_2 );
  if (( encoder == HIGH ) && ( statoEncoderOld[1] == LOW )) {
    /* c' e' stato un fronte positivo su encoder 2 */
    if (direzione[1] > 0) { /* sto andando avanti */   posizione[1] ++; }
    if (direzione[1] < 0) { /* sto andando indietro */ posizione[1] --; }
    
    /* calcolo RPM */
    velocitaRPM[1] = direzione[1] * (3000000L / (t1 - t0Encoder[1])); 
    t0Encoder[1] = t1;
  }
  statoEncoderOld[1] = encoder;
  
  encoder = digitalRead( ENCODER_3 );
  if (( encoder == HIGH ) && ( statoEncoderOld[2] == LOW )) {
    /* c' e' stato un fronte positivo su encoder 3 */
    if (direzione[2] > 0) { /* sto andando avanti */   posizione[2] ++; }
    if (direzione[2] < 0) { /* sto andando indietro */ posizione[2] --; }
    
    /* calcolo RPMenza */
    velocitaRPM[2] = direzione[2] * (3000000L / (t1 - t0Encoder[2])); 
    t0Encoder[2] = t1;
  }
  statoEncoderOld[2] = encoder;
  
  encoder = digitalRead( ENCODER_4 );
  if (( encoder == HIGH ) && ( statoEncoderOld[3] == LOW )) {
    /* c' e' stato un fronte positivo su encoder 4 */
    if (direzione[3] > 0) { /* sto andando avanti */   posizione[3] ++; }
    if (direzione[3] < 0) { /* sto andando indietro */ posizione[3] --; }
    
    /* calcolo RPM */
    velocitaRPM[3] = direzione[3] * (3000000L / (t1 - t0Encoder[3])); 
    t0Encoder[3] = t1;
  }
  statoEncoderOld[3] = encoder;

  /* check se si ferma la ruota */
  if ((t1 - t0Encoder[0]) > 1000000L) { velocitaRPM[0] = 0; }
  if ((t1 - t0Encoder[1]) > 1000000L) { velocitaRPM[1] = 0; }
  if ((t1 - t0Encoder[2]) > 1000000L) { velocitaRPM[2] = 0; }
  if ((t1 - t0Encoder[3]) > 1000000L) { velocitaRPM[3] = 0; }
}
