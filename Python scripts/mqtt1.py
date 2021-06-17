import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("Connected with result code", str(rc))
    
    client.subscribe("test/ljsc1111")
    
def on_message(client, userdata, msg):
    print(str(msg.payload))
    
def on_publish(client, userdata, mid):
    print("Message sent and received")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.on_publish = on_publish

#client.username_pw_set(username="mqtt@pi@user1",password="user1@pi@123456")
client.connect("test.mosquitto.org", 1883, 60)

data = "Hello from 103.220"
client.publish("test/ljsc1111", data, 1, False)

client.loop_forever()