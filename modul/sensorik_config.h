// WLAN
#define WIFI_SSID "DEINWLANNAME"
#define WIFI_PASSWORD "DEINWLANPW"

//NTP Time Server
#define ntpServer "time.cloudflare.com"

//MQTT Config
#define clientname "berlin"
#define mqtt_password "DEINMQTTPW"
#define mqtt_server "wateringvision.de"
#define mqtt_port 1883

//MQTT Topics
#define feedback_topic "/feedback/berlin"
#define request_topic "/requestSensor/berlin"
#define response_topic "/responseSensor/berlin"

//Ultraschall Pins
#define echoPin 17
#define trigPin 16

//Feuchtigkeitspins
#define capacitivePin 35
#define electricPin 34

//I2C
#define SDA_PIN 0
#define SCL_PIN 4
