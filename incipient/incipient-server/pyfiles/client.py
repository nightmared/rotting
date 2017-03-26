from enum import Enum
from utils import log, LogLevel, parse_commands
import paho.mqtt.client as mqtt
from utils import getprop


# Very simple class which just instantiate every doors by querying their information to the web server
class DoorManager:
    doors = []

    def __init__(self):
        # TODO: implement a server-side API to query doors owned by the user and attached to that peripheral instead of hardcoding the informations
        self.doors.append(Door(1, 7))

    def stop(self):
        for some in self.doors:
            some.stop()

    def start(self):
        for some in self.doors:
            some.start()


class DoorState(Enum):
    opened = 1
    closed = 2
    opening = 3
    closing = 4
    unknown = 5


class Door:
    state = DoorState.closed
    id = -1
    pin = -1
    client = mqtt.Client()
    userid = int(getprop("userid"))

    def __init__(self, door_id, gpio):
        self.id = door_id
        self.pin = gpio
        log(LogLevel.notice, "door {} is added and is located at port {}".format(self.id, self.pin))

        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        # WARNING: hardcoded password here !!!
        self.client.username_pw_set("1", "try")
        self.client.message_retry_set(3)
        self.client.connect(getprop("mqtt_server"), int(getprop("mqtt_port")), keepalive=int(getprop("mqtt_keepalive")))

    def start(self):
        self.client.loop_forever()

    def stop(self):
        self.client.loop_stop()

    def on_connect(self, client, userdata, flags, rc):
        log(LogLevel.notice, "Connected with result code " + str(rc) + " - " + mqtt.error_string(rc))

        client.subscribe('user/' + str(self.userid) + '/door/' + str(self.id))

    def on_message(self, client, userdata, msg):
        try:
            payload = parse_commands(msg.payload)
            log(LogLevel.notice, "MQTT payload received @ " + msg.topic + " : " + str(payload))
            log(LogLevel.notice, "doors[{}].{}()".format(str(self.id), payload["command"]))
        except KeyError:
            log(LogLevel.error, "The message don't contain a command instruction")
        except IndexError:
            log(LogLevel.error, "Invalid message received !")

    def open(self):
        log(LogLevel.notice, "door " + str(self.id) + " opening")
        return True

    def close(self, speed=1):
        log(LogLevel.notice, "door " + str(self.id) + " closing at speed " + str(speed))
        return True
