
const http = require('http');
const bodyParser = require('body-parser');
var express = require('express');
const fs = require('fs')
http.globalAgent.keepAlive = true;


var app = express();


app.use(express.static('./'));
app.use(bodyParser.urlencoded({ extended: true }));

var jsonparser = bodyParser.json()
var textparser = bodyParser.text()

const folderName = '/tmp/matrix'
try {
    if (!fs.existsSync(folderName)) {
      fs.mkdirSync(folderName)
    }
  } catch (err) {
    console.error(err)
  }



app.post('/post', jsonparser, (req, res) => {
    console.log("New data received");
    res.sendStatus(200);
    try {
        fs.writeFileSync(folderName + '/IPC', JSON.stringify(req.body))
            //file written successfully
    } catch (err) {
        console.error(err)
    }
});


app.post('/draw', textparser, (req, res) => {
  console.log("getting draw request");
  res.sendStatus(200);
  console.log(req.body);
  try {
    
      fs.writeFileSync(folderName + '/DRAW', req.body)
          //file written successfully
  } catch (err) {
      console.error(err)
  }
});


var server = app.listen(8000);

// This is the important stuff
server.keepAliveTimeout = (60 * 1000) + 1000;
server.headersTimeout = (60 * 1000) + 2000;