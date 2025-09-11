/*
  Simple Document Scanner - ESP32 Controller

  This simplified sketch handles just LED and buzzer control:
  - Turn on LED when document is detected
  - Turn on LED + buzzer when document is saved
  - Turn off LED when document is lost

  Hardware Required:
  - ESP32 Dev Board
  - LED (any color)
  - Buzzer/Speaker
  - 220Ω resistor for LED

  Author: Bakhtiyar  
  Date: September 2025
*/

// Pin Definitions (change according to your wiring)
#define LED_PIN     2      // Built-in LED or external LED
#define BUZZER_PIN  4      // Buzzer pin

// State variables
bool documentDetected = false;
bool ledState = false;
unsigned long lastBuzzTime = 0;
int docCount = 0;

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Turn off everything initially
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Startup indication
  Serial.println("=== Simple Document Scanner Controller ===");
  Serial.println("ESP32 ready! Waiting for scanner commands...");

  // Brief startup flash
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }

  Serial.println("Commands:");
  Serial.println("- DOC_DETECTED: Turn on LED");
  Serial.println("- DOC_SAVED: LED + Buzzer");
  Serial.println("- DOC_LOST: Turn off LED");
}

void loop() {
  // Check for serial commands from the document scanner
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    handleCommand(command);
  }

  // Small delay to prevent overwhelming the processor
  delay(10);
}

void handleCommand(String command) {
  Serial.println("Received: " + command);

  if (command == "DOC_DETECTED") {
    // Document detected - turn on LED
    documentDetected = true;
    digitalWrite(LED_PIN, HIGH);
    ledState = true;

    Serial.println("LED ON - Document detected!");

  } else if (command == "DOC_SAVED") {
    // Document saved - LED + buzzer
    docCount++;
    digitalWrite(LED_PIN, HIGH);
    ledState = true;

    // Sound buzzer for success
    playSuccessSound();

    Serial.println("LED + BUZZER - Document saved! (Total: " + String(docCount) + ")");

  } else if (command == "DOC_LOST") {
    // Document lost - turn off LED
    documentDetected = false;
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    ledState = false;

    Serial.println("LED OFF - Document lost");

  } else if (command == "SCANNER_OFF") {
    // Scanner shutting down - turn off everything
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    ledState = false;
    documentDetected = false;

    Serial.println("Scanner OFF - Session ended");
    Serial.println("Total documents scanned: " + String(docCount));

  } else {
    Serial.println("Unknown command: " + command);
  }
}

void playSuccessSound() {
  // Simple success beep sequence
  for(int i = 0; i < 3; i++) {
    // High pitch beep
    tone(BUZZER_PIN, 1000, 200);
    delay(250);

    // Short pause between beeps
    noTone(BUZZER_PIN);
    delay(100);
  }

  // Final longer beep
  tone(BUZZER_PIN, 1500, 500);
  delay(600);
  noTone(BUZZER_PIN);

  lastBuzzTime = millis();
}

void playDetectionSound() {
  // Simple detection beep (optional)
  tone(BUZZER_PIN, 800, 150);
  delay(200);
  noTone(BUZZER_PIN);
}

// Optional: Status LED blink pattern when idle
void statusBlink() {
  static unsigned long lastBlink = 0;
  static bool blinkState = false;

  if (millis() - lastBlink > 2000 && !documentDetected) {
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState ? HIGH : LOW);
    lastBlink = millis();
  }
}