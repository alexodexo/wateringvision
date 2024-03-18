#include "sensorik_config.h"

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

long duration;  //Ultraschall, of sound wave travel
int distance;   //Ultraschall, for the distance measurement

int capacitiveValue;
int capacitivePercent;

int electricValue;
int elecPercent;

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;  // Erstelle ein BME280 Objekt
int temperature;
int pressure;
int humidity;

#include <ArduinoJson.h>
#include <Arduino.h>


void setup() {
  pinMode(trigPin, OUTPUT);

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

  if (!bme.begin(0x76)) {  // Initialisiere den BME280 Sensor
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  updateDisplayData();
}



void updateDisplayData() {
  static unsigned long lastUpdate = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdate >= 2000) {  // Prüfe, ob 2 Sekunden vergangen sind
    lastUpdate = currentMillis;              // Aktualisiere die letzte Aktualisierungszeit

    messure();  // Aktualisiere Messwerte
    lcd.clear();
    // Aktualisiere die obere Zeile mit Bodenfeuchtigkeit
    char topLine[11];  // Char-Array für "Soil: 100%"
    snprintf(topLine, sizeof(topLine), "Soil: %d%%", elecPercent);
    lcd.setCursor(0, 0);  // Setze den Cursor auf den Anfang der ersten Zeile
    lcd.print(topLine);   // Drucke die Bodenfeuchtigkeit

    printHHMM();  // Aktualisiere die Zeit am Ende der ersten Zeile, die Funktion kümmert sich um die korrekte Position

    // Aktualisiere die untere Zeile mit Temperatur, Luftdruck und Luftfeuchtigkeit
    char bottomLine[17];  // Char-Array für die untere Zeile
    snprintf(bottomLine, sizeof(bottomLine), "%dC %dhPa %d%%", temperature, pressure, humidity);
    lcd.setCursor(0, 1);    // Setze den Cursor auf den Anfang der zweiten Zeile
    lcd.print(bottomLine);  // Drucke die gesammelten Messwerte
  }
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


  if (strcmp(topic, request_topic) == 0) {
    handleCallSensor();
  } else {
    Serial.print("Unbekanntes Topic: ");
    Serial.println(topic);
    client.publish(feedback_topic, "Ungültiger Befehl übermittelt.");
  }
}




void handleCallSensor() {
  messure();

  char sensorDataString[60];
  createSensorDataString(distance, electricValue, capacitiveValue, temperature, pressure, humidity, sensorDataString, sizeof(sensorDataString));
  Serial.println(sensorDataString);

  if (client.publish(response_topic, sensorDataString)) {
    Serial.println("Sensorwert erfolgreich veröffentlicht!");
    client.publish(feedback_topic, "Sensordaten erfolgreich veröffentlicht");

  } else {
    Serial.println("Fehler beim Veröffentlichen des Sensorwerts.");
    client.publish(feedback_topic, "Fehler beim Übermitteln der Sensordaten");
  }
}

void messure() {
  bmeMesure();
  ultrasonicMessure();
  electricMessure();
  capacitiveMesure();
}

void createSensorDataString(int ultra, int elec, int cap, int temp, int pres, int humi, char* dataString, int maxLength) {
  snprintf(dataString, maxLength, "ultra:%d;elec:%d;cap:%d;temp:%d;pres:%d;humi:%d", ultra, elec, cap, temp, pres, humi);
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
      client.subscribe(request_topic, 1);


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



void bmeMesure() {
  Serial.print("Temperature = ");
  temperature = bme.readTemperature();
  Serial.print(temperature);
  Serial.println(" *C");

  Serial.print("Airpressure = ");
  pressure = bme.readPressure() / 100.0F;
  Serial.print(pressure);
  Serial.println(" hPa");

  Serial.print("Airhumidity = ");
  humidity = bme.readHumidity();
  Serial.print(humidity);
  Serial.println(" %");
}

long ultrasonicMessure() {
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);  //
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;  // Speed of sound wave divided by 2 (go and back)
  Serial.print("Distance in cm: ");
  Serial.println(distance);
  return distance;
}


int electricMessure() {
  electricValue = analogRead(electricPin);
  elecPercent = map(electricValue, 4095, 1000, 0, 100);
  Serial.print("Moisture % (elec): ");
  Serial.println(elecPercent);
  return elecPercent;
}



int capacitiveMesure() {
  capacitiveValue = analogRead(capacitivePin);
  capacitivePercent = map(capacitiveValue, 3000, 2100, 0, 100);
  Serial.print("Moisture % (cap): ");
  Serial.println(capacitivePercent);
  return capacitivePercent;
}
