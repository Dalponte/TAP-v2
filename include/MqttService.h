#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoMqttClient.h>
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
    // Get singleton instance
    static MqttService &getInstance()
    {
        if (!_instance)
        {
            _instance = new MqttService();
        }
        return *_instance;
    }

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

    // Make Atm_mqtt_client public for TapService (temporary solution)
    Atm_mqtt_client _mqtt;

private:
    // Private constructor for singleton
    MqttService();

    // No copy or assignment
    MqttService(const MqttService &) = delete;
    MqttService &operator=(const MqttService &) = delete;

    EthernetClient _ethClient;
    MqttClient _mqttClient{_ethClient};
    MessageCallback _messageCallback;

    // Static reference for callback and singleton
    static MqttService *_instance;
};
