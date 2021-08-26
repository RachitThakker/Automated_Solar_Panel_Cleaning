#include <string.h>
#include <SPI.h>
#include <NewPing.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <GravityTDS.h>
#include <EEPROM.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 110, 45); //IP of this Arduino
IPAddress server(192, 168, 110, 36); // IP of Raspberry Pi

const int led = LED_BUILTIN;
int relay6 = 8; // For DOL Stop Relay
int relay5 = 3; // For DOL Start Relay
int relay1 = 4; // For Valve1
int relay2 = 5; // For Valve2
int relay3 = 6; // For Valve3
int relay4 = 7; // For Valve4
int flowIn = 2; // For Water Flow sensor
#define ONE_WIRE_BUS 9
int manualButton = 10;
#define ECHO_PIN     11  // TX Arduino pin tied to echo pin on the ultrasonic sensor.
#define TRIGGER_PIN  12  // RX Arduino pin tied to trigger pin on the ultrasonic sensor.
#define MAX_DISTANCE 400

#define pressnum A0 // Arduino pin defined on pressure sensor
#define tdsnum A1 // Arduino pin defined on tds sensor

int tdsi = -999;
float pressureg = -999;

float tempc1;

GravityTDS gravityTds;

volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_min; // Calculated litres/hour

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

void flow () // Interrupt function
{
  flow_frequency++;
}

void setup()
{
  Serial.begin(9600);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  pinMode(manualButton, INPUT);
  pinMode(led, OUTPUT);
  pinMode(flowIn, INPUT);

  digitalWrite(flowIn, HIGH);
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

  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  // show the addresses we found on the bus
  //  Serial.print("Device 0 Address: ");
  //  printAddress(insideThermometer);
  Serial.println();
  sensors.setResolution(insideThermometer, 9);
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC);
  Serial.println();

  //gravity config to arduino and tds sensor
  gravityTds.setPin(tdsnum);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization

  attachInterrupt(digitalPinToInterrupt(flowIn), flow, RISING); // Setup Interrupt
  sei(); // Enable interrupts
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
  else if (readStr == "ARDUINO_THERE?")
  {
    client.publish(fromArduinoTopic, "Hello from Arduino!");
  }
  else if (readStr == "TURN_ON_MOTOR")
  {
    digitalWrite(relay5, LOW);
    delay(500);
    digitalWrite(relay5, HIGH);
    client.publish(fromArduinoTopic, "MOTORON");
  }
}

//Beginning of snippet
void sendSensorDataToPi(bool every4Min)
{
  //Call functions to read and calculate sensor data
  //Convert values to string
  int wl = calcWaterLevel();
  float tp = calcTemp();
  int Fl = calcFlowrate();
  float Pr;
  int tds;

  if (every4Min == false) {
    Pr = calcPressure();
    tds = calcTds();
  }

  if (every4Min == true) {
    Pr = pressureg;
    tds = tdsi;
  }

  String wl1 = String(wl);
  String Fl1 = String(Fl);
  String tp1 = String(tp);
  String Pr1 = String(Pr);
  String tds1 = String(tds);

  char wl2[10], Fl2[10], tp2[10], Pr2[10], tds2[10];
  wl1.toCharArray(wl2, 10);
  Fl1.toCharArray(Fl2, 10);
  tp1.toCharArray(tp2, 10);
  Pr1.toCharArray(Pr2, 10);
  tds1.toCharArray(tds2, 10);

  //Defining the format of sending data
  //Initializing string to avoid unpredictable results
  char data[200] = "temperature=";
  strcat(data, tp2);
  strcat(data, "&level=");
  strcat(data, wl2);
  strcat(data, "&flow=");
  strcat(data, Fl2);
  strcat(data, "&tds=");
  strcat(data, tds2);
  strcat(data, "&pressure=");
  strcat(data, Pr2);
  strcat(data, "&endpressurevalve1=0&endpressurevalve2=0&endpressurevalve3=0&endpressurevalve4=0");

  // data = data + tp2 + "&level=" + wl2 + "&flow=" + Fl2 + "&tds=" + tds2 + "&pressure=" + Pr2 + "&endpressurevalve1=0&endpressurevalve2=0&endpressurevalve3=0&endpressurevalve4=0";

  // unsigned int len = strlen(data) + 1;

  //Create character array buffer to enter as parameter in 'client.publish' function
  // char buf[len];
  // data.toCharArray(buf, len);

  //Publish data to Raspberry Pi via MQTT Client named 'client'
  Serial.println(data);
  client.publish(sensorTopic, data);
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
    if (client.connect("arduinoClientljsc2222", username, password)) {
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
      sendSensorDataToPi(false);
      delay(2000); // Send Pressure and Flowrate every 2 secs
    }
    digitalWrite(relay2, LOW); //Open valve 2, while valve 1 is still open
    sendValveDataToPi(2, 1);
    delay(2000);
    digitalWrite(relay1, HIGH); //Close valve 1
    sendValveDataToPi(1, 0);

    timeMillis = millis();
    while (millis() <= timeMillis + 38000) // Run loop for 38 secs
    {
      sendSensorDataToPi(false);
      delay(2000); // Send Pressure and Flowrate every 2 secs
    }
    digitalWrite(relay3, LOW); //Open valve 3, while valve 2 is still open
    sendValveDataToPi(3, 1);
    delay(2000);
    digitalWrite(relay2, HIGH); // Close valve 2
    sendValveDataToPi(2, 0);

    timeMillis = millis();
    while (millis() <= timeMillis + 28000) // Run loop for 28 secs
    {
      sendSensorDataToPi(false);
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
      sendSensorDataToPi(false);
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
    client.publish(fromArduinoTopic, "FINISHED");
  }
  else
  {
    client.publish(fromArduinoTopic, "WATER_SUPPLY_INSUFFICIENT");
  }
}

float calcTemp()
{
  //Code to calculate temperature
  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempC(insideThermometer);
  tempc1 = tempC;
  if (tempC == DEVICE_DISCONNECTED_C)
  {
    return -999;
  }

  return tempC;
}

int calcWaterLevel()
{
  //Code to calculate water level
  const int min1 = 8;
  const int max1 = 60;
  int ct = sonar.ping_cm();
  float dt = (ct * 0.3937 - min1) / (max1 - min1);
  dt = dt * 100;

  return dt;
}

float calcPressure()
{
  //Code to calculate pressure
  const float pressureZero =  100.4;
  const float pressureMax = 921.9;
  const float maxPSI = 725.189;
  const int sensorreadDelay = 1000;
  const double bar = 0.0689476;
  float pressureValue = 0.0;
  pressureValue = analogRead(pressnum);
  pressureValue = ((pressureValue - pressureZero) * maxPSI) / (pressureMax - pressureZero); //conversion equation to convert analog reading to psi
  //Serial.println(putvalue);
  //Serial.print(pressureValue);
  //Serial.println("  psi");
  pressureg = pressureValue * bar;
  //Serial.print(barvalue);
  //Serial.println("  bar");
  //delay(sensorreadDelay);
  return (pressureValue * bar);
}

int calcFlowrate()
{
  //Code to calculate water flow rate
  l_min = (flow_frequency / 29); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
  flow_frequency = 0; // Reset Counter
  //Serial.print(l_min, DEC); // Print litres/minute
  //Serial.println(" L/minute");
  return l_min;
}

int calcTds()
{
  //Code to calculate TDS number of water
  gravityTds.setTemperature(tempc1);
  gravityTds.update();  //sample and calculate
  tdsi = gravityTds.getTdsValue();  // then get the value
  //Serial.print(tdsValue,0);
  //Serial.println("ppm");
  return tdsi;
}

unsigned long startTime = millis();
void loop()
{
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if ((millis() - startTime) % 240000 == 0)
  {
    Serial.println();
    sendSensorDataToPi(true);
    Serial.println("Every 4 minutes.");
    Serial.println();
    delay(1);
  }
}
