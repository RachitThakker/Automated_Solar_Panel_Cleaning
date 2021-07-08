#include <SPI.h>
#include <NewPing.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 110, 45); //IP of this Arduino
//IPAddress server(5, 196, 95, 208); // IP for "test.mosquitto.org"
IPAddress server(192, 168, 110, 36); // IP of Raspberry Pi

const int led = LED_BUILTIN;
int relay1 = 4; // For Valve1
int relay2 = 5; // For Valve2
int relay3 = 6; // For Valve3
int relay4 = 7; // For Valve4
int relay5 = 3; // For DOL Start Relay
int relay6 = 2; // For DOL Stop Relay
//int flowIn = 8; // For Water Flow sensor
int manualButton = 10;

#define ONE_WIRE_BUS 9

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

EthernetClient ethClient;
PubSubClient client(ethClient);
char fromArduinoTopic[] = "ljsc1111/arduino/from";
char toArduinoTopic[] = "ljsc1111/arduino/to";
char sensorTopic[] = "arduino/sensor";
char valveTopic[] = "arduino/valve";
char username[] = "mqtt@pi@user1";
char password[] = "user1@pi@123456";

void setup()
{
  Serial.begin(57600);

  pinMode(led, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  pinMode(manualButton, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  digitalWrite(relay5, HIGH);
  digitalWrite(relay6, HIGH);

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);

  sensors.begin();
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(insideThermometer, 9);
}

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

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

  if (readStr == "BEGIN")
  {
    beginCleaning();
  }
  else if (readStr == "SHUT_MOTOR")
  {
    digitalWrite(relay6, LOW);
    delay(500);
    digitalWrite(relay6, HIGH);
    client.publish(fromArduinoTopic, "MOTOROFF");
  }
}

//Beginning of snippet
void sendSensorDataToPi()
{
  //Call functions to read and calculate sensor data
  //Convert values to string
  String wl = String(calcWaterLevel());
  String tp = String(calcTemp());
  String Pr = String(calcPressure());
  String Fl = String(calcFlowrate());
  String tds = String(calcTds());

  //Defining the format of sending data
  //Initializing string to avoid unpredictable results
  String data = "temperature=";
  data = data + tp + "&level=" + wl + "&flow=" + Fl + "&tds=" + tds + "&pressure=" + Pr + "&endpressurevalve1=0&endpressurevalve2=0&endpressurevalve3=0&endpressurevalve4=0";
  unsigned int len = data.length() + 1;

  //Create character array buffer to enter as parameter in 'client.publish' function
  char buf[len];
  data.toCharArray(buf, len);

  //Publish data to Raspberry Pi via MQTT Client named 'client'
  client.publish(sensorTopic, buf);
}
//End of snippet

void sendValveDataToPi(int valveNum, int valveStat)
{
  String valveData = "valveNumber=" + String(valveNum) + "&valveStatus=" + String(valveStat);
  unsigned int len = valveData.length() + 1;

  //Create character array buffer to enter as parameter in 'client.publish' function
  char buf[len];
  valveData.toCharArray(buf, len);

  //Publish data to Raspberry Pi via MQTT Client named 'client'
  client.publish(valveTopic, buf);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClientljsc1111", username, password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(fromArduinoTopic, "Hello from Arduino!");
      // ... and resubscribe
      client.subscribe(toArduinoTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void beginCleaning()
{
  client.publish(fromArduinoTopic, "BEGINNING");
  if (true) //Condition to check water level is above threshold
  {
    digitalWrite(relay5, LOW); // DOL Starter ACTIVATED
    digitalWrite(relay1, LOW); // Open valve 1
    sendValveDataToPi(1, 1); // First parameter is valve number. Second parameter is Open='1' OR Close='0'.
    delay(2000); // Wait for 2 secs for pump to start properly

    client.publish(fromArduinoTopic, "BEGAN");

    unsigned long timeMillis = millis();
    while (millis() <= timeMillis + 28000) // Run loop for 28 secs
    {
      sendSensorDataToPi();
      delay(2000); // Send Pressure and Flowrate every 2 secs
    }
    digitalWrite(relay2, LOW); //Open valve 2, while valve 1 is still open
    sendValveDataToPi(2, 1);
    delay(2000);
    digitalWrite(relay1, HIGH); //Close valve 1
    sendValveDataToPi(1, 0);

    timeMillis = millis();
    while (millis() <= timeMillis + 38000) // Run loop for 39 secs
    {
      sendSensorDataToPi();
      delay(2000); // Send Pressure and Flowrate every 2 secs
    }
    digitalWrite(relay3, LOW); //Open valve 3, while valve 2 is still open
    sendValveDataToPi(3, 1);
    delay(2000);
    digitalWrite(relay2, HIGH); // Close valve 2
    sendValveDataToPi(2, 0);

    timeMillis = millis();
    while (millis() <= timeMillis + 28000) // Run loop for 29 secs
    {
      sendSensorDataToPi();
      delay(2000); // Send Pressure and Flowrate every 2 secs
    }
    digitalWrite(relay4, LOW); //Open valve 4, while valve 3 is still open
    sendValveDataToPi(4, 1);
    delay(2000);
    digitalWrite(relay3, HIGH); // Close valve 3
    sendValveDataToPi(3, 0);

    timeMillis = millis();
    while (millis() <= timeMillis + 40000) //Run loop for 40 secs
    {
      sendSensorDataToPi();
      delay(2000); // Send Pressure and Flowrate every 2 secs
    }
    digitalWrite(relay6, LOW); // Activate DOL Stopper relay
    delay(500);
    digitalWrite(relay5, HIGH); //Deavtivate DOL Starter relay
    delay(1000);
    digitalWrite(relay4, HIGH); // Close valve 4
    sendValveDataToPi(4, 0);
    digitalWrite(relay6, HIGH); //Deavtivate DOL Stopper relay
    client.publish(fromArduinoTopic, "MOTOROFF");

  }
  client.publish(fromArduinoTopic, "FINISHED");
}

float calcTemp()
{
  //Code to calculate temperature
  /*sensors.requestTemperatures(); // Send the command to get temperatures
    float tempC = sensors.getTempC(deviceAddress);
    if (tempC == DEVICE_DISCONNECTED_C)
    {
    return -999;
    }*/
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

int calcTds()
{
  //Code to calculate TDS number of water
  return 112; //Placeholder value
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
}
