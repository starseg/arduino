int pinos[3] = {13,12,11};
int pinLED;
float valorLDR;

float min = 100;
float max = 1000;
float luminosidade;

void setup() {
  for (pinLED = 0; pinLED < 3; pinLED++) {
    pinMode(pinos[pinLED], OUTPUT);
  }
  pinMode(A0, INPUT);
}

void loop() {
  valorLDR = analogRead(A0);
  luminosidade = (valorLDR - min) / (max - min) * 3;
  for(pinLED = 0; pinLED < 3; pinLED++) {
    if(pinLED < luminosidade) {
      digitalWrite(pinos[pinLED], HIGH);
    } else {
      digitalWrite(pinos[pinLED], LOW);
    }
  }
  delay(1000);
}
