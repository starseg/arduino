int red_light = 13;
int yellow_light = 12;
int green_light = 11;

int button = 7;

int pedestrian_red_light = 3;
int pedestrian_green_light = 2;

int traffic_light;
int button_state;
int button_previous_state;

int blink_time;
int blink_state;

void setup() {
  pinMode(red_light, OUTPUT);
  pinMode(yellow_light, OUTPUT);
  pinMode(green_light, OUTPUT);
  pinMode(button, INPUT);
  pinMode(pedestrian_red_light, OUTPUT);
  pinMode(pedestrian_green_light, OUTPUT);

  traffic_light = 1;
  button_previous_state = digitalRead(button);

  blink_time = 0;
  blink_state = HIGH;
}

void loop() {

  button_state = digitalRead(button);
  if(button_state == LOW && button_previous_state == HIGH) { // só quando soltar o botão
    if(traffic_light < 4) {
      traffic_light = traffic_light + 1;
    } else {
      traffic_light = 1;
    }
  }
  button_previous_state = button_state;

  if(traffic_light == 1) { // semáforo verde
    digitalWrite(green_light, HIGH);
    digitalWrite(yellow_light, LOW);
    digitalWrite(red_light, LOW);
    
    digitalWrite(pedestrian_red_light, HIGH);
    digitalWrite(pedestrian_green_light, LOW);
  }
  if(traffic_light == 2) { // semáforo amarelo
    digitalWrite(green_light, LOW);
    digitalWrite(yellow_light, HIGH);
    digitalWrite(red_light, LOW);
    
    digitalWrite(pedestrian_red_light, HIGH);
    digitalWrite(pedestrian_green_light, LOW);
  }
  if(traffic_light == 3) {  // semáforo vermelho
    digitalWrite(green_light, LOW);
    digitalWrite(yellow_light, LOW);
    digitalWrite(red_light, HIGH);
    
    digitalWrite(pedestrian_red_light, LOW);
    digitalWrite(pedestrian_green_light, HIGH);
  }
  
  if(traffic_light == 4) {  // semáforo pedestre piscando
    digitalWrite(green_light, LOW);
    digitalWrite(yellow_light, LOW);
    digitalWrite(red_light, HIGH);

    blink_time += 1;
    if(blink_time == 400) {
      blink_state = !blink_state;
      blink_time = 0;
    }
    
    digitalWrite(pedestrian_red_light, blink_state);
    digitalWrite(pedestrian_green_light, LOW);
  }
  delay(1);
}
