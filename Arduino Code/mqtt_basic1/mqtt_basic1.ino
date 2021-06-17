/*
 Basic MQTT example

 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 110, 45);
//IPAddress server(5, 196, 95, 208);
IPAddress server(192, 168, 110, 36);

const int led = 12;

void callback(char* topic, byte* payload, unsigned int length) {
  
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");

  String readStr;
  for (int i=0;i<length;i++) {
//    Serial.print((char)payload[i]);
    readStr += (char)payload[i];
  }
  Serial.print(readStr);
  if (readStr == "ledon")
  {
    digitalWrite(led,HIGH);
    Serial.println(" high");
  }

  if (readStr == "ledoff")
  {
    digitalWrite(led, LOW);
    Serial.println(" low");
  }
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClientljsc1111", "mqtt@pi@user1", "user1@pi@led3456")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("ljsc1111/arduino/from","hello from Arduino");
      // ... and resubscribe
      client.subscribe("ljsc1111/arduino/to");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(57600);

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
