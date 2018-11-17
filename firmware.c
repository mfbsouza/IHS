void setup() {
  Serial.begin(9600);
  pinMode(12, INPUT);
  pinMode(11, INPUT);
  pinMode(10, INPUT);
  pinMode(9, INPUT);
  pinMode(8, INPUT);
}

void loop() {
  if(digitalRead(12) > 0)
    Serial.print('q');
  if(digitalRead(11) > 0)
    Serial.print('d');
  if(digitalRead(10) > 0)
    Serial.print('c');
  if(digitalRead(9) > 0)
    Serial.print('b');
  if(digitalRead(8) > 0)
    Serial.print('a');
  delay(150);
}