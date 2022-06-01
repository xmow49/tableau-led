
const http = require('http');
const bodyParser = require('body-parser');
var express = require('express');

http.globalAgent.keepAlive = true;

var app = express();

app.use(express.static('/home/pi/newcode/tableau-led/server'));
app.use(bodyParser.urlencoded({ extended: true }));

var server = app.listen(8000);

server.keepAliveTimeout = (60 * 1000) + 1000;
server.headersTimeout = (60 * 1000) + 2000;

console.log("Listen on port 8000");