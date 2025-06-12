#include <WiFi.h>
#include <esp_now.h>

const int ledButtonPin = 4;
const int helloButtonPin = 15;
const int milordButtonPin = 5;
const String encryptionKey = "secret"; // 🔐 ключ

uint8_t peerAddress[] = {0xE8, 0xDB, 0x84, 0x96, 0xE8, 0x5D};

typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message myData;

String xorEncryptDecrypt(String data, String key) {
  String output = "";
  for (int i = 0; i < data.length(); i++) {
    output += char(data[i] ^ key[i % key.length()]);
  }
  return output;
}

void sendEncryptedMessage(String plainText) {
  String encrypted = xorEncryptDecrypt(plainText, encryptionKey);
  encrypted.toCharArray(myData.msg, 32);
  esp_now_send(peerAddress, (uint8_t *)&myData, sizeof(myData));
  Serial.print("Надіслано (зашифровано): ");
  Serial.println(encrypted);
}

void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Надіслано успішно" : "Помилка надсилання");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledButtonPin, INPUT_PULLUP);
  pinMode(helloButtonPin, INPUT_PULLUP);
  pinMode(milordButtonPin, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("MAC ESP32: " + WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Помилка ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(peerAddress)) {
    esp_now_add_peer(&peerInfo);
  }
}

void loop() {
  if (digitalRead(ledButtonPin) == LOW) {
    delay(200);
    sendEncryptedMessage("LED_ON");
  }
  if (digitalRead(helloButtonPin) == LOW) {
    delay(200);
    sendEncryptedMessage("Привіт ESP");
  }
  if (digitalRead(milordButtonPin) == LOW) {
    delay(200);
    sendEncryptedMessage("Мілорд, казна пустеєт");
  }
}
