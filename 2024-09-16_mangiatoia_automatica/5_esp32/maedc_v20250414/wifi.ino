#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"
#include <ESP_Mail_Client.h>

uint32_t time_is_configured = 0;
int ora_ore, ora_minuti;

bool mail_sent;

SMTPSession smtp;
Session_Config config;
SMTP_Message message;

void smtpCallback(SMTP_Status status);

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

  MailClient.networkReconnect(true);
  //smtp.debug(1);
  smtp.callback(smtpCallback);

  config.server.host_name = SMTP_SERVER;
  config.server.port = SMTP_PORT;
  config.login.email = SMTP_USERNAME;
  config.login.password = SMTP_PASSWORD;
  //config.login.user_domain = F("127.0.0.1");
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  //config.time.gmt_offset = 3;
  //config.time.day_light_offset = 0;

  time_is_configured = 0;
  ora_ore = 0;
  ora_minuti = 0;
  mail_sent = true;
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
      configTime(getFusoOrario() * 3600, getOraLegale() ? (3600) : (0), getTimeURL().c_str(), "time.google.com");
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
    } else {
      printLocalTime(false);
    }

    // Gestione invio email
    if (!mail_sent) {
      /* Connect to the server */
      if (!smtp.connect(&config)) {
        MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        return false;
      }

      if (!smtp.isLoggedIn()) {
        Serial.println("Error, Not yet logged in.");
        return false;
      } else {
        if (smtp.isAuthenticated()) {
          Serial.println("Successfully logged in.");
        } else {
          Serial.println("Connected with no Auth.");
        }
      }

      // Invia l'email
      MailClient.sendMail(&smtp, &message);
      if ((smtp.statusCode() == 221) && (smtp.errorCode() == 0)) {
        Serial.println("Email inviata con successo!");
        mail_sent = true;
      } 
      else {
        MailClient.printf("Errore nell'invio dell'email, Status Code: %d, Error Code: %d, Reason: %s\n",
                          smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        // TODO : gestione del reinvio ritardato
      }

      MailClient.printf("Free Heap: %d\n", MailClient.getFreeHeap());
    }

    return true;
  }
  /* else */
  if (time_is_configured != 0) { 
    printLocalTime(false); 
  }
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
  printLocalTime(true);
}

int16_t getMinutiDay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) { return -1; }
  return (60 * ora_ore) + ora_minuti;
}

void prepareEmail(const String subject, const String htmlMsg ) {
  message.sender.name  = VERIFIED_SENDER_NAME;
  message.sender.email = VERIFIED_SENDER_EMAIL;
  message.subject = subject;
  message.addRecipient(MAIL_TO_NAME, MAIL_TO_EMAIL);

  message.html.content = htmlMsg;
  message.html.charSet = F("utf-8");
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  // message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
  // message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));
  // smtp.setTCPTimeout(10);

  mail_sent = false;
}

void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    // MailClient.printf used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    MailClient.printf("Message sent success: %d\n", status.completedCount());
    MailClient.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

      MailClient.printf("Message No: %d\n", i + 1);
      MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
      MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      MailClient.printf("Recipient: %s\n", result.recipients.c_str());
      MailClient.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}
