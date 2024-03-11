const mqtt = require('mqtt');
const mysql = require('mysql');

// MQTT Broker Konfiguration
const client = mqtt.connect('mqtt://wateringvision.de', {
    port: 1883,
    clientId: 'myClient'
  });

// MariaDB Verbindungskonfiguration
const db = mysql.createConnection({
  host: 'localhost',
  user: 'myUsername',
  password: 'myPassword',
  database: 'sensor'
});

db.connect(err => {
  if (err) {
    console.error('Fehler beim Verbinden mit der MariaDB:', err);
    return;
  }
  console.log('Erfolgreich mit MariaDB verbunden.');
});

// Subscribe auf "/responseSensor/#"
client.on('connect', () => {
  console.log('Mit MQTT Broker verbunden.');
  client.subscribe('/responseSensor/#', (err) => {
    if (!err) {
      // Publiziere auf "/requestSensor/", um alle Sensoren aufzufordern
      client.publish('/requestSensor/', 'requestData');
    }
  });
});

// Handler für empfangene Nachrichten
client.on('message', (topic, message) => {
  console.log(`Nachricht empfangen. Topic: ${topic}. Nachricht: ${message.toString()}`);
  
  const tableName = topic.split('/')[2]; // Erhalte den ESP-Namen aus dem Topic
  const dataString = message.toString();
  const dataParts = dataString.split(';').reduce((acc, part) => {
    const [key, value] = part.split(':');
    acc[key] = value;
    return acc;
  }, {});

  const query = `INSERT INTO ?? (timestamp, ultra, elec, cap, temp, pres, humi) VALUES (NOW(), ?, ?, ?, ?, ?, ?)`;
  const values = [tableName, dataParts.ultra, dataParts.elec, dataParts.cap, dataParts.temp, dataParts.pres, dataParts.humi];

  db.query(query, values, (err, results) => {
    if (err) {
      console.error('Fehler beim Einfügen in die Datenbank:', err);
      return;
    }
    console.log('Daten erfolgreich in MariaDB eingefügt:', results);
  });
});
