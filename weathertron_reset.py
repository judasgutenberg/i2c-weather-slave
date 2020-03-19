import time 
import RPi.GPIO as GPIO

 
#GPIO 18 is connected to the reset on the Arduino, allowing us to reboot it now and then
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.OUT)
GPIO.output(18, GPIO.HIGH)
GPIO.output(18, GPIO.LOW)
time.sleep(0.1) 
GPIO.output(18, GPIO.HIGH)

print("Arduino reset")


	
	

