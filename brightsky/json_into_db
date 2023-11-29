const mariadb = require('mariadb');
const moment = require('moment-timezone');
const fs = require('fs');

// Read the JSON file
const rawData = fs.readFileSync('weather.json');
const weatherData = JSON.parse(rawData);

// Database connection configuration
const pool = mariadb.createPool({
  host: '127.0.0.1',
  user: 'test',
  password: 'test',
  database: 'weather',
  connectionLimit: 5,
});

async function processData() {
  // Create a connection
  let conn;
  try {
    conn = await pool.getConnection();

    // Delete existing entries in the database
    await conn.query('DELETE FROM allData');

    // Iterate over each weather entry
    for (const entry of weatherData.weather) {
      // Parse and convert timestamp to CET
      const timestampCET = moment(entry.timestamp).tz('Europe/Berlin');

      // Round values
      const temperature = Math.round(entry.temperature);
      const windSpeed = Math.round(entry.wind_speed);
      const precipitation = Math.round(entry.precipitation);
      const precipitationProbability = Math.round(entry.precipitation_probability);

      // Insert data into the database
      await conn.query(
        'INSERT INTO allData (date, time, temperature, wind_speed, precipitation, precipitation_probability, icon) VALUES (?, ?, ?, ?, ?, ?, ?)',
        [
          timestampCET.format('YYYY-MM-DD'),
          timestampCET.format('HH:mm:ss'),
          temperature,
          windSpeed,
          precipitation,
          precipitationProbability,
          entry.icon,
        ]
      );
    }

    console.log('Data inserted successfully.');
  } catch (err) {
    console.error('Error: ', err);
  } finally {
    if (conn) conn.release(); // release connection
  }
}

// Process the data
processData();
