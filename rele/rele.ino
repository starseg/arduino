#define rele 7
#define button 6

void setup() {
  pinMode(rele, OUTPUT);
  pinMode(button, INPUT_PULLUP);
}

void loop() {
  int button_state = digitalRead(button);

  digitalWrite(rele, button_state);

}
