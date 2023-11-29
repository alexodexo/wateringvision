const express = require('express');
const cors = require('cors'); // Füge das cors-Modul hinzu
const app = express();
const fs = require('fs');

// CORS konfigurieren
app.use(cors());

// API-Endpoint für die Wettervorhersage
app.get('/api/forecast', (req, res) => {
  // Lese die JSON-Datei
  const rawData = fs.readFileSync('/home/alexpi/api/forecast.json');
  const forecastData = JSON.parse(rawData);

  // Sende die Daten als JSON
  res.json(forecastData);
});

// Starte den Express-Server
const port = 3000; // Wähle einen geeigneten Port
app.listen(port, () => {
  console.log(`Server läuft auf Port ${port}`);
});
