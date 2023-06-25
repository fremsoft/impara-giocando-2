#define PIN_MOT1_ENA  8  // LOGICA NEGATIVA
#define PIN_MOT1_DIR  3  // LOW = CW, HIGH = CCW
#define PIN_MOT1_PUL  2

#define PIN_MOT2_ENA  9  // LOGICA NEGATIVA
#define PIN_MOT2_DIR  5  // LOW = CW, HIGH = CCW
#define PIN_MOT2_PUL  4


void setup() {

  pinMode ( PIN_MOT1_ENA, OUTPUT );
  pinMode ( PIN_MOT1_DIR, OUTPUT );
  pinMode ( PIN_MOT1_PUL, OUTPUT );

  pinMode ( PIN_MOT2_ENA, OUTPUT );
  pinMode ( PIN_MOT2_DIR, OUTPUT );
  pinMode ( PIN_MOT2_PUL, OUTPUT );

  digitalWrite ( PIN_MOT1_ENA, LOW );
  digitalWrite ( PIN_MOT1_DIR, HIGH );
  digitalWrite ( PIN_MOT1_PUL, LOW );
  
  digitalWrite ( PIN_MOT2_ENA, LOW );
  digitalWrite ( PIN_MOT2_DIR, HIGH );
  digitalWrite ( PIN_MOT2_PUL, LOW );
  
}

void loop() {

  delay(1);
  digitalWrite ( PIN_MOT1_PUL, HIGH );
  digitalWrite ( PIN_MOT2_PUL, HIGH );
  
  delay(1);
  digitalWrite ( PIN_MOT1_PUL, LOW );
  digitalWrite ( PIN_MOT2_PUL, LOW );

}
