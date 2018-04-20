#include <TimeLib.h>

#include <VL6180.h>
#include <Wire.h>
#include <Arduino.h>
#include <SmeNfc.h>
#include <SmeSFX.h>
#include <Time.h>

#define SIGFOX_FRAME_LENGTH 12

bool DEBUG = true;

time_t t = now();

bool isStart = true;

struct data {
  int startTime;
  int endTime;
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
  smeProximity.begin();
  
  SigFox.begin(19200);
  initSigfox();
  
  ledGreenLight(LOW);
}

void loop() {  
  checkSomethingOn();
  
  if(DEBUG) {
    SerialUSB.println("Thats one loop!");
  }
  delay(100);
  ledGreenLight(LOW);
  ledRedLight(LOW);
}

void checkSomethingOn() {
  int proximity = smeProximity.rangePollingRead();
  SerialUSB.print("Proximity ");
  SerialUSB.println(proximity);
  
  int threshold = 50;

  if(proximity < threshold && isStart){
    isStart = false;
    frame.startTime = second(now());
    SerialUSB.print("Second t1 ");
    SerialUSB.println(second(now()));
    SerialUSB.print("startTime ");
    SerialUSB.println(frame.startTime);
  } else if (proximity > threshold && !isStart){
    frame.endTime = second(now());
    SerialUSB.print("Second t2 ");
    SerialUSB.println(second(now()));
    isStart = true;
    SerialUSB.print("endTime ");
    SerialUSB.println(frame.endTime);
    sendSigfox(&frame, sizeof(data));
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
  SerialUSB.print("Frame ");
  SerialUSB.println(frame);
  uint8_t* bytes = (uint8_t*)data;
  //SerialUSB.println(bytes);
  
  if (len < SIGFOX_FRAME_LENGTH){
    //fill with zeros
    uint8_t i = SIGFOX_FRAME_LENGTH;
    while (i-- > len){
      frame += "00";
    }
  }

  //0-1 == 255 --> (0-1) > len
  for(uint8_t i = len-1; i < len; --i) {
    if (bytes[i] < 16) {frame+="0";}
    frame += String(bytes[i], HEX);
    SerialUSB.print("Frame ");
    SerialUSB.println(frame);
  }
  
  return frame;
}
