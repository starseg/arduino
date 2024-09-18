// ---------------------------------------------------------------------------
// Example NewPing library sketch that does a ping about 20 times per second.
// ---------------------------------------------------------------------------

#include <NewPing.h>

#define TRIGGER_PIN  11  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     12  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define led 7
#define buzzer 6

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.

  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
}

unsigned long tempoAnterior = 0; // Armazena o tempo da última mudança de estado do LED
const long intervalo = 500; // Intervalo de 1 segundo (1000 ms)
int estadoLed = LOW; // Armazena o estado atual do LED (LOW = apagado, HIGH = aceso)

void loop() {
  unsigned long tempoAtual = millis();
  delay(300);                     // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  Serial.print("Ping: ");
  Serial.print(sonar.ping_cm()); // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");
  if(sonar.ping_cm() < 30) {
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }
  if(sonar.ping_cm() < 10) {
    analogWrite(buzzer, 32);
  } else {
    analogWrite(buzzer, 0);
  }
}