#include <WiFi.h>
#include "time.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SDA_PIN 0
#define SCL_PIN 4

const char* ssid = "DEINWLANNAME";
const char* password = "DEINWLANPW";

const char* ntpServer = "time.cloudflare.com";
const long gmtOffset_sec = 3600;      // GMT +1: 3600 Sekunden Offset
const int   daylightOffset_sec = 0; // Lass die Anpassung automatisch erfolgen

struct tm timeinfo;  // Deklariere timeinfo als globale Variable

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Erstelle ein LCD-Objekt. Adresse: 0x27, 16 Zeichen und 2 Zeilen

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialisiere das LCD
  lcd.init();
  lcd.backlight();

  // Verbinde mit WiFi
  connectToWiFi(ssid, password);

  // Initialisiere Zeit
  initializeTime();
}

void loop() {
  String currentTime = getHHMM();  // Hole die aktuelle Zeit als String
  Serial.println(currentTime);     // Ausgabe in der Konsole
  lcd.setCursor(0, 0);             // Setze den Cursor auf den Anfang der ersten Zeile
  lcd.print(currentTime);          // Zeige die Zeit auf dem LCD an
  delay(10000);                    // Aktualisiere die Zeit alle 10 Sekunden
}

void connectToWiFi(const char* ssid, const char* password) {
  Serial.println("Verbinde mit WiFi Netzwerk");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi verbunden.");
}

void initializeTime() {
  // Konfiguriere Zeitserver und Zeitzone
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Warte auf Zeit synchronisation...");
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Zeit synchronisiert");
}

String getHHMM() {
  if (!getLocalTime(&timeinfo)) {
    return "Fehler";  // Falls keine Zeit abgerufen werden kann
  }
  char timeString[6];  // HH:MM + Null-Terminator
  strftime(timeString, sizeof(timeString), "%H:%M", &timeinfo);
  return String(timeString);  // Konvertiere den char-Array in ein String-Objekt und gebe es zurück
}
