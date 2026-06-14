// Aplikacja Flutter (Android): zdalne sterowanie LED-em na ESP32 przez MQTT.
// Łączy się z brokerem MQTT (broker.hivemq.com), wyświetla animowaną żarówkę
// i przycisk Włącz/Wyłącz, który wysyła ON/OFF na topic esp32/led/command.
// Stan UI synchronizowany jest z esp32/led/state. Kropka w AppBar pokazuje status połączenia.
// Po utracie połączenia automatycznie ponawia próbę co 5 sekund.
// Wersja: 2026-06-14 16:32

import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart';
import 'package:mqtt_client/mqtt_server_client.dart';

const String mqttBroker = 'broker.hivemq.com';
const int mqttPort = 1883;
const String topicCmd = 'esp32/led/command';
const String topicState = 'esp32/led/state';

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

  @override
  void initState() {
    super.initState();
    _connect();
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

  void _onConnected() =>
      setState(() {
        _connected = true;
        _status = 'Połączono z $mqttBroker';
      });

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
          ],
        ),
      ),
    );
  }
}
