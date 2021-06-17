/*
 Publishing in the callback

  - connects to an MQTT server
  - subscribes to the topic "inTopic"
  - when a message is received, republishes it to "outTopic"

  This example shows how to publish messages within the
  callback function. The callback function header needs to
  be declared before the PubSubClient constructor and the
  actual callback defined afterwards.
  This ensures the client reference in the callback function
  is valid.

*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 110, 45);
IPAddress server(192,168,110,36);
char username[] = "mqtt@pi@user1";
char password[] = "user1@pi@123456";

// Callback function header
void callback(char topic, byte payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

// Callback function
void callback(char topic, byte payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.

  // Allocate the correct amount of memory for the payload copy
  byte p = (byte)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  Serial.println(p);

//  if ((p1.indexOf("LED_ON") > 0))
//  {
//    digitalWrite(12, HIGH);
//  }
//
//  if ((p1.indexOf("LED_OFF") > 0))
//  {
//    digitalWrite(12, LOW);
//  }

  // Free the memory
  free(p);
}

void setup()
{
  Serial.begin(57600);
  Ethernet.begin(mac, ip);
  if (client.connect("arduinoClient", username, password)) {
    client.publish("ljsc1111/arduino/from","hello from arduino");
    client.subscribe("ljsc1111/arduino/to");
  }
}

void loop()
{
  client.loop();
}
