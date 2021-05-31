#include "dht.h"
#define dht_apin A0 // Analog Pin sensor is connected to
 int led1 = 13;
int led2 = 12;
//boolean Led1=True;
//boolean Led2=True;
int i=0;
int j=0;
dht DHT;
 
void setup(){
 pinMode(led1, OUTPUT);  
  pinMode(led2, OUTPUT);
  Serial.begin(9600);
  delay(500);//Delay to let system boot
  Serial.println("DHT11 Humidity & temperature Sensor\n\n");
  delay(1000);//Wait before accessing Sensor
 
}//end "setup()"
 
void loop(){
  //Start of Program 

    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    DHT.read11(dht_apin);
    Serial.print("Current humidity = ");
    Serial.print(DHT.humidity);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(DHT.temperature); 
    Serial.println("C  ");
    delay(1000);
    digitalWrite(led1, LOW);
    delay(1000);
    digitalWrite(led2, LOW);
    delay(2000);
    
    /*DHT.read11(dht_apin);
    Serial.print("Current humidity = ");
    Serial.print(DHT.humidity);
    Serial.print("%  ");
    Serial.print("temperature = ");
    Serial.print(DHT.temperature); 
    Serial.println("C  ");
    
    delay(5000);//Wait 5 seconds before accessing sensor again.*/
 
  //Fastest should be once every two seconds.
 
}// end loop() 
