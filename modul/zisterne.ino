#include "ultra_settings.h"

#define echoPin 17
#define trigPin 16

#include <WiFi.h>
WiFiClient espClient;

#include <PubSubClient.h>
const char* mqtt_server = "wateringvision.de";
const int mqtt_port = 1883;
PubSubClient client(espClient);

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0;

long duration;
int distance;
int distanceToWater;
int waterDepth;
int fillLevelPercent;
int currentLiters;


void calculateCisternData() {
  distanceToWater = getDistance();          // Abstand zur Wasseroberfläche in cm
  Serial.print("Abstand: ");
  Serial.println(distanceToWater);

  waterDepth = maxDepth - distanceToWater;  // Tatsächliche Wassertiefe
  // Umrechnen der Wassertiefe in einen Prozentwert
  fillLevelPercent = map(waterDepth, 0, maxDepth, 0, 100);
  Serial.print("Füllstand %: ");
  Serial.println(fillLevelPercent);

  // Berechnung des aktuellen Literanzahl (Volumens() basierend auf der Wassertiefe
  currentLiters = (waterDepth * maxVolume) / maxDepth;
  Serial.print("Literanazahl %: ");
  Serial.println(currentLiters);
}


void setup() {
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an OUTPUT
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

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {  //WLAN-Statusüberprüfung
    reconnect();              //WLAN-Wiederverbindung
  }
  client.loop();  //MQTT-Check
  sensorDisplay();
}

void sensorDisplay() {
  calculateCisternData();

  if (millis() - previousMillis >= 1500) {
    previousMillis += 1500;  // Aktualisiere die Zeit für das nächste Intervall

    lcd.clear();  // Lösche den Displayinhalt, um die neuen Werte anzuzeigen

    // Zeige den Füllstand in Prozent auf der ersten Zeile des Displays an
    lcd.setCursor(0, 0);  // Setze den Cursor an den Anfang der ersten Zeile
    lcd.print("Fuellstand: ");
    lcd.print(fillLevelPercent);
    lcd.print("%");

    // Zeige die Literanzahl auf der zweiten Zeile des Displays an
    lcd.setCursor(0, 1);  // Setze den Cursor an den Anfang der zweiten Zeile
    lcd.print("Liter: ");
    lcd.print(currentLiters);
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

  handleCallSensor();
}

void handleCallSensor() {
  calculateCisternData();

  char sensorDataString[40];
  createSensorDataString(0, fillLevelPercent, currentLiters, sensorDataString, sizeof(sensorDataString));
  Serial.println(sensorDataString);

  // Veröffentliche die Nachricht auf "kopenhagen/setSensor"
  if (client.publish("/responseSensor/kopenhagen", sensorDataString)) {
    Serial.println("Sensorwert erfolgreich veröffentlicht!");
    client.publish("/feedback/kopenhagen", "Sensordaten erfolgreich veröffentlicht");

  } else {
    Serial.println("Fehler beim Veröffentlichen des Sensorwerts.");
    client.publish("/feedback/kopenhagen", "Fehler beim Übermitteln der Sensordaten");
  }
}

void createSensorDataString(int fillPercent, int fillLiters, char* dataString, int maxLength) {
  snprintf(dataString, maxLength, "fillPercent:%d;fillLiters:%d", fillPercent, fillLiters);
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
      client.subscribe("/requestSensor/kopenhagen", 1);
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






int getDistance() {
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
  // Displays the distance on the Serial Monitor
  Serial.print("Distance in cm: ");
  Serial.println(distance);
  return distance;
}
