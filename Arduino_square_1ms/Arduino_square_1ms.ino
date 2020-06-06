void setup() {
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(13, HIGH); // sets the digital pin 13 on
  delay(1);               // waits for a second
  digitalWrite(13, LOW);  // sets the digital pin 13 off
  delay(1);               // waits for a second
}
