import React, { useEffect, useState, useContext } from 'react';
import WebSocketContext from './WebSocketContext';

const Weather = () => {
  // Entfernen des location states, da wir die Koordinaten nicht anzeigen
  const [city, setCity] = useState('');
  const [weatherData, setWeatherData] = useState(null);
  const { socket, isConnected } = useContext(WebSocketContext);

  useEffect(() => {
    // Reduzierung des Effekts auf das Abrufen der Stadt und Wetterdaten
    const getLocationAndWeather = async () => {
      // Funktion, um die Stadt von den Koordinaten abzurufen
      const fetchAddressFromCoordinates = async () => {
        if (!navigator.geolocation) {
          console.log("Geolocation is not supported by this browser.");
          return;
        }
        
        navigator.geolocation.getCurrentPosition(async (position) => {
          const { latitude, longitude } = position.coords;
          const url = `https://nominatim.openstreetmap.org/reverse?format=json&lat=${latitude}&lon=${longitude}&addressdetails=1`;
          try {
            const response = await fetch(url);
            if (!response.ok) throw new Error('Failed to fetch address');
            const data = await response.json();
            const city = data.address.city || data.address.town || data.address.village || 'Unbekannte Stadt';
            setCity(city);
            
            // Wetterdaten vom Backend über WebSocket anfordern
            if (isConnected) {
              socket.emit('requestWeatherData', { latitude, longitude });
              socket.on('weatherData', setWeatherData);
              socket.on('weatherDataError', console.error);
            }
          } catch (error) {
            console.error("Error fetching address: ", error);
          }
        }, (error) => {
          console.error("Geolocation error: ", error);
        });
      };

      await fetchAddressFromCoordinates();
    };

    getLocationAndWeather();

    // Aufräumen
    return () => {
      if (isConnected) {
        socket.off('weatherData', setWeatherData);
        socket.off('weatherDataError', console.error);
      }
    };
  }, [socket, isConnected]);

  return (
    <div>
      {weatherData ? (
        <>
          <h2>Wetterprognose</h2>
          <p>kommende 24h in {city}</p>
          <p>Aktuelle Temperatur: {weatherData.currentTemperature} °C</p>
          <p>Aktuelle Windgeschwindigkeit: {weatherData.currentWindSpeed} km/h</p>
          <p>Aktuelle Niederschlagswahrscheinlichkeit: {weatherData.currentPrecipitationProbability} %</p>
          <p>Aktuelle Bewölkung: {weatherData.currentCloudCover} %</p>
          <p>Niedrigste Temperatur: {weatherData.lowestTemperature} °C</p>
          <p>Höchste Temperatur: {weatherData.highestTemperature} °C</p>
          <p>Gesamtniederschlag: {weatherData.totalPrecipitation} mm</p>
          <p>Gesamtsolarstrahlung: {weatherData.totalSolar} kWh/m²</p>
        </>
      ) : (
        <p>Lade Wetterdaten...</p>
      )}
    </div>
  );
};

export default Weather;
