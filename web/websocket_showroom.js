//const http = require('http');
const fs = require('fs');
const https = require('https');
const mqtt = require('mqtt');
const socketIo = require('socket.io');
const fetch = require('node-fetch');
const path = require('path');

const domain = 'wateringvision.de';
const logFilePath = path.join('/root/web/web/backend', 'userActivityLog.txt');

const privateKeyPath = `/etc/letsencrypt/live/${domain}/privkey.pem`;
const certificatePath = `/etc/letsencrypt/live/${domain}/fullchain.pem`;
const caBundlePath = `/etc/letsencrypt/live/${domain}/chain.pem`;

const credentials = {
  key: fs.readFileSync(privateKeyPath, 'utf8'),
  cert: fs.readFileSync(certificatePath, 'utf8'),
  ca: fs.readFileSync(caBundlePath, 'utf8')
};

const httpsServer = https.createServer(credentials);

const io = socketIo(httpsServer, {
  cors: {
    origin: "*", // Passen Sie dies an, um CORS-Fehler zu vermeiden
    methods: ["GET", "POST"]
  }
});

io.on('connection', (socket) => {

  const sessionId = socket.handshake.query.sessionId;
  console.log(`Client verbunden mit Session-ID: ${sessionId}`);
  socket.join(sessionId);

  const userIp = socket.handshake.headers['x-forwarded-for'] || socket.conn.remoteAddress;
  console.log(`User IP: ${userIp}`);

  socket.on('callSensor', () => {
    console.log('Sensordaten werden abgerufen...');
    mqttClient.publish('/requestSensor/berlin', 'call', { qos: 1 });
  });

  socket.on('openValve', () => {
    console.log('Ventil wird geöffnet...');
    mqttClient.publish('/operateValve/berlin', '3', { qos: 1 });
  });

  socket.on('requestWeatherData', async (coords) => {
    const { latitude, longitude } = coords;
    const userIp = socket.handshake.headers['x-forwarded-for'] || socket.conn.remoteAddress; // IP-Adresse des Nutzers

    try {
      logUserActivity(coords, userIp);
      const rawData = await getWeatherData(latitude, longitude);
      //console.log('rawData:', rawData.weather);
      const processedData = await processWeatherData(rawData); // Verarbeitung der Rohdaten
      console.log('weatherData:', processedData);
      io.to(sessionId).emit('weatherData', processedData); // Sendet verarbeitete Daten
    } catch (error) {
      console.error('Fehler beim Abrufen der Wetterdaten:', error);
      io.to(sessionId).emit('weatherDataError', error.message);
    }
  });
  

  socket.on('disconnect', () => {
    console.log('Client-Verbindung geschlossen');
  });
});



const mqttClient = mqtt.connect('mqtt://wateringvision.de', {
  port: 1883,
  clientId: 'backend',
  username: 'backend', // Benutzername hinzufügen
  password: 'mqttpw'    // Dein Passwort hinzufügen
});

mqttClient.on('connect', () => {
  console.log('MQTT Client verbunden');
  mqttClient.subscribe('/responseSensor/berlin');
});

mqttClient.on('message', (topic, message) => {
  if (topic === '/responseSensor/berlin') {
    const parsedData = parseSensorData(message.toString());
    io.emit('sensorData', parsedData); // Sendet an alle verbundenen Clients
  }
});

// Funktion für Reverse-Geocoding und Logging
async function logUserActivity(coords, userIp) {
  const { latitude, longitude } = coords;
  const url = `https://nominatim.openstreetmap.org/reverse?format=json&lat=${latitude}&lon=${longitude}`;

  try {
    const response = await fetch(url);
    if (!response.ok) throw new Error('Failed to fetch address');
    const data = await response.json();
    const formattedAddress = data.display_name;

    const logEntry = {
      timestamp: new Date().toISOString(),
      userIp,
      coords,
      formattedAddress
    };

    // Log-Eintrag als String formatieren
    const logString = `Timestamp: ${logEntry.timestamp}, IP: ${logEntry.userIp}, Coordinates: (${logEntry.coords.latitude}, ${logEntry.coords.longitude}), Address: ${logEntry.formattedAddress}\n`;

    // Log-Eintrag in der Konsole ausgeben
    console.log(logString);

    // Log-Eintrag in die Datei schreiben
    fs.appendFile(logFilePath, logString, (err) => {
      if (err) {
        console.error('Error writing to log file:', err);
      }
    });
  } catch (error) {
    console.error("Error fetching address for logging: ", error);
  }
}


async function getWeatherData(latitude, longitude) {
  const now = new Date();
const timezoneOffset = now.getTimezoneOffset() * 60000; // Zeitzone in Millisekunden
const localISOTime = new Date(now - timezoneOffset).toISOString().slice(0, -1);

// URL-Aufbau, es wird nur 'date' verwendet
const url = `https://api.brightsky.dev/weather?lat=${latitude}&lon=${longitude}&date=${localISOTime}`;

  try {
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    const rawData = await response.json();
    console.log('API SUCCESS');
    return rawData;
  } catch (error) {
    console.error('Fehler beim Abrufen der Wetterdaten:', error);
    throw error; // Weitergeben des Fehlers für die Fehlerbehandlung
  }
}



async function processWeatherData(rawData) {
  // rawData.weather enthält das Array mit den Wetterdaten
  const weatherData = rawData.weather;

  // Extrahiere die benötigten Daten
  const processedData = {
    currentTemperature: weatherData[0].temperature,
    currentWindSpeed: weatherData[0].wind_speed,
    currentPrecipitationProbability: weatherData[0].precipitation_probability || 0, // Nutze 0 als Standardwert, falls null
    currentCloudCover: weatherData[0].cloud_cover,
    lowestTemperature: Math.min(...weatherData.map(data => data.temperature)),
    highestTemperature: Math.max(...weatherData.map(data => data.temperature)),
    totalPrecipitation: weatherData.reduce((sum, data) => sum + (data.precipitation || 0), 0),
    totalSolar: weatherData.reduce((sum, data) => sum + (data.solar || 0), 0)
  };

  return processedData;
}


function parseSensorData(dataString) {
  const dataParts = dataString.split(';');
  const data = {};

  dataParts.forEach(part => {
    const [key, value] = part.split(':');
    data[key] = value;
  });

  return {
    ultra: data.ultra,
    elec: data.elec,
    cap: data.cap,
    temp: data.temp,
    pres: data.pres,
    humi: data.humi
  };
}

httpsServer.listen(8080, () => {
  console.log('HTTPS Server läuft auf Port 8080');
});
