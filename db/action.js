const mysql = require('mysql');
const { calculateAveragePrecipitationProbability } = require('./weather');
const mqtt = require('mqtt');


// Konfiguration für die Verbindung zur `system` Datenbank
const systemConnection = mysql.createConnection({
  host: 'localhost',
  user: 'deinMariaDBUser',
  password: 'deinMariaDBPW',
  database: 'system'
});

const sensorDbConnection = mysql.createConnection({
  host: 'localhost',
  user: 'deinMariaDBUser',
  password: 'deinMariaDBPW',
  database: 'sensor'
});



// MQTT-Verbindung aufbauen
const mqttClient = mqtt.connect('mqtt://wateringvision.de', {
  port: 1883,
  clientId: 'waterGo',
  username: 'deinMQTTUSername',
  password: 'deinMQTTPW'
});

mqttClient.on('connect', () => {
  console.log('MQTT-Client verbunden');
});

mqttClient.on('error', (error) => {
  console.error('MQTT-Verbindungsfehler:', error);
});


// Die Funktion, die für jeden Eintrag in der Tabelle `waterline` aufgerufen wird

async function evaluate(sensorName, valveName, sollWert) {
  console.log(`Bewerte Daten: sensorName=${sensorName}, valveName=${valveName}, sollWert=${sollWert}`);
  
  // Baue die Abfrage, um den neuesten Eintrag aus der entsprechenden Tabelle zu erhalten
  sensorDbConnection.query(`SELECT * FROM ${mysql.escapeId(sensorName)} ORDER BY timestamp DESC LIMIT 1`, async (err, results) => {
    if (err) {
      console.error('Fehler beim Abrufen der Sensordaten:', err);
      return;
    }
    
    // Logge den kompletten neusten Eintrag aus der Tabelle
    console.log('Neuester Eintrag aus der Tabelle:', results[0]);
    const elecValue = results[0].elec; // Annahme: 'elec' ist ein Feld in deinen Ergebnissen

    // Da calculateAveragePrecipitationProbability ein Promise zurückgibt, verwende await
    try {
      const avgPrecipitationProb = await calculateAveragePrecipitationProbability();
      console.log(`Durchschnittliche Niederschlagswahrscheinlichkeit: ${avgPrecipitationProb}`);
      
      if(sollWert > elecValue) {
        if(avgPrecipitationProb < 50) {
          waterGo(valveName); // Ruf waterGo auf, wenn die Bedingungen erfüllt sind
        }
      }
    } catch (error) {
      console.error('Fehler beim Berechnen der Niederschlagswahrscheinlichkeit:', error);
    }
  });
}

function waterGo(valveName) {
  const topic = `/operateValve/${valveName}`;
  const payload = '90'; // Der Payload, der gesendet werden soll

  mqttClient.publish(topic, payload, { qos: 1 }, (error) => {
    if (error) {
      console.error(`Fehler beim Senden der Nachricht auf ${topic}:`, error);
    } else {
      console.log(`Nachricht mit Payload ${payload} erfolgreich auf ${topic} gesendet.`);
    }
  });
}

// Verbinde mit der Datenbank und führe die Abfrage aus
systemConnection.connect(err => {
  if (err) {
    console.error('Fehler beim Verbinden mit der Datenbank:', err);
    return;
  }
  console.log('Verbunden mit der Datenbank.');

  // Abfrage aller Einträge der Tabelle `waterline`
  systemConnection.query('SELECT * FROM waterline', (err, results) => {
    if (err) {
      console.error('Fehler beim Abrufen der Daten:', err);
      return;
    }
    console.log('Abgerufene Daten:', results);

    // Rufe die `evaluate`-Funktion für jeden Eintrag auf
    results.forEach(row => {
      evaluate(row.sensorName, row.valveName, row.sollWert);
    });

    // Schließe die Verbindung
    systemConnection.end(err => {
      if (err) {
        console.error('Fehler beim Schließen der Datenbankverbindung:', err);
      } else {
        console.log('Datenbankverbindung wurde geschlossen.');
      }
    });
  });
});

sensorDbConnection.connect(err => {
  if (err) {
    console.error('Fehler beim Verbinden mit der sensor-Datenbank:', err);
    return;
  }
  console.log('Verbunden mit der sensor-Datenbank.');
});
