#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Hochy_dodomu";
const char* password = "12345678";  

const uint8_t trigPin = 5;
const uint8_t echoPin = 18;
const float speedVoice = 0.0343;

enum SensorState : uint8_t {
  IDLE,
  TRIG_HIGH,
  WAIT_ECHO
};

SensorState state = IDLE;

uint32_t lastMeasureTime = 0;
uint32_t trigStartTime = 0;
const uint32_t trigPulseLength = 15;   
const uint32_t measureInterval = 500; 

WebServer server(80);

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>Distance Monitor</title>
    <style>
      body { font-family: sans-serif; text-align: center; padding-top: 50px; }
      h1 { font-size: 2.5em; }
      #value { font-size: 2em; color: #007BFF; }
    </style>
    <script>
      async function updateDistance() {
        try {
          const res = await fetch('/distance');
          const text = await res.text();
          document.getElementById('value').textContent = text + " cm";
        } catch (e) {
          document.getElementById('value').textContent = "Error";
        }
      }
      setInterval(updateDistance, 1000);
      window.onload = updateDistance;
    </script>
  </head>
  <body>
    <h1>Ultrasonic Distance</h1>
    <div id="value">--</div>
  </body>
</html>
)rawliteral";

float lastDistance = 0;

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);

  WiFi.softAP(ssid, password);
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP()); 


  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/distance", []() {
    server.send(200, "text/plain", String(lastDistance));
  });

  server.begin();
}

void loop() {
  server.handleClient();

  static uint32_t nowMillis = millis();
  static uint32_t nowMicros = micros();

  switch (state) {
    case IDLE:
      if (nowMillis - lastMeasureTime >= measureInterval) {
        lastMeasureTime = nowMillis;
        digitalWrite(trigPin, HIGH);
        trigStartTime = nowMicros;
        state = TRIG_HIGH;
      }
      break;

    case TRIG_HIGH:
      if (nowMicros - trigStartTime >= trigPulseLength) {
        digitalWrite(trigPin, LOW);
        state = WAIT_ECHO;
      }
      break;

    case WAIT_ECHO: {
      uint32_t duration = pulseIn(echoPin, HIGH, 10000);  
      if (duration > 0) {
        lastDistance = duration * speedVoice / 2.0;
        Serial.print("Distance: ");
        Serial.print(lastDistance);
        Serial.println(" cm");
      } else {
        Serial.println("Error: no echo");
        lastDistance = -1;
      }
      state = IDLE;
      break;
    }
  }
}
