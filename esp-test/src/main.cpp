// Firmware Seeed XIAO ESP32-C3: miganie zewnętrzną diodą LED.
// XIAO ESP32-C3 nie ma wbudowanej diody sterowanej GPIO.
// Podłącz: GPIO2 (D0) → rezystor 220Ω → anoda LED → katoda → GND.

#include <Arduino.h>

#define LED_PIN 2

void setup() {
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
}
