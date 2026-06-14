// Firmware Seeed XIAO ESP32-C3: miganie wbudowaną diodą LED.
// Wbudowana dioda użytkownika jest na GPIO 10 i działa w logice active-low
// (HIGH = zgaszona, LOW = zapalona).

#include <Arduino.h>

#define LED_PIN 10

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // zgaszona na start
}

void loop() {
    digitalWrite(LED_PIN, LOW);  // zapal
    delay(500);
    digitalWrite(LED_PIN, HIGH); // zgaś
    delay(500);
}
