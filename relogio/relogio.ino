/*
 * Q0000
 * AUTOR:   BrincandoComIdeias
 * LINK:    https://www.youtube.com/brincandocomideias ; https://cursodearduino.net/
 * SKETCH:  MOTOR PASSO 
 * DATA:    04/10/2018
 */
 
#include <Stepper.h>
#include <PushButton.h>

#define pinBotao1 4
#define pinBotao2 12

const int stepsPerRevolution = 2050;  // VALOR DE PASSOS PARA UMA VOLTA

// INSTANCIANDO O MOTOR DE PASSO E CONFIGURANDO OS PINOS
Stepper myStepper(stepsPerRevolution, 6, 7, 8, 9);

// INSTANCIANDO BOTÕES
PushButton botao1(pinBotao1);
PushButton botao2(pinBotao2);

void setup() {
  // DEFININDO ROTAÇÕES POR MINUTO, NOS TESTES A VELOCIDADE MÁXIMA DO MEU MOTOR 28BYJ-48 FOI DE 7 RPM
  myStepper.setSpeed(1);
  // INCIANDO MONITOR SERIAL
  Serial.begin(9600);
}

void loop() {
  // VARIÁVEIS LOCAIS
  static byte numeroHoras = 0;

  // MÉTODO PARA LER ESTADO DO BOTAO
  botao1.button_loop();botao2.button_loop();

  // AÇÕES PARA CADA CONDIÇÃO
  if(botao1.pressed() == true && numeroHoras == 0){
    Serial.println("Girando 1 Revolução");
    myStepper.step(stepsPerRevolution); 
  } else if ( botao1.pressed() == true && numeroHoras != 0){
    Serial.print("Girando ");Serial.print(numeroHoras); Serial.println(" Horas");
    myStepper.step( numeroHoras * (stepsPerRevolution / 12) );
  }

  if (botao2.pressed()){
    numeroHoras++;
    if (numeroHoras > 11) {
       numeroHoras = 0;
    }
    
    Serial.print("Configurado para: ");Serial.print(numeroHoras); Serial.println(" Horas");
  }
}

