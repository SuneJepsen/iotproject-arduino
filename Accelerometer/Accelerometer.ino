#include <TimeLib.h>

#include <LSM9DS1.h>
#include <Wire.h>
#include <Arduino.h>
#include <Time.h>

#define SIGFOX_FRAME_LENGTH 12

bool DEBUG = true;

time_t t;

bool isStart = true;

struct data {
  int startTime;
  int endTime;
  // Write the title backwards, because sigfox
  const char title[4] = "CCA";
};

data frame;

void setup() {
  ledGreenLight(HIGH);
  frame.startTime = 0;
  frame.endTime = 0;

  
  if(DEBUG) {
      SerialUSB.begin(115200);
      SerialUSB.println("Welcome");
        while (!SerialUSB) {
        ; 
      }
  }   
  smeAccelerometer.begin();
  
  SigFox.begin(19200);
  initSigfox();
  sendSigfox(&frame, sizeof(data)-1);
  ledGreenLight(LOW);
}

void loop() {  
  t = now();
  
  checkSomethingOn();
  
  if(DEBUG) {
    SerialUSB.println("Thats one loop!");
  }
  delay(100);
  ledGreenLight(LOW);
  ledRedLight(LOW);
}

void checkSomethingOn() {
  int z = smeAccelerometer.readZ();
  SerialUSB.print("Z axis ");
  SerialUSB.println(z);
  
  int threshold = 100;

  if(z < -threshold && isStart){
    isStart = false;
    frame.startTime = t;
    SerialUSB.print("Second t1 ");
    SerialUSB.println(t);
    SerialUSB.print("startTime ");
    SerialUSB.println(frame.startTime);
  } else if (z > threshold && !isStart){
    frame.endTime = t;
    SerialUSB.print("Second t2 ");
    SerialUSB.println(t);
    isStart = true;
    SerialUSB.print("endTime ");
    SerialUSB.println(frame.endTime);
    sendSigfox(&frame, sizeof(data)-1);
  } 
}

void initSigfox(){
  SigFox.print("+++");
  while (!SigFox.available()){
    delay(100);
  }
  while (SigFox.available()){
    byte serialByte = SigFox.read();
    if (DEBUG){
      SerialUSB.print("Serial Byte ");
      SerialUSB.println(serialByte);
    }
  }
  if (DEBUG){
    SerialUSB.println("\n ** Setup OK **");
  }
}

bool sendSigfox(const void* data, uint8_t len){
  String frame = getSigfoxFrame(data, len);
  String status = "";
  char output;
  if (DEBUG){
    SerialUSB.print("AT$SF=");
    SerialUSB.println(frame);
  }
  SigFox.print("AT$SF=");
  SigFox.print(frame);
  SigFox.print("\r");
  while (!SigFox.available());
  
  while(SigFox.available()){
    output = (char)SigFox.read();
    status += output;
    delay(10);
  }
  if (DEBUG){
    SerialUSB.print("Status \t");
    SerialUSB.println(status);
  }
  if (status == "OK\r"){
    //Success :)
    return true;
  }
  else{
    return false;
  }
}

String getSigfoxFrame(const void* data, uint8_t len){
  String frame = "";
  uint8_t* bytes = (uint8_t*)data;
  //SerialUSB.println(bytes);

  //0-1 == 255 --> (0-1) > len
  for(uint8_t i = len-1; i < len; --i) {
    if (bytes[i] < 16) {frame+="0";}
    frame += String(bytes[i], HEX);
    SerialUSB.print("Frame ");
    SerialUSB.println(frame);
  }
  
  return frame;
}

