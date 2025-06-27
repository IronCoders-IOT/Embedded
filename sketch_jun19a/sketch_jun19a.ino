#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "KISI WIFI-1";
const char* password = "09724426";
const char* serverName = "http://IPPC:5001/api/v1/water-monitoring/data-records";

const int trigPin = 5;     // D5 for HC-SR04 TRIG
const int echoPin = 18;    // D18 for HC-SR04 ECHO (use voltage divider)
const int ledPin  = 2;     // Built-in LED of ESP32
const int tdsPin = 34;     // Analog pin for TDS sensor (A0 or D34)

String lastQualityText = "";  // Variable to store the last quality sent

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Analog read test on pin 34 (TDS)");

  analogSetPinAttenuation(tdsPin, ADC_11db); 

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
  Serial.print("📡 Assigned IP: ");
  Serial.println(WiFi.localIP());

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.println("🚀 Starting sensors...");
}

String getQualityText(float tds) {
  if (tds <= 100) return "Excellent";
  else if (tds <= 300) return "Good";
  else if (tds <= 600) return "Fair";
  else if (tds <= 900) return "Poor";
  else if (tds <= 1200) return "Not drinkable";
  else return "Contaminated water";
}

void loop() {
  int tdsValue = analogRead(tdsPin);
  Serial.println("🔌 Analog value read on pin 34 (TDS): " + String(tdsValue));

  // Distance sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  float distance = 0.0;
  if (duration == 0) {
    Serial.println("❌ No signal detected from HC-SR04.");
    digitalWrite(ledPin, LOW);
  } else {
    distance = duration * 0.034 / 2;
    Serial.print("✅ Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    digitalWrite(ledPin, (distance < 10) ? HIGH : LOW);
  }

  // TDS calculation
  float voltage = tdsValue * (3.3 / 4095.0);
  Serial.print("💧 Voltage: ");
  Serial.print(voltage, 3);

  float tds = (133.42 * voltage * voltage * voltage) 
            - (255.86 * voltage * voltage) 
            + (857.39 * voltage);

  String qualityText = getQualityText(tds);
  Serial.print(" | TDS: ");
  Serial.print(tds, 2);
  Serial.print(" ppm → Quality: ");
  Serial.println(qualityText);

  // Only send if quality has changed
  if (qualityText != lastQualityText) {
    if (WiFi.status() == WL_CONNECTED) {
      sendDataToServer(distance, tds, qualityText);
      lastQualityText = qualityText; // Update the last value sent
    } else {
      Serial.println("⚠️ WiFi not connected.");
    }
  } else {
    Serial.println("Water quality unchanged, POST not sent.");
  }

  delay(5000);
}

void sendDataToServer(float distance, float tds, String qualityText) {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", "test-api-key-123");

  StaticJsonDocument<256> doc;
  doc["device_id"] = "esp32-01"; 
  doc["bpm"] = 0;
  doc["eventType"] = "water-measurement";
  doc["qualityValue"] = qualityText;
  doc["levelValue"] = distance;
  doc["sensorId"] = 1;

  String requestBody;
  serializeJson(doc, requestBody);

  Serial.println("📤 Sending data: " + requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("✅ HTTP Code: ");
    Serial.println(httpResponseCode);
    Serial.print("📨 Response: ");
    Serial.println(response);
  } else {
    Serial.print("❌ Request error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
