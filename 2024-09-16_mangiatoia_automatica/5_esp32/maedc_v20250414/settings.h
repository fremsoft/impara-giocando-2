//#define PIN_LED      32
#define PIN_BUZZER   32   //forse ci mettiamo un buzzer al posto del LED

#define HX711_DT     35
#define HX711_SCK    25
#define HX711_GAIN  128   // 25 ck
//#define HX711_GAIN   64    // 27 ck

#define TFT_CS        5
#define TFT_DC       21
#define TFT_RST      22

#define TOUCH_CS     15
#define TOUCH_IRQ    14

#define SCREEN_W    320
#define SCREEN_H    240

/*
#define SD_CS        13     non c'è SD CARD
*/

#define ENCODER_A    36
#define ENCODER_B    39
#define ENCODER_SW   34

#define SERVO_PIN    12 
#define SERVO_POWER  26

#define INPUT_REFILL 27

#define RS485_RX     16
#define RS485_TX     17
#define RS485_TXEN   33

#define T_BUZZER_MS  100

/*
#define FONT_THIN    &Roboto_Thin_24
#define FONT_MEDIUM  &Orbitron_Light_24
#define FONT_BIG     &Orbitron_Light_32
#define FONT_CORSIVO &Satisfy_24
#define FONT_CORSIVO_GRASSETTO &Yellowtail_32
*/
#define FONT_SMALL   &FreeSans9pt7b
#define FONT_MEDIUM  &FreeSans12pt7b
#define FONT_LARGE   &FreeSans18pt7b
#define FONT_BIG     &FreeSans24pt7b
                        
#ifndef _STRUCT_PASTO_
#define _STRUCT_PASTO_
struct Pasto {
  uint8_t ora;
  uint8_t minuti;
  uint16_t grammi;
  bool attivo;
};
#endif

#define MAX_PORZIONE_GRAMMI             1000
#define MARGINE_PESATURA_VELOCITA_G        5
#define MAX_PESO_PER_PESATA_G             50
#define TIMEOUT_SINGOLA_EROGAZIONE_MS  60000

#define DEG_SERVO_CHIUSO   10
#define DEG_SERVO_APERTO   180

#define SPEED_COCLEA_VELOCE   800
#define SPEED_COCLEA_LENTO    500
#define SPEED_COCLEA_FERMO      0

// commentare se si vuole la gestione dell'ora legale automatica
//#define MANUAL_DAYLIGHTSAVING  

#include "C:\Users\frems\Documents\credenziali-smtp2go.h"
//#define SMTP_USERNAME    "Username SMTP2GO"
//#define SMTP_PASSWORD    "Password SMTP2GO"

#define SMTP_SERVER       "mail.smtp2go.com";
/*   25  or esp_mail_smtp_port_25
 *  465 or esp_mail_smtp_port_465
 *  587 or esp_mail_smtp_port_587   */
#define SMTP_PORT         esp_mail_smtp_port_587
#define VERIFIED_SENDER_NAME   "#MAEDC" 
#define VERIFIED_SENDER_EMAIL  "smtp2go@fremsoft.it" 
#define MAIL_TO_NAME      "Emanuele Frisoni"
#define MAIL_TO_EMAIL     "fremsoft@libero.it"

