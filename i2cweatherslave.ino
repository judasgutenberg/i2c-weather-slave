/*  Judas Gutenberg (aka Gus Mueller), February 18, 2020
 * compiles data on a small Arduino (I use a Mini Pro at 8MHz) to be read by a
 * Raspberry Pi over I2C
 * based on the Sparkfun Weather Shield example
 */
#include <Keypad.h>
#include "Wire.h"
#define I2C_SLAVE_ADDR 20
#define INTERRUPT_OUT 0
 

// digital I/O pins
const byte WSPEED = 3;
const byte RAIN = 2;
const byte STAT1 = 7;
const byte STAT2 = 8;

// analog I/O pins
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;
const byte WDIR = A0;


//Global Variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long lastSecond; //The millis counter to see when a second rolls by
byte seconds; //When it hits 60, increase the current minute
byte seconds_2m; //Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
byte minutes; //Keeps track of where we are in various arrays of data
byte minutes_10m; //Keeps track of where we are in wind gust/dir over last 10 minutes array of data

long lastWindCheck = 0;
long rainTime = millis();
volatile long lastRainIRQ = millis();
volatile long previousRainIRQ = 0;
volatile long smallestRainIRQDelta = 100000;
int rainHour[60];
int dailyRain = 0;
int rainCupThousandths = 11;
 

volatile long lastWindIRQ = 0;
volatile long previousWindIRQ = 0;
volatile byte windClicks = 0;
volatile long smallestWindIRQDelta = 100000;
 
 
 
 
char lastKey;

void setup(){
  Wire.begin(I2C_SLAVE_ADDR); 
  Wire.onRequest(requestEvent);
  pinMode(INTERRUPT_OUT, OUTPUT);   
  digitalWrite(INTERRUPT_OUT, LOW);

  Serial.begin(9600);
  Serial.println("Weather Shield Example");

  pinMode(STAT1, OUTPUT); //Status LED Blue
  pinMode(STAT2, OUTPUT); //Status LED Green

  pinMode(WSPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
  pinMode(RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor

  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);

  seconds = 0;
  lastSecond = millis();

  // attach external interrupt pins to IRQ functions
  attachInterrupt(0, rainIRQ, FALLING);
  attachInterrupt(1, wspeedIRQ, FALLING);
  // turn on interrupts
  interrupts();
}

void loop()
{
  Serial.print(millis());
  Serial.print("\t");
  Serial.print(windClicks);
  Serial.print("\t");
  Serial.print(lastWindIRQ);
  Serial.print("\t");
  Serial.print(smallestWindIRQDelta);
  Serial.print("\t");
  Serial.print(dailyRain);
  Serial.print("\t");
  Serial.print(lastRainIRQ);
  Serial.print("\t");
  Serial.print(smallestRainIRQDelta);
  Serial.print("\t");
  int val = analogRead(0);
  Serial.print(val);
  Serial.println(" ");
  delay(2000);
}


//send a byte to the I2C master.  
//on the ATTiny, the the master calls this x times, not just once as I'd originally thought
//but in the true Wire library it seems you can send whole arrays, which is all that matters here
void requestEvent()
{
  byte command = Wire.read();
  Serial.println(command);
  command = Wire.read();
  Serial.println(command);
  command = Wire.read();
  Serial.println(command);
  writeWireLong(millis());
  Wire.write("\t");
  writeWireLong(windClicks);
  Wire.write("\t");
  writeWireLong(lastWindIRQ);
  Wire.write("\t");
  writeWireLong(smallestWindIRQDelta);
  Wire.write("\t");
  writeWireLong(dailyRain);
  Wire.write("\t");
  writeWireLong(lastRainIRQ);
  Wire.write("\t");
  writeWireLong(smallestRainIRQDelta);
  Wire.write("\t");
  
 
}


void writeWireLong(long val) {
  byte buffer[4];
  buffer[0] = val >> 24;
  buffer[1] = val >> 16;
  buffer[2] = val >> 8;
  buffer[3] = val;
  Wire.write(buffer, 4);
}

 



//Interrupt routines (these are called by the hardware interrupts, not by the main code)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void rainIRQ()
// Count rain gauge bucket tips as they occur
// Activated by the magnet and reed switch in the rain gauge, attached to input D2
{
    previousRainIRQ = lastRainIRQ;
    lastRainIRQ = millis(); // grab current time
    volatile long rainIRQDelta = rainTime - lastRainIRQ; // calculate interval between this and last event

    if (rainIRQDelta > 10) // ignore switch-bounce glitches less than 10ms after initial edge
    {

        if(rainIRQDelta> 0 && smallestRainIRQDelta < rainIRQDelta) {

          smallestRainIRQDelta = rainIRQDelta;
        }
        
        dailyRain += rainCupThousandths; //Each dump is 0.011" of water
        rainHour[minutes] += rainCupThousandths; //Increase this minute's amount of rain

        lastRainIRQ = rainTime; // set up for next event
    }
}


void wspeedIRQ()
// Activated by the magnet in the anemometer (2 ticks per rotation), attached to input D3
{
    if (millis() - lastWindIRQ > 10) // Ignore switch-bounce glitches less than 10ms (142MPH max reading) after the reed switch closes
    {
        previousWindIRQ = lastWindIRQ;
        lastWindIRQ = millis(); //Grab the current time
        volatile long windIRQDelta = lastWindIRQ - previousWindIRQ;
        if(windIRQDelta> 0 && windIRQDelta < smallestWindIRQDelta) {

          smallestWindIRQDelta = windIRQDelta;
        }
        windClicks++; //There is 1.492MPH for each click per second.
    }
}
