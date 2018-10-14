////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Arduino-based project to shatter a wine glass with sound using a flashlight-sized device
//  Author: Tyler Gragg
//  Start Date: 9/02/2018
//
//  Credit to:
//  ivansedel - LinkedList Library - https://github.com/ivanseidel/LinkedList
//  Tim Eckel - ToneAC Library - https://bitbucket.org/teckel12/arduino-toneac/wiki/Home
//  amandaghassaei - Frequency capture using interupts - https://www.instructables.com/id/Arduino-Frequency-Detection/
//  brianlow - Rotary Library - https://github.com/brianlow/Rotary
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

//Debug Flag
boolean printDebug = true;

//Frequency capture
LinkedList<int> freqData;
LinkedList<int> NOT_DATA;
int modeHold;
int modeCount = 1;
int modeSubCount = 1;
boolean gotData = false;
boolean badData = true;

//Frequency emitting
int freqModifier = 0;
int modeValue;

//Tuning by way of rotary encoder
int val;
#define encoderButtonPin 4
#define encoderPinA 2
#define encoderPinB 3
Rotary r = Rotary(encoderPinA, encoderPinB);

//Buttons to trigger microphone and speaker
#define speakerButton 5
#define microphoneButton 6

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
   SETUP AND INTERUPT
*/

void setup() {
  pinMode(13, OUTPUT); //led indicator pin
  pinMode(microphoneButton, INPUT_PULLUP);  //Microphone Pin
  pinMode(speakerButton, INPUT_PULLUP);

  if (printDebug) {
    Serial.begin(9600);
  }

  resetMicInterupt();
}

void resetMicInterupt() {
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

ISR(ADC_vect) {//when new ADC value ready

  prevData = newData;//store previous value
  newData = ADCH;//get value from A0
  if (prevData < 127 && newData >= 127) { //if increasing and crossing midpoint
    period = timer;//get period
    timer = 0;//reset timer
  }


  if (newData == 0 || newData == 1023) { //if clipping
    PORTB |= B00100000;//set pin 13 high- turn on clipping indicator led
    clipping = 1;//currently clipping
  }

  timer++;//increment timer at rate of 38.5kHz
}

/*
   MAIN LOOP
*/

void loop() {
  if (clipping) { //if currently clipping
    PORTB &= B11011111;//turn off clippng indicator led
    clipping = 0;
  }

  frequency = 38462 / period; //timer rate/period

  unsigned char result = r.process(); // See if the rotary encoder has moved
  if (result) {
    if (result == DIR_CW) freqModifier++;  // If we moved clockwise, increase, otherwise, decrease
    else freqModifier--;

    if (freqModifier < -50) freqModifier = -50;  // Clip the value from -50 to 50
    else if (freqModifier > 50) freqModifier = 50;
    if (printDebug) {
      Serial.print("FreqMod: ");
      Serial.println(freqModifier);
    }
  }

  if (digitalRead(encoderButtonPin) == LOW) {  // Resets the modifier if the Encoder Button is pressed
    freqModifier = 0;
    if (printDebug) {
      Serial.print("FreqMod: ");
      Serial.println(freqModifier);
    }
  }

  if (digitalRead(microphoneButton) == HIGH) { // The interrupt is running all the time, but we only store the values if the
    if (frequency > 300 && frequency < 1300) {
      Serial.print(frequency);
      Serial.println(" hz");
    }
    freqModifier = 0;  // Reset the modifier if we record a new value
  }

}



