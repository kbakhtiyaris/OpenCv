#include <WiFi.h>            // ESP32 Wi-Fi stack [21]
#include <HTTPClient.h>      // HTTP client for GET/POST [21]
#include <Arduino.h>         // Core definitions [21]

const char* ssid     = "TurkTelekom_T5236";            // Wi-Fi SSID [21]
const char* password = "9cnYauXH";        // Wi-Fi password [21]

const char* server   = "http://192.168.1.104:8080";   // Flask base URL on LAN [27]
const char* stateURL = "/api/desired_state";           // Endpoint path [14]

const int RELAY_PIN = 23;          // Choose a GPIO connected to relay IN [38]
String lastState = "init";         // Track last applied state [21]

void setup() {
  Serial.begin(115200);                         // Debug serial [21]
  pinMode(RELAY_PIN, OUTPUT);                   // Relay control pin [38]
  digitalWrite(RELAY_PIN, HIGH);                 // Start OFF (depends on relay active level) [38]

  WiFi.begin(ssid, password);                   // Start Wi-Fi [21]
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {       // Wait for connect [21]
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected, IP: " + WiFi.localIP().toString()); // Show IP [21]
}

void applyState(const String& desired) {
  if (desired == "on") {
    digitalWrite(RELAY_PIN, LOW);   // Turn fan ON (for active LOW relay)
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // Turn fan OFF
  }
}


void loop() {
  static unsigned long lastPoll = 0;            // Timer for polling [21]
  if (millis() - lastPoll >= 2000) {           // Poll every 2 seconds [21]
    lastPoll = millis();
    if (WiFi.status() == WL_CONNECTED) {        // Ensure Wi-Fi [21]
      WiFiClient client;                        // TCP client [21]
      HTTPClient http;                          // HTTP helper [21]
      String url = String(server) + stateURL;   // Build URL [21]
      http.begin(client, url);                  // Begin request [21]
      int code = http.GET();                    // Perform GET [21]
      if (code > 0) {                           // Success [21]
        String payload = http.getString();      // Body string [21]
        // Very simple parse: look for "on" or "off"
        String desired = payload.indexOf("\"on\"") >= 0 ? "on" : "off";  // Minimal JSON parse [21]
        if (desired != lastState) {             // Only switch on change [21]
          applyState(desired);                  // Set relay [38]
          lastState = desired;                  // Update cache [21]
          Serial.printf("State -> %s\n", desired.c_str()); // Log [21]
        }
      } else {
        Serial.printf("HTTP error: %d\n", code); // Error code [21]
      }
      http.end();                               // Free resources [21]
    } else {
      Serial.println("WiFi disconnected");      // Network issue [21]
    }
  }
}
