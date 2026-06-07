# ESP32-S3 Camera Stream

HTTP MJPEG stream z kamerki OV2640 na module Seeed XIAO ESP32-S3 Sense.

## Wymagania

- Seeed XIAO ESP32-S3 Sense (z kamerką OV2640)
- [PlatformIO](https://platformio.org/)

## Konfiguracja

Utwórz plik `src/credentials.h` (nie jest wersjonowany):

```cpp
#pragma once
#define WIFI_SSID     "nazwa_sieci"
#define WIFI_PASSWORD "haslo"
```

## Uruchomienie

```bash
~/.platformio/penv/bin/pio run -t upload
~/.platformio/penv/bin/pio device monitor
```

Po restarcie modułu w Serial Monitorze pojawi się adres IP. Otwórz go w przeglądarce — zobaczysz live stream.

## Endpointy

| URL | Opis |
|-----|------|
| `http://<IP>/` | Live stream MJPEG |
