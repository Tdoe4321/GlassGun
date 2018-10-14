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
#include <Rotary.h>
//#include <Bounce2.h>

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
Rotary r = Rotary(encoderPinA,encoderPinB);
//Bounce encoderA = Bounce();
//Bounce encoderB = Bounce();
//int encoderBState = LOW;
//int encoderAState = LOW;
//int encoderPos = 0;
//int encoderPinALast = LOW;
//int encoderInput = LOW;

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

  //attachInterrupt(digitalPinToInterrupt(encoderPinA), encoderAChange, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(encoderPinB), encoderBChange, CHANGE);

//  encoderA.attach(encoderPinA, INPUT);
//  encoderA.interval(25);
//  encoderB.attach(encoderPinB, INPUT);
//  encoderB.interval(25);
    
  pinMode(13,OUTPUT); //led indicator pin
  pinMode(microphoneButton, INPUT);  //Microphone Pin
  
  if(printDebug){
    Serial.begin(9600);
  }
  
  resetMicInterupt();
}

void resetMicInterupt(){
  cli();//diable interrupts
  
  //set up continuous sampling of analog pin 0
  
  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  
  sei();//enable interrupts
}

/*
void resetRotaryInterupt(){
  cli();
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();
}

ISR(PCINT2_vect) {
  unsigned char result = r.process();
  if (result == DIR_NONE) {
    // do nothing
  }
  else if (result == DIR_CW) {
    //Serial.println("ClockWise");
    freqModifier++;
  }
  else if (result == DIR_CCW) {
    //Serial.println("CounterClockWise");
    freqModifier--;
  }
}
*/

ISR(ADC_vect) {//when new ADC value ready

  prevData = newData;//store previous value
  newData = ADCH;//get value from A0
  if (prevData < 127 && newData >=127){//if increasing and crossing midpoint
    period = timer;//get period
    timer = 0;//reset timer
  }
  
  
  if (newData == 0 || newData == 1023){//if clipping
    PORTB |= B00100000;//set pin 13 high- turn on clipping indicator led
    clipping = 1;//currently clipping
  }
  
  timer++;//increment timer at rate of 38.5kHz
}

/*
 * MAIN LOOP
 */

void loop(){  
  if (clipping){//if currently clipping
    PORTB &= B11011111;//turn off clippng indicator led
    clipping = 0;
  }

  frequency = 38462/period;//timer rate/period

  unsigned char result = r.process(); // See if the rotary encoder has moved
  if(result){
    firstHold = true;
    if(result == DIR_CW) freqModifier++;
    else freqModifier--;
    
    if(freqModifier < -50) freqModifier = -50;
    else if(freqModifier > 50) freqModifier = 50;
    if(printDebug){
      Serial.print("FreqMod: ");
      Serial.println(freqModifier);
    }
  }

  
  if(digitalRead(encoderButtonPin) == HIGH){
    freqModifier = 0;
  }

  if(digitalRead(microphoneButton) == HIGH){
    gotData = true;
    if(frequency > 300 && frequency < 1500){
      if(printDebug){    
        Serial.print(frequency);
        Serial.println(" hz");
        //Serial.println(freqData.size());
      }
      if(freqData.size() <= 270){ // Won't let arduino stall
        freqData.add(frequency);
      }
    }
    freqModifier = 0;
  }
  
  if(!gotData && freqData.size() != 0){
    resetMicInterupt();
    getMode();
  }
  
  gotData = false;

  if(digitalRead(speakerButton) == HIGH){
    if(firstHold){
      startTime = millis();
      firstHold = false;
    }
    else{
      if(printDebug){
        Serial.print("Freqeuncy at: ");
        Serial.println(modeValue + freqModifier);
      }
      if(millis() - startTime < stepVolumeLength){
        toneAC(modeValue + freqModifier, 6);
      }
      else if(millis() - startTime > stepVolumeLength && millis() - startTime < 2*stepVolumeLength){
        toneAC(modeValue + freqModifier, 8);
      }
      else if(millis() - startTime > 2*stepVolumeLength){
        toneAC(modeValue + freqModifier);
      }
    }
  }
  else{
    toneAC(0);
    firstHold = true;
  }

}

/*
 * COMPARATOR FUNCTION
 * Custom Comparitor function for sorting min -> max
 */
 
int minToMax(int &a, int &b){
  if (max(a, b) == a){
    return -1;
  }
  else return 1;
}

/*
 * FIND MODE FROM DATA
 * Returns the 'mode' of the freqData
 */
 
void getMode(){
  freqData.sort(minToMax);

  modeHold = freqData.get(0);
  modeValue = modeHold;
  for(int i = 0; i < freqData.size(); i++){
    if(freqData.get(i) == modeHold){
      ++modeCount;
    }
    else{
      if(modeCount > modeSubCount){
        modeSubCount = modeCount;
        modeValue = modeHold;
      }
      modeCount = 1;
      modeHold = freqData.get(i);
    }
  }
  
  if(printDebug){
    Serial.println("--------");
    Serial.println(modeValue);
  }
  
  modeCount = 1;
  modeSubCount = 1;

  freqData.clear();
}

/*
 * ENCODER CHECK
 * Checks to see if the rotary encoder has been turned or not
 * and updates the freqModifier if it has
 */

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
/*
  if(encoderAState != encoderPinALast){
    if(encoderBState != encoderAState){
      freqModifier++;
    }
    else{
      freqModifier--;
    }
  }
  encoderPinALast = encoderAState;

  */
  
}
/*
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

*/

