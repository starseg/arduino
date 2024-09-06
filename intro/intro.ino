
void setup() {
  // put your setup code here, to run once:
  pinMode(9, OUTPUT); // led vermelho
  pinMode(10, OUTPUT); // led azul
  pinMode(12, INPUT_PULLUP); //botão

}

void loop() {
  // put your main code here, to run repeatedly:
  // int interval = 1000;
  int button_state = digitalRead(12);

  digitalWrite(10, button_state);
  digitalWrite(9, !button_state);


  // digitalWrite(10, HIGH);
  // delay(interval);
  // digitalWrite(10, LOW);
  // digitalWrite(9, HIGH);
  // delay(interval);
  // digitalWrite(9, LOW);
}
