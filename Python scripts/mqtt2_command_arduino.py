import paho.mqtt.client as mqtt
import os

print("Process ID:", os.getpid())

ourTopic = "ljsc1111/arduino/to"

# The callback for when the clientCloud receives a CONNACK response from the server.


def on_connect_cloud(clientCloud, userdata, flags, rc):
    print("Connected to test.mosquitto.org with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    clientCloud.subscribe(ourTopic)


def on_connect_pi(clientPi, userdata, flags, rc):
    print("Connected to raspberry pi with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    clientPi.subscribe(ourTopic)

# The callback for when a PUBLISH message is received from the server.


def on_message_cloud(clientCloud, userdata, msg):
    data = str(msg.payload)
    # print(msg.topic+" "+data)
    publishToPi(ourTopic, data)


def on_message_pi(clientPi, userdata, msg):
    print(msg.topic+" "+str(msg.payload))


def publishToPi(currentTopic, data):
    clientPi.publish(currentTopic, data, 0, False)


clientCloud = mqtt.Client()
clientPi = mqtt.Client()

clientCloud.on_connect = on_connect_cloud
clientCloud.on_message = on_message_cloud

clientPi.on_connect = on_connect_pi
clientPi.on_message = on_message_pi

clientCloud.username_pw_set(username="rw", password="readwrite")
clientPi.username_pw_set(username="mqtt@pi@user1", password="user1@pi@123456")

clientCloud.connect("test.mosquitto.org", 1884, 240)
clientPi.connect("localhost", 1883, 240)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
clientCloud.loop_forever()
clientPi.loop_forever()
