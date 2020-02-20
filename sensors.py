import time
import smbus

i2c_ch = 1
bus = smbus.SMBus(i2c_ch)

i2c_address = 20

bus.write_byte_data(i2c_address, 14, 17)
val = bus.read_i2c_block_data(i2c_address,5)#get raw wind direction
print(val)

val = bus.read_i2c_block_data(i2c_address,3)#get  wind direction
print(val)

