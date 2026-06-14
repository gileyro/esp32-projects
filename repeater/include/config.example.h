#pragma once
// Wersja: 2026-06-14

// Wspólne dla wszystkich węzłów mesh
#define MESH_PREFIX     "esp32mesh"
#define MESH_PASSWORD   "mesh_password"
#define MESH_PORT       5555

// Ustaw true TYLKO na węźle podłączonym do routera WiFi (węzeł root)
#define IS_ROOT         false

// Dane routera WiFi — wymagane tylko gdy IS_ROOT = true
#define WIFI_SSID       "twoje_ssid"
#define WIFI_PASSWORD   "twoje_haslo"
