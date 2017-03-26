`use strict`

const EventEmitter = require('events');
const util = require('util');
const http = require('http');
const mqtt = require('mqtt');

// listener to now if we are ready (aka we have got all the data) if the method is POST
function ReadyEmitter() {
    EventEmitter.call(this);
}
util.inherits(ReadyEmitter, EventEmitter);

function doHttpPost(url, query, callback) {
  var postData = JSON.stringify(query);

  var options = {
    hostname: '127.0.0.1',
    port: 8080,
    path: url,
    method: 'POST',
    headers: {
      'Content-Type': 'application/x-www-form-urlencoded',
      'Content-Length': postData.length
    }
  };

  var req = http.request(options, (res) => {
    var body = [];
    res.on('data', (chunk) => {
      body.push(chunk);
    }).on('end', () => {
      if (res.statusCode == 200) {
        callback(JSON.parse(Buffer.concat(body).toString()));
      } else {
        callback();
      }
    })
  });

  // write data to request body
  req.write(postData);
  req.end();
}

function getConfig(query, callback) {
  doHttpPost('/doors/get_passwd', query, (passwd) => {
    doHttpPost('/users/login', query.auth, (user) => {
      if (passwd === undefined || user === undefined) {
        callback(1);
        return;
      }
      callback(0, {
        "username": query.id,
        "password": passwd['value'],
        "topic": "user/" + user.id + "/door/" + query.id
      });
    });
  });
}

var messageValue = {
  'open': {
    'ack': 0,
    'err': 1
  }
}

function doorOpen(topic, id, passwd, callback) {
  var client = mqtt.connect({host: 'localhost', port: 1883, username: id.toString(), password: passwd});

  client.on('connect', function () {
    client.subscribe(topic);
    client.publish(topic, 'open');
  });

  client.on('message', function (topic, message) {
    var msg = message.toString();
    var arr = msg.split(':');
    if (arr.length == 2) {
      client.end();
      var method = arr[0];
      var value = arr[1];
      // done, shall prevent this listener to be triggered multiple times
      client.removeAllListeners('message');
      // Not beautiful, but hell, I'm not a frontend developper !
      if (method in messageValue && value in messageValue[method]) {
        callback(messageValue[method][value]);
      }
    }
  });
  setTimeout(() => { client.emit('message', topic, 'open:err'); }, 500);
}

module.exports.ReadyEmitter = ReadyEmitter;
module.exports.doHttpPost = doHttpPost;
module.exports.getConfig = getConfig;
module.exports.doorOpen = doorOpen;
