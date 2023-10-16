#define PWM0A              6
#define PWM0B              5
#define PWM2A             11
#define PWM2B              3

#define MOTORE1           PWM2A
#define MOTORE2           PWM2B
#define MOTORE3           PWM0B
#define MOTORE4           PWM0A

#define DIR_CLK            4
#define DIR_EN             7
#define DIR_SER            8
#define DIR_LATCH         12

#define MOTORE_FERMO    0
#define MOTORE_AVANTI   1
#define MOTORE_INDIETRO 2
        
                 /* M4B M3B M4A M2B M1B M1A M2A M3A */  
int regScorr[8] = { 0,  0,  0,  0,  0,  0,  0,  0 };

void scriviRegScorr() {
  int i;

  for ( i=0 ; i<8 ; i++ ) {

    digitalWrite ( DIR_SER   , regScorr[i] );
    delayMicroseconds( 10 );
    digitalWrite ( DIR_CLK   , HIGH );
    delayMicroseconds( 10 );
    digitalWrite ( DIR_CLK   , LOW );
    delayMicroseconds( 10 );
    
  }

  digitalWrite ( DIR_LATCH   , HIGH );
  delayMicroseconds( 10 );
  digitalWrite ( DIR_LATCH   , LOW );
  delayMicroseconds( 10 );
  digitalWrite ( DIR_EN   , LOW );  /* attivo basso */
    
}

void initMotori() {
  // i pwm non serve definirli

  pinMode ( DIR_CLK   , OUTPUT );
  pinMode ( DIR_EN    , OUTPUT );
  pinMode ( DIR_SER   , OUTPUT );
  pinMode ( DIR_LATCH , OUTPUT );

  digitalWrite ( DIR_CLK   , LOW );
  digitalWrite ( DIR_EN    , HIGH );   /* attivo basso */
  digitalWrite ( DIR_SER   , LOW );
  digitalWrite ( DIR_LATCH , LOW );

  scriviRegScorr();
}

void setMotore1(int f) {

  if ( f == MOTORE_AVANTI ) {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[4] = 0;
      regScorr[5] = 1;
  }
  else if ( f == MOTORE_INDIETRO ) {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[4] = 1;
      regScorr[5] = 0;
  }
  else /* if ( f == MOTORE_FERMO ) */ {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[4] = 0;
      regScorr[5] = 0;
  }

  scriviRegScorr();
}

void setMotore2(int f) {

  if ( f == MOTORE_AVANTI ) {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[3] = 0;
      regScorr[6] = 1;
  }
  else if ( f == MOTORE_INDIETRO ) {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[3] = 1;
      regScorr[6] = 0;
  }
  else /* if ( f == MOTORE_FERMO ) */ {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[3] = 0;
      regScorr[6] = 0;
  }

  scriviRegScorr();
}

void setMotore3(int f) {

  if ( f == MOTORE_AVANTI ) {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[1] = 0;
      regScorr[7] = 1;
  }
  else if ( f == MOTORE_INDIETRO ) {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[1] = 1;
      regScorr[7] = 0;
  }
  else /* if ( f == MOTORE_FERMO ) */ {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[1] = 0;
      regScorr[7] = 0;
  }

  scriviRegScorr();
}

void setMotore4(int f) {

  if ( f == MOTORE_AVANTI ) {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[0] = 0;
      regScorr[2] = 1;
  }
  else if ( f == MOTORE_INDIETRO ) {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[0] = 1;
      regScorr[2] = 0;
  }
  else /* if ( f == MOTORE_FERMO ) */ {
      /* 0-M4B 1-M3B 2-M4A 3-M2B 4-M1B 5-M1A 6-M2A 7-M3A */  
      regScorr[0] = 0;
      regScorr[2] = 0;
  }

  scriviRegScorr();
}

void setPwmMotore1 (int d) {
  int dd = constrain(d, 0, 255);
  analogWrite ( MOTORE1, dd );
}

void setPwmMotore2 (int d) {
  int dd = constrain(d, 0, 255);
  analogWrite ( MOTORE2, dd );
}

void setPwmMotore3 (int d) {
  int dd = constrain(d, 0, 255);
  analogWrite ( MOTORE3, dd );
}

void setPwmMotore4 (int d) {
  int dd = constrain(d, 0, 255);
  analogWrite ( MOTORE4, dd );
}

/* sempre tra -255 e +255*/
void setVelocitaMotore1 (int v) {

  int vv = constrain(v, -255, 255);

  if ( vv == 0 ) {
    setMotore1( MOTORE_FERMO );
    setPwmMotore1 ( 0 );
  }
  else if ( vv > 0 ) {
    setMotore1( MOTORE_AVANTI );
    setPwmMotore1 ( vv );
  }
  else /* if ( vv < 0 ) */ {
    setMotore1( MOTORE_INDIETRO );
    setPwmMotore1 ( 0 - vv );
  }
} 

/* sempre tra -255 e +255*/
void setVelocitaMotore2 (int v) {

  int vv = constrain(v, -255, 255);

  if ( vv == 0 ) {
    setMotore2( MOTORE_FERMO );
    setPwmMotore2 ( 0 );
  }
  else if ( vv > 0 ) {
    setMotore2( MOTORE_AVANTI );
    setPwmMotore2 ( vv );
  }
  else /* if ( vv < 0 ) */ {
    setMotore2( MOTORE_INDIETRO );
    setPwmMotore2 ( 0 - vv );
  }
} 

/* sempre tra -255 e +255*/
void setVelocitaMotore3 (int v) {

  int vv = constrain(v, -255, 255);

  if ( vv == 0 ) {
    setMotore3( MOTORE_FERMO );
    setPwmMotore3 ( 0 );
  }
  else if ( vv > 0 ) {
    setMotore3( MOTORE_AVANTI );
    setPwmMotore3 ( vv );
  }
  else /* if ( vv < 0 ) */ {
    setMotore3( MOTORE_INDIETRO );
    setPwmMotore3 ( 0 - vv );
  }
} 

/* sempre tra -255 e +255*/
void setVelocitaMotore4 (int v) {

  int vv = constrain(v, -255, 255);

  if ( vv == 0 ) {
    setMotore4( MOTORE_FERMO );
    setPwmMotore4 ( 0 );
  }
  else if ( vv > 0 ) {
    setMotore4( MOTORE_AVANTI );
    setPwmMotore4 ( vv );
  }
  else /* if ( vv < 0 ) */ {
    setMotore4( MOTORE_INDIETRO );
    setPwmMotore4 ( 0 - vv );
  }
} 
