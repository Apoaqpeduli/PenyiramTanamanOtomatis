#define pinSensor A0
void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(analogRead(pinSensor));
  delay(500);
}
