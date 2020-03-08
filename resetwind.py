import time
import smbus
import mysql
import mysql.connector
 
i2c_ch = 1
bus = smbus.SMBus(i2c_ch)

i2c_address = 20
 
bus.write_byte_data(i2c_address,5, 0)#set angle offset to 12
 











	
	

