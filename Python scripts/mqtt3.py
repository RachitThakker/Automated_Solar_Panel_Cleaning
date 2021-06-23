import paho.mqtt.client as mqtt
import urllib.request
import urllib.parse
import urllib.error
import datetime

ourTopic = "arduino/sensor"

# The callback for when clientPi receives a CONNACK response from the Pi MQTT server.


def on_connect_pi(clientPi, userdata, flags, rc):
    print("Connected to raspberry pi with result code "+str(rc))

    # Subscribing inside on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    clientPi.subscribe(ourTopic)


# The callback for when a PUBLISH message is received from the server.
def on_message_pi(clientPi, userdata, msg):
    currentTime = str(datetime.datetime.now())[:19]
    currentTopic = str(msg.topic)
    currentData = str(msg.payload)
    if (currentTopic == ourTopic):
        insertOnline(currentData, currentTime)


# The function to create proper URL and send HTTP request.
def insertOnline(currentData, currentTime):
    currentData = currentData[2:]
    if (currentData.endswith("'")):
        currentData = currentData[:-1]
    
    # The incomplete URL which inserts into MySQL on phpMyAdmin
    url = "http://192.168.1.6:8080/solarcleaning/Solardata2.php?timestamp=" + currentTime + "&"

    # Sample complete URL: http://192.168.1.6:8080/solarcleaning/Solardata2.php?timestamp=2021-08-22%2016:25:12&flowrate=1995&temperature=23&pressure=2.9&water_level=123

    # The complete URL
    url = url + currentData
    url = url.replace(" ", "%20")
    print(url)

    # The below line sends an HTTP request. It is similar to entering
    # the URL in a web browser
    data = urllib.request.urlopen(url).read().decode()
    print(data)


clientPi = mqtt.Client()

clientPi.on_connect = on_connect_pi
clientPi.on_message = on_message_pi

# clientPi.username_pw_set(username="sample@user1", password="sample@password1")
clientPi.username_pw_set(username="mqtt@pi@user1", password="user1@pi@123456")

clientPi.connect("localhost", 1883, 240)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
clientPi.loop_forever()

