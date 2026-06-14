#pragma once

#define WIFI_SSID       "twoje_ssid"
#define WIFI_PASSWORD   "twoje_haslo"

#define MQTT_BROKER     "broker.hivemq.com"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "esp32c3-led"

#define TOPIC_CMD       "mgil/esp32c3/led/command"
#define TOPIC_STATE     "mgil/esp32c3/led/state"
#define TOPIC_STATUS    "mgil/esp32c3/led/status"

// Zewnętrzna dioda LED: GPIO2 (D0) → rezystor 220Ω → anoda LED → katoda → GND
#define LED_PIN         2
