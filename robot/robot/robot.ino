#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"             // disable brownout problems
#include "soc/rtc_cntl_reg.h"    // disable brownout problems
#include "esp_http_server.h"
#include <NewPing.h>
#include "SD_MMC.h"

// Replace with your network credentials
const char* ssid     = "starsegescritorio";
const char* password = "esc242369";

#define PART_BOUNDARY "123456789000000000000987654321"

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
  #define RED_LED_PIN       33
  #define LED_PIN            4
#else
  #error "Camera model not selected"
#endif


#define MOTOR_1_PIN_1    12
#define MOTOR_1_PIN_2    13
#define MOTOR_2_PIN_1    15
#define MOTOR_2_PIN_2    14
#define enablePin  2

// Echo e Trigger pins
#define TRIGGER_PIN  3  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     1  // Arduino pin tied to echo pin on the ultrasonic sensor.



// sonar instance
NewPing sonar(TRIGGER_PIN, ECHO_PIN, 100);
int MAX_DISTANCE = 20;

// Setting PWM properties
const int freq = 30000;
const int reso = 8;
int dutyCycle = 0;

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;


const char PROGMEM INDEX_HTML[] = R"rawliteral(

  <!DOCTYPE html>
  <html lang="pt-BR">
    <head>
      <meta charset="UTF-8" />
      <meta name="viewport" content="width=device-width, initial-scale=1" />
      <title>Robô vigia Star Seg</title>
      <style>
        :root {
          color-scheme: dark;
        }
        html {
          font-family: Helvetica;
          display: inline-block;
          margin: 0px auto;
          text-align: center;
        }
        .button {
          -webkit-user-select: none;
          -moz-user-select: none;
          -ms-user-select: none;
          user-select: none;
          outline: none;
          -webkit-tap-highlight-color: transparent;
          background-color: transparent;
          border: none;
          border-radius: 100px;
          color: #333;
          padding: 0.5rem;
          text-decoration: none;
          font-size: 64px;
          cursor: pointer;
          aspect-ratio: 1/1;
          display: flex;
          justify-content: center;
          align-items: center;
        }
        .button:active {
          color: #2c1704;
          scale: 1.2;
        }
        .joystick {
          width: 250px;
          height: 250px;
          border-radius: 1000px;
          background-color: #eab308;
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          margin-left: 3rem;
        }
        .side-arrows {
          display: flex;
          width: 100%;
          justify-content: space-between;
          align-items: center;
        }
        input[type="range"] {
          -webkit-appearance: none; /* Remove o estilo padrão no WebKit */
        }

        input[type="range"]::-webkit-slider-runnable-track {
          width: 100%;
          height: 8px;
          background: #a58525; /* Cor da barra */
          border-radius: 5px;
        }

        input[type="range"]::-webkit-slider-thumb {
          -webkit-appearance: none; /* Remove o estilo padrão */
          width: 20px;
          height: 20px;
          background: #eab308; /* Cor do thumb */
          border-radius: 50%;
          cursor: pointer;
          margin-top: -6px; /* Alinha o thumb com a barra */
        }

        input[type="range"]::-moz-range-track {
          background: #eab308; /* Cor da barra no Firefox */
          height: 8px;
          border-radius: 5px;
        }

        input[type="range"]::-moz-range-thumb {
          width: 20px;
          height: 20px;
          background: #eab308; /* Cor do thumb no Firefox */
          border-radius: 50%;
          cursor: pointer;
        }

        input[type="range"]::-ms-track {
          width: 100%;
          height: 8px;
          background: transparent; /* Remove a cor padrão no Edge */
          border-color: transparent;
          color: transparent;
        }

        input[type="range"]::-ms-fill-lower {
          background: #eab308; /* Cor da barra preenchida no Edge */
        }

        input[type="range"]::-ms-fill-upper {
          background: #ffbf00; /* Cor da barra restante no Edge */
        }

        input[type="range"]::-ms-thumb {
          width: 20px;
          height: 20px;
          background: #eab308; /* Cor do thumb no Edge */
          border-radius: 50%;
          cursor: pointer;
        }
        .rotate-message {
          display: none; /* Oculta a mensagem por padrão */
          position: fixed;
          top: 0;
          left: 0;
          width: 100%;
          height: 100%;
          background-color: rgb(0, 0, 0);
          color: white;
          display: flex;
          justify-content: center;
          align-items: center;
          font-size: 24px;
          text-align: center;
          z-index: 1000;
        }

        .container {
          display: flex;
          justify-content: space-around;
        }

        #photo {
          width: auto;
          max-width: 100%;
          height: 400px;
        }

      </style>
      <script src="https://unpkg.com/@phosphor-icons/web"></script>
      <script>
        function toggleCheckbox(x) {
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/action?go=" + x, true);
          xhr.send();
        }

        function updateMotorSpeed(pos) {
          document.getElementById('motorSpeed').innerHTML = pos;
          var xhr = new XMLHttpRequest()
          xhr.open("GET", "/speed?value=" + pos, true);
          xhr.send()
        }

        function updateLed(value) {
          document.getElementById('ledIntensity').innerHTML = value;
          var xhr = new XMLHttpRequest()
          xhr.open("GET", "/led?value=" + value, true);
          xhr.send()
        }
      </script>
    </head>
    <body>
      <h1>Robô vigia Star Seg</h1>
      <div class="rotate-message" id="rotateMessage">
        Por favor, vire o dispositivo para o modo paisagem.
      </div>
      <div class="container">
        <div class="joystick">
          <button
            class="button"
            onmousedown="toggleCheckbox('forward')"
            ontouchstart="toggleCheckbox('forward')"
            onmouseup="toggleCheckbox('stop')"
            ontouchend="toggleCheckbox('stop')"
          >
            <i class="ph-arrow-fat-up ph-fill ph"></i>
          </button>
          <div class="side-arrows">
            <button
              class="button"
              onmousedown="toggleCheckbox('left')"
              ontouchstart="toggleCheckbox('left')"
              onmouseup="toggleCheckbox('stop')"
              ontouchend="toggleCheckbox('stop')"
            >
              <i class="ph-arrow-fat-left ph-fill ph"></i>
            </button>
            <button
              class="button"
              onmousedown="toggleCheckbox('right')"
              ontouchstart="toggleCheckbox('right')"
              onmouseup="toggleCheckbox('stop')"
              ontouchend="toggleCheckbox('stop')"
            >
              <i class="ph-arrow-fat-right ph-fill ph"></i>
            </button>
          </div>
          <button
            class="button"
            onmousedown="toggleCheckbox('backward')"
            ontouchstart="toggleCheckbox('backward')"
            onmouseup="toggleCheckbox('stop')"
            ontouchend="toggleCheckbox('stop')"
          >
            <i class="ph-arrow-fat-down ph-fill ph"></i>
          </button>
        </div>
        <img src="" id="photo" />
      </div>

      <p>Velocidade: <span id="motorSpeed">20</span>%</p>
      <input
        type="range"
        min="0"
        max="100"
        step="10"
        id="motorSlider"
        oninput="updateMotorSpeed(this.value)"
        value="20"
      />

      <p>Iluminação do Led: <span id="ledIntensity">0</span>%</p>
      <input
        type="range"
        min="0"
        max="100"
        step="10"
        id="ledSlider"
        oninput="updateLed(this.value)"
        value="0"
      />
      <script>
        function checkOrientation() {
          if (window.innerWidth < window.innerHeight) {
            document.getElementById("rotateMessage").style.display = "flex";
          } else {
            document.getElementById("rotateMessage").style.display = "none";
          }
        }

        window.addEventListener("load", checkOrientation);
        window.addEventListener("resize", checkOrientation);
        window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
      </script>
    </body>
  </html>

)rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  char variable[32] = {0,};
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", variable, sizeof(variable)) == ESP_OK) {
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  sensor_t * s = esp_camera_sensor_get();
  int res = 0;

  int distancia = sonar.ping_cm();
  
  if(!strcmp(variable, "forward")) {
    Serial.println("Forward");

    if (distancia < MAX_DISTANCE && distancia > 0) {
      digitalWrite(MOTOR_1_PIN_2, 0);  
      digitalWrite(MOTOR_2_PIN_2, 0); 
    } else {
      digitalWrite(MOTOR_1_PIN_1, 1);
      digitalWrite(MOTOR_1_PIN_2, 0);
      digitalWrite(MOTOR_2_PIN_1, 0);
      digitalWrite(MOTOR_2_PIN_2, 1);
    }

  }
  else if(!strcmp(variable, "left")) {
    Serial.println("Left");

    if (distancia < MAX_DISTANCE && distancia > 0) {
      digitalWrite(MOTOR_1_PIN_2, 0);  
      digitalWrite(MOTOR_2_PIN_2, 0); 
    } else {
      digitalWrite(MOTOR_1_PIN_1, 0);
      digitalWrite(MOTOR_1_PIN_2, 1);
      digitalWrite(MOTOR_2_PIN_1, 0);
      digitalWrite(MOTOR_2_PIN_2, 1);
    }

  }
  else if(!strcmp(variable, "right")) {
    Serial.println("Right");

    if (distancia < MAX_DISTANCE && distancia > 0) {
      digitalWrite(MOTOR_1_PIN_2, 0);  
      digitalWrite(MOTOR_2_PIN_2, 0); 
    } else {
      digitalWrite(MOTOR_1_PIN_1, 1);
      digitalWrite(MOTOR_1_PIN_2, 0);
      digitalWrite(MOTOR_2_PIN_1, 1);
      digitalWrite(MOTOR_2_PIN_2, 0);
    }

  }
  else if(!strcmp(variable, "backward")) {
    Serial.println("Backward");
    digitalWrite(MOTOR_1_PIN_1, 0);
    digitalWrite(MOTOR_1_PIN_2, 1);
    digitalWrite(MOTOR_2_PIN_1, 1);
    digitalWrite(MOTOR_2_PIN_2, 0);
  }

  else if(!strcmp(variable, "stop")) {
    Serial.println("Stop");
    digitalWrite(MOTOR_1_PIN_1, 0);
    digitalWrite(MOTOR_1_PIN_2, 0);
    digitalWrite(MOTOR_2_PIN_1, 0);
    digitalWrite(MOTOR_2_PIN_2, 0);
  }
  else {
    res = -1;
  }

  if(res){
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

static esp_err_t speed_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  int value;
  char value_str[32] = {0,};
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "value", value_str, sizeof(value_str)) == ESP_OK) {
        value = atoi(value_str);
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
    }
  }

  sensor_t * s = esp_camera_sensor_get();
  int res = 0;
  
  Serial.println("Velocidade é: " + String(value));
  if (value == 0) {
    ledcWrite(enablePin, 0);
    digitalWrite(MOTOR_1_PIN_1, 0);
    digitalWrite(MOTOR_1_PIN_2, 0);
    digitalWrite(MOTOR_2_PIN_1, 0);
    digitalWrite(MOTOR_2_PIN_2, 0); 
  } else { 
    dutyCycle = map(value, 20, 100, 200, 255);
    ledcWrite(enablePin, dutyCycle);
    Serial.println("Motor speed set to " + String(value));
  }  

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

static esp_err_t led_handler(httpd_req_t *req){
  char*  buf;
  size_t buf_len;
  int value;
  char value_str[32] = {0,};
  
  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char*)malloc(buf_len);
    if(!buf){
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "value", value_str, sizeof(value_str)) == ESP_OK) {
        value = atoi(value_str);
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
    }
  }

  sensor_t * s = esp_camera_sensor_get();
  int res = 0;
  
  Serial.println("A intensidade do led é: " + String(value));
  
  if (value == 0) {
    ledcWrite(LED_PIN, 0);
  } else { 
    dutyCycle = map(value, 20, 100, 20, 200);
    ledcWrite(LED_PIN, dutyCycle);
    Serial.println("Led intensity set to " + String(value));
  }  

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri       = "/action",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t stream_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t speed_uri = {
      .uri       = "/speed",
      .method    = HTTP_GET,
      .handler   = speed_handler,
      .user_ctx  = NULL
  };

  httpd_uri_t led_uri = {
      .uri       = "/led",
      .method    = HTTP_GET,
      .handler   = led_handler,
      .user_ctx  = NULL
  };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    httpd_register_uri_handler(camera_httpd, &speed_uri);
    httpd_register_uri_handler(camera_httpd, &led_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

unsigned long pingTimer;
unsigned long pingSpeed = 50;

void setup() {
   Serial.begin(115200);

  // Set the Motor pins as outputs
  pinMode(MOTOR_1_PIN_1, OUTPUT);
  pinMode(MOTOR_1_PIN_2, OUTPUT);
  pinMode(MOTOR_2_PIN_1, OUTPUT);
  pinMode(MOTOR_2_PIN_2, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  // Configure PWM Pins
  ledcAttach(enablePin, freq, reso);
  ledcAttach(LED_PIN, freq, reso);
    
  // Initialize PWM with duty cycle
  ledcWrite(enablePin, 200);
  ledcWrite(LED_PIN, 0);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  startCameraServer();

  pingTimer = millis();
}

void verifyDistance() {
  int distancia = sonar.ping_cm();

  //Serial.print("Ping: ");
  //Serial.print(distancia);
  //Serial.println("cm");

  if (distancia < MAX_DISTANCE && distancia > 0) {
    ledcWrite(LED_PIN, 150);
    digitalWrite(MOTOR_1_PIN_2, 0);  
    digitalWrite(MOTOR_2_PIN_2, 0); 
  }
}

void loop() {
  if (millis() >= pingTimer) {   
     pingTimer += pingSpeed; 
     verifyDistance();
  }
}