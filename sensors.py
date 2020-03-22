from __future__ import division 
import time 
import smbus 
import smbus2 
import mysql 
import mysql.connector
import bme280
import RPi.GPIO as GPIO


def writeDataRecord(temperature, pressure, humidity, wind_direction, precipitation, wind_speed, wind_increment, rain_increment, millis, failed_attempts):
	mydb = mysql.connector.connect(
	  host="localhost",
	  user="weathertron",
	  passwd="tron",
	  database="weathertron"
	)
	sql = "INSERT INTO weather_data(recorded, temperature, pressure, humidity, wind_direction, precipitation, wind_speed, wind_increment, rain_increment, millis, failed_attempts) VALUES (now(), " + str(temperature) + "," + str(pressure) + "," + str(humidity) + "," + str(wind_direction) + "," + str(precipitation) + "," + str(wind_speed) + "," + str(wind_increment) +"," + str(rain_increment) + "," + str(millis) + "," + str(failed_attempts) + ")"
	#print("++++++")
	#print(sql)
	#val = (temperature, pressure, humidity, wind_direction, precipitation, wind_speed, wind_increment)
	mycursor = mydb.cursor()
	mycursor.execute(sql)
	mydb.commit()

def intFromBytes(bytes):
	if len(bytes) > 0:
		return bytes[0]*256 + bytes[1]
		
def collapseDelimitedBytesIntoIntegers(delimitedBytes):
	outArray = []
	currentByteArray = []
	for byte in delimitedBytes:
		if byte!= 9:
			currentByteArray.append(byte)
		else: 
			currentByteArray.reverse()
			thisInt = 0
			scale = 0
			for currentByte in currentByteArray:
				thisInt =  thisInt + currentByte * (256 ** scale)
				scale+=1
			outArray.append(thisInt)
			thisInt = 0
			currentByteArray = []
	return outArray

#set the reset line to the arduino so it is not being reset
GPIO.setwarnings(False) 
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.OUT)
GPIO.output(18, GPIO.HIGH)


i2c_ch = 1
bus = smbus.SMBus(i2c_ch)


port = 1
bmeAddress = 0x76 # Adafruit BME280 address. Other BME280s may be different
temperature,pressure,humidity = bme280.readBME280All()
i2c_address = 20

try:
	bus.write_byte_data(i2c_address, 14, 17)
except:
	#if you cannot reach the arduino, reset it
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(18, GPIO.OUT)
	GPIO.output(18, GPIO.HIGH)
	GPIO.output(18, GPIO.LOW)
	time.sleep(0.1) 
	GPIO.output(18, GPIO.HIGH)
	time.sleep(0.1) 

wind_direction = 360
loopTimes = 0
while wind_direction == 360:
	rawWindDirection = bus.read_i2c_block_data(i2c_address,3)#get  wind direction
	wind_direction = intFromBytes(rawWindDirection)
	loopTimes += 100000
millis = 0

while millis < 2:
	

	rawHumidityArray  = bus.read_i2c_block_data(i2c_address,8)#get humidity info
	fixedHumidityArray = collapseDelimitedBytesIntoIntegers(rawHumidityArray);
	#print("humidity----")
	#print(fixedHumidityArray[0])
	humidity = fixedHumidityArray[0]
	#bus.read_i2c_block_data(i2c_address,6)#delete accumlated rain -- doesn't seem to work
	#bus.read_i2c_block_data(i2c_address,5)#delete accumlated gust -- doesn't seem to work
	
	
	#make six readings of rain and get the millis from them. if any are off
	millisAray = [0] * 6
	for iteration in range(6):
		try:
			rainArray = bus.read_i2c_block_data(i2c_address,2)#get rain info
			fixedRainArray = collapseDelimitedBytesIntoIntegers(rainArray);
			rain_amount = fixedRainArray[1]
			millis = fixedRainArray[0]
			millisAray[iteration] = millis
			rain_increment = millis - fixedRainArray[2]
		except:
			millis = 0

	print(max(millisAray), min(millisAray))
	if max(millisAray) - min(millisAray) > 400:
		millis = 0
	#if millis is small, maybe there was an error, so wait a tenth of a second before doing the wind lookup
	if millis<1000:
		time.sleep(0.1) 
	
	#print("--rain--");
	#print(fixedRainArray)
	try:
		windArray = bus.read_i2c_block_data(i2c_address,1)#get wind info
		
		fixedWindArray = collapseDelimitedBytesIntoIntegers(windArray);
		otherMillis = fixedWindArray[0]
		wind_increment = fixedWindArray[0] - fixedWindArray[2]
	except:
		otherMillis = 0
	#we get millis from both rain and windspeed data, taken close to one another, 
	#so if they are too far apart, one of them must've been garbled
	print("millis check----")
	print(millis, otherMillis)
	if abs(millis - otherMillis) > 400:
		millis = 0 #if so, set millis to zero so we don't exit this loop
		
 

	#print("--wind--");
	#print(windArray)
	#print(fixedWindArray)
	#print(wind_increment)
	#print(float(wind_increment/1000))
	#print("***********")
	if wind_increment == 0:
		wind_increment = 0.001
	
	decimalHumidity = float(humidity/100)
	
	wind_speed =  1.41/float(wind_increment/1000) 
	#print(wind_speed)
	loopTimes += 1
	
writeDataRecord(temperature, pressure, decimalHumidity, wind_direction, rain_amount, wind_speed, wind_increment, rain_increment, millis, loopTimes)








	
	

