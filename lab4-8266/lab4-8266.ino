#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
}

const int ledPin = 2;
const String encryptionKey = "secret"; // 🔐 той самий ключ

typedef struct struct_message {
  char msg[32];
} struct_message;

String xorEncryptDecrypt(String data, String key) {
  String output = "";
  for (int i = 0; i < data.length(); i++) {
    output += char(data[i] ^ key[i % key.length()]);
  }
  return output;
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  struct_message myData;
  memcpy(&myData, incomingData, sizeof(myData));

  String encryptedMsg = String(myData.msg);
  String decryptedMsg = xorEncryptDecrypt(encryptedMsg, encryptionKey);

  Serial.println("Отримано (зашифр.): " + encryptedMsg);
  Serial.println("Розшифровка: " + decryptedMsg);

  if (decryptedMsg == "LED_ON") {
    digitalWrite(ledPin, LOW);
    delay(500);
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("Повідомлення: " + decryptedMsg);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("MAC ESP8266: " + WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.println("Помилка ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  uint8_t peerAddress[] = {0x3C, 0x8A, 0x1F, 0xA4, 0x3B, 0x00}; // MAC ESP32
  esp_now_add_peer(peerAddress, ESP_NOW_ROLE_CONTROLLER, 0, NULL, 0);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {

}
