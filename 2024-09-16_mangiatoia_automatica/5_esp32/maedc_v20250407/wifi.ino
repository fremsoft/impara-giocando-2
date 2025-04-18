#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"

uint32_t time_is_configured = 0;
int ora_ore, ora_minuti;

void startWifi() {
  WiFi.begin(getSSID(), getPasswd());

  /**
   * NTP server address could be acquired via DHCP,
   *
   * NOTE: This call should be made BEFORE esp32 acquires IP address via DHCP,
   * otherwise SNTP option 42 would be rejected by default.
   * NOTE: configTime() function call if made AFTER DHCP-client run
   * will OVERRIDE acquired NTP server address
   */
  esp_sntp_servermode_dhcp(1);  // (optional)

  time_is_configured = 0;
  ora_ore = 0; ora_minuti = 0;
}

bool checkWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    if (time_is_configured == 0) {
      // set notification call-back function
      sntp_set_time_sync_notification_cb(timeavailable);

      // VERIFICA SU TERMINALE:
      // time_t now = time(NULL);
      // Serial.println(ctime(&now));  

#ifdef MANUAL_DAYLIGHTSAVING
      /**
       * This will set configured ntp servers and constant TimeZone/daylightOffset
       * should be OK if your time zone does not need to adjust daylightOffset twice a year,
       * in such a case time adjustment won't be handled automagically.
       */
      configTime(getFusoOrario() * 3600, getOraLegale() ? (3600):(0), getTimeURL().c_str(), "time.google.com");
#else      
      configTime(0, 0, getTimeURL().c_str(), "time.google.com");

      /* Impostazione dell'ora legale per l'Italia in standard POSIX:
         https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap08.html

          "CET-1CEST,M3.5.0/2,M10.5.0/3"

          significa:
            "CET-1" - Central European Time, UTC+1
            "CEST"  - Central European Summer Time
            "M3.5.0/2" - inizia l’ultima domenica di marzo alle 2:00
            "M10.5.0/3" - finisce l’ultima domenica di ottobre alle 3:00
      */
      //setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
      setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
      tzset();
#endif
      
      time_is_configured = millis();
    }
    else {
      printLocalTime( false );
    }

    return true;
  }
  /* else */
  if (time_is_configured != 0) { printLocalTime( false ); }
  return false;
}

void printLocalTime(bool print_it) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    if (print_it) { 
      Serial.println("No time available (yet)"); 
    }
    ora_ore = ora_minuti = 0;
    return;
  }
  if (print_it) { 
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }

  ora_ore = timeinfo.tm_hour;
  ora_minuti = timeinfo.tm_min;
}

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
  printLocalTime( true );
}

int16_t getMinutiDay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) { return -1; }
  return (60*ora_ore)+ ora_minuti;
}