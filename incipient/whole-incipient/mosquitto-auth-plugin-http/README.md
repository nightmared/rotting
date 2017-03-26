mosquitto-auth-plugin-http
==========================

This module is based on the work of Hadley Rich, who own the intellectual property of it. I've made some modifications to it to fit my needs, feel free to use it as long as it doesn't infringe the original copyright stated here: https://github.com/hadleyrich/mosquitto-auth-plugin-http/blob/master/LICENSE. My (tiny) modifications are free as a speech, so do whathever you want with theses snippets of code.

A simple authentication plugin for the Mosquitto MQTT broker (http://mosquitto.org) which makes a
POST to an HTTP server for both username/password checks and ACL checks.

Inspired by the rabbitmq-auth-backend-http (https://github.com/simonmacmullen/rabbitmq-auth-backend-http)
and the mosquitto-auth-plugin-keystone (https://github.com/brc859844/mosquitto-auth-plugin-keystone).

### Requirements
Please install development headers of mosquitto and of json-c.
