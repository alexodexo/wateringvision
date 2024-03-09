const aedes = require('aedes')();
const net = require('net');
const fs = require('fs');
const path = require('path');

const userCredentialsPath = path.join(__dirname, 'userCredentials.txt'); 

aedes.authenticate = function (client, username, password, callback) {
    fs.readFile(userCredentialsPath, 'utf8', (err, data) => {
        if (err) {
            console.log(err);
            return callback(err, false);
        }
        const lines = data.split('\n');
        const user = lines.find(line => {
            const [user, pass] = line.split(':');
            return user === username && pass === password.toString();
        });
        if (user) {
            callback(null, true);
        } else {
            const error = new Error('Authentifizierung fehlgeschlagen');
            error.returnCode = 4; // MQTT return code for bad username or password
            callback(error, false);
        }
    });
};

const server = net.createServer(aedes.handle);

server.listen(1883, function () {
    console.log('Aedes MQTT broker listening on port 1883');
});
