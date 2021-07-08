import paho.mqtt.client as mqtt
import urllib.request
import urllib.parse
import urllib.error
import sqlite3
import datetime
import re
import logging
import os

print("Process ID:", os.getpid())

# Create and configure logger
logging.basicConfig(filename="mqtt_data.log",
                    format='%(asctime)s %(message)s')

# Creating an object
logger = logging.getLogger()

# Setting the threshold of logger to DEBUG
logger.setLevel(logging.DEBUG)

# Connect to sqlite database
conn = sqlite3.connect("solarcleaning_offline.sqlite")
cur = conn.cursor()

# If table exists, do nothing. If table doesn't exist, create it.
command = '''
CREATE TABLE IF NOT EXISTS sensordata
(ID INTEGER PRIMARY KEY AUTOINCREMENT, date TEXT, temperature REAL, level INTEGER,
flow INTEGER, tds INTEGER, pressure REAL, endpressurevalve1 REAL, endpressurevalve2 REAL,
endpressurevalve3 REAL, endpressurevalve4 REAL);
'''
cur.execute(command)
conn.commit()

command = '''
CREATE TABLE IF NOT EXISTS valvedata
(id INTEGER PRIMARY KEY AUTOINCREMENT, date TEXT, time TEXT,
valveNumber INTEGER, valveStatus INTEGER)
'''
cur.execute(command)
conn.commit()

#The topic 'sensorTopic' is where sensor data will be received from Arduino.
#The topic 'valveTopic' is where valve close/open data will be received from Arduino.
sensorTopic = "arduino/sensor"
valveTopic = "arduino/valve"
toArduinoTopic = "ljsc1111/arduino/to"
fromArduinoTopic = "ljsc1111/arduino/from"

# The callback for when clientPi receives a CONNACK response from the Pi MQTT server.
def on_connect_pi(clientPi, userdata, flags, rc):
    print("Connected to raspberry pi with result code "+str(rc))

    # Subscribing inside on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    clientPi.subscribe(sensorTopic)
    clientPi.subscribe(valveTopic)
    clientPi.subscribe(toArduinoTopic)
    clientPi.subscribe(fromArduinoTopic)
    
    print("Subscribed to topics on PI server")

# The callback for when a PUBLISH message is received from the server.
def on_message_pi(clientPi, userdata, msg):
    #Record the time as soon as message is received.
    currentTime = str(datetime.datetime.now())
    currentTopic = str(msg.topic)
    currentData = str(msg.payload)[2:-1]
    
    print("From PI:", currentTopic, currentData)
    
    #Do this if sensor data is received from Arduino
    if (currentTopic == sensorTopic):
        insertOffline(currentData, currentTime)
        insertOnline(currentData, currentTime)
    
    #Do this if valve data is received from Arduino.
    if (currentTopic == valveTopic):
        insertOfflineValve(currentData, currentTime)
        insertOnlineValve(currentData, currentTime)


# The function to create proper URL and send HTTP request.
def insertOnline(currentData, currentTime):
    #Remove the unit smaller than seconds because MySQL doesn't support it.
    currentTime = currentTime[:19]

    # The incomplete URL which inserts into MySQL on phpMyAdmin
    url = "https://solar-31-ho.000webhostapp.com/Allsensordata.php?date=" + currentTime + "&"

    # Sample complete URL: https://solar-31-ho.000webhostapp.com/Allsensordata.php?
    # date=2021-06-23 13:30:00&temperature=23&level=78&flow=1223&tds=125&pressure=2.2&
    # endpressurevalve1=2.1&endpressurevalve2=2.2&endpressurevalve3=2.3&endpressurevalve4=2.4

    # The complete URL
    #Add Arduino pre-formatted data string to the URL
    url = url + currentData
    
    #Replace white space with '%20' or urllib won't be able to handle it.
    url = url.replace(" ", "%20")
    print(url)
    
    # The below line sends an HTTP request. It is similar to entering
    # the URL in a web browser
    try:
        data = urllib.request.urlopen(url, timeout=10).read().decode()
        print(data)
        print("Online insertion successful.")
    except Exception as e:
        # This will execute when the data is not sent to phpmyadmin server
        logger.error("\nData: "+currentTime+" "+currentData, exc_info=e)
        print(datetime.datetime.now(),
              "Data not sent to phpMyAdmin due to an issue. Check mqtt_data.log for details.")


#The function to insert sensor data into SQLite offline.
def insertOffline(currentData, currentTime):
    
    #Find the numerical values
    #It is possible here to write exception code for out of bound list or empty list
    temperSensor = re.findall("temperature=([0-9]+\.*[0-9]*)?", currentData)[0]
    flowSensor = re.findall("flow=([0-9]+\.*[0-9]*)?", currentData)[0]
    pressureSensor = re.findall("pressure=([0-9]+\.*[0-9]*)?", currentData)[0]
    levelSensor = re.findall("level=([0-9]+\.*[0-9]*)?", currentData)[0]
    tdsSensor = re.findall("tds=([0-9]+\.*[0-9]*)?", currentData)[0]
    endPressureSensor1 = re.findall("endpressurevalve1=([0-9]+\.*[0-9]*)?", currentData)[0]
    endPressureSensor2 = re.findall("endpressurevalve2=([0-9]+\.*[0-9]*)?", currentData)[0]
    endPressureSensor3 = re.findall("endpressurevalve3=([0-9]+\.*[0-9]*)?", currentData)[0]
    endPressureSensor4 = re.findall("endpressurevalve4=([0-9]+\.*[0-9]*)?", currentData)[0]

    #typecast according to SQLite data types
    temperSensor = float(temperSensor)
    flowSensor = int(flowSensor)
    pressureSensor = float(pressureSensor)
    levelSensor = int(levelSensor)
    tdsSensor = int(tdsSensor)
    endPressureSensor1 = float(endPressureSensor1)
    endPressureSensor2 = float(endPressureSensor2)
    endPressureSensor3 = float(endPressureSensor3)
    endPressureSensor4 = float(endPressureSensor4)

    insertCommand = '''
    INSERT INTO sensordata(date,temperature,level,flow,tds,pressure,
    endpressurevalve1,endpressurevalve2,endpressurevalve3,endpressurevalve4) 
    VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    '''

    insertTuple = (currentTime, temperSensor, levelSensor, flowSensor, tdsSensor,
                   pressureSensor, endPressureSensor1, endPressureSensor2, endPressureSensor3, endPressureSensor4)

    try:
        cur.execute(insertCommand, insertTuple)
        print("Offline data insertion successful.")
    except Exception as e:
        logger.error("\nData: "+currentTime+" "+currentData, exc_info=e)
        print(datetime.datetime.now(), "Data not updated locally due to an issue. Check mqtt_data.log for details.")
    conn.commit()
    #Save database. "Obviously." -Sherlock
    
def insertOnlineValve(currentData, currentTime):
    #Remove the unit smaller than seconds because MySQL doesn't support it.
    currentDate = currentTime[:10]
    currentTimeUpload = currentTime[11:19]

    # The incomplete URL which inserts into MySQL on phpMyAdmin
    url = "https://solar-31-ho.000webhostapp.com/Allvalvedata.php?date=" + currentDate + "&time=" + currentTimeUpload + "&"

    # Sample complete URL: https://solar-31-ho.000webhostapp.com/Allvalvedata.php?
    # date=2021-06-23&time=13:30:00&valveNumber=2&valveStatus=0

    # The complete URL
    #Add Arduino pre-formatted data string to the URL
    url = url + currentData
    
    #Replace white space with '%20' or urllib won't be able to handle it.
    url = url.replace(" ", "%20")
    print(url)
    
    # The below line sends an HTTP request. It is similar to entering
    # the URL in a web browser
    try:
        data = urllib.request.urlopen(url, timeout=10).read().decode()
        print(data)
        print("Online insertion successful.")
    except Exception as e:
        # This will execute when the data is not sent to phpmyadmin server
        logger.error("\nData: "+currentTime+" "+currentData, exc_info=e)
        print(datetime.datetime.now(),
              "Data not sent to phpMyAdmin due to an issue. Check mqtt_data.log for details.")

def insertOfflineValve(currentData, currentTime):
    #Find the numerical values
    #It is possible here to write exception code for out of bound list or empty list
    valveNum = re.findall("valveNumber=([0-9]+\.*[0-9]*)?", currentData)[0]
    valveStat = re.findall("valveStatus=([0-9]+\.*[0-9]*)?", currentData)[0]
    
    currentDate = currentTime[:10]
    currentTimeInsert = currentTime[11:]

    #typecast according to SQLite data types
    valveNum = int(valveNum)
    valveStat = int(valveStat)

    insertCommand = '''
    INSERT INTO valvedata(date, time, valveNumber, valveStatus) 
    VALUES(?, ?, ?, ?)
    '''

    insertTuple = (currentDate, currentTimeInsert, valveNum, valveStat)

    try:
        cur.execute(insertCommand, insertTuple)
        print("Offline data insertion successful.")
    except Exception as e:
        logger.error("\nData: "+currentTime+" "+currentData, exc_info=e)
        print(datetime.datetime.now(), "Data not updated locally due to an issue. Check mqtt_data.log for details.")
    conn.commit()
    #Save database. "Obviously." -Sherlock

#Create clientPi object that communicates with Raspberry Pi
clientPi = mqtt.Client()

#Tell the object instance clientPi what functions we have defined for it.
clientPi.on_connect = on_connect_pi
clientPi.on_message = on_message_pi

#Set authentication details.
clientPi.username_pw_set(username="mqtt@pi@user1", password="user1@pi@123456")

#The 60 is 'keepalive' parameter.
clientPi.connect("localhost", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
clientPi.loop_forever()
