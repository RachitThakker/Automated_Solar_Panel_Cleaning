import urllib.request, urllib.parse, urllib.error
import schedule, time, os

url = "http://192.168.1.35/?button1on"
def task():
    print("Job function called.")
    data = urllib.request.urlopen(url).read().decode()
    print(data)
    print("Process ID: ", os.getpid(), "\n")

# schedule.every().day.at("04:00").do(task)
schedule.every(3).seconds.do(task)

while True:
    schedule.run_pending()
    time.sleep(1)
