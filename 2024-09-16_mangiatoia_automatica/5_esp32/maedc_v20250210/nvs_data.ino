#include <Preferences.h>

// !!! ATTENZIONE !!!
// massima lunghezza keys o namespace e' 15 caratteri
Preferences preferences;

String wifi_ssid;
String wifi_pass;
int16_t fuso_orario;
int16_t ora_legale;
String time_server;
bool usa_ntp;

struct Pasto pasti[5];

uint16_t grammi_pasto_now;

bool datiStorageCambiati;

void setupDatiNV(void) {
  if (!preferences.begin("storage", false)) {
    Serial.println("ERRORE: NVS non disponibile!");
  } else {
    Serial.println("NVS inizializzato correttamente.");

    datiStorageCambiati = preferences.getBool("prima_scrittura", true);
    Serial.print("Prima scrittura: ");
    Serial.println((datiStorageCambiati) ? ("Sì") : ("No"));

    wifi_ssid = preferences.getString("wifi_ssid", "Fremsoft Inc.");
    wifi_pass = preferences.getString("wifi_pass", "fremsoft");
    fuso_orario = preferences.getInt("fuso_orario", 1);
    ora_legale = preferences.getInt("ora_legale", 0);
    time_server = preferences.getString("time_server", "pool.ntp.org");
    grammi_pasto_now = preferences.getInt("g_pasto_now", 100);

    Serial.print("WiFi SSID: ");         Serial.println(wifi_ssid);
    Serial.print("WiFi Password: ");     Serial.println(wifi_pass);
    Serial.print("Fuso orario: ");       Serial.println(fuso_orario);
    Serial.print("Ora legale: ");        Serial.println((ora_legale) ? ("Sì") : ("No"));
    Serial.print("Server orario: ");     Serial.println(time_server);
    Serial.print("Grammi pasto now: ");  Serial.println(grammi_pasto_now);

    for (int i = 0; i < 5; i++) {
      pasti[i].ora    = preferences.getInt(("ora_" + String(i)).c_str(),  8 + 3 * i);
      pasti[i].minuti = preferences.getInt(("minuti_" + String(i)).c_str(),   0);
      pasti[i].grammi = preferences.getInt(("grammi_" + String(i)).c_str(),  100);
      pasti[i].attivo = preferences.getBool(("attivo_" + String(i)).c_str(), false);

      Serial.printf("Pasto #%d -> Ore: %2d:%02d, Grammi: %d, attivo: %s\n",
                    i + 1, pasti[i].ora, pasti[i].minuti, pasti[i].grammi, (pasti[i].attivo) ? ("ON") : ("OFF"));
    }
  }

  preferences.end();

  salvataggioDatiNV();
}

void salvataggioDatiNV() {
  if (datiStorageCambiati) {
    if (!preferences.begin("storage", false)) {
      Serial.println("ERRORE: NVS non disponibile!");
    } else {
      Serial.println("NVS inizializzato correttamente.");

      // SCRIVO I NUOVI DATI
      preferences.clear();
      
      // assicura che non usera' mai piu' i dati di default
      preferences.putBool("prima_scrittura", false);

      // memorizzazione della configurazione di rete
      preferences.putString("wifi_ssid", wifi_ssid);
      preferences.putString("wifi_pass", wifi_pass);
      preferences.putInt("fuso_orario", fuso_orario);
      preferences.putInt("ora_legale", ora_legale);
      preferences.putString("time_server", time_server);
      preferences.putBool("usa_ntp", usa_ntp);

      preferences.putInt("g_pasto_now", grammi_pasto_now);

      // memorizzazione dei pasti
      for (int i = 0; i < 5; i++) {
        preferences.putInt(("ora_" + String(i)).c_str(), pasti[i].ora);
        preferences.putInt(("minuti_" + String(i)).c_str(), pasti[i].minuti);
        preferences.putInt(("grammi_" + String(i)).c_str(), pasti[i].grammi);
        preferences.putBool(("attivo_" + String(i)).c_str(), pasti[i].attivo);
      }

      preferences.end();

      datiStorageCambiati = false;
      Serial.println("Dati salvati in NVS");
    }
  }
}

String getSSID(void) {
  return wifi_ssid;
}

String getPasswd(void) {
  return wifi_pass;
}

String getTimeURL(void) {
  return time_server;
}

int16_t getFusoOrario(void) {
  return fuso_orario;
}

bool getOraLegale(void) {
  return ora_legale;
}

Pasto getPasto(int indice) {
  return pasti[indice];
}

uint16_t getGrammiPastoNow(void) {
  return grammi_pasto_now;
}

void setGrammiPastoNow(uint16_t peso) {
  if (peso != grammi_pasto_now) {
    grammi_pasto_now = peso;
    datiStorageCambiati = true;
  }
}
