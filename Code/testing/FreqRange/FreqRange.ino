int freq = 0;
int pot = A0;
int speaker = 8;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(pot, INPUT);
  pinMode(speaker, OUTPUT);  
}

void loop() {
  // put your main code here, to run repeatedly:
  freq = map(analogRead(pot), 0, 1024, 0, 6500);
  tone(speaker, freq);

  Serial.println(freq);
}
