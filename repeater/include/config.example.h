#pragma once
// Wersja: 2026-06-14 20:04

// ── Sieć mesh (wspólna dla wszystkich węzłów) ──────────────────────────────
// Urządzenia końcowe (telefon, laptop) łączą się do tej sieci na dowolnym węźle.
#define MESH_SSID       "esp32mesh"
#define MESH_PASSWORD   "mesh_password"

// ── Tryb węzła ────────────────────────────────────────────────────────────
// Ustaw true TYLKO na węźle podłączonym bezpośrednio do routera WiFi.
#define IS_ROOT         false

// ── Dane routera WiFi (tylko węzeł root, IS_ROOT = true) ──────────────────
#define WIFI_SSID       "twoje_ssid"
#define WIFI_PASSWORD   "twoje_haslo"
