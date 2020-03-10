import time
import smbus
import smbus2
import mysql
import mysql.connector

import bme280



 
 

def writeDataRecord(temperature, pressure, humidity, wind_direction, precipitation, wind_speed, wind_increment):
	mydb = mysql.connector.connect(
	  host="localhost",
	  user="weathertron",
	  passwd="tron",
	  database="weathertron"
	)
	sql = "INSERT INTO weather_data(recorded, temperature, pressure, humidity, wind_direction, precipitation, wind_speed, wind_increment) VALUES (now(), " + str(temperature) + "," + str(pressure) + "," + str(humidity) + "," + str(wind_direction) + "," + str(precipitation) + "," + str(wind_speed) + "," + str(wind_increment) + ")"
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


i2c_ch = 1
bus = smbus.SMBus(i2c_ch)


port = 1
bmeAddress = 0x76 # Adafruit BME280 address. Other BME280s may be different


temperature,pressure,humidity = bme280.readBME280All()

i2c_address = 20

bus.write_byte_data(i2c_address, 14, 17)
#val = bus.read_i2c_block_data(i2c_address,5)#get raw wind direction
#print(val)


direction = 360 
while direction == 360: #direction should never be 360
	val = bus.read_i2c_block_data(i2c_address,3)#get  wind direction
	direction = intFromBytes(val)
print(val)

rainArray = bus.read_i2c_block_data(i2c_address,2)#get rain info
#bus.read_i2c_block_data(i2c_address,6)#delete accumlated rain -- doesn't seem to work
#bus.read_i2c_block_data(i2c_address,5)#delete accumlated gust -- doesn't seem to work
fixedRainArray = collapseDelimitedBytesIntoIntegers(rainArray);
print("--rain--");
print(fixedRainArray)
windArray = bus.read_i2c_block_data(i2c_address,1)#get wind info

fixedWindArray = collapseDelimitedBytesIntoIntegers(windArray);
windDelta = fixedWindArray[0] - fixedWindArray[2]
print("--wind--");
print(windArray)
print(fixedWindArray)

writeDataRecord(temperature, pressure, 0, direction, fixedRainArray[1], fixedWindArray[1] , windDelta)












	
	

