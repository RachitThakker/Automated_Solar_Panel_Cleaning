#include <SPI.h>
#include <NewPing.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 45); //IP of this Arduino
//IPAddress server(5, 196, 95, 208); // IP for "test.mosquitto.org"
IPAddress server(192, 168, 1, 19); // IP of Raspberry Pi

const int led = LED_BUILTIN;

EthernetClient ethClient;
PubSubClient client(ethClient);

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

void callback(char* topic, byte* payload, unsigned int length) {

  //  Serial.print("Message arrived [");
  //  Serial.print(topic);
  //  Serial.print("] ");

  String readStr;
  for (int i = 0; i < length; i++) {
    //    Serial.print((char)payload[i]);
    readStr += (char)payload[i];
  }
  Serial.print(readStr);
  if (readStr == "LED_ON")
  {
    digitalWrite(led, HIGH);
    Serial.println(" high");
  }

  if (readStr == "LED_OFF")
  {
    digitalWrite(led, LOW);
    Serial.println(" low");
  }
}

//Beginning of snippet
void sendToPi()
{
  //Call functions to read and calculate sensor data
  //Convert values to string
  String wl = String(calcWaterLevel());
  String tp = String(calcTemp());
  String Pr = String(calcPressure());
  String Fl = String(calcFlowrate());

  //Defining the format of sending data
  //Initializing string to avoid unpredictable results
  String data = "flowrate=";
  data = data + Fl + "&temperature=" + tp + "&pressure=" + Pr + "&water_level=" + wl;
  unsigned int len = data.length() + 1;

  //Create character array buffer to enter as parameter in 'client.publish' function
  char buf[len];
  data.toCharArray(buf, len);
  
  //Publish data to Raspberry Pi via MQTT Client named 'client'
  client.publish("arduino/sensor", buf);
}
//End of snippet

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClientljsc1111", "sample@user1", "sample@password1")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("arduino/from", "hello from Arduino");
      // ... and resubscribe
      client.subscribe("arduino/to");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

float calcTemp()
{
  //Code to calculate temperature
  return 37.5; //Placeholder value
}

int calcWaterLevel()
{
  //Code to calculate water level
  return 98; //Placeholder value
}

float calcPressure()
{
  //Code to calculate pressure
  return 2.4; //Placeholder value
}

int calcFlowrate()
{
  //Code to calculate water flow rate
  return 2096; //Placeholder value
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  sendToPi();
  client.loop();
}
