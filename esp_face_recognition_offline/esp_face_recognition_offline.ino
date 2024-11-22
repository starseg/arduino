#include "esp_camera.h"
#include "face_recognition_112_v1_s8.hpp"
#include "face_recognition_tool.hpp"
#include "fb_gfx.h"
#include "human_face_detect_mnp01.hpp"
#include "human_face_detect_msr01.hpp"
#include "ra_filter.h"
#include <vector>

//Configurações de pinos de camera da GOOUUU ESP32-S3-CAM
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5
#define Y9_GPIO_NUM 16
#define Y8_GPIO_NUM 17
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 12
#define Y5_GPIO_NUM 10
#define Y4_GPIO_NUM 8
#define Y3_GPIO_NUM 9
#define Y2_GPIO_NUM 11
#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13


#define ENROLL_BUTTON 47  // botão para cadastro de novas faces

#define BUZZER_PIN 21  // buzzer para indicar resposta

const size_t ESTIMATED_FACE_SIZE = 200 * 1024;  // tamanho estimado que um rosto ocupa na memória (200kb)
#define FR_CONFIDENCE_THRESHOLD 0.7             // fator minimo para o sistema aceitar o rosto (de 0 a 1)

camera_fb_t *fb = NULL;  // ponteiro do buffer da camera

FaceRecognition112V1S8 recognizer;  // instancia da classe de reconhecimento facial

// Usando duas etapas para verificação facial (mais lento porém preciso)
HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);

bool is_enrolling = false;  // 0: not enrolling, 1: enrolling

// função para reconhecer o rosto de um usuário
static int run_face_recognition(fb_data_t *fb, std::list<dl::detect::result_t> *results) {
  std::vector<int> landmarks = results->front().keypoint;

  Tensor<uint8_t> tensor;
  tensor.set_element((uint8_t *)fb->data).set_shape({ fb->height, fb->width, 3 }).set_auto_free(false);

  face_info_t recognize = recognizer.recognize(tensor, landmarks);

  if (recognize.id >= 0) {

    if (recognize.similarity >= FR_CONFIDENCE_THRESHOLD) {
      authorized(&recognize);
      return recognize.id;
    } else {
      run_face_recognition(fb, results);
    }
  
  } else {
    Serial.println("Não Autorizado!!");
    return -1;
  }

  return -1;
}

// função para cadastrar o rosto
static int enroll_face(fb_data_t *fb, std::list<dl::detect::result_t> *results) {

  Serial.println("Realizando o cadastro de uma face...");

  int enrolled_count = recognizer.get_enrolled_id_num();
  size_t free_memory = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
  size_t available_faces = free_memory / ESTIMATED_FACE_SIZE;

  std::vector<int> landmarks = results->front().keypoint;
  Tensor<uint8_t> tensor;
  tensor.set_element((uint8_t *)fb->data).set_shape({ fb->height, fb->width, 3 }).set_auto_free(false);

  if (enrolled_count >= available_faces) {
    Serial.println("Limite de rostos atingido com base na memória disponível.");
    is_enrolling = false;
    return -1;
  }

  // faz 3 tentativas de verificação antes de tentar o cadastro
  for (int i = 0; i <= 3; i++) {
    if (recognizer.recognize(tensor, landmarks).id > 0) {
      Serial.println("Rosto ja cadastrado");
      is_enrolling = false;
      return -1;
    }
  }


  if (is_enrolling) {
    int id = recognizer.enroll_id(tensor, landmarks, "", true);
    Serial.printf("Rosto cadastrado com o id: %d/n", id);
    is_enrolling = false;
    return id;
  }

  return -1;
}

// função que roda caso o usuário seja autorizado
void authorized(face_info_t *recognize) {
  Serial.printf("Recognized ID: %d", recognize->id);
  Serial.printf(" com similaridade de: %0.2f\n", recognize->similarity);

  analogWrite(BUZZER_PIN, 32);
  delay(500);
  analogWrite(BUZZER_PIN, 0);
}

// função para inicializar a camera
bool initCamera() {
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.frame_size = FRAMESIZE_240X240;
  config.pixel_format = PIXFORMAT_RGB565;
  config.fb_count = 2;

  // nicializa a camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("ERROR: Camera init failed with code 0x%x", err);
    return false;
  }

  return true;
}

// setup das variáveis do código
void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ENROLL_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);

  if (!initCamera()) {
    Serial.println("Camera init failed!");
  }

  ra_filter_init(&ra_filter, 20);

  recognizer.set_partition(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fr");
  recognizer.set_ids_from_flash();

  Serial.println("Camera de detecção facial");

  if (recognizer.get_enrolled_id_num() <= 0) {
    Serial.println("Nenhum rosto cadastrado na memória, pressione o botão e segure até que o LED acenda");
  }
}

// verificação dos comandos em looping
void loop() {
  if (digitalRead(ENROLL_BUTTON) == LOW) {
    is_enrolling = true;
    delay(1000);
  }


  fb = esp_camera_fb_get();

  if (!fb) {
    Serial.printf("ERROR: Camera capture failed\n");
  } else {

    std::list<dl::detect::result_t> &candidates = s1.infer((uint16_t *)fb->buf, { (int)fb->height, (int)fb->width, 3 });
    std::list<dl::detect::result_t> &results = s2.infer((uint16_t *)fb->buf, { (int)fb->height, (int)fb->width, 3 }, candidates);

    if (results.size() > 0) {
      int face_id = 0;

      size_t out_len, out_width, out_height;
      uint8_t *out_buf;
      bool s;

      out_len = fb->width * fb->height * 3;
      out_width = fb->width;
      out_height = fb->height;
      out_buf = (uint8_t *)malloc(out_len);
      if (!out_buf) {
        log_e("out_buf malloc failed");
      }

      //convert to rgb888 for better perfomances
      s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
      free(out_buf);

      if (!s) {
        free(out_buf);
        log_e("conversion to rgb888 failed");
      }

      esp_camera_fb_return(fb);

      fb_data_t rfb;
      rfb.width = out_width;
      rfb.height = out_height;
      rfb.data = out_buf;
      rfb.bytes_per_pixel = 3;
      rfb.format = FB_BGR888;

      // Se estiver no modo de cadastro, cadastra um rosto, se não verifica se existem rostos cadastrados para realizar o reconhecimento
      if (is_enrolling) {

        digitalWrite(LED_BUILTIN, HIGH);
        face_id = enroll_face(&rfb, &results);
        digitalWrite(LED_BUILTIN, LOW);

      } else if (recognizer.get_enrolled_id_num() > 0) {

        face_id = run_face_recognition(&rfb, &results);
        delay(1000);
      }
    } else {
      if (is_enrolling) {
        Serial.println("Rosto não encontrado");
        is_enrolling = false;
      }
    }
  }

  // Release the framebuffer
  esp_camera_fb_return(fb);
}
