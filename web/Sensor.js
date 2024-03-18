import React, { useState, useEffect, useContext } from 'react';
import WebSocketContext from './WebSocketContext';

function Sensor() {
  const [sensorData, setSensorData] = useState({
    ultra: "Lädt...",
    elec: "Lädt...",
    cap: "Lädt...",
    temp: "Lädt...",
    pres: "Lädt...",
    humi: "Lädt..."
  });
  const { socket } = useContext(WebSocketContext);
  const [isPolling, setIsPolling] = useState(false);

  useEffect(() => {
    if (socket) {
      socket.on('sensorData', (data) => {
        setSensorData(data);
      });

      socket.on('disconnect', () => {
        console.log('WebSocket-Verbindung wurde geschlossen.');
      });
    }

    return () => {
      if (socket) {
        socket.off('sensorData');
        socket.off('disconnect');
      }
    };
  }, [socket]);

  useEffect(() => {
    let interval;
    if (isPolling && socket && socket.connected) {
      interval = setInterval(() => {
        socket.emit('callSensor');
        console.log('Call-Nachricht gesendet.');
      }, 1500); // Ruft sendMessage jede Sekunde auf
    }
    return () => clearInterval(interval); // Bereinigung
  }, [isPolling, socket]);

  const startPolling = () => {
    setIsPolling(true);
  };

  return (
    <div className="App">
      <button onClick={startPolling}>Sensorwerte automatisch abrufen</button>
      <div>
        <p>Feuchtigkeitswert: {sensorData.elec} %</p>
        <p>Temperatur: {sensorData.temp} °C</p>
        <p>Luftdruck: {sensorData.pres} hPa</p>
        <p>Luftfeuchtigkeit: {sensorData.humi} %</p>
      </div>
    </div>
  );
}

export default Sensor;
