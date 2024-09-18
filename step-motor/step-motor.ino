#include <Stepper.h>

const int stepsPerRevolution = 2048;  // Número de passos por revolução do motor
#define button 2

Stepper myStepper(stepsPerRevolution, 6, 7, 8, 9);

void setup() {
  myStepper.setSpeed(10);  // Define a velocidade do motor (em rotações por minuto)
  pinMode(button, INPUT_PULLUP);
}

void loop() {
  int stepsToMove = 90 * (stepsPerRevolution / 360);  // Calcula o número de passos para 90 graus
  if(digitalRead(button) == LOW) {
    
    myStepper.step(-1);
  } else {
    myStepper.step(0);
  }
}
