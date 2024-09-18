#include <Servo.h>  // Biblioteca para controle de servos

Servo servoMotor;    // Cria o objeto servoMotor

int pos = 0;         // Variável para armazenar a posição atual do servo

void setup() {
  servoMotor.attach(8);  // Associa o servo à porta 8
}

void loop() {
  // Girar de 0 até 180 graus (giro em passos de 15 graus)
  for (pos = 0; pos <= 360; pos += 15) {
    servoMotor.write(pos);  // Manda o servo ir para a posição 'pos'
    delay(500);             // Espera 500 milissegundos
  }

  // Voltar de 180 para 0 graus
  for (pos = 360; pos >= 0; pos -= 15) {
    servoMotor.write(pos);  // Manda o servo voltar para a posição 'pos'
    delay(500);             // Espera 500 milissegundos
  }
}
