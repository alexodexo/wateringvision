//In diesem Script wir die Bright Sky DB abgerufen. Date = aktueller Tag + aktuelle Stunde (in UTC); Last_Date = Tag in drei Tagen
//Die Antwort wird als .json Datei abgespeichert

const http = require('https');
const fs = require('fs');

// Funktion zum Formatieren der aktuellen Stunde im UTC-Format
function formatCurrentHourUTC() {
  const currentHourUTC = new Date().getUTCHours();
  return String(currentHourUTC).padStart(2, '0');
}

// Funktion zum Formatieren des Datums im ISO 8601 Format
function formatDateISO8601(date) {
  const year = date.getUTCFullYear();
  const month = String(date.getUTCMonth() + 1).padStart(2, '0');
  const day = String(date.getUTCDate()).padStart(2, '0');
  return `${year}-${month}-${day}`;
}

// Aktuelles Datum und aktuelle Stunde im UTC-Format
const currentDateUTC = new Date();
const currentHourUTC = formatCurrentHourUTC();

// Datum in drei Tagen im ISO 8601 Format
const lastDate = new Date();
lastDate.setDate(lastDate.getDate() + 3);

const options = {
  method: 'GET',
  hostname: 'api.brightsky.dev',
  port: null,
  path: `/weather?date=${formatDateISO8601(currentDateUTC)}T${currentHourUTC}%3A00%2B01%3A00&last_date=${formatDateISO8601(lastDate)}&lat=50.17837&lon=8.63148&tz=Europe%2FBerlin&units=dwd`,
  headers: {
    Accept: 'application/json'
  }
};

const req = http.request(options, function (res) {
  const chunks = [];

  res.on('data', function (chunk) {
    chunks.push(chunk);
  });

  res.on('end', function () {
    const body = Buffer.concat(chunks);
    const jsonResponse = body.toString();

    // API-Antwort als JSON-Datei speichern
    fs.writeFileSync('weather.json', jsonResponse);
    console.log('API-Antwort erfolgreich in weather.json gespeichert.');
  });
});

req.end();
