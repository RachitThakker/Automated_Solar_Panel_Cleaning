/*
 Basic MQTT example with Authentication

  - connects to an MQTT server, providing username
    and password
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(198, 168, 110, 37);
char server[] = "test.mosquitto.org";

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

EthernetClient ethClient;
PubSubClient client(server, 1884, callback, ethClient);

void setup()
{
  Ethernet.begin(mac, ip);
  Serial.begin(115200);
  // Note - the default maximum packet size is 128 bytes. If the
  // combined length of clientId, username and password exceed this use the
  // following to increase the buffer size:
  // client.setBufferSize(255);
  
  if (client.connect("arduinoClient", "rw", "readwrite")) {
    client.publish("ljsc1111/arduino/from","hello from Arduino");
    client.subscribe("ljsc1111/arduino/to");
  }
}

void loop()
{
  client.loop();
}
