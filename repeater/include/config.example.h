#pragma once
// Wersja: 2026-06-14 23:10

// ── Sieć mesh (wspólna dla wszystkich węzłów) ──────────────────────────────
// Telefon łączy się do tej sieci, by pobrać /mesh-info.
#define MESH_SSID       "esp32mesh"
#define MESH_PASSWORD   "mesh_password"

// ── Tryb węzła ────────────────────────────────────────────────────────────
// Wpływa tylko na pole "role" w odpowiedzi /mesh-info.
#define IS_ROOT         false
