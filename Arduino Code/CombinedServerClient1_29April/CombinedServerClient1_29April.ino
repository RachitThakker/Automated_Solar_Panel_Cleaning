
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //assign arduino mac address
byte ip[] = {192, 168, 1, 35 }; // ip in lan assigned to arduino

EthernetServer server(80); //server port arduino server will use
EthernetClient client;
char serverName[] = "192.168.1.32"; // IP of server where database is stored

int relay1 = 8, relay2 = 9;
String readString; //used by server to capture GET request

//////////////////////

void setup() {

  pinMode(led1, OUTPUT); //pin selected to control
  digitalWrite(led1, LOW);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.begin(9600);
  Serial.println(F("server/client 1.0 test 13/4/2021")); // keep track of what is loaded
  Serial.println("Enter 192.168.1.35 in your browser"); //create client from your browser
  Serial.println(F("Press button ON to test client")); // what to do to test client
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
          //Serial.print(c);
        }

        //if HTTP request has ended
        if (c == '\n') {

          ///////////////
          Serial.print(readString); //print to serial monitor for debuging

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
            //client.println("Refresh: 10");  // refresh the page automatically every 5 sec
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("<title>Solar Cleaning WebServer</title>");
            client.println("<style>");
            client.println("button {");
            client.println("  background-color: #f44336;\n  color: white;\n  padding: 14px 20px;\n  margin: 8px 0;\n  border:none;\n  cursor:pointer;\n  width: 60%;\n  height: 250px;");
            client.println("}");
            client.println("button: hover {");
            client.println("  opacity: 0.8;");
            client.println("}");
            client.println("</style>");
            client.println("</head>");
            client.println("<body style=background-color:#222222>");
            client.println("<center>");
            client.println("<a href=\"/?button1on\"\"><button>BUTTON ON</button></a>");
            client.println("<a href=\"/?button2off\"\"><button>BUTTON OFF</button></a>");
            client.println("</center>");
            client.println("</body>");
            client.println("</html>");
          }

          delay(1);
          //stopping client
          client.stop();

          ///////////////////// control arduino pin
          if (readString.indexOf("?button1on") > 0) 
          {
            if (calcWaterLevel() > 99)
            {
              digitalWrite(relay1, HIGH);
              digitalWrite(relay2, HIGH);
              delay(5000);  // Delay to give valves time to open
              client.println("<h4>Cleaning Started.</h4>");
              {
                SendData();
                delay(10000);
                SendData();
                delay(10000);
                SendData();
                delay(10000);
                SendData();
              }
              digitalWrite(relay1, LOW);
              SendData();
              delay(10000);
              digitalWrite(relay2, LOW);
              client.println("<h4>Cleaning Finished.</h4>");
            }
          }

          if (readString.indexOf("?button2off") > 0) 
          {
            digitalWrite(relay1, LOW);
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
  if (client.connect(serverName, 8080)) {
    Serial.println("connected");
    delay(1000);

    //Call calc functions to calculate data
    float wl = calcWaterLevel();
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
    client.println("Host: 192.168.1.32");
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

float calcWaterLevel()
{
  //Code to calculate water level
  return 123;
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
