import time 
import RPi.GPIO as GPIO

#GPIO 18 is connected to the reset on the Arduino, allowing us to reboot it now and then
#for now, set it high so the Arduino can function
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.OUT)
GPIO.output(18, GPIO.HIGH)
 
print("Arduino set not to reset")


	
	

