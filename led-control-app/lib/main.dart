// Aplikacja Flutter (Android): zdalne sterowanie LED-em na ESP32 przez MQTT.
// Łączy się z brokerem MQTT (broker.hivemq.com), wyświetla animowaną żarówkę
// i przycisk Włącz/Wyłącz, który wysyła ON/OFF na topic mgil/esp32c3/led/command.
// Stan UI synchronizowany jest z mgil/esp32c3/led/state. Kropka w AppBar pokazuje status połączenia.
// Po utracie połączenia automatycznie ponawia próbę co 5 sekund.
// Jeśli telefon jest podłączony do węzła mesh ESP32, wyświetla jego ID.
// Wersja: 2026-06-14 20:11

import 'dart:async';
import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';
import 'package:network_info_plus/network_info_plus.dart';
import 'package:http/http.dart' as http;

const String mqttBroker = 'broker.hivemq.com';
const int mqttPort = 1883;
const String topicCmd = 'mgil/esp32c3/led/command';
const String topicState = 'mgil/esp32c3/led/state';

void main() => runApp(const LedApp());

class LedApp extends StatelessWidget {
  const LedApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'LED Control',
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark(useMaterial3: true),
      home: const LedPage(),
    );
  }
}

class LedPage extends StatefulWidget {
  const LedPage({super.key});

  @override
  State<LedPage> createState() => _LedPageState();
}

class _LedPageState extends State<LedPage> {
  late MqttServerClient _client;
  bool _ledOn = false;
  bool _connected = false;
  bool _disposed = false;
  String _status = 'Łączenie...';
  String? _meshNodeId; // ID węzła mesh lub null jeśli brak
  Timer? _nodeCheckTimer;

  @override
  void initState() {
    super.initState();
    _connect();
    _checkMeshNode();
    // Sprawdzaj co 30s — węzeł może się zmienić przy roamingu
    _nodeCheckTimer = Timer.periodic(const Duration(seconds: 30), (_) => _checkMeshNode());
  }

  Future<void> _checkMeshNode() async {
    try {
      final gateway = await NetworkInfo().getWifiGatewayIP();
      if (gateway == null || _disposed) return;

      final response = await http
          .get(Uri.parse('http://$gateway/mesh-info'))
          .timeout(const Duration(seconds: 3));

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        if (mounted) setState(() => _meshNodeId = data['id'] as String?);
      } else {
        if (mounted) setState(() => _meshNodeId = null);
      }
    } catch (_) {
      if (mounted) setState(() => _meshNodeId = null);
    }
  }

  Future<void> _connect() async {
    if (_disposed) return;
    setState(() => _status = 'Łączenie...');

    final clientId = 'flutter-${DateTime.now().millisecondsSinceEpoch}';
    _client = MqttServerClient(mqttBroker, clientId)
      ..port = mqttPort
      ..keepAlivePeriod = 30
      ..onDisconnected = _onDisconnected
      ..onConnected = _onConnected
      ..logging(on: false);

    _client.connectionMessage = MqttConnectMessage()
        .withClientIdentifier(clientId)
        .startClean();

    try {
      await _client.connect();
    } catch (e) {
      if (_disposed) return;
      setState(() => _status = 'Błąd połączenia – ponawiam za 5s...');
      _client.disconnect();
      Future.delayed(const Duration(seconds: 5), _connect);
      return;
    }

    _client.subscribe(topicState, MqttQos.atLeastOnce);
    _client.updates?.listen(_onData);
  }

  void _onConnected() {
    setState(() {
      _connected = true;
      _status = 'Połączono z $mqttBroker';
    });
    _checkMeshNode();
  }

  void _onDisconnected() {
    if (_disposed) return;
    setState(() {
      _connected = false;
      _status = 'Rozłączono – ponawiam za 5s...';
    });
    Future.delayed(const Duration(seconds: 5), _connect);
  }

  void _onData(List<MqttReceivedMessage<MqttMessage>> messages) {
    for (final msg in messages) {
      if (msg.topic == topicState) {
        final payload = (msg.payload as MqttPublishMessage).payload.message;
        final text = MqttPublishPayload.bytesToStringAsString(payload);
        setState(() => _ledOn = text == 'ON');
      }
    }
  }

  void _toggle() {
    final cmd = _ledOn ? 'OFF' : 'ON';
    final builder = MqttClientPayloadBuilder()..addString(cmd);
    _client.publishMessage(topicCmd, MqttQos.atLeastOnce, builder.payload!);
  }

  @override
  void dispose() {
    _disposed = true;
    _nodeCheckTimer?.cancel();
    _client.disconnect();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('LED Control'),
        actions: [
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16),
            child: Icon(
              Icons.circle,
              size: 14,
              color: _connected ? Colors.greenAccent : Colors.redAccent,
            ),
          ),
        ],
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            AnimatedContainer(
              duration: const Duration(milliseconds: 300),
              decoration: BoxDecoration(
                shape: BoxShape.circle,
                boxShadow: _ledOn
                    ? [BoxShadow(color: Colors.yellow.withOpacity(0.6), blurRadius: 40, spreadRadius: 10)]
                    : [],
              ),
              child: Icon(
                Icons.lightbulb,
                size: 140,
                color: _ledOn ? Colors.yellow : Colors.grey.shade700,
              ),
            ),
            const SizedBox(height: 48),
            FilledButton(
              onPressed: _connected ? _toggle : null,
              style: FilledButton.styleFrom(
                backgroundColor: _ledOn ? Colors.red.shade700 : Colors.green.shade700,
                padding: const EdgeInsets.symmetric(horizontal: 56, vertical: 18),
              ),
              child: Text(
                _ledOn ? 'Wyłącz LED' : 'Włącz LED',
                style: const TextStyle(fontSize: 20),
              ),
            ),
            const SizedBox(height: 24),
            Text(
              _status,
              style: TextStyle(
                color: _connected ? Colors.greenAccent : Colors.orange,
                fontSize: 13,
              ),
            ),
            if (_meshNodeId != null) ...[
              const SizedBox(height: 8),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Icon(Icons.router, size: 14, color: Colors.blueAccent),
                  const SizedBox(width: 6),
                  Text(
                    'Węzeł mesh: $_meshNodeId',
                    style: const TextStyle(fontSize: 12, color: Colors.blueAccent),
                  ),
                ],
              ),
            ],
          ],
        ),
      ),
    );
  }
}
