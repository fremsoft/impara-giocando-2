#include <Arduino.h>
#include <WiFi.h>
//#include <time.h>
#include <esp_sntp.h>
#include <ESP_Mail_Client.h>
#include "C:\Users\frems\Documents\credenziali-smtp2go.h"

#define WIFI_SSID         "Fremsoft Inc."
#define WIFI_PASSWORD     "fremsoft"

//#define SMTP_USERNAME    "Username SMTP2GO"
//#define SMTP_PASSWORD    "Password SMTP2GO"

#define SMTP_SERVER       "mail.smtp2go.com";
/*  25  or esp_mail_smtp_port_25
 * 465 or esp_mail_smtp_port_465
 * 587 or esp_mail_smtp_port_587
 */
#define SMTP_PORT         esp_mail_smtp_port_587
#define VERIFIED_SENDER   "smtp2go@fremsoft.it" 
#define MAIL_TO_NAME      "Emanuele Frisoni"
#define MAIL_TO_EMAIL     "fremsoft@libero.it"

SMTPSession smtp;

void smtpCallback(SMTP_Status status);
void timeavailable(struct timeval *t);

void setup() {
  // Inizializza la connessione seriale
  Serial.begin(115200);

  // Connetti ESP32 alla rete WiFi
  WiFi.begin( WIFI_SSID, WIFI_PASSWORD );
  esp_sntp_servermode_dhcp(1);  // (optional)

  Serial.print("Connessione in corso...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connesso a WiFi con IP: ");
  Serial.println(WiFi.localIP());

  sntp_set_time_sync_notification_cb(timeavailable);

  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
  
  MailClient.networkReconnect(true);
  smtp.debug(1);
  smtp.callback(smtpCallback);

  Session_Config config;
  config.server.host_name = SMTP_SERVER;
  config.server.port = SMTP_PORT;
  config.login.email = SMTP_USERNAME;
  config.login.password = SMTP_PASSWORD;
  //config.login.user_domain = F("127.0.0.1");
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  //config.time.gmt_offset = 3;
  //config.time.day_light_offset = 0;

  SMTP_Message message;
  message.sender.name = F("#MAEDC");
  message.sender.email = VERIFIED_SENDER;
  message.subject = F("Test Email da #MAEDC");
  message.addRecipient( MAIL_TO_NAME, MAIL_TO_EMAIL );

  String htmlMsg = "<p>Ciao, questa è una prova dalla tua <b>#MAEDC</b> con ESP32!</p>";
  message.html.content = htmlMsg;
  message.html.charSet = F("utf-8");
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  // message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;
  // message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));
  // smtp.setTCPTimeout(10);

  /* Connect to the server */
  if (!smtp.connect(&config))
  {
    MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn())
  {
    Serial.println("Error, Not yet logged in.");
  }
  else
  {
    if (smtp.isAuthenticated()) { Serial.println("Successfully logged in."); }
    else { Serial.println("Connected with no Auth."); }
  }

  // Invia l'email
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Email inviata con successo!");
  } else {
    MailClient.printf("Errore nell'invio dell'email, Status Code: %d, Error Code: %d, Reason: %s\n", 
                      smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  }

  MailClient.printf("Free Heap: %d\n", MailClient.getFreeHeap());
}

void loop() {
  /* do nothing */
}

void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    // MailClient.printf used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    MailClient.printf("Message sent success: %d\n", status.completedCount());
    MailClient.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
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

// Callback function (gets called when time adjusts via NTP)
void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP!");
}