const { Console } = require('console');
const https = require('https');
let cachedWeatherData = null;
let lastFetchTime = null;

// Funktion, um die aktuelle Stunde im UTC-Format zu formatieren
function formatCurrentHourUTC() {
  const currentHourUTC = new Date().getUTCHours();
  return `${currentHourUTC}`.padStart(2, '0');
}

// Funktion, um das Datum im ISO 8601 Format zu formatieren
function formatDateISO8601(date) {
  return date.toISOString().split('T')[0];
}

function fetchWeatherData() {
  return new Promise((resolve, reject) => {
    console.log("Wetterdaten werden angefragt, wenn älter als eine Stunde");
    const oneHour = 3600000; // Millisekunden
    const currentTime = new Date();

    const currentDateUTC = new Date();
    const lastDate = new Date();
    lastDate.setDate(lastDate.getDate() + 2);

    // Prüfe, ob der letzte Fetch weniger als eine Stunde her ist
    if (lastFetchTime && (currentTime - lastFetchTime) < oneHour) {
      console.log('Verwende gecachte Wetterdaten, also letze Anfrage war kürzer als 1h');
      resolve(cachedWeatherData);
      return;
    }

    console.log('Rufe frische Wetterdaten von der API ab');
    const options = {
      method: 'GET',
      hostname: 'api.brightsky.dev',
      port: null,
      path: `/weather?date=${formatDateISO8601(currentDateUTC)}T${formatCurrentHourUTC()}:00+00:00&last_date=${formatDateISO8601(lastDate)}&lat=50.17837&lon=8.63148`,
      headers: { Accept: 'application/json' }
    };

    const req = https.request(options, (res) => {
      const chunks = [];
      res.on('data', (chunk) => {
        chunks.push(chunk);
      });
      res.on('end', () => {
        const body = Buffer.concat(chunks);
        cachedWeatherData = JSON.parse(body.toString());
        lastFetchTime = new Date();
        resolve(cachedWeatherData);
      });
    });

    req.on('error', (error) => {
      reject(error);
    });

    req.end();
  });
}

// Funktion zur Berechnung des Durchschnitts der Niederschlagswahrscheinlichkeit
function calculateAveragePrecipitationProbability() {
  return fetchWeatherData().then(weatherData => {
    console.log("Starte die Berechnung der durchschnittlichen Regenwahrscheinlichkeit");
    let totalProbability = 0;
    let count = 0;
    weatherData.weather.forEach(entry => {
      if (entry.precipitation_probability !== null) {
        totalProbability += entry.precipitation_probability;
        count += 1;
      }
    });
    console.log(count);
    return count > 0 ? totalProbability / count : null;
  });
}
module.exports = { calculateAveragePrecipitationProbability };



  
async function main() {
  try {
    console.log('Abrufen der Wetterdaten...');
    const weatherData = await fetchWeatherData();
    console.log('Wetterdaten erfolgreich abgerufen.');

    // Berechne die durchschnittliche Niederschlagswahrscheinlichkeit
    const averagePrecipitationProbability = calculateAveragePrecipitationProbability(weatherData);
    console.log(`Durchschnittliche Niederschlagswahrscheinlichkeit: ${averagePrecipitationProbability}`);
  } catch (error) {
    console.error('Fehler beim Abrufen der Wetterdaten:', error);
  }
}
