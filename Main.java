const { SerialPort } = require('serialport');
const express = require('express');
const app = express();
const port = new SerialPort({ path: 'COM3', baudRate: 9600 });

app.use(express.static('public'));

app.get('/data', (req, res) => {
  port.on('data', (data) => {
    res.json(JSON.parse(data.toString()));
  });
});

app.listen(3000, () => {
  console.log('Server running at http://localhost:3000');
});