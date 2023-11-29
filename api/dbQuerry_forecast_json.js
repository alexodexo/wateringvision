const mariadb = require('mariadb');
const fs = require('fs');

// Database connection configuration
const pool = mariadb.createPool({
  host: '127.0.0.1',
  user: 'test', //neuer nutzer kommt noch
  password: 'test', //
  database: 'weather',
  connectionLimit: 5,
});

async function fetchForecast() {
  let conn;

  try {
    conn = await pool.getConnection();

    // SQL-Abfrage, um die gewünschten Daten zu erhalten
    const query = `
      SELECT temperature
      FROM allData
      WHERE time = '19:00:00'
      ORDER BY id ASC
      LIMIT 1
    `;

    // Führe die Abfrage aus
    const result = await conn.query(query);

    // Überprüfe, ob Daten gefunden wurden
    if (result.length > 0) {
      const forecastValue = result[0].temperature;

      // Speichere die Daten in der forecast.json-Datei
      const forecastData = { forecast: forecastValue };
      fs.writeFileSync('forecast.json', JSON.stringify(forecastData));

      console.log('Forecast abgerufen und in forecast.json gespeichert:', forecastValue);
    } else {
      console.log('Keine Daten gefunden.');
    }
  } catch (err) {
    console.error('Fehler beim Abrufen des Forecasts:', err);
  } finally {
    if (conn) conn.release(); // Verbindung freigeben
  }
}

// Funktion aufrufen, um den Forecast abzurufen und zu speichern
fetchForecast();
