`use strict`

const http = require('http');
const helpers = require('./helpers.js');


function doorOpen(res, body) {
  helpers.getConfig(body, (err, conf) => {
    if (err == 1) {
      res.statusCode = 400;
      res.end();
    } else {
      helpers.doorOpen(conf['topic'], conf['username'], conf['password'], (err) => {
        if (err != 0) {
          res.write('{"status": 1, "err": "unknown"}');
        } else {
          res.write('{"status": 0}');
        }
        res.end();
      });
    }
  });
}

const router = {
  'POST': {
    '/doors/open': doorOpen
  }
}

var server = http.createServer().listen(8888);
server.on('request', function serveHTTP(req, res) {
    var body = [];

    var ready = new helpers.ReadyEmitter();

    if (req.method in router && req.url in router[req.method]){
      ready.once('ready', function dispatchQueries()  {
        // Accept only JSON-formatted POST request
        router[req.method][req.url](res, JSON.parse(Buffer.concat(body).toString()))
      });
    } else {
      res.statusCode = 404;
      res.end()
    }
    // No data to be sent over get so we are good to go
    if (req.method == 'GET') {
      ready.emit('ready');
    }
    req.on('data', (chunk) => {
      body.push(chunk);
    }).on('end', () => {
      ready.emit('ready');
    });
});
