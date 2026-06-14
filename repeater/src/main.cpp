// Firmware Seeed XIAO ESP32-C3: węzeł rozszerzający zasięg WiFi z NAT routingiem.
//
// Topologia:
//   [Router] ──── [Węzeł root] ──── [Węzeł relay 1] ──── [Węzeł relay 2] ...
//
// Węzeł root (IS_ROOT=true):
//   - STA łączy się z domowym routerem WiFi
//   - AP tworzy sieć MESH_SSID
//   - NAT przekazuje ruch z AP do internetu przez STA
//
// Węzeł relay (IS_ROOT=false):
//   - STA skanuje i łączy się z najsilniejszym sygnałem MESH_SSID
//   - AP tworzy własną sieć MESH_SSID na unikalnej podsieci (z MAC)
//   - NAT przekazuje ruch dalej w kierunku routera
//   - Urządzenia końcowe łączą się do dowolnego węzła relay
//
// Wersja: 2026-06-14 20:04

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "lwip/lwip_napt.h"
#include "config.h"

// Unikalny trzeci oktet podsieci AP wyprowadzony z MAC (zakres 10–209)
static uint8_t apSubnet() {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    return mac[5] % 200 + 10;
}

static void startAP() {
    uint8_t sub = IS_ROOT ? 4 : apSubnet();
    IPAddress apIP(192, 168, sub, 1);
    IPAddress apMask(255, 255, 255, 0);

    WiFi.softAPConfig(apIP, apIP, apMask);
    WiFi.softAP(MESH_SSID, MESH_PASSWORD);

    // Włącz NAT — ruch z AP trafi przez STA do upstream
    ip_napt_enable(apIP, 1);

    Serial.printf("AP: %s  IP: %s\n", MESH_SSID, apIP.toString().c_str());
}

static void connectRoot() {
    Serial.printf("ROOT: łączenie z %s...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nROOT: połączono, IP: %s\n", WiFi.localIP().toString().c_str());
}

static void connectRelay() {
    Serial.println("RELAY: skanowanie sieci mesh...");

    // Szukaj MESH_SSID z najsilniejszym sygnałem (RSSI)
    int best = -100, bestIdx = -1;
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        if (WiFi.SSID(i) == MESH_SSID && WiFi.RSSI(i) > best) {
            best = WiFi.RSSI(i);
            bestIdx = i;
        }
    }
    WiFi.scanDelete();

    if (bestIdx < 0) {
        Serial.println("RELAY: brak sieci mesh, ponawianie za 5s...");
        delay(5000);
        connectRelay();
        return;
    }

    Serial.printf("RELAY: łączenie z %s (RSSI: %d dBm)...\n", MESH_SSID, best);
    WiFi.begin(MESH_SSID, MESH_PASSWORD);
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - t > 15000) {
            Serial.println("RELAY: timeout, ponawianie...");
            WiFi.disconnect();
            delay(1000);
            connectRelay();
            return;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nRELAY: połączono upstream, IP: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);

#if IS_ROOT
    connectRoot();
#else
    connectRelay();
#endif

    startAP();
    Serial.println("Węzeł gotowy.");
}

void loop() {
    // Automatyczne ponowne połączenie STA po zerwaniu
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Połączenie utracone, ponawiam...");
#if IS_ROOT
        connectRoot();
#else
        connectRelay();
#endif
    }
    delay(5000);
}
