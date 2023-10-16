/*
    Questo Sketch serve a far andare la Smart Car a guida
    autonoma comandata da un MASTER (ad esempio ESP32CAM)
    mediante la linea seriale.

    Questo sketch opera in multitasking grazie all'uso
    delle macchine a stati.

    Il controllo della velocità e della posizione delle
    ruote della Smart Car avviene mediante lo shield di
    controllo moroti DC della MH Electronics.

    Al pin  4 è connesso DIR_CLK
    Al pin  7 è connesso DIR_EN (attivo basso)
    Al pin  8 è connesso DIR_SER
    Al pin 12 è connesso DIR_LATCH

    Sul fronte positivo di DIR_CLK viene caricato
    DIR_SER sul registro a scorrimento.

    Sul fronte positivo di DIR_LATCH viene caricato
    in uscita il byte intero del registro a scorrimento.

    Il primo bit che carico va su M4B e di conseguenza
    la successione dei segnali è la seguente:

    primo M4B, M3B, M4A, M2B, M1B, M1A, M2A, ultimo M3A

    Mentre i PWM che regolano le velocità dei motori sono 4:
    PWM0A - MOTORE 4 - Pin  6
    PWM0B - MOTORE 3 - Pin  5
    PWM1A - SERVO 2  - Pin  9
    PWM1B - SERVO 1  - Pin 10
    PWM2A - MOTORE 1 - Pin 11
    PWM2B - MOTORE 2 - Pin  3

    Negli ingressi analogici A0..A3 (corrispondenti a 14..17)
    sono collegati dei sensori che rilevano 20 impulsi per giro.
    Se ogni ruota ha un diametro di 68mm, ad ogni impulso
    corrisponde un avanzamento pari a 10,6mm.

    Questo programma li gestisce a interrupt, usando la libreria
    <PinChangeInterrupt.h> di NicoHood, perciò la precisione è
    massima anche se il loop impegna molte risorse.

    Inoltre è collegato a SERVO1 (PWM1B - 10) un
    servomotore SG90 per far ruotare il sonar da sinistra
    a destra.

    Il sonar HC-SR04 è collegato ai pin:

    A4 - Trigger
    A5 - Echo

    Visualizza l'esperienza di laboratorio completa:
    https://youtube.com/live/HDsDUQvpj10

*/

#include <PinChangeInterrupt.h>
#include <ArduinoJson.h>

int velocita;
extern int velocitaRPM[4];
int setPointVelocitaRPM[4];
int pwmRuota[4];
extern long posizione[4];

unsigned long tempoRitardoControllo;
#define RITARDO_CONTROLLO_MS 20

extern int angolo_sonar;
extern unsigned long distanza_sonar;
unsigned long tempoRitardoSonar;
#define RITARDO_SONAR_MS          200

/*
 * Queste due costanti diventano due variabili
 
#define INCREMENTO_GRADI_SONAR     10
#define APERTURA_GRADI_SONAR       30
#define N_PUNTI_MISURA_LATERALI_SONAR  (APERTURA_GRADI_SONAR/INCREMENTO_GRADI_SONAR)
#define N_PUNTI_MISURA_SONAR  (1+((APERTURA_GRADI_SONAR * 2)/INCREMENTO_GRADI_SONAR))
 */
 
#define N_PUNTI_MISURA_SONAR_MAX  100
#define TEMPO_STEP_SCANSIONE_MS  1000.0       
unsigned long distanze_sonar[N_PUNTI_MISURA_SONAR_MAX];
unsigned long t_cadenza_sonar, t_timeout_sonar;
int           direzione_spazzolata_sonar;  /* 1 = AVANTI; -1 = INDIETRO */
int           sonar_angolo_partenza;
int           sonar_angolo_arrivo;
int           sonar_angolo_step;
int           sonar_indice_array;
int           sonar_angolo_partenza_intermedia;
int           sonar_angolo_prossima_scansione;
unsigned long sonar_t0_spostamento;


enum { 
       STAI_FERMO, 
       VAI_A_SX, 
       VAI_A_DX, 
       VAI_DRITTO, 
       GIRA_90_GRADI,
       CMD_RUN, 
       CMD_GOTO
     } stato_macchina_motore;

enum { 
       SONAR_STANDBY, 
       SONAR_SPOSTAMENTO, 
       SONAR_SCANSIONE_ELEMENTARE 
     } stato_macchina_sonar;
     
#define SOGLIA_ARRIVO 2
long p_target[4] = { 0,  0,  0,  0 };
long vsx, vdx, target_sx, target_dx;
long psx, pdx;

#define JSON_STRING_SIZE 250
DynamicJsonDocument docJson(JSON_STRING_SIZE);
char jsonStr[JSON_STRING_SIZE];
int  jsonLen;


/* definizione delle routine del ciclo principale */

void macchinaFerma() {

  stato_macchina_motore = STAI_FERMO;

  pwmRuota[0] = 0;
  pwmRuota[1] = 0;
  pwmRuota[2] = 0;
  pwmRuota[3] = 0;

  setVelocitaMotore1( pwmRuota[0] );
  setVelocitaMotore2( pwmRuota[1] );
  setVelocitaMotore3( pwmRuota[2] );
  setVelocitaMotore4( pwmRuota[3] );

  setPointVelocitaRPM[0] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[1] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[2] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[3] = 0;  /* espresso in RPM */

}

void macchinaStatiSonar() {
  int i;
  unsigned long d;

  if (stato_macchina_sonar == SONAR_SPOSTAMENTO) {
    float v = (sonar_angolo_prossima_scansione - sonar_angolo_partenza_intermedia) / TEMPO_STEP_SCANSIONE_MS;
    int p = sonar_angolo_partenza_intermedia + (int)(v * (millis() - sonar_t0_spostamento));
    sonarSpostaTorretta( p );

    if (v > 0) {
      if ( p >= sonar_angolo_prossima_scansione ) {
        /* avvia scansione */
        delay(100); /* da togliere */
        sonarSparaUltrasuoni();
        t_timeout_sonar = millis();
        stato_macchina_sonar = SONAR_SCANSIONE_ELEMENTARE;
      }
    }
    else /* v < 0 */ {
      if ( p <= sonar_angolo_prossima_scansione ) {
        /* avvia scansione */
        delay(100); /* da togliere */
        sonarSparaUltrasuoni();
        t_timeout_sonar = millis();
        stato_macchina_sonar = SONAR_SCANSIONE_ELEMENTARE;
      }
    }
  }
  else if ( stato_macchina_sonar == SONAR_SCANSIONE_ELEMENTARE ) {
    
    noInterrupts();
    d = distanza_sonar;
    interrupts();

    if (( d != 0 ) || ( (millis() - t_timeout_sonar) > 1000 )) {

      /* ho la misura, posso uscire da questo stato */
      distanze_sonar[sonar_indice_array] = d;

      /* calcolo nuovo indice */
      if ((sonar_indice_array + 1) < N_PUNTI_MISURA_SONAR_MAX) {
        sonar_indice_array ++;
      }

      /* calcolo nuova posizione */
      bool finito = false;
      if ( sonar_angolo_arrivo > sonar_angolo_partenza ) {
        sonar_angolo_prossima_scansione += sonar_angolo_step;
        if ( sonar_angolo_prossima_scansione > sonar_angolo_arrivo) {
          finito = true;
        }
      }
      else {
        sonar_angolo_prossima_scansione -= sonar_angolo_step;
        if ( sonar_angolo_prossima_scansione < sonar_angolo_arrivo) {
          finito = true;
        }
      }

      if ( ! finito ) {

        sonar_angolo_partenza_intermedia = angolo_sonar;
        stato_macchina_sonar  = SONAR_SPOSTAMENTO;
        sonar_t0_spostamento  = millis();
        
      }
      else {
        /* devo trasmettere i valori dell'array */
          
        Serial.print("{\"cmd\":\"SONAR\",\"data\":[");
        for (int i=0; i<sonar_indice_array; i++) {
          Serial.print(distanze_sonar[i]);
          if ((i+1) < sonar_indice_array) {
            Serial.print(",");
          }
        }
        Serial.println("]}");

        stato_macchina_sonar = SONAR_STANDBY;
      } 
    }
  }
  else { /* SONAR_STANDBY  */
   
  }
}

void macchinaStatiMotore() {

  int n_target_ok;
  int v[4];
  long p[4];

  /* VERIFICARE A POLLING GLI INGRESSI ENCODER */
  checkEncoder();

  noInterrupts();
  v[0] = velocitaRPM[0];
  v[1] = velocitaRPM[1];
  v[2] = velocitaRPM[2];
  v[3] = velocitaRPM[3];

  psx = p[0] = posizione[0];
  pdx = p[1] = posizione[1];
  p[2] = posizione[2];
  p[3] = posizione[3];
  interrupts();

  if ( stato_macchina_motore == CMD_RUN ) {
    // va a "ruota libera"
    setVelocitaMotore1( vsx ); setDirezione( 0, vsx );
    setVelocitaMotore2( vdx ); setDirezione( 1, vdx );
    setVelocitaMotore3( vsx ); setDirezione( 2, vsx );
    setVelocitaMotore4( vdx ); setDirezione( 3, vdx );
  }
  else {
    /* controllo velocita ruote */
    if ((millis() - tempoRitardoControllo) > RITARDO_CONTROLLO_MS) {
      tempoRitardoControllo = millis();

      n_target_ok = 0;
      for (int nm = 0; nm < 4; nm++) {

        if (p[nm] - p_target[nm] > SOGLIA_ARRIVO) {
          setPointVelocitaRPM[nm] = -20 + (p_target[nm] - p[nm]);
        }
        else if (p[nm] - p_target[nm] < -SOGLIA_ARRIVO) {
          setPointVelocitaRPM[nm] = 20 + (p_target[nm] - p[nm]);
        }
        else { /* (p[nm] == p_target[nm]) */
          setPointVelocitaRPM[nm] = 0;
          n_target_ok ++;
        }

        if (setPointVelocitaRPM[nm] > 0) {
          if (v[nm] < setPointVelocitaRPM[nm]) {
            if (pwmRuota[nm] < 255) {
              pwmRuota[nm] ++;
            }
          }
          else if (v[nm] > setPointVelocitaRPM[nm]) {
            if (pwmRuota[nm] > 0)   {
              pwmRuota[nm] --;
            }
          }
        }
        else if (setPointVelocitaRPM[nm] < 0) {
          if (v[nm] > setPointVelocitaRPM[nm]) {
            if (pwmRuota[nm] > -255) {
              pwmRuota[nm] --;
            }
          }
          else if (v[nm] < setPointVelocitaRPM[nm]) {
            if (pwmRuota[nm] < 0)   {
              pwmRuota[nm] ++;
            }
          }
        }
        else {
          pwmRuota[nm] = 0;
        }
      }

      setVelocitaMotore1( pwmRuota[0] ); setDirezione(0, pwmRuota[0]);
      setVelocitaMotore2( pwmRuota[1] ); setDirezione(1, pwmRuota[1]);
      setVelocitaMotore3( pwmRuota[2] ); setDirezione(2, pwmRuota[2]);
      setVelocitaMotore4( pwmRuota[3] ); setDirezione(3, pwmRuota[3]);

      if ( stato_macchina_motore == CMD_GOTO ) {
        p_target[0] = target_sx;
        p_target[1] = target_dx;
        p_target[2] = target_sx;
        p_target[3] = target_dx;

        vsx = pwmRuota[0];
        vdx = pwmRuota[1];
      }

      if (n_target_ok == 4) {
        /* ho raggiunto il target, considero adesso dove andare */

        //decisioneCosaFare();

#if 0
        Serial.print(N_PUNTI_MISURA_SONAR);
        Serial.print(" - ");

        for (int i = 0; i < N_PUNTI_MISURA_SONAR; i++) {
          Serial.print(distanze_sonar[i]);
          Serial.print(", ");
        }

        const String s[] = {"STAI_FERMO", "VAI_A_SX", "VAI_A_DX", "VAI_DRITTO", "GIRA_90_GRADI"};
        Serial.println( s[stato_macchina_motore] );
#endif

        switch (stato_macchina_motore) {
          case VAI_A_SX :
            p_target[0] += 15;
            p_target[1] += 30;
            p_target[2] += 15;
            p_target[3] += 30;
            break;

          case VAI_A_DX :
            p_target[0] += 30;
            p_target[1] += 15;
            p_target[2] += 30;
            p_target[3] += 15;
            break;

          case VAI_DRITTO :
            p_target[0] += 20;
            p_target[1] += 20;
            p_target[2] += 20;
            p_target[3] += 20;
            break;

          case GIRA_90_GRADI :
            p_target[0] += 20;
            p_target[1] -= 20;
            p_target[2] += 20;
            p_target[3] -= 20;
            break;

          default :
            break;
        }
      }
    }
  }

#if 0
  Serial.print(p[0]);
  Serial.print(",");
  //Serial.println(pwmRuota[0]);
  Serial.print(p[1]);
  Serial.print(",");
  Serial.print(p[2]);
  Serial.print(",");
  Serial.println(p[3]);
#endif

}

void setup() {
  int i;

  Serial.begin( 115200 );

  initMotori();
  initEncoder();
  initSonar();

  pwmRuota[0] = 0;
  pwmRuota[1] = 0;
  pwmRuota[2] = 0;
  pwmRuota[3] = 0;

  setVelocitaMotore1( pwmRuota[0] );
  setVelocitaMotore2( pwmRuota[1] );
  setVelocitaMotore3( pwmRuota[2] );
  setVelocitaMotore4( pwmRuota[3] );

  setPointVelocitaRPM[0] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[1] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[2] = 0;  /* espresso in RPM */
  setPointVelocitaRPM[3] = 0;  /* espresso in RPM */

  t_timeout_sonar = millis();
  
  tempoRitardoControllo = millis();

  velocita = 0;

  jsonLen = 0;

  stato_macchina_motore = STAI_FERMO;
  stato_macchina_sonar  = SONAR_STANDBY;

}

void loop() {
  char c;

  /* acquisizione della stringa da seriale */
  while (Serial.available() != 0) {
    c = (char)Serial.read();
    if (c == '{') {
      jsonLen = 0;
    }

    jsonStr[jsonLen] = c;
    jsonLen++; if (jsonLen >= JSON_STRING_SIZE) {
      jsonLen--;
    }
    jsonStr[jsonLen] = 0;

    if (c == '}') {

      DeserializationError error = deserializeJson(docJson, jsonStr);

      if (error) {
        /* todo: elimiare o modificare questa risposta,
            per esempio rispondendo con un NACK via JSON
        */
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }
      else {
        const char* cmd = docJson["cmd"];
        String str = cmd;

        if ( str.compareTo("RUN") == 0 ) {
          long sx = docJson["sx"];
          long dx = docJson["dx"];

          stato_macchina_motore = CMD_RUN;
          vsx = sx;
          vdx = dx;

          Serial.println("{\"cmd\":\"RUN\",\"data\":\"ACK\"}");
        }
        else if ( str.compareTo("GOTO") == 0 ) {
          long sx = docJson["sx"];
          long dx = docJson["dx"];

          stato_macchina_motore = CMD_GOTO;
          target_sx = sx;
          target_dx = dx;

          Serial.println("{\"cmd\":\"GOTO\",\"data\":\"ACK\"}");
        }
        else if ( str.compareTo("STATUS") == 0 ) {

          Serial.print("{\"cmd\":\"STATUS\",\"data\":{");
          Serial.print("\"psx\":");
          Serial.print(psx);
          Serial.print(",\"pdx\":");
          Serial.print(pdx);
          Serial.print(",\"vsx\":");
          Serial.print(vsx);
          Serial.print(",\"vdx\":");
          Serial.print(vdx);
          Serial.println("}}");

        }
        else if ( str.compareTo("SONAR") == 0 ) {

          long _min = docJson["min"];
          long _max = docJson["max"];
          long _step = docJson["step"];

          sonar_angolo_partenza = 90 - _min;   // direzione inversa
          sonar_angolo_arrivo   = 90 - _max;   // direzione inversa
          sonar_angolo_step     = abs(_step);
          sonar_indice_array    = 0;
          sonar_angolo_partenza_intermedia = angolo_sonar;
          sonar_angolo_prossima_scansione = sonar_angolo_partenza;
          stato_macchina_sonar  = SONAR_SPOSTAMENTO;
          sonar_t0_spostamento  = millis();

        }
        else {

          Serial.print("{\"cmd\":\"");
          Serial.print(str);
          Serial.println("\",\"data\":\"NACK\"}");

        }
      }
    }
  }

  macchinaStatiSonar();
  macchinaStatiMotore();

}
