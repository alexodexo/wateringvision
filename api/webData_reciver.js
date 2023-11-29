const express = require('express');
const mariadb = require('mariadb');
const bodyParser = require('body-parser');
const cors = require('cors');
const app = express();

const port = 3001; // Wähle einen geeigneten Port

// Middleware für das Parsen des Request-Bodies
app.use(bodyParser.json());

// CORS-Middleware hinzufügen
app.use(cors());

// API-Endpoint zum Empfangen von Daten von der Website
app.post('/api/storeDuration', async (req, res) => {
  try {
    // Hole den Wert aus dem Request-Body
    const { duration } = req.body;

    // Erstelle eine Verbindung zur Datenbank
const pool = mariadb.createPool({
  host: '127.0.0.1',
  user: 'test', //neuer nutzer kommt noch
  password: 'test', //
  database: 'weather',
  connectionLimit: 5,
});

    const conn = await pool.getConnection();

    // Füge den Wert in die Datenbank ein
    await conn.query('INSERT INTO webData (duration) VALUES (?)', [duration]);

    // Beende die Datenbankverbindung
    conn.release();

    // Sende eine Erfolgsmeldung zurück
    res.json({ success: true, message: 'Daten erfolgreich gespeichert.' });
  } catch (error) {
    console.error('Fehler beim Speichern der Daten:', error);
    res.status(500).json({ success: false, message: 'Interner Serverfehler.' });
  }
});

// Starte den Express-Server
app.listen(port, () => {
  console.log(`Server läuft auf Port ${port}`);
});


