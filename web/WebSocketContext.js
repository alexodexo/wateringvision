import React, { createContext, useRef, useEffect, useState } from 'react';
import io from 'socket.io-client';
import { v4 as uuidv4 } from 'uuid'; // Füge diesen Import hinzu

// Erstelle einen Kontext
const WebSocketContext = createContext(null);

export const WebSocketProvider = ({ children }) => {
  const socket = useRef(null);
  const [isConnected, setIsConnected] = useState(false);

  useEffect(() => {

    // Hole die Session-ID aus dem localStorage oder generiere eine neue
    const sessionId = localStorage.getItem('sessionId') || uuidv4();
    localStorage.setItem('sessionId', sessionId);

    // Initialisiere die Socket-Verbindung
    socket.current = io('wss://wateringvision.de:8080', {
      query: { sessionId }
    });

    socket.current.on('connect', () => {
      console.log('Verbunden mit dem WebSocket-Server.');
      setIsConnected(true);
    });

    socket.current.on('disconnect', () => {
      console.log('WebSocket-Verbindung wurde geschlossen..');
      setIsConnected(false);
    });

    // Bereinigung beim Unmount
    return () => {
      socket.current.disconnect();
    };
  }, []);

  return (
    <WebSocketContext.Provider value={{ socket: socket.current, isConnected }}>
      {children}
    </WebSocketContext.Provider>
  );
};

export default WebSocketContext;
