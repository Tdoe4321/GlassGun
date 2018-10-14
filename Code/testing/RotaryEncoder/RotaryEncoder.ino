////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Arduino-based project to shatter a wine glass with sound using a flashlight-sized device
//  Author: Tyler Gragg
//  Start Date: 9/02/2018
//
//  Credit to:
//  ****** - LinkedList Library
//  ****** - ToneAC Library
//  ****** - Frequency capture using interupts
//  ****** - Bounce2 Library
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Extended Description:
//  This project was started to create a hand-held device that would be able to both find the resonant
//  frequency of a wine glass by tapping it, then reproducing that frequency at a higher volume and 
//  eventyally break it. I used a small 60W amplifier board, and a tiny Electret Microphone.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <LinkedList.h>
#include <toneAC.h>
#include <Bounce2.h>

//Debug Flag
boolean printDebug = true;

//Frequency capture
LinkedList<int> freqData;
int modeHold;
int modeCount = 1;
int modeSubCount = 1;
boolean gotData = false;

//Frequency emitting
int freqModifier = 0;
int modeValue;
boolean firstHold = true;
unsigned long startTime = 0;

//Tuning by way of rotary encoder
int val;
#define encoderButtonPin 4
#define encoderPinA 2
#define encoderPinB 3
//Bounce encoderA = Bounce();
//Bounce encoderB = Bounce();
int encoderBState = LOW;
int encoderAState = LOW;
int encoderPos = 0;
int encoderPinALast = LOW;
int encoderInput = LOW;

//Buttons to trigger microphone and speaker
#define speakerButton 5
#define microphoneButton 6

//Playing parameters
#define stepVolumeLength 3000 //time in ms

//clipping indicator variables
boolean clipping = 0;

//data storage variables
byte newData = 0;
byte prevData = 0;

//freq variables
unsigned int timer = 0;//counts period of wave
unsigned int period;
int frequency;

/*
 * SETUP AND INTERUPT
 */

void setup(){
  //pinMode(encoderPinA, INPUT);
  //pinMode(encoderPinB, INPUT);

  attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), encoderBChange, CHANGE);
}


/*
 * MAIN LOOP
 */

void loop(){
  //encoderA.update();
  //encoderB.update();
 
  //encoderCheck(); // See if the rotary encoder has moved
  if(freqModifier < -50) freqModifier = -50;
  else if(freqModifier > 50) freqModifier = 50;
  if(printDebug){
    Serial.print("FreqMod: ");
    Serial.println(freqModifier);
  }

}


void encoderCheck(){
//  encoderInput = digitalRead(encoderPinA);
//  if((encoderPinALast == LOW) && (encoderInput = HIGH)){
//    if(digitalRead(encoderPinB) == LOW){
//      freqModifier--;
//    }
//    else{
//      freqModifier++;
//    }
//  }
//  encoderPinALast = encoderInput;

  if(encoderAState != encoderPinALast){
    if(encoderBState != encoderAState){
      freqModifier++;
    }
    else{
      freqModifier--;
    }
  }
  encoderPinALast = encoderAState;
  
}

void encoderAChange(){
  //Serial.println("A CHANGED");
  //encoderPinALast = encoderAState;
  //if(encoderAState == HIGH) encoderAState = LOW;
  //else if(encoderAState == LOW) encoderAState = HIGH;
  //freqModifier++;
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if(interrupt_time - last_interrupt_time > 200){
  if((encoderBState) == LOW){
      freqModifier--;
    }
    else{
      freqModifier++;
    }
  }
  last_interrupt_time = interrupt_time;
}

void encoderBChange(){
  //Serial.println("B CHANGED");
  if(encoderBState == HIGH) encoderBState = LOW;
  else if(encoderBState == LOW) encoderBState = HIGH;
}

