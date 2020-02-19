import time
import smbus

i2c_ch = 1
bus = smbus.SMBus(i2c_ch)

i2c_address = 20
#still trying to figure out how to send a byte to a slave, see:
#https://stackoverflow.com/questions/60292104/how-do-i-read-a-byte-from-a-slave-i2c-device-while-handling-a-request-from-the-m
#bus.write_byte_data(i2c_address, 113,111)
val = bus.read_i2c_block_data(i2c_address,12)
#bus.write_byte(i2c_address, 123)

print(val)

