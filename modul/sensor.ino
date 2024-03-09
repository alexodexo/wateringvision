#include "secrets.h"
#include <WiFi.h>

WiFiClient espClient;

#include <PubSubClient.h>
const char* mqtt_server = "wateringvision.de";
const int mqtt_port = 1883;
PubSubClient client(espClient);

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define echoPin 17
#define trigPin 16
long duration;  //of sound wave travel
int distance;   //for the distance measurement

#define capacitivePin 35
int capacitiveValue;

#define electricPin 34
int electricValue;

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SDA_PIN 0
#define SCL_PIN 4
Adafruit_BME280 bme;  // Erstelle ein BME280 Objekt
int temperature;
int pressure;
int humidity;

#include <ArduinoJson.h>
#include <Arduino.h>
#define relaisPin 27
unsigned long startZeit = 0;  // Speichert den Startzeitpunkt der Verzögerung
int verzogerungsDauer = 0;    // Speichert die Dauer der Verzögerung in Sekunden
bool timerAktiv = false;      // Flag, um zu überprüfen, ob der Timer läuft

unsigned long previousMillis = 0; //Sensordisplay Startwert

void setup() {
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
  pinMode(trigPin, OUTPUT);

  pinMode(relaisPin, OUTPUT);
  digitalWrite(relaisPin, HIGH);

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
  sensorDisplay();
}

void sensorDisplay(){
  lcd.setCursor(0, 0);
  lcd.print("Feuchtigkeitwert");
  if (millis() - previousMillis >= 1500) {
    previousMillis += 1500; // Aktualisiere die Zeit für das nächste Intervall
    int percent = electricMessure();
    String percentString = String(percent) + "%";
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(6, 1);
    lcd.print(percentString);
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
  messure();

  char sensorDataString[60];
  createSensorDataString(distance, electricValue, capacitiveValue, temperature, pressure, humidity, sensorDataString, sizeof(sensorDataString));
  Serial.println(sensorDataString);

  // Veröffentliche die Nachricht auf "berlin/setSensor"
  if (client.publish("/responseSensor/berlin", sensorDataString)) {
    Serial.println("Sensorwert erfolgreich veröffentlicht!");
    client.publish("/feedback/berlin", "Sensordaten erfolgreich veröffentlicht");

  } else {
    Serial.println("Fehler beim Veröffentlichen des Sensorwerts.");
    client.publish("/feedback/berlin", "Fehler beim Übermitteln der Sensordaten");
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
      client.subscribe("/requestSensor/berlin", 1);
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
  int elecPercent = map(electricValue, 4095, 1000, 0, 100);
  Serial.print("Moisture % (elec): ");
  Serial.println(elecPercent);
  return elecPercent;
}


int capacitiveMesure() {
  capacitiveValue = analogRead(capacitivePin);
  Serial.print("Capacitive Value: ");
  Serial.println(capacitiveValue);
  return capacitiveValue;
}
