//Simple Audio In
//by Amanda Ghassaei
//https://www.instructables.com/id/Arduino-Audio-Input/
//Sept 2012

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
*/

int incomingAudio;

void setup(){
	Serial.begin(9600);
}

void loop(){
  incomingAudio = analogRead(A0);//read input from A0
  //do stuff with the variable "incomingAudio"
  Serial.println(incomingAudio);
}