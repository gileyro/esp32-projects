// Firmware Seeed XIAO ESP32-C3: sterowanie zewnętrzną diodą LED przez MQTT.
// Łączy się z WiFi i brokerem MQTT, nasłuchuje komend ON/OFF na topic mgil/esp32c3/led/command,
// włącza/wyłącza LED i publikuje aktualny stan na mgil/esp32c3/led/state (retain).
// Połączenie WiFi i MQTT jest automatycznie odtwarzane po zerwaniu.
// Wersja: 2026-06-14 23:55

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
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" OK — " + WiFi.localIP().toString());
}

void connectMqtt() {
    while (!mqtt.connected()) {
        Serial.print("MQTT...");
        if (mqtt.connect(MQTT_CLIENT_ID)) {
            Serial.println("OK");
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
