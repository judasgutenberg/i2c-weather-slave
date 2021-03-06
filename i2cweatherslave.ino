/* Judas Gutenberg, Feb 19 2020
 * compiles data on a small Arduino (I use a Mini Pro at 8MHz) to be read by a
 * Raspberry Pi over I2C
 * based on the Sparkfun Weather Shield example
 */
 
#include "Wire.h"
#define I2C_SLAVE_ADDR 20
#define INTERRUPT_OUT 0
#include "DHT.h"


// digital I/O pins
const byte WSPEED = 3;
const byte RAIN = 2;
const byte STAT1 = 7;
const byte STAT2 = 8;
const byte DHT11PIN = 5;
const byte rpResetter = 7;

// analog I/O pins
const byte WDIR = A0;
const long maxMillisToRebootRP = 2400000;


//we are using a DHT22 hygrometer
DHT dht(DHT11PIN, DHT22);

//////////////////////////////////////////////////
//globals 
//////////////////////////////////////////////////
long lastSecond; //The millis counter to see when a second rolls by
byte seconds; //When it hits 60, increase the current minute
byte seconds_2m; //Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
byte minutes; //Keeps track of where we are in various arrays of data
byte minutes_10m; //Keeps track of where we are in wind gust/dir over last 10 minutes array of data
int angleCorrection = 0;

long lastWindCheck = 0;
long highLong = 100000;
volatile long lastRainIRQ = millis();
volatile long previousRainIRQ = 0;
volatile long smallestRainIRQDelta = highLong;
 
long dailyRain = 0;
int rainCupThousandths = 11;
 
volatile long lastWindIRQ = 0;
volatile long previousWindIRQ = 0;
volatile long windClicks = 0;
volatile long smallestWindIRQDelta = highLong;

volatile byte readMode = 0; //different readModes happen
volatile int receivedValue = 0;


int humidity = 0;
int temperatureFromHumidity = 0;

long lastRPsignal = millis();
long millisNow = millis();

void setup(){
  dht.begin();
  Wire.begin(I2C_SLAVE_ADDR);
  Wire.onReceive(receieveEvent); 
  Wire.onRequest(requestEvent);
  pinMode(INTERRUPT_OUT, OUTPUT);   
  
  digitalWrite(INTERRUPT_OUT, LOW);

  
  pinMode(rpResetter, OUTPUT);
  digitalWrite(rpResetter, HIGH);

  Serial.begin(9600);
  Serial.println("Starting up Arduino Slave...");

  pinMode(WSPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
  pinMode(RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor
  
  seconds = 0;
  lastSecond = millis();

  // attach external interrupt pins to IRQ functions
  attachInterrupt(digitalPinToInterrupt(RAIN), rainIRQ, FALLING);
  attachInterrupt(digitalPinToInterrupt(WSPEED), wspeedIRQ, FALLING);
  // turn on interrupts
  interrupts();
}

void loop(){
  millisNow = millis();
  //if it's been more than 20 minutes since a raspberry pi i2c signal:
  Serial.print("millis: ");
  Serial.print(millisNow);
  Serial.print(" ");
  Serial.print(lastRPsignal);
  Serial.println(" ");
  if(millisNow > maxMillisToRebootRP && millisNow - lastRPsignal > maxMillisToRebootRP) {
    //reset the raspberry pi!!
    Serial.println("Raspberry Pi RESET Time!");
    digitalWrite(rpResetter, LOW);
    delay(100);
    digitalWrite(rpResetter, HIGH);
    delay(100);
    lastRPsignal = millis();
  
  }
  /*
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
  */
  int val = rawToDirection(analogRead(0), angleCorrection);
  Serial.print("direction: ");
  Serial.print(analogRead(0));
  Serial.print(" ");
  Serial.print(val);
  Serial.println(" ");

  humidity = dht.readHumidity() * 100;
  temperatureFromHumidity = dht.readTemperature() * 100;
 
 
  //Serial.print(millis());
  //Serial.print(" Humidity (%): ");
  //Serial.print(humidity);
  //Serial.print(" Temperature (C): ");
  //Serial.println(temperatureFromHumidity);
  delay(2000);
}

//send a byte to the I2C master.  
//on the ATTiny, the the master calls this x times, not just once as I'd originally thought
//but in the true Wire library it seems you can send whole arrays, which is all that matters here
void requestEvent(){
  lastRPsignal = millisNow;
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
    int val = rawToDirection(analogRead(0), angleCorrection);
    writeWireInt(val);
    Wire.write("\t");
  } else if(readMode == 4) { //just give me raw wind direction stuff
    int val = analogRead(0);
    writeWireInt(val);
    Wire.write("\t");
  //if between 100 and 103 
  //give me the analog read from the four available anlog in pins (4 & 5 are taken up with I2C)
  } else if(readMode > 99 && readMode < 104) { 
    int val = analogRead(readMode-100);
    writeWireInt(val);
    Wire.write("\t");
  } else if(readMode == 5) { //clear the recent gust storage
    smallestWindIRQDelta = highLong;
    windClicks = 0;
    Wire.write("\t");
    Wire.write("\t");
  } else if(readMode == 6) { //clear the recent downpour storage
    smallestRainIRQDelta = highLong;
    dailyRain = 0;
    Wire.write("\t");
    Wire.write("\t");
  } else if(readMode == 7) {//set angle correction
    Serial.print("ANGLE CORRECTION:");
    Serial.println(receivedValue);
    angleCorrection = receivedValue;
   } else if(readMode == 8) { //humidity
    writeWireInt(humidity);
    Wire.write("\t");
    writeWireInt(temperatureFromHumidity);
    Wire.write("\t");
   } else if(readMode == 9) { //give me wind direction and raw direction
    int val = analogRead(analogRead(0));
    writeWireInt(val);
    Wire.write("\t");
    int realDirection = rawToDirection(val, angleCorrection);
    writeWireInt(realDirection);
    Wire.write("\t");
   } else if(readMode == 10) { //just give me millis
    writeWireInt(millis());
    Wire.write("\t");
   } else if(readMode == 11) { //just give me the time since last we got a raspberry pi command
    writeWireLong(lastRPsignal);
    Wire.write("\t");
    writeWireLong(millis() - lastRPsignal);
    Wire.write("\t");
    writeWireLong(millis());
    Wire.write("\t");
   } else {
    Wire.write("\t");
  }
}

void receieveEvent() {
  lastRPsignal = millisNow;
  //Serial.println("receive event");
  byte byteCount = 0;
  byte byteCursor = 0;
  byte receivedValues[4];
  byte receivedByte = 0;
  byte command = 0;
  byte byteRead = 0;
  receivedValue = 0;
  while(0 < Wire.available()) // loop through all but the last
  {
    byteRead = Wire.read();
    
    if(byteCount ==0) {
      readMode = byteRead;
      command = byteRead;
      //Serial.println(command);
    } else {
      receivedByte = byteRead;
      //Serial.println("got more than a command");
      receivedValues[byteCursor] = receivedByte;
      //Serial.println(receivedByte);
      byteCursor++;
    }
    byteCount++;
  }
  for(byte otherByteCursor = byteCursor; otherByteCursor>0; otherByteCursor--) {
    receivedValue = receivedValue + receivedValues[otherByteCursor-1] * pow(256, byteCursor-1)  ;
    //Serial.println("qoot: ");
    //Serial.print(byteCursor-1);
    //Serial.print(":");
    //Serial.print(receivedValue);
  }
  if(command == 7) { //have to do these here i guess
   //Serial.println("setting angle correction");
   angleCorrection = receivedValue;
  } else if(command == 5) { //clear the recent gust storage
    //Serial.println("resetting wind count");
    smallestWindIRQDelta = highLong;
    windClicks = 0;
  } else if(command == 6) { //clear the recent downpour storage
    //Serial.println("resetting rain counts");
    smallestRainIRQDelta = highLong;
    dailyRain = 0;
  }
}

//converts the sparkfun resistance data from the windvane into degrees from north
//this allows for the resistance to wander from their set values by plus or minus 0.1 percent or so
//you may need to adjust these values for your particular windvane
int rawToDirection(int raw, int angleCorrection){
  int out  = 360;
  if(raw> 751 && raw<800) {
    out = 0;
  } else if(raw> 400 && raw < 520) {
    out = 22;
  } else if(raw> 92 && raw < 95) {
    out = 67;
  } else if(raw> 88 && raw < 125) {
    out = 90; //was 67
  } else if(raw> 180 && raw < 220) {
    out = 135;  
  } else if(raw> 285 && raw < 321) {
    out = 180;
  } else if(raw> 125 && raw < 180) {
    out = 158;
  } else if(raw> 220 && raw < 310) {
    out = 203;
  } else if(raw> 60 && raw < 89) {
    out = 110;
  } else if(raw> 940 && raw < 950) {
    out = 270;
  } else if(raw> 440 && raw < 485) {
    out = 45;
  } else if(raw> 629 && raw < 648) {
    out = 225;
  } else if(raw> 830 && raw < 880) {
    out = 292;
  } else if(raw> 885 && raw < 895) {
    out = 315;
  } else if(raw> 680 && raw < 751) {
    out = 340;
  } else if(raw> 600 && raw < 629) {
    out = 248; //
 
  }
  //Serial.print("XX");
  //Serial.println(angleCorrection);
  if(angleCorrection > 0) {
    out = out + angleCorrection;
    if(out>359) {
      out = out - 360;
    }
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
    volatile long rainIRQDelta = lastRainIRQ - previousRainIRQ; // calculate interval between this and last event
    if ((unsigned long)(rainIRQDelta) >= 10) { //better handle rollover
    //if (rainIRQDelta > 10) { 
        // ignore switch-bounce glitches less than 10ms after initial edge
        if(rainIRQDelta> 0 &&  rainIRQDelta < smallestRainIRQDelta) {
          smallestRainIRQDelta = rainIRQDelta;
        }
        dailyRain += rainCupThousandths; //Each dump is 0.011" of water
    }
}


void wspeedIRQ() {
// Activated by the magnet in the anemometer (2 ticks per rotation), attached to input D3
  // Ignore switch-bounce glitches less than 10ms (142MPH max reading) after the reed switch closes
    if ((unsigned long)(millis() - lastWindIRQ) >= 10) { //better handle rollover
    //if (millis() - lastWindIRQ > 10) {
        previousWindIRQ = lastWindIRQ;
        lastWindIRQ = millis(); //Grab the current time
        volatile long windIRQDelta = lastWindIRQ - previousWindIRQ;
        if(windIRQDelta> 0 && windIRQDelta < smallestWindIRQDelta) {
          smallestWindIRQDelta = windIRQDelta;
        }
        windClicks++; //There is 1.492MPH for each click per second.
    }
}
