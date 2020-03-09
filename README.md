# Weathertron
This is a sketch for an Arduino used just to read volatile, interrupt-based sensor data and present it over I2C.
Many weather sensors require interrupts to work correctly.  Since the Raspberry Pi doesn't really support interrupts, it's best to 
read data from such sensors using a special-purpose Arduino. For this application, I use a tiny Arduino Mini Pro running at 
8 Mhz.  It reads data from the interrupt-requiring Sparkfun weather sensors:  the one for measuring rain and the one for measuring wind
speed. It also reads the analog value from the wind direction sensor (since Raspberry Pis cannot read analog values without some
sort of assistance).  There is a provision for setting an angle correction for the wind direction sensor should the sensors not be oriented correctly.

The Arduino sketch is designed so it can read any of its analog lines when commanded to.

To receive the sensor data via I2C on a Raspberry Pi,  
sensors.py.  In this version, data is converted from bytes into decimals and saved to a MySQL database.

To buy these sensors, go here:
https://www.sparkfun.com/products/15901


