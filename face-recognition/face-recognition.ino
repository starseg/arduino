#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include "esp_http_server.h"

// WiFi Configurations
const char* ssid = "starsegescritorio";
const char* password = "esc232469";
const char* apssid = "esp32-cam";
const char* appassword = "12345678";

// GPIO Definitions for Camera
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

httpd_handle_t stream_httpd = NULL;
httpd_handle_t photo_httpd = NULL;

const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 Camera</title>
      <style>
          body {
              font-family: Arial, sans-serif;
              text-align: center;
              padding: 20px;
          }
          h1 {
              color: #333;
          }
          #stream {
              width: 640px;
              height: 480px;
              margin: 20px 0;
          }
          button {
              padding: 10px 20px;
              font-size: 18px;
              margin: 10px;
              cursor: pointer;
          }
      </style>
    </head>
    <body>
      <h1>ESP32 Camera Stream</h1>
      <img id="stream" src="/stream" alt="Camera Stream">
      <br>
      <button onclick="captureImage()">Capturar Imagem</button>
      <script>
          function captureImage() {
              fetch('/capture')
              .then(response => response.blob())
              .then(blob => {
                  const imageUrl = URL.createObjectURL(blob);
                  const imageWindow = window.open(imageUrl);
              })
              .catch(err => {
                  console.log('Error capturing image: ', err);
              });
          }
      </script>
    </body>
  </html>
)rawliteral";

esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

esp_err_t stream_handler(httpd_req_t *req) {
  WiFiClient client = req->client();

  // Open the camera
  camera_fb_t *fb = NULL;
  String response;

  while (1) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      httpd_resp_send_500(req);
      return ESP_OK;
    }

    // Send the image in multipart format
    response = "--123456789000000000000987654321\r\n";
    response += "Content-Type: image/jpeg\r\n";
    response += "Content-Length: " + String(fb->len) + "\r\n\r\n";
    httpd_resp_send_chunk(req, response.c_str(), response.length());
    httpd_resp_send_chunk(req, (const char*) fb->buf, fb->len);
    httpd_resp_send_chunk(req, "\r\n", 2);

    esp_camera_fb_return(fb);
    delay(30);  // Streaming interval
  }

  return ESP_OK;
}

esp_err_t capture_photo_handler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    httpd_resp_send_500(req);
    return ESP_OK;
  }

  // Save the photo to file (optional)
  File file = SD_MMC.open("/photo.jpg", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    httpd_resp_send_500(req);
    return ESP_OK;
  }
  file.write(fb->buf, fb->len);
  file.close();

  // Send the image as a response
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_send(req, (const char*) fb->buf, fb->len);

  esp_camera_fb_return(fb);
  return ESP_OK;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  httpd_uri_t capture_uri = {
    .uri = "/capture",
    .method = HTTP_GET,
    .handler = capture_photo_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&index_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(index_httpd, &index_uri);
  }

  // Start the HTTP server for streaming
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }

  // Start the HTTP server for capturing photos
  if (httpd_start(&photo_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(photo_httpd, &capture_uri);
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize the camera
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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  // Set up WiFi
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  Serial.println("Starting Camera Server...");
  startCameraServer();
}

void loop() {
  // Nothing needed here, the server is handled by HTTPD
}
