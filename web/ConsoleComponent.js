import React, { useContext, useEffect, useState } from 'react';
import WebSocketContext from './WebSocketContext';
import './ConsoleComponent.css'; // Stelle sicher, dass du eine entsprechende CSS-Datei erstellst

const ConsoleComponent = () => {
  const { socket } = useContext(WebSocketContext);
  const [messages, setMessages] = useState([]);

  useEffect(() => {
    // Sicherstellen, dass socket nicht null ist, bevor auf Methoden zugegriffen wird
    if (socket) {
      // Event-Handler-Funktion für eingehende MQTT-Nachrichten
      const onMqttMessage = (data) => {
        setMessages(prevMessages => [...prevMessages, data]);
      };

      // WebSocket Event-Listener registrieren
      socket.on('mqttMessage', onMqttMessage);

      // Cleanup-Funktion beim Unmount der Komponente
      return () => {
        socket.off('mqttMessage', onMqttMessage);
      };
    }
  }, [socket]); // Abhängigkeit [socket] stellt sicher, dass der Effekt ausgeführt wird, wenn sich das socket-Objekt ändert

  return (
    <div className="console-container">
      <h2>Kommunikationslog</h2>
      <div className="console">
        {messages.slice(0).reverse().map((msg, index) => ( // Kehre die Reihenfolge der Nachrichten um
          <div key={index} className="console-message">
            <span className="console-timestamp">[{msg.receivedAt}]</span>
            <span className="console-topic">{msg.topic}:</span>
            <span className="console-content">{msg.message}</span>
          </div>
        ))}
      </div>
    </div>
  );
};

export default ConsoleComponent;
