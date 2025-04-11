#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Atm_mqtt_client.h"

// Structure to represent parsed pour data
struct PourRequest
{
    const char *id;
    int pulses;
    bool isValid;
    const char *errorMessage;
};

class MqttService
{
public:
    MqttService(Atm_mqtt_client &mqtt);

    void begin(uint8_t *mac, IPAddress ip, const char *broker, int port, const char *clientId = "client_id");
    void publish(const char *topic, const char *payload);
    void publishJson(const char *topic, JsonDocument &doc);

    // Register message callback
    typedef void (*MessageCallback)(const char *topic, const char *message);
    void onMessage(MessageCallback callback);

    // Static callback for MQTT message handling
    static void handleMqttMessage(int idx, int v, int up);

    // Static method to parse pour request JSON
    static PourRequest parsePourRequest(const char *message);

private:
    Atm_mqtt_client &_mqtt;
    MessageCallback _messageCallback;

    // Static reference for callback
    static MqttService *_instance;
};
