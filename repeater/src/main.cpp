// Firmware Seeed XIAO ESP32-C3: węzeł sieci mesh WiFi opartej na painlessMesh.
// Węzły automatycznie wykrywają się i tworzą samoleczącą się sieć.
// Jeden węzeł (IS_ROOT=true w config.h) łączy się z routerem i daje
// pozostałym węzłom dostęp do internetu przez mesh.
// Wszystkie węzły co 5s rozgłaszają swoje ID przez sieć mesh.
// Wersja: 2026-06-14 16:30

#include <Arduino.h>
#include <painlessMesh.h>
#include "config.h"

Scheduler scheduler;
painlessMesh mesh;

void sendMessage();
Task taskSend(5000, TASK_FOREVER, &sendMessage);

void sendMessage() {
    String msg = "Node " + String(mesh.getNodeId()) + " alive";
    mesh.sendBroadcast(msg);
    Serial.println("Sent: " + msg);
}

void onMessage(uint32_t from, String &msg) {
    Serial.printf("Received from %u: %s\n", from, msg.c_str());
}

void onNewConnection(uint32_t nodeId) {
    Serial.printf("New node: %u | total nodes: %u\n", nodeId, mesh.getNodeList().size() + 1);
}

void onDroppedConnection(uint32_t nodeId) {
    Serial.printf("Lost node: %u | total nodes: %u\n", nodeId, mesh.getNodeList().size() + 1);
}

void setup() {
    Serial.begin(115200);

    mesh.setDebugMsgTypes(ERROR | STARTUP);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &scheduler, MESH_PORT);
    mesh.onReceive(&onMessage);
    mesh.onNewConnection(&onNewConnection);
    mesh.onDroppedConnection(&onDroppedConnection);

#if IS_ROOT
    // Węzeł root: łączy się z routerem i udostępnia internet reszcie sieci mesh
    mesh.setRoot(true);
    mesh.setContainsRoot(true);
    mesh.stationManual(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Mode: ROOT (bridge to router)");
#else
    mesh.setContainsRoot(true);
    Serial.println("Mode: NODE (relay)");
#endif

    scheduler.addTask(taskSend);
    taskSend.enable();

    Serial.printf("Node ID: %u\n", mesh.getNodeId());
}

void loop() {
    mesh.update();
}
