#include <SPI.h>
#include <NewPing.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //assign arduino mac address
byte ip[] = {192, 168, 103, 35 }; // ip in lan assigned to arduino

EthernetServer server(80); //server port arduino server will use
EthernetClient client;
char serverName[] = "192.168.103.220"; // IP of server where database is stored

int relay1 = 4; // For Valve1
int relay2 = 5; // For Valve2
int relay3 = 6; // For Valve3
int relay4 = 7; // For Valve4
int relay5 = 3; // For DOL Start Relay
int relay6 = 2; // For DOL Stop Relay
//int relay7 = 8;
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

String readString; //used by server to capture GET request

void setup() {

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  pinMode(manualButton, INPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
  digitalWrite(relay5, HIGH);
  digitalWrite(relay6, HIGH);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);

  sensors.begin();
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(insideThermometer, 9);

  Serial.print("Enter ");
  Serial.println(Ethernet.localIP());
  Serial.print(" in your browser");
}

 unsigned long beforeLoopTime = millis();
//sendTempNLevel();

void loop() {

  unsigned long loopStartTime = millis();
//    if(loopStartTime > (beforeLoopTime + 6000)) // send temp and level every 4 minutes
//          {
//            sendTempNLevel();
//          }
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string
          readString += c;
        }

        //if HTTP request has ended
        if (c == '\n') {

          Serial.print(readString); //print to serial monitor for debugging

          //now output HTML data header
          if (readString.indexOf('?') >= 0) { //don't send new page
            client.println(F("HTTP/1.1 204"));
            client.println();
            client.println();
          }
          else {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("<title>Solar Cleaning WebServer</title>");
            client.println("<style>");
            client.println("button {");
            client.println("  background-color: #f44336;\n  color: white;\n  padding: 14px 20px;\n  margin: 8px 0;\n  border:none;\n  cursor:pointer;\n  width: 60%;\n  height: 250px;");
            client.println("}");
            client.println("button:hover {opacity: 0.5;}");
            client.println("</style>");
            client.println("</head>");
            client.println("<body style=background-color:#222222>");
            client.println("<center>");
            client.println("<h1 color='white'>Automated Solar Panel Cleaning System</h1>");
            client.println("<a href=\"/?button1on\"\"><button>BUTTON ON</button></a>");
            client.println("<br><br><br>");
            client.println("<a href=\"/?button2off\"\"><button>MOTOR OFF</button></a>");
            client.println("</center>");
            client.println("</body>");
            client.println("</html>");
          }

          delay(1);
          //stopping client
          client.stop();

          int buttonRead = digitalRead(manualButton);
          if ((readString.indexOf("?button1on") > 0))
          {
            if (1) // add water level condition here
            {
              digitalWrite(relay5, LOW); // DOL Starter ACTIVATED
              unsigned long timeMillis = millis();

              digitalWrite(relay1, LOW); // Open valve 1
              delay(5000); // Wait for 5 secs for pump to start properly

              while (millis() <= timeMillis + 2900) // Run loop for 29 secs
              {
                SendData();
                delay(1000); // Send Pressure and Flowrate every 2 secs
              }
              digitalWrite(relay2, LOW); //Open valve 2, while valve 1 is still open
              delay(1000);
              digitalWrite(relay1, HIGH); //Close valve 1

              timeMillis = millis();
              while (millis() <= timeMillis + 3900) // Run loop for 39 secs
              {
                SendData();
                delay(1000); // Send Pressure and Flowrate every 2 secs
              }
              digitalWrite(relay3, LOW); //Open valve 3, while valve 2 is still open
              delay(1000);
              digitalWrite(relay2, HIGH); // Close valve 2

              timeMillis = millis();
              while (millis() <= timeMillis + 2900) // Run loop for 29 secs
              {
                SendData();
                delay(1000); // Send Pressure and Flowrate every 2 secs
              }
              digitalWrite(relay4, LOW); //Open valve 4, while valve 3 is still open
              delay(1000);
              digitalWrite(relay3, HIGH); // Close valve 3

              timeMillis = millis();
              while (millis() <= timeMillis + 4000) //Run loop for 40 secs
              {
                SendData();
                delay(1000); // Send Pressure and Flowrate every 2 secs
              }
              digitalWrite(relay6, LOW); // Activate DOL Stopper relay
              //              delay(500);

              delay(500);
              digitalWrite(relay5, HIGH); //Deavtivate DOL Starter relay
              delay(1000);
              digitalWrite(relay4, HIGH); // Close valve 4
              digitalWrite(relay6, HIGH); //Deavtivate DOL Stopper relay

            }
          }

          buttonRead = digitalRead(manualButton);
          if ((readString.indexOf("?button2off") > 0)) //Emergency Turn OFF DOL
          {
            digitalWrite(relay6, LOW);
            delay(500);
            digitalWrite(relay6, HIGH);
          }

          //clearing string for next read
          readString = "";

        }
      }
    }
  }
  beforeLoopTime = millis();
}

void sendTempNLevel()
{
  float w1 = sonar.ping_cm();
  Serial.println(w1);
  client.stop();
  if (client.connect(serverName, 80)) {
    Serial.println("connected for temp & water_level");

    //Call calc functions to calculate data
    float wl = sonar.ping_cm();
    float tp = calcTemp();

    // Make a HTTP request:
    client.print("GET /solarcleaning/Solardata.php?flowrate=");     //YOUR URL
    client.print(0);
    client.print("&temperature=");
    client.print(tp);
    client.print("&pressure=");
    client.print(0);
    client.print("&water_level=");
    client.print(wl);
    client.print(" ");      //SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    client.println("Host: 192.168.103.220");
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed for temp & water_level");
  }
  client.stop();

}

void SendData()   //CONNECTING WITH MYSQL
{
  float w1 = sonar.ping_cm();
  Serial.println(w1);
  client.stop();
  if (client.connect(serverName, 80)) {
    Serial.println("connected for all data");
    delay(10);

    //Call calc functions to calculate data
    float wl = sonar.ping_cm();
    //    float wl = 45;
    float tp = calcTemp();
    float Pr = calcPressure();
    float Fl = calcFlowrate();

    // Make a HTTP request:
    client.print("GET /solarcleaning/Solardata.php?flowrate=");     //YOUR URL
    client.print(Fl);
    client.print("&temperature=");
    client.print(tp);
    client.print("&pressure=");
    client.print(Pr);
    client.print("&water_level=");
    client.print(wl);
    client.print(" ");      //SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    client.println("Host: 192.168.103.220");
    client.println("Connection: close");
    client.println();
  } else {
    //if you didn't get a connection to the server:
    Serial.println("connection failed for all data");
  }
  client.stop();
}

float calcTemp()
{
  //Code to calculate temperature
  //  sensors.requestTemperatures(); // Send the command to get temperatures
  ////  float tempC = sensors.getTempC(deviceAddress);
  //  if(tempC == DEVICE_DISCONNECTED_C)
  //  {
  //    return -999;
  //  }
  return -222;
}

float calcPressure()
{
  //Code to calculate pressure
  return 2.4;
}

float calcFlowrate()
{
  //Code to calculate water flow rate
  return 2096;
}
