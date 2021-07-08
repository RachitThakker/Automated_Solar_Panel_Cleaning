import paho.mqtt.client as mqtt
import os

print("Process ID:", os.getpid())

toArduinoTopic = "ljsc1111/arduino/to"
fromArduinoTopic = "ljsc1111/arduino/from"

cloudMqttServer = "broker.emqx.io"

# The callback for when the clientCloud receives a CONNACK response from the server.
def on_connect_cloud(clientCloud, userdata, flags, rc):
    print("Connected to", cloudMqttServer, "with result code", str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    clientCloud.subscribe(toArduinoTopic)


def on_connect_pi(clientPi, userdata, flags, rc):
    print("Connected to raspberry pi with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    clientPi.subscribe(toArduinoTopic)
    clientPi.subscribe(fromArduinoTopic)

# The callback for when a PUBLISH message is received from the server.
def on_message_cloud(clientCloud, userdata, msg):
    data = str(msg.payload)
    #print(msg.topic+" "+data)
    if (msg.topic == toArduinoTopic):
        publishToPi(toArduinoTopic, data)


def on_message_pi(clientPi, userdata, msg):
    data = str(msg.payload)
    print(msg.topic+" "+data)
    
    #Condition for fromArduinoTopic which will send received Arduino msg to cloud MQTT server.
    #The website is connected to cloud MQTT Server.
    #Arduino messages: 'BEGINNING', 'BEGAN', 'FINISHED', 'MOTOROFF'.
    if (msg.topic == fromArduinoTopic):
        publishToCloud(fromArduinoTopic, data)


def publishToPi(currentTopic, data):
    data = data[2:-1]
    clientPi.publish(currentTopic, data, 0, False)
    
    print(data)

def publishToCloud(currentTopic, data):
    data = data[2:-1]
    clientCloud.publish(currentTopic, data, 0, False)

clientCloud = mqtt.Client()
clientPi = mqtt.Client()

clientCloud.on_connect = on_connect_cloud
clientCloud.on_message = on_message_cloud

clientPi.on_connect = on_connect_pi
clientPi.on_message = on_message_pi

#clientCloud.username_pw_set(username="rw", password="readwrite")
clientPi.username_pw_set(username="mqtt@pi@user1", password="user1@pi@123456")

clientCloud.connect(cloudMqttServer, 1883, 60)
clientPi.connect("localhost", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
while True:
    clientCloud.loop_start()
    clientPi.loop_start()
