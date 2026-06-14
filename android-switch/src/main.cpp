// Firmware Seeed XIAO ESP32-C3: sterowanie zewnętrzną diodą LED przez MQTT.
// Łączy się z WiFi i brokerem MQTT, nasłuchuje komend ON/OFF na topic mgil/esp32c3/led/command,
// włącza/wyłącza LED i publikuje aktualny stan na mgil/esp32c3/led/state (retain).
// Połączenie WiFi i MQTT jest automatycznie odtwarzane po zerwaniu.
// Publikuje status online/offline przez MQTT LWT na topic mgil/esp32c3/led/status.
// Wersja: 2026-06-15 00:20

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
bool ledState = false;

void publishState() {
    mqtt.publish(TOPIC_STATE, ledState ? "ON" : "OFF", true);
}

void onMessage(char* topic, byte* payload, unsigned int len) {
    String msg;
    for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];

    if (msg == "ON") {
        ledState = true;
        digitalWrite(LED_PIN, HIGH);
    } else if (msg == "OFF") {
        ledState = false;
        digitalWrite(LED_PIN, LOW);
    }
    publishState();
    Serial.printf("LED: %s\n", ledState ? "ON" : "OFF");
}

void connectWifi() {
    if (WiFi.status() == WL_CONNECTED) return;
    Serial.print("WiFi");
    // Czekamy 10s na autoReconnect zanim wymuśimy pełny restart
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(" OK (auto) — " + WiFi.localIP().toString());
        return;
    }
    // AutoReconnect nie zadziałał — pełny restart stosu WiFi
    Serial.println(" timeout, restart WiFi...");
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" OK — " + WiFi.localIP().toString());
}

void connectMqtt() {
    while (!mqtt.connected()) {
        Serial.print("MQTT...");
        // LWT: broker opublikuje "offline" gdy ESP32 zerwie połączenie
        if (mqtt.connect(MQTT_CLIENT_ID, nullptr, nullptr,
                         TOPIC_STATUS, 0, true, "offline")) {
            Serial.println("OK");
            mqtt.publish(TOPIC_STATUS, "online", true);
            mqtt.subscribe(TOPIC_CMD);
            publishState();
        } else {
            Serial.printf("err %d, retry 5s\n", mqtt.state());
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    connectWifi();
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setCallback(onMessage);
    connectMqtt();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi utracone, ponawiam...");
        mqtt.disconnect();
        connectWifi();
    }
    if (!mqtt.connected()) connectMqtt();
    mqtt.loop();
}
