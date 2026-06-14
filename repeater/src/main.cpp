// Firmware Seeed XIAO ESP32-C3: węzeł sieci mesh dla detekcji pozycji.
//
// WAŻNE: Ten firmware NIE przekazuje ruchu internetowego.
// Urządzenia ESP32 (android-switch) łączą się bezpośrednio z domowym routerem WiFi.
// Sieć mesh (MESH_SSID) służy wyłącznie temu, by aplikacja Flutter mogła wykryć,
// do którego węzła jest podłączony telefon (GET /mesh-info → {"id":"XXXX","role":"root|relay"}).
//
// Topologia:
//   [Router WiFi] ─── [android-switch ESP32] (MQTT przez internet)
//   [Węzeł root]  ─── [Węzeł relay 1] ─── [Węzeł relay 2] ...
//    SSID: esp32mesh   SSID: esp32mesh     SSID: esp32mesh
//
// Każdy węzeł tworzy AP z MESH_SSID; telefon łączy się z najbliższym i pyta /mesh-info.
// Wersja: 2026-06-14 23:10

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include "config.h"

WebServer httpServer(80);
String nodeId;

// ID węzła: ostatnie 4 znaki MAC AP w hex (np. "A3F2")
static String buildNodeId() {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    char buf[5];
    snprintf(buf, sizeof(buf), "%02X%02X", mac[4], mac[5]);
    return String(buf);
}

static void startAP() {
    WiFi.softAP(MESH_SSID, MESH_PASSWORD);
    Serial.printf("AP: %s  IP: %s  ID: %s  rola: %s\n",
                  MESH_SSID,
                  WiFi.softAPIP().toString().c_str(),
                  nodeId.c_str(),
                  IS_ROOT ? "root" : "relay");
}

static void startHttpServer() {
    httpServer.on("/mesh-info", HTTP_GET, []() {
        String json = "{\"id\":\"" + nodeId + "\","
                      "\"role\":\"" + String(IS_ROOT ? "root" : "relay") + "\"}";
        httpServer.send(200, "application/json", json);
    });
    httpServer.begin();
    Serial.println("HTTP: gotowy na /mesh-info");
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP);

    nodeId = buildNodeId();
    Serial.printf("Node ID: %s\n", nodeId.c_str());

    startAP();
    startHttpServer();
    Serial.println("Węzeł gotowy.");
}

void loop() {
    httpServer.handleClient();
}
