#include "valve_config.h"

#include <WiFi.h>
WiFiClient espClient;

#include "time.h"
const long gmtOffset_sec = 3600;   // GMT +1: 3600 Sekunden Offset
const int daylightOffset_sec = 0;  // Lass die Anpassung automatisch erfolgen
struct tm timeinfo;                // Deklariere timeinfo als globale Variable
String hhmm;

#include <PubSubClient.h>
PubSubClient client(espClient);

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long startZeit = 0;  // Speichert den Startzeitpunkt der Verzögerung
int verzogerungsDauer = 0;    // Speichert die Dauer der Verzögerung in Sekunden
bool timerAktiv = false;      // Flag, um zu überprüfen, ob der Timer läuft


void setup() {

  pinMode(relaisPin, OUTPUT);
  digitalWrite(relaisPin, HIGH);

  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("Hello World!");
  lcd.setCursor(0, 1);
  lcd.print("Modul is booting");
  delay(1000);

  setup_wifi();
  initializeTime();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (timerAktiv) {
    if (millis() - startZeit >= verzogerungsDauer * 1000) {  // VerzögerungsDauer in Millisekunden umrechnen
      closeValve();
    }
  } else {
    updateDisplayData();
  }
}

void closeValve() {
  digitalWrite(relaisPin, HIGH);  // Schalte Pin 27 wieder HIGH -->> Ventil zu
  timerAktiv = false;             // Stoppe den Timer
  Serial.println("Timer abgelaufen, Pin 27 wurde HIGH gesetzt.");

  lcd.setCursor(0, 1);
  lcd.print("   geschlossen   ");

  client.publish(feedback_topic, "Ventil wurde erfolgreich geschlossen");
  delay(700);
}

void callback(char* topic, byte* message, unsigned int length) {
  lcd.setCursor(0, 0);
  lcd.print(" Serveranfrage  ");
  lcd.setCursor(0, 1);
  lcd.print("wird bearbeitet ");

  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  // Konvertiere die eingehende Nachricht in einen String
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.print("Payloud on the toppic: ");
  Serial.println(msg);


  if (strcmp(topic, valve_topic) == 0) {
    handleControlValve(msg);
  } else if (strcmp(topic, notaus_topic) == 0) {
    closeValve();
  } else {
    Serial.print("Unbekanntes Topic: ");
    Serial.println(topic);
    client.publish(feedback_topic, "Ungültiger Befehl übermittelt.");
  }
}

void updateDisplayData() {
  static unsigned long lastUpdate = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= 2000) {  // Prüfe, ob 2 Sekunden vergangen sind
    lastUpdate = currentMillis;              // Aktualisiere die letzte Aktualisierungszeit
    lcd.clear();
    printHHMM();  // Aktualisiere die Zeit am Ende der ersten Zeile, die Funktion kümmert sich um die korrekte Position
  }
}


void handleControlValve(String message) {
  int messageValue = message.toInt();

  if (messageValue >= 0 && messageValue <= 3600) {
    Serial.print("Empfangener Wert: ");
    Serial.println(messageValue);

    verzogerungsDauer = messageValue;  // Speichere die Verzögerungsdauer
    digitalWrite(relaisPin, LOW);      // Schalte Pin 27 LOW
    startZeit = millis();              // Speichere den Startzeitpunkt
    timerAktiv = true;                 // Starte den Timer

    lcd.setCursor(0, 0);
    lcd.print(" Ventil-Status: ");
    lcd.setCursor(0, 1);
    lcd.print("open for ");
    lcd.setCursor(12, 1);
    lcd.print(messageValue);

    Serial.println("Pin 27 wurde LOW gesetzt. Timer gestartet.");
    client.publish(feedback_topic, "Ventil wurde erfolgreich geöffnet");


  } else {
    // Der Wert liegt außerhalb des zulässigen Bereichs oder die Konvertierung war nicht erfolgreich
    Serial.println("Empfangener Wert ist ungültig.");
    client.publish(feedback_topic, "Ungültiger Bewässerungsintervall");
  }
}


void reconnect() {
  while (!client.connected()) {
    lcd.setCursor(0, 0);
    lcd.print("  MQTT Status:  ");
    lcd.setCursor(0, 1);
    lcd.print("   connecting   ");
    delay(850);

    Serial.println("Attempting MQTT connection...");
    if (client.connect(clientname, clientname, mqtt_password)) {
      lcd.setCursor(0, 1);
      lcd.print("   connected    ");
      Serial.println("Connected to MQTT broker");
      client.subscribe(valve_topic, 1);
      client.subscribe(notaus_topic, 1);

    } else {
      Serial.print("Failed, rc=");
      int clientState = client.state();
      Serial.print(clientState);
      lcd.setCursor(0, 1);
      lcd.print("  errorcode: ");
      lcd.setCursor(14, 1);
      lcd.print(clientState);
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  lcd.setCursor(0, 0);
  lcd.print("  WLAN Staus:  ");
  lcd.setCursor(0, 1);
  lcd.print(" not connected  ");

  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.setCursor(0, 1);
  lcd.print("   connected    ");

  Serial.println("");
  Serial.println("WiFi connected");
  delay(1000);
}

void initializeTime() {
  // Konfiguriere Zeitserver und Zeitzone
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  lcd.setCursor(0, 0);
  lcd.print("   Timeserver   ");
  lcd.setCursor(0, 1);
  lcd.print(" not connected  ");


  Serial.println("Warte auf Zeit synchronisation...");
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("Zeit synchronisiert: ");
  Serial.println(getHHMM());


  lcd.setCursor(0, 1);
  lcd.print("   connected    ");
}

String getHHMM() {
  if (!getLocalTime(&timeinfo)) {
    return "Fehler";  // Falls keine Zeit abgerufen werden kann
  }
  char timeString[6];  // HH:MM + Null-Terminator
  strftime(timeString, sizeof(timeString), "%H:%M", &timeinfo);
  return String(timeString);  // Konvertiere den char-Array in ein String-Objekt und gebe es zurück
}

void printHHMM() {
  Serial.println("Zeit wird auf dem Display aktualisiert");
  hhmm = getHHMM();
  lcd.setCursor(11, 0);
  lcd.print(hhmm);
  Serial.print("Aktuelle Zeit: ");
  Serial.println(hhmm);
}
