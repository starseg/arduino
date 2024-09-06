int pinLed[9] = {12,11,10,9,8,7,6,5,4};
int numeroLed;

void setup() {
  int x;
  for (x = 0; x < 9; x ++) {
    pinMode(pinLed[x], OUTPUT);
  }
}

void loop() {
  for (numeroLed = 0; numeroLed < 9; numeroLed++) {
    digitalWrite(pinLed[numeroLed], HIGH);
    delay(100);
  }
  for (numeroLed = 8; numeroLed >= 0; numeroLed--) {
    digitalWrite(pinLed[numeroLed], LOW);
    delay(300);
  }
}
