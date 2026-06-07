#pragma once

#define WIFI_SSID       "twoje_ssid"
#define WIFI_PASSWORD   "twoje_haslo"

#define MQTT_BROKER     "broker.hivemq.com"
#define MQTT_PORT       1883
#define MQTT_CLIENT_ID  "esp32s3-led"

#define TOPIC_CMD       "esp32/led/command"
#define TOPIC_STATE     "esp32/led/state"

// GPIO wbudowanej diody LED na ESP32-S3 DevKitC-1
#define LED_PIN         2
