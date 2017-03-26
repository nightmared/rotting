import paho.mqtt.client as mqtt
import json

def loadConfig():
    with open('config') as json_file:
        config = json.load(json_file)
        json_file.close()
        return config

conf = loadConfig()
topic = 'user/' + str(conf['door']['userId']) + '/door/' + str(conf['door']['id'])

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(topic)

def on_message(client, userdata, msg):
    if msg.payload == b"open":
        client.publish(topic, "open:ack")


client = mqtt.Client()
client.username_pw_set(str(conf['door']['id']), conf['door']['password'])
client.on_connect = on_connect
client.on_message = on_message
client.connect(conf['mqttConnection']['host'], conf['mqttConnection']['port'], 30)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
