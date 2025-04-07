// Programma di gestione della scheda MAEDC v1.1
// per erogare fino a 5 pasti al giorno.
//
// Guarda la lezione integrale:
// https://youtube.com/live/9WoMDbCGiEg

// Questo sketch usa le seguenti librerie:
//  - TFT_eSPI by Bodmer                 v2.5.43 (per TFT e TOUCH)
//  - ESP32Servo by Kevin H., John K.B.  v3.0.5  (per Servo SG90)

// Definizione dei pin in Users\...\Documenti\Arduino\libraries\TFT_eSPI\User_Setup.h

#include "settings.h"
#include "images.h"
#include <SPI.h>
#include <TFT_eSPI.h>

extern TFT_eSPI tft;
extern TFT_eSprite tftBuff;

extern int actualScreen, nextScreen, currentElement, currentChar;
extern int16_t yOffset, elementValues[15];

int statoCoclea;   // 0 = ferma,  n = speed

int statoServo;    // 0 = chiuso, 1 = aperto
int degServo; 

int statoErogazioneCibo; // 0 = riposo; 
                         // 1 = apertura sportellino (chiude diaframma)
                         // 2 = erogazione cibo
                         // 3 = scarico cibo (chiusura sportellino)
uint16_t grammiDaErogare; 

uint32_t t0, t0_erogazione;
bool     timeout_erogazione_expired;

int16_t  t_ultima_erogazione_minuti;
int16_t  minuti_day_old;

void setup() {

  Serial.begin( 115200 );
  //printFreeRAM();

  pinMode     ( PIN_BUZZER, OUTPUT );   
  digitalWrite( PIN_BUZZER, LOW );

  pinMode     ( INPUT_REFILL, INPUT );   

  setupDatiNV();
  startWifi();

  setupBilancia();
  setupTFT();
  setupEncoder();
  setupServo();
  setupRS485( 19200 );

  delay( 3000 );

  statoCoclea = 0;  // ferma

  statoServo  = 0;  // chiuso
  degServo = DEG_SERVO_CHIUSO;

  statoErogazioneCibo = 0; // 0 = riposo; 
  grammiDaErogare = 0;

  t0 = t0_erogazione = 0;
  timeout_erogazione_expired = false;

  t_ultima_erogazione_minuti = -1;
  minuti_day_old = -1;

  speedCoclea( statoCoclea );
  
  servoON();
  servoSetPos(degServo);
  delay(1000);
  servoOFF();

  //printFreeRAM();
}

int c=0;
void loop() {
  String strTmp;
  char str[16];
  int32_t grammi;
  int el, xCursor, yCursor;  
  int degSetPoint;    
  int16_t  minuti_temp, minuti_now;

  // YOFFSET
  // serve per disegnare mezza schermata alla volta poiche' 
  // non c'è abbastanza RAM per creare uno sprite grande quanto
  // tutto lo schermo
  if (yOffset == 0) { yOffset = SCREEN_H/2; } else { yOffset = 0; }

  touchManageClick();

  // GESTIONE DELLE SCHERMATE
  switch ( actualScreen ) {

    case 1 : // schermata principale
      tftBuffStartScreen();

      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_BIG );

      // calcolo del prossimo pasto
      // solo se l'ora e' sincronizzata
      minuti_now = getMinutiDay();  // vale -1 quando non e' sincronizzato

      // se ho appena sincronizzato l'orologio...
      if ((minuti_now >= 0) && (minuti_day_old < 0))
      {
        t_ultima_erogazione_minuti = minuti_now;
      }

      if ( minuti_day_old >= 0) {
        minuti_temp = 24 * 60; /* cerco il minimo partendo dal massimo */
        for (int i=0; i<5; i++) {
          struct Pasto p = getPasto(i);

          if ( p.attivo ) {
            if ( ((60*p.ora) + (p.minuti)) < minuti_temp ) {
              if ( ((60*p.ora) + (p.minuti)) > t_ultima_erogazione_minuti) {
                minuti_temp = (60*p.ora) + (p.minuti);
                grammiDaErogare = p.grammi;
                sprintf(str, "%2d:%02d %dg", p.ora, p.minuti, p.grammi);
              }
            } 
          }
        }

        if ( minuti_temp >= (24*60) ) {
          // non ha trovato un pasto disponibile
          sprintf(str, "Wait midnight");

          if (minuti_day_old > minuti_now) {
            // e' passata la mezzanotte
            // ripristino tutti i pasti
            t_ultima_erogazione_minuti = -1;
          }
        }
        
        if ( minuti_temp <= minuti_now ) {
          // e' ora di mangiare
          statoErogazioneCibo = 0;  // 0 = riposo;
          actualScreen = nextScreen = 3;
          
          t_ultima_erogazione_minuti = minuti_temp;
          sprintf(str, "Serving food");
        }
      }
      else {
        sprintf(str, "Waiting NTP");
      }
      minuti_day_old = minuti_now;  
           
      tftBuff.drawString(str, 160, 90-yOffset);

      tftBuffEndScreen();

      /* salva i dati non volatili 
         solo nella schermata principale 
         per evitare di scrivere troppe volte
         e solo se sono cambiati i dati */
      salvataggioDatiNV(); 
      break;

    case 2 : // schermata erogazione manuale
      grammi = getPosEncoder();
      if (grammi < 1) { grammi = 1; setPosEncoder(grammi); }
      if (grammi > MAX_PORZIONE_GRAMMI) { grammi = MAX_PORZIONE_GRAMMI; setPosEncoder(grammi); }

      tftBuffStartScreen();
      
      // CASELLA SELEZIONE GRAMMI
      tftBuff.fillSmoothRoundRect(30, 65-yOffset, 160, 50,  5, TFT_NAVY);
      tftBuff.fillSmoothRoundRect(31, 66-yOffset, 158, 48,  3, TFT_WHITE);
      
      tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_LARGE );
      sprintf(str, " %dg ", grammi);
      tftBuff.drawString(str, 110, 90-yOffset);

      // PULSANTE GO
      tftBuff.fillSmoothRoundRect(210, 65-yOffset, 80, 50,  5, TFT_NAVY);

      tftBuff.setTextColor(TFT_WHITE, TFT_NAVY);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_LARGE );
      tftBuff.drawString("GO", 250, 90-yOffset);

      tftBuffEndScreen();
      break;

    case 3 : // erogazione in corso
      if (
           ((grammiDaErogare != 0 ) && (!timeout_erogazione_expired))
            || 
           (statoErogazioneCibo != 0)
         ) {
        // devo erogare le crocchette
        if (statoErogazioneCibo == 1) {
          // 1 = apertura sportellino (chiude diaframma)
          degSetPoint = DEG_SERVO_APERTO;
          
          if ( degSetPoint > degServo ) {
            if (degSetPoint > degServo) { degServo += 5; }
            /*if (degSetPoint < degServo) { degServo -= 5; }*/

            servoSetPos(degServo);
            //delay(10);
          }
          else {
            delay(100);
            servoOFF();

            setTara();
            do {
              getPeso();
            } while (abs (getPesoNettoFiltrato()) > 1.0);

            t0_erogazione = millis();
            timeout_erogazione_expired = false;
            statoErogazioneCibo = 2;
          }         
        }
        else 
        if (statoErogazioneCibo == 2)  {
          // 2 = erogazione cibo
          uint16_t p;
          getPeso();
          p = (uint16_t)getPesoNettoFiltrato();
          timeout_erogazione_expired = (millis() - t0_erogazione) > TIMEOUT_SINGOLA_EROGAZIONE_MS;

          if ((p < grammiDaErogare) && (p < MAX_PESO_PER_PESATA_G) && (!timeout_erogazione_expired)) {
            // devo erogare
            if ( p < (grammiDaErogare-MARGINE_PESATURA_VELOCITA_G)) {
              // potrei andare veloce
              if ( p < (MAX_PESO_PER_PESATA_G-MARGINE_PESATURA_VELOCITA_G)) {
                // posso andare veloce
                if (statoCoclea != SPEED_COCLEA_VELOCE) {
                  statoCoclea = SPEED_COCLEA_VELOCE;
                  speedCoclea( statoCoclea );
                }
              }
              else {
                // devo andare lento  
                if (statoCoclea != SPEED_COCLEA_LENTO) {
                  statoCoclea = SPEED_COCLEA_LENTO;
                  speedCoclea( statoCoclea );
                }
              }
            }
            else {
              // devo andare lento
              if (statoCoclea != SPEED_COCLEA_LENTO) {
                statoCoclea = SPEED_COCLEA_LENTO;
                speedCoclea( statoCoclea );
              }
            }
          }
          else {
            // devo scaricare
            statoCoclea = SPEED_COCLEA_FERMO;
            speedCoclea( statoCoclea );
 
            if (grammiDaErogare < p) {
              grammiDaErogare = 0;
            }
            else {
              grammiDaErogare = grammiDaErogare - p;
            }

            servoON();
            delay(100);  

            statoErogazioneCibo = 3;
          }
        }
        else
        if (statoErogazioneCibo == 3) {
          // 3 = scarico cibo (chiusura sportellino)
          degSetPoint = DEG_SERVO_CHIUSO;
          
          if ( degSetPoint < degServo ) {
            /*if (degSetPoint > degServo) { degServo += 5; }*/
            if (degSetPoint < degServo) { degServo -= 5; }

            servoSetPos(degServo);
            //delay(10);
          }
          else {
            delay(100);
            servoOFF();

            delay(2000);           

            statoErogazioneCibo = 0;
          }
        }
        else /* if (statoErogazioneCibo == 0) */ {
          // 0 = riposo

          if (! timeout_erogazione_expired) {
            servoON();
            delay(100);  

            statoErogazioneCibo = 1;
          }
          
        }
      }
      else {
        // erogazione terminata

        servoOFF();
        speedCoclea( SPEED_COCLEA_FERMO );

        if (grammiDaErogare != 0 ) {
          ///////////////////////////////////
          //
          // TODO: invio email di ALLARME
          //
          ///////////////////////////////////
          grammiDaErogare = 0;
        }
  
        t0 = millis();
        actualScreen = nextScreen = 4;
      } 

      tftBuffStartScreen();

      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_BIG );
      sprintf(str, "%.1fg / %ug", getPesoNettoFiltrato(), grammiDaErogare);
      tftBuff.drawString(str, 160, 90-yOffset);

      tftBuffEndScreen();
      break;

    case 4 : // finito di erogare le crocchette (nel bene o nel male)
      tftBuffStartScreen();

      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_BIG );
      if (!timeout_erogazione_expired) {
        sprintf(str, "Enjoy :)"); /* BUON APPETITO */
      } 
      else {
        sprintf(str, "Alarm !!!");
      } 
      tftBuff.drawString(str, 160, 90-yOffset);

      tftBuffEndScreen();

      if ((millis() - t0) > 3000) {
        timeout_erogazione_expired = false;
        actualScreen = nextScreen = 1;
      }
      break;

    case 5 : // programmazione dei pasti
      /* currentElement e' il campo che sto editando 
        -1 o comunque un numero negativo (nessuno)
         0 = ore    1 = minuti     2 = porzione   1' pasto
         3 = ore    4 = minuti     5 = porzione   2' pasto
         6 = ore    7 = minuti     8 = porzione   3' pasto
         9 = ore   10 = minuti    11 = porzione   4' pasto
        12 = ore   13 = minuti    14 = porzione   5' pasto
       */
      if (currentElement >= 0) {
        if ((currentElement%3) == 0) {  // ore
          elementValues[currentElement] = getPosEncoder(); 
          if (elementValues[currentElement] < 0) { elementValues[currentElement] = 0; setPosEncoder(elementValues[currentElement]); }
          if (elementValues[currentElement] > 23) { elementValues[currentElement] = 23; setPosEncoder(elementValues[currentElement]); }
        }
        else
        if ((currentElement%3) == 1) {  // minuti
          elementValues[currentElement] = getPosEncoder();
          if (elementValues[currentElement] < 0) { elementValues[currentElement] = 0; setPosEncoder(elementValues[currentElement]); }
          if (elementValues[currentElement] > 59) { elementValues[currentElement] = 59; setPosEncoder(elementValues[currentElement]); }
        }
        else /* if ((currentElement%3) == 2) */ { // grammi
          elementValues[currentElement] = getPosEncoder();
          if (elementValues[currentElement] < 1) { elementValues[currentElement] = 1; setPosEncoder(elementValues[currentElement]); }
          if (elementValues[currentElement] > MAX_PORZIONE_GRAMMI) { elementValues[currentElement] = MAX_PORZIONE_GRAMMI; setPosEncoder(elementValues[currentElement]); }
        }
      }

      tftBuffStartScreen();
      
      el = 0;
      for (int i=0; i<5; i++) {
        struct Pasto p = getPasto(i);

        // NUMERO DELLA RICETTA
        tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        sprintf(str, "%d", i+1);
        tftBuff.drawString(str, 3+(30/2), 45+(34/2) + (38*i) - yOffset);

        // SIMBOLO DEI DUEPUNTI
        tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        tftBuff.drawString(":", 83+(14/2), 45+(34/2) + (38*i) - yOffset);

        // CASELLA DELLE ORE 
        tftBuff.fillSmoothRoundRect(35, 46 + (38*i) - yOffset, 50, 34,  5, TFT_NAVY);
        if (el == currentElement) {
          tftBuff.setTextColor(TFT_WHITE, TFT_NAVY);
          p.ora = elementValues[el]; setPasto(i, p);
        }
        else {
          elementValues[el] = p.ora;
          tftBuff.fillSmoothRoundRect(36, 47 + (38*i) - yOffset, 48, 32,  3, TFT_WHITE);
          tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        }
      
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        sprintf(str, "%2d", p.ora);
        tftBuff.drawString(str, 35 + (50/2), 45 + (38*i) + (34/2) - yOffset);
        el ++;

        // CASELLA DEI MINUTI
        tftBuff.fillSmoothRoundRect(96, 46 + (38*i) - yOffset, 50, 34,  5, TFT_NAVY);
        if (el == currentElement) {
          tftBuff.setTextColor(TFT_WHITE, TFT_NAVY);
          p.minuti = elementValues[el]; setPasto(i, p);
        }
        else {
          elementValues[el] = p.minuti;
          tftBuff.fillSmoothRoundRect(97, 47 + (38*i) - yOffset, 48, 32,  3, TFT_WHITE);
          tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        }
      
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        sprintf(str, "%02d", p.minuti);
        tftBuff.drawString(str, 96 + (50/2), 45 + (38*i) + (34/2) - yOffset);
        el ++;
        
        // CASELLA DEI GRAMMI
        tftBuff.fillSmoothRoundRect(160, 46 + (38*i) - yOffset, 94, 34,  5, TFT_NAVY);
        if (el == currentElement) {
          tftBuff.setTextColor(TFT_WHITE, TFT_NAVY);
          p.grammi = elementValues[el]; setPasto(i, p);
        }
        else {
          elementValues[el] = p.grammi;
          tftBuff.fillSmoothRoundRect(161, 47 + (38*i) - yOffset, 92, 32,  3, TFT_WHITE);
          tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
        }
      
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        sprintf(str, "%dg", p.grammi);
        tftBuff.drawString(str, 160 + (94/2), 45 + (38*i) + (34/2) - yOffset);
        el ++;

        // PULSANTE ACTIVE
        if (p.attivo) {
          tftBuff.pushImage(264, 47 + (38*i) - yOffset, 32, 32, (uint16_t *)_check_32x32_map);
        }
        else {
          tftBuff.pushImage(264, 47 + (38*i) - yOffset, 32, 32, (uint16_t *)_not_check_32x32_map);
        }
      }

      tftBuffEndScreen();
      break;

    case 6 : // menu impostazioni
      tftBuffStartScreen();

      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_BIG );
      sprintf(str, "Settings");
      tftBuff.drawString(str, 160, 90-yOffset);

      tftBuffEndScreen();
      break;

    case 7 : // impostazione orologio
      elementValues[0] = getPosEncoder(); 
      if (elementValues[0] < 32) { elementValues[0] = 32; setPosEncoder(elementValues[0]); }
      if (elementValues[0] > 127) { elementValues[0] = 255; setPosEncoder(elementValues[0]); }
      
      tftBuffStartScreen();
      
      tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_SMALL );
      tftBuff.drawString("url NTP:", 5 + (70/2), 56 + (26/2) - yOffset);
      
      // CASELLA DELLA URL
      tftBuff.fillSmoothRoundRect(80, 55 - yOffset, 204, 32, 5, TFT_NAVY);
      tftBuff.fillSmoothRoundRect(81, 56 - yOffset, 202, 30, 3, TFT_WHITE);
      tftBuff.setTextDatum(TL_DATUM); 
      tftBuff.setFreeFont(FONT_SMALL);
      
      strTmp = getTimeURL();
      xCursor = 80 + 4; 
      yCursor = 86 - 10;
      for (int i = 0; i < strTmp.length(); i++) {
        xCursor += tftBuff.drawChar(strTmp[i], xCursor, yCursor - yOffset);
      }

      if ((millis() % 1000) > 600) {
        tftBuff.fillRect(xCursor, 57 - yOffset, 20, 28, TFT_NAVY);
        tftBuff.setTextColor(TFT_WHITE, TFT_NAVY);
        tftBuff.drawChar(elementValues[0], xCursor + 2, yCursor - yOffset);
      } 

      tftBuff.pushImage(286, 55 - yOffset, 32, 32, (uint16_t *)_canc_32x32_map);

#ifdef MANUAL_DAYLIGHTSAVING
      // ORA LEGALE
      tftBuff.fillSmoothCircle(60, 120-yOffset, 14, TFT_NAVY);
      tftBuff.fillSmoothCircle(60, 120-yOffset, 12, TFT_WHITE);
      if (getOraLegale()) { tftBuff.fillSmoothCircle(60, 120-yOffset, 8, TFT_NAVY); }        

      tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
      tftBuff.setTextDatum(TL_DATUM);
      tftBuff.setFreeFont( FONT_MEDIUM );
      tftBuff.drawString("summer time", 84, 110 - yOffset);
#endif

      tftBuffEndScreen();
      break;

    case 8 : // impostazione WiFi
      elementValues[0] = getPosEncoder(); 
      if (elementValues[0] < 32) { elementValues[0] = 32; setPosEncoder(elementValues[0]); }
      if (elementValues[0] > 127) { elementValues[0] = 255; setPosEncoder(elementValues[0]); }
      
      tftBuffStartScreen();
      
      tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_SMALL );
      tftBuff.drawString("SSID:", 5 + (70/2), 56 + (26/2) - yOffset);
      
      // CASELLA DELLA SSID  (currentElement = 1)
      tftBuff.fillSmoothRoundRect(80, 55 - yOffset, 204, 32, 5, TFT_NAVY);
      tftBuff.fillSmoothRoundRect(81, 56 - yOffset, 202, 30, 3, TFT_WHITE);
      tftBuff.setTextDatum(TL_DATUM); 
      tftBuff.setFreeFont(FONT_SMALL);
      
      strTmp = getSSID();
      xCursor = 80 + 4; 
      yCursor = 86 - 10;
      for (int i = 0; i < strTmp.length(); i++) {
        xCursor += tftBuff.drawChar(strTmp[i], xCursor, yCursor - yOffset);
      }

      if (currentElement == 1) {
        if ((millis() % 1000) > 600) {
          tftBuff.fillRect(xCursor, 57 - yOffset, 20, 28, TFT_NAVY);
          tftBuff.setTextColor(TFT_WHITE, TFT_NAVY);
          tftBuff.drawChar(elementValues[0], xCursor + 2, yCursor - yOffset);
        } 
        tftBuff.pushImage(286, 55 - yOffset, 32, 32, (uint16_t *)_canc_32x32_map);
      }
      
      tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_SMALL );
      tftBuff.drawString("Passwd:", 5 + (70/2), 101 + (26/2) - yOffset);
      
      // CASELLA DELLA password  (currentElement = 2)
      tftBuff.fillSmoothRoundRect(80, 100 - yOffset, 204, 32, 5, TFT_NAVY);
      tftBuff.fillSmoothRoundRect(81, 101 - yOffset, 202, 30, 3, TFT_WHITE);
      tftBuff.setTextDatum(TL_DATUM); 
      tftBuff.setFreeFont(FONT_SMALL);
      
      strTmp = getPasswd();
      xCursor = 80 + 4; 
      yCursor = 131 - 10;
      for (int i = 0; i < strTmp.length(); i++) {
        xCursor += tftBuff.drawChar('*', xCursor, yCursor - yOffset);
      }

      if (currentElement == 2) {
        if ((millis() % 1000) > 600) {
          tftBuff.fillRect(xCursor, 102 - yOffset, 20, 28, TFT_NAVY);
          tftBuff.setTextColor(TFT_WHITE, TFT_NAVY);
          tftBuff.drawChar(elementValues[0], xCursor + 2, yCursor - yOffset);
        } 
        tftBuff.pushImage(286, 100 - yOffset, 32, 32, (uint16_t *)_canc_32x32_map);
      }

      tftBuffEndScreen();
      break;

    case 9 : // manutenzione
      tftBuffStartScreen();

      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_BIG );
      sprintf(str, "Manual mode");
      tftBuff.drawString(str, 160, 90-yOffset);

      // PULSANTE START STOP COCLEA
      if(statoCoclea == 0) { 
        // coclea ferma
        tftBuff.fillSmoothCircle(55, 175-yOffset, 47, TFT_NAVY);
        tftBuff.fillSmoothCircle(55, 175-yOffset, 44, TFT_GREEN);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        tftBuff.setTextColor(TFT_NAVY, TFT_GREEN);
        tftBuff.drawString("START", 55, 175-yOffset);
      }
      else {
        // coclea in movimento
        tftBuff.fillSmoothCircle(55, 175-yOffset, 47, TFT_BLACK);
        tftBuff.fillSmoothCircle(55, 175-yOffset, 44, TFT_RED);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        tftBuff.setTextColor(TFT_BLACK, TFT_RED);
        tftBuff.drawString("STOP", 55, 175-yOffset);
      }

      // PULSANTE OPEN CLOSE SPORTELLINI
      if(statoServo == 0) { 
        // sportello chiuso/diaframma aperto
        tftBuff.fillSmoothCircle(155, 175-yOffset, 47, TFT_NAVY);
        tftBuff.fillSmoothCircle(155, 175-yOffset, 44, TFT_GREEN);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        tftBuff.setTextColor(TFT_NAVY, TFT_GREEN);
        tftBuff.drawString("OPEN", 155, 175-yOffset);
      }
      else {
        // sportello aperto/diaframma chiuso
        tftBuff.fillSmoothCircle(155, 175-yOffset, 47, TFT_BLACK);
        tftBuff.fillSmoothCircle(155, 175-yOffset, 44, TFT_RED);
        tftBuff.setTextDatum(CC_DATUM);
        tftBuff.setFreeFont( FONT_MEDIUM );
        tftBuff.setTextColor(TFT_BLACK, TFT_RED);
        tftBuff.drawString("CLOSE", 155, 175-yOffset);
      }

      // CASELLA DEL PESO 
      tftBuff.fillSmoothRoundRect(255-(90/2), 175-(60/2) - yOffset, 90, 60,  8, TFT_NAVY);
      tftBuff.fillSmoothRoundRect(257-(90/2), 177-(60/2) - yOffset, 86, 56,  6, TFT_WHITE);
      tftBuff.setTextColor(TFT_NAVY, TFT_WHITE);
      tftBuff.setTextDatum(CC_DATUM);
      tftBuff.setFreeFont( FONT_MEDIUM );
      getPeso();
      sprintf(str, "%.1fg", getPesoNettoFiltrato());
      tftBuff.drawString(str, 255, 175 - yOffset);

      tftBuffEndScreen();
      break;

    default : 
      actualScreen = nextScreen = 1; 
      break;

  }
  
  //if ( digitalRead(INPUT_REFILL) )
  delay(25);
}
