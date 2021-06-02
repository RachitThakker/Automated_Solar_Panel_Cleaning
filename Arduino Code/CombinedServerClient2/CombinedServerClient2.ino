#include <SPI.h>
#include <NewPing.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //assign arduino mac address
byte ip[] = {192, 168, 103, 35 }; // ip in lan assigned to arduino

EthernetServer server(80); //server port arduino server will use
EthernetClient client;
char serverName[] = "192.168.103.220"; // IP of server where database is stored

int relay1 = 2; // For Valve1
int relay2 = 3; // For Valve2
int relay3 = 4; // For Valve3
int relay4 = 5; // For Valve4
int relay5 = 6; // For DOL Stop Relay
int relay6 = 7; // For DOL Start Relay
int relay7 = 8;

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

String readString; //used by server to capture GET request

void setup() {

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(relay5, OUTPUT);
  pinMode(relay6, OUTPUT);
  pinMode(relay7, OUTPUT);
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  digitalWrite(relay5, LOW);
  digitalWrite(relay6, LOW);
  digitalWrite(relay7, LOW);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);
  Serial.print("Enter ");
  Serial.println(Ethernet.localIP());
  Serial.print(" in your browser");
}

void loop() {

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
            client.println("<a href=\"/?button1on\"\"><button>BUTTON ON</button></a>");
            client.println("<br><br><br>");
            client.println("<a href=\"/?button2off\"\"><button>BUTTON OFF</button></a>");
            client.println("</center>");
            client.println("</body>");
            client.println("</html>");
          }

          delay(1);
          //stopping client
          client.stop();

          if (readString.indexOf("?button1on") > 0)
          {
            if (sonar.ping_cm() > 99)
            {
              digitalWrite(relay7, HIGH);
              digitalWrite(relay2, HIGH);
              delay(5000);  // Delay to give valves time to open
              {
                SendData();
                delay(10000);
                SendData();
                delay(10000);
                SendData();
                delay(10000);
                SendData();
              }
              digitalWrite(relay7, LOW);
              SendData();
              delay(10000);
              digitalWrite(relay2, LOW);
            }
          }

          if (readString.indexOf("?button2off") > 0)
          {
            digitalWrite(relay7, LOW);
            digitalWrite(relay2, LOW);
          }

          //clearing string for next read
          readString = "";

        }
      }
    }
  }
}

void SendData()   //CONNECTING WITH MYSQL
{
  client.stop();
  if (client.connect(serverName, 80)) {
    Serial.println("connected");
    delay(10);

    //Call calc functions to calculate data
    float wl = sonar.ping_cm();
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
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  client.stop();
}

float calcTemp()
{
  //Code to calculate temperature
  return 22;
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
