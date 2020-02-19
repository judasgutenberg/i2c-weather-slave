# i2c-weather-slave
An Arduino sketch for an Arduino used just to read volatile, interrupt-based sensor data and present it over I2C.
Many weather sensors require interrupts to work correctly.  Since the Raspberry Pi doesn't really support interrupts, it's best to 
read data from such sensors using a special-purpose Arduino. For this application, I use a tiny Arduino Mini Pro running at 
8 Mhz.  It reads data from the interrupt-requiring Sparkfun weather sensors:  the one for measuring rain and the one for measuring wind
speed. It also reads the analog value from the wind direction sensor (since Raspberry Pis cannot read analog values without some
sort of assistance).

I use I2C to communicate from the Arduino back to the Raspberry Pi.  
sensors.py is a brief Python script for reading the Arduino data.

To buy these sensors, go here:
https://www.sparkfun.com/products/15901


