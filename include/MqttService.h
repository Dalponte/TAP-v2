#pragma once

#include <Arduino.h>
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

    // Register message callback
    typedef void (*MessageCallback)(const char *topic, const char *message);
    void onMessage(MessageCallback callback);

    // New callback specifically for pour requests
    typedef void (*PourStartCallback)(const char *message);
    void onPourStart(PourStartCallback callback);

    // Static callback for MQTT message handling - updated signature
    static void handleMqttMessage(int messageSize);

    // Callbacks for connection events - now static
    static void onConnected(int idx, int v, int up);
    static void onDisconnected(int idx, int v, int up);

    // Process a received message
    void processMessage();

    // Get current message content
    const char *getMessageTopic() const;
    const char *getMessageContent() const;

    // Static method to parse pour request JSON
    static PourRequest parsePourRequest(const char *message);

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
    PourStartCallback _pourStartCallback; // New callback for pour start events

    // Buffer for message storage
    char _messageBuffer[256];

    static MqttService *_instance;
};
