/* Judas Gutenberg, Feb 19 2020
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
const byte WDIR = A0;

//////////////////////////////////////////////////
//globals 
//////////////////////////////////////////////////
long lastSecond; //The millis counter to see when a second rolls by
byte seconds; //When it hits 60, increase the current minute
byte seconds_2m; //Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
byte minutes; //Keeps track of where we are in various arrays of data
byte minutes_10m; //Keeps track of where we are in wind gust/dir over last 10 minutes array of data

long lastWindCheck = 0;
long rainTime = millis();
long highLong = 100000;
volatile long lastRainIRQ = millis();
volatile long previousRainIRQ = 0;
volatile long smallestRainIRQDelta = highLong;
 
int dailyRain = 0;
int rainCupThousandths = 11;
 
volatile long lastWindIRQ = 0;
volatile long previousWindIRQ = 0;
volatile byte windClicks = 0;
volatile long smallestWindIRQDelta = highLong;

volatile byte readMode = 0; //different readModes happen


void setup(){
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(receieveEvent); 
  Wire.onRequest(requestEvent);
  pinMode(INTERRUPT_OUT, OUTPUT);   
  digitalWrite(INTERRUPT_OUT, LOW);

  Serial.begin(9600);
  Serial.println("Weather Shield Example");

  pinMode(STAT1, OUTPUT); //Status LED Blue
  pinMode(STAT2, OUTPUT); //Status LED Green

  pinMode(WSPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
  pinMode(RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor

 

  seconds = 0;
  lastSecond = millis();

  // attach external interrupt pins to IRQ functions
  attachInterrupt(digitalPinToInterrupt(2), rainIRQ, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), wspeedIRQ, FALLING);
  // turn on interrupts
  interrupts();
}

void loop(){
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
  int val = rawToDirection(analogRead(0));
  Serial.print(val);
  Serial.println(" ");
  delay(2000);
}


//send a byte to the I2C master.  
//on the ATTiny, the the master calls this x times, not just once as I'd originally thought
//but in the true Wire library it seems you can send whole arrays, which is all that matters here
void requestEvent(){
  //the usual data dump
  if(readMode == 0) {
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
  } else if(readMode == 1) { //just give me wind stuff
    writeWireLong(millis());
    Wire.write("\t");
    writeWireLong(windClicks);
    Wire.write("\t");
    writeWireLong(lastWindIRQ);
    Wire.write("\t");
    writeWireLong(smallestWindIRQDelta);
  } else if(readMode == 2) { //just give me rain stuff
    writeWireLong(millis());
    Wire.write("\t");
    writeWireLong(dailyRain);
    Wire.write("\t");
    writeWireLong(lastRainIRQ);
    Wire.write("\t");
    writeWireLong(smallestRainIRQDelta);
  } else if(readMode == 3) { //just give me degree wind direction
    int val = rawToDirection(analogRead(0));
    writeWireInt(val);
  } else if(readMode == 4) { //just give me raw wind direction stuff
    int val = analogRead(0);
    writeWireInt(val);
  //if between 100 and 103 
  //give me the analog read from the four available anlog in pins (4 & 5 are taken up with I2C)
  } else if(readMode > 99 && readMode < 104) { 
    int val = analogRead(readMode-100);
    writeWireInt(val);
  } else if(readMode == 5) { //clear the recent gust storage
    smallestWindIRQDelta = highLong;
    Wire.write("\t");
    Wire.write("\t");
  } else if(readMode == 6) { //clear the recent downpour storage
    smallestRainIRQDelta = highLong;
    Wire.write("\t");
    Wire.write("\t");
  } else {
   Wire.write("\t");
  }
}

void receieveEvent() {
  Serial.println("receive event");
  byte byteCount = 0;
  while(0 < Wire.available()) // loop through all but the last
  {
    byte command = Wire.read();
    if(byteCount ==0) {
      readMode = command;
      Serial.println(command);
    }
    byteCount++;
  }
}

//converts the sparkfun resistance data from the windvane into degrees from north
//this allows for the resistance to wander from their set values by plus or minus 0.1 percent or so
//you may need to adjust these values for your particular windvane
int rawToDirection(int raw){
  int out  = 360;
  if(raw> 785 && raw<795) {
    out = 0;
  } else if(raw> 89 && raw < 95) {
    out = 90;
  } else if(raw> 89 && raw < 96) {
    out = 90;
  } else if(raw> 285 && raw < 295) {
    out = 180;
  } else if(raw> 940 && raw < 950) {
    out = 270;
  } else if(raw> 460 && raw < 470) {
    out = 45;
  } else if(raw> 180 && raw < 190) {
    out = 135;
  } else if(raw> 629 && raw < 636) {
    out = 225;
  } else if(raw> 885 && raw < 895) {
    out = 315;
  }
  return out;
}

void writeWireLong(long val) {
  byte buffer[4];
  buffer[0] = val >> 24;
  buffer[1] = val >> 16;
  buffer[2] = val >> 8;
  buffer[3] = val;
  Wire.write(buffer, 4);
}

void writeWireInt(int val) {
  byte buffer[2];
  buffer[0] = val >> 8;
  buffer[1] = val;
  Wire.write(buffer, 2);
}
 
//////////////////////////////////////////////////
//interrupt routines 
//////////////////////////////////////////////////
void rainIRQ() {
// Count rain gauge bucket tips as they occur
// Activated by the magnet and reed switch in the rain gauge, attached to input D2
    previousRainIRQ = lastRainIRQ;
    lastRainIRQ = millis(); // grab current time
    volatile long rainIRQDelta = rainTime - lastRainIRQ; // calculate interval between this and last event
    if (rainIRQDelta > 10) // ignore switch-bounce glitches less than 10ms after initial edge
    {
        if(rainIRQDelta> 0 && smallestRainIRQDelta < rainIRQDelta) {
          smallestRainIRQDelta = rainIRQDelta;
        }
        dailyRain += rainCupThousandths; //Each dump is 0.011" of water
        lastRainIRQ = rainTime; // set up for next event
    }
}


void wspeedIRQ() {
// Activated by the magnet in the anemometer (2 ticks per rotation), attached to input D3
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
