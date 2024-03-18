import React, { useContext } from 'react';
import WebSocketContext from './WebSocketContext'; // Pfad anpassen, falls nötig

function Valve() {
  const { socket } = useContext(WebSocketContext);

  const handleOpenValve = () => {
    // Verwende socket.emit, um die Nachricht an den WebSocket-Server zu senden
    socket.emit('openValve');
    console.log('Ventil-Öffnung gesendet.');
  };

  return <button onClick={handleOpenValve}>Öffne Ventil</button>;
}

export default Valve;
