''' IMPORTANT !!! PLEASE READ !!! '''
# This script will command the Arduino board to turn ON
# the relays at 4 AM everyday. For this, it must always be running.

# It is necessary to install 'schedule' package for Python.
# Use command 'pip install schedule' to do the same.

# This script must execute at Start-up automatically.
# To do so, place this script in the location given below:

# For Windows:
# C:\Users\USER_NAME\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup

# Make sure this file opens by python.exe IDE otherwise
# it might open as a .py readable file.

import urllib.request, urllib.parse, urllib.error
import schedule, time, os

url1 = "http://192.168.103.35/?button1on"
url2 = "http://192.168.103.35/?button2off"
print("Process ID: ", os.getpid(), "\n")

def task():
    print("Scheduled function called.")
    data = urllib.request.urlopen(url1)
    print("Process ID: ", os.getpid(), "\n")

schedule.every().day.at("04:00").do(task)
# schedule.every(50).seconds.do(task)

while True:
    schedule.run_pending()
    time.sleep(1)
