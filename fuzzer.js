const io = require('socket.io-client');

// Connect to the Socket.IO server
const socket = io('https://gameserver.pakamia.ke', {
  path: '/socket.io',
  transports: ['websocket']
});

socket.on('connect', () => {
  console.log('Connected to the server.');

  // Send fuzzed messages
  fuzzMessages.forEach(msg => {
    console.log(`Sending: ${msg}`);
    socket.emit('message', msg);
  });
});

socket.on('message', (data) => {
  console.log(`Received: ${data}`);
});

socket.on('disconnect', () => {
  console.log('Disconnected from the server.');
});

socket.on('error', (error) => {
  console.log(`Error: ${error}`);
});


// List of fuzzed messages to send
const fuzzMessages = [
    '42["7c0b37955cf21c7f2f3773c1268edc08",{"token":"XzxepxJShf04mw91DT49","coin":"kes","wallet":"NOT NEED","amount":100,"network":"BEP20"}]'
    // Add more fuzzed messages here
];

// Close the connection after a delay
// Close the Socket.IO connection after a delay
setTimeout(() => {
    socket.close();  // or use socket.disconnect();
    console.log('Connection closed.');
}, 10000); // Adjust the delay as needed

