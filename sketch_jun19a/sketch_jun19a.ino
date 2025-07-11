#include <WiFi.h>           // Library to manage WiFi connection on ESP32
#include <HTTPClient.h>     // Library to perform HTTP requests

// Pin definitions for sensors and actuators
#define TRIG_PIN 5          // Trigger pin for ultrasonic sensor
#define ECHO_PIN 18         // Echo pin for ultrasonic sensor
#define TDS_PIN 34          // Analog input pin for TDS sensor
#define LED_ROJO_PIN 23     // Pin for the red indicator LED

// WiFi and server configuration
const char* ssid = "iPhone de Rod Aguilar"; // WiFi network name
const char* password = "123456789";         // WiFi password
const char* serverName = "https://cuddly-islands-repeat.loca.lt/api/v1/water-monitoring/data-records"; // Server URL to send data
const char* apiKey = "test";                // API key for authentication

// Variables to store last readings
long lastRawDuration = -1;                  // Last measured duration from the ultrasonic sensor
int lastRawTDS = -1;                        // Last TDS value read
const long RAW_MOVEMENT_THRESHOLD = 14;     // Threshold to detect significant distance changes
const int TDS_MOVEMENT_THRESHOLD = 10;      // Threshold to detect significant TDS changes

void setup() {
  Serial.begin(115200);                     // Initialize serial communication
  pinMode(TRIG_PIN, OUTPUT);                // Set TRIG pin as output
  pinMode(ECHO_PIN, INPUT);                 // Set ECHO pin as input
  pinMode(LED_ROJO_PIN, OUTPUT);            // Set LED pin as output
  digitalWrite(LED_ROJO_PIN, LOW);          // Turn off LED at start

  WiFi.begin(ssid, password, 6);            // Try to connect to WiFi, channel 6

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {   // Wait until connection is established
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  long raw_duration = getRawDuration();     // Get the ultrasonic pulse duration
  int raw_tds = analogRead(TDS_PIN);        // Read the analog value from the TDS sensor

  Serial.print("Raw duration (us): ");
  Serial.println(raw_duration);
  Serial.print("Raw TDS: ");
  Serial.println(raw_tds);

  // Detect if there are significant changes in distance or TDS
  bool distanceChanged = (lastRawDuration < 0 || abs(raw_duration - lastRawDuration) > RAW_MOVEMENT_THRESHOLD);
  bool tdsChanged = (lastRawTDS < 0 || abs(raw_tds - lastRawTDS) > TDS_MOVEMENT_THRESHOLD);

  if (distanceChanged || tdsChanged) {
    sendData(raw_tds, raw_duration);        // Send data to server if there are changes
    lastRawDuration = raw_duration;         // Update last values
    lastRawTDS = raw_tds;
  }

  delay(1000);                             // Wait 1 second before next reading
}

// Function to measure the ultrasonic pulse duration (distance)
long getRawDuration() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Wait up to 30 ms for the pulse
  return duration;
}

// Function to send data to the server via HTTP POST
void sendData(int raw_tds, long raw_distance) {
  if (WiFi.status() == WL_CONNECTED) {     // Verify WiFi connection
    HTTPClient http;
    http.begin(serverName);                // Start connection to server
    http.addHeader("Content-Type", "application/json"); // Specify content type
    http.addHeader("X-API-Key", apiKey);  // Add API key header

    // Build JSON with data to send
    String json = "{\"device_id\":\"esp32-01\",\"raw_tds\":";
    json += String(raw_tds);
    json += ",\"raw_distance\":";
    json += String(raw_distance);
    json += "}";

    Serial.print("Sending JSON: ");
    Serial.println(json);

    int httpResponseCode = http.POST(json); // Send POST request

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    String payload = http.getString();      // Get server response
    Serial.println("Server response:");
    Serial.println(payload);

    // If response is not "No significant event...", briefly turn on the red LED
    if (payload.indexOf("No significant event, no POST sent.") == -1) {
      digitalWrite(LED_ROJO_PIN, HIGH);
      delay(300); 
      digitalWrite(LED_ROJO_PIN, LOW); 
    }

    http.end();                            // End HTTP connection
  } else {
    Serial.println("WiFi not connected!");
  }
}
