#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "KISI WIFI-1";
const char* password = "09724426";
const char* serverName = "http://IPPC:5001/api/v1/water-monitoring/data-records";

const int trigPin = 5;     // D5 para HC-SR04 TRIG
const int echoPin = 18;    // D18 para HC-SR04 ECHO (usar divisor de voltaje)
const int ledPin  = 2;     // LED integrado del ESP32
const int tdsPin = 34;     // Pin analógico para el sensor TDS (A0 o D34)

String lastQualityText = "";  // Variable para guardar última calidad enviada

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Prueba de lectura analógica en pin 34 (TDS)");

  analogSetPinAttenuation(tdsPin, ADC_11db); 

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado!");
  Serial.print("📡 IP asignada: ");
  Serial.println(WiFi.localIP());

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.println("🚀 Iniciando sensores...");
}

String getQualityText(float tds) {
  if (tds <= 100) return "Excelente";
  else if (tds <= 300) return "Buena";
  else if (tds <= 600) return "Aceptable";
  else if (tds <= 900) return "Mala";
  else if (tds <= 1200) return "No potable";
  else return "Agua contaminada";
}

void loop() {
  int tdsValue = analogRead(tdsPin);
  Serial.println("🔌 Valor analógico leído en pin 34 (TDS): " + String(tdsValue));

  // Sensor de distancia
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  float distance = 0.0;
  if (duration == 0) {
    Serial.println("❌ No se detectó señal del HC-SR04.");
    digitalWrite(ledPin, LOW);
  } else {
    distance = duration * 0.034 / 2;
    Serial.print("✅ Distancia: ");
    Serial.print(distance);
    Serial.println(" cm");
    digitalWrite(ledPin, (distance < 10) ? HIGH : LOW);
  }

  // Cálculo TDS
  float voltage = tdsValue * (3.3 / 4095.0);
  Serial.print("💧 Voltaje: ");
  Serial.print(voltage, 3);

  float tds = (133.42 * voltage * voltage * voltage) 
            - (255.86 * voltage * voltage) 
            + (857.39 * voltage);

  String qualityText = getQualityText(tds);
  Serial.print(" | TDS: ");
  Serial.print(tds, 2);
  Serial.print(" ppm → Calidad: ");
  Serial.println(qualityText);

  // Solo enviar si la calidad ha cambiado
  if (qualityText != lastQualityText) {
    if (WiFi.status() == WL_CONNECTED) {
      sendDataToServer(distance, tds, qualityText);
      lastQualityText = qualityText; // Actualiza el último valor enviado
    } else {
      Serial.println("⚠️ WiFi no conectado.");
    }
  } else {
    Serial.println("La calidad del agua no ha cambiado, no se envía POST.");
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

  Serial.println("📤 Enviando datos: " + requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("✅ Código HTTP: ");
    Serial.println(httpResponseCode);
    Serial.print("📨 Respuesta: ");
    Serial.println(response);
  } else {
    Serial.print("❌ Error en la petición: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
